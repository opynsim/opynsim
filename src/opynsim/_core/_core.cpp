#include "_core.h"

#include <opynsim/_core/arrow.h>
#include <opynsim/_core/config.h>
#include <opynsim/_core/examples.h>
#include <opynsim/_core/graphics.h>
#include <opynsim/_core/tps3d.h>
#include <opynsim/_core/ui.h>

#include <liboscar/utilities/assertions.h>
#include <liboscar/utilities/enum_helpers.h>
#include <liboscar/utilities/string_helpers.h>
#include <libopynsim/platform/opynsim_app.h>
#include <libopynsim/data_frame.h>
#include <libopynsim/model.h>
#include <libopynsim/model_specification.h>
#include <libopynsim/model_state.h>
#include <libopynsim/model_state_stage.h>
#include <libopynsim/opynsim.h>
#include <libopynsim/symbol.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>

#include <array>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

using namespace opyn;

namespace nb = nanobind;

namespace {
    std::unique_ptr<OPynSimApp> g_lazy_loaded_app;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    template<typename CppType>
    struct PythonTypeMapper {
        using cpp_type = CppType;
        using python_type = CppType;

        static python_type to_python(cpp_type&& v) { return python_type{std::move(v)}; }
    };

    template<>
    struct PythonTypeMapper<osc::Vector3d> {
        using cpp_type = osc::Vector3d;
        using python_type = nb::ndarray<nb::numpy, double, nb::shape<3>>;

        static python_type to_python(cpp_type&& v) // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        {
            auto* data = new double[3]{v[0], v[1], v[2]};  // NOLINT(cppcoreguidelines-owning-memory)
            const std::array<size_t, 1> shape = {3};
            return {
                data,
                1,
                shape.data(),
                nb::capsule(data, [](void* p) noexcept { delete[] static_cast<double*>(p); })  // NOLINT(cppcoreguidelines-owning-memory)
            };
        }
    };

    template<typename... Ts>
    auto to_python_mapper_typelist(osc::Typelist<Ts...>)
    {
        return osc::Typelist<typename PythonTypeMapper<Ts>::python_type...>{};
    }

    using SupportedPythonOutputValues = decltype(to_python_mapper_typelist(std::declval<SupportedOutputValueTypes>()));
    using PythonOutputValue = osc::VariantOfTypelistElements<SupportedPythonOutputValues>;

    PythonOutputValue to_python_output(OutputValue&& output_value)
    {
        return std::visit(osc::Overload{
            []<typename T>(T&& v) -> PythonOutputValue { return PythonTypeMapper<T>::to_python(std::forward<T>(v)); }
        }, std::move(output_value));  // NOLINT(hicpp-move-const-arg,performance-move-const-arg)
    }

    template<typename ArrowType, typename PrivateDataType>
    void arrow_release_callback(ArrowType* ptr)
    {
        delete static_cast<PrivateDataType*>(ptr->private_data);
        ptr->release = nullptr;
    }

    // Provides a C++-friendly wrapper around Apache Arrow's `ArrowSchema`.
    struct ArrowSchemaWrapper : ArrowSchema {
        explicit ArrowSchemaWrapper(const Series& series) : ArrowSchema{}
        {
            using PrivateData = Series;
            auto pdata = std::make_unique<PrivateData>(series);

            this->format = "g";  static_assert(std::same_as<Series::value_type, double>);
            this->name = pdata->name().c_str();
            this->metadata = nullptr;
            this->flags = int64_t{0};
            this->n_children = int64_t{0};
            this->children = nullptr;
            this->dictionary = nullptr;
            this->release = arrow_release_callback<ArrowSchema, PrivateData>;
            this->private_data = pdata.release();
        }

        explicit ArrowSchemaWrapper(const DataFrame& data_frame) : ArrowSchema{}
        {
            struct PrivateData final {
                explicit PrivateData(const DataFrame& data_frame) :
                    data_frame_{data_frame}
                {
                    children_.reserve(data_frame_.width());
                    children_pointers_.reserve(data_frame_.width());
                    for (const auto& series : data_frame_) {
                        auto& wrapper = children_.emplace_back(std::make_unique<ArrowSchemaWrapper>(series));
                        children_pointers_.push_back(wrapper.get());
                    }
                }
                DataFrame data_frame_;
                std::vector<std::unique_ptr<ArrowSchemaWrapper>> children_;
                std::vector<ArrowSchema*> children_pointers_;
            };
            auto pdata = std::make_unique<PrivateData>(data_frame);

            this->format = "+s";
            this->name = "";
            this->metadata = nullptr;
            this->flags = int64_t{0};
            this->n_children = static_cast<int64_t>(pdata->children_pointers_.size());
            this->children = pdata->children_pointers_.data();
            this->dictionary = nullptr;
            this->release = arrow_release_callback<ArrowSchema, PrivateData>;
            this->private_data = pdata.release();
        }

        ArrowSchemaWrapper(const ArrowSchemaWrapper&) = delete;
        ArrowSchemaWrapper(ArrowSchemaWrapper&&) noexcept = delete;
        ~ArrowSchemaWrapper() noexcept
        {
            if (this->release) {
                this->release(this);
            }
        }
        ArrowSchemaWrapper& operator=(const ArrowSchemaWrapper&) = delete;
        ArrowSchemaWrapper& operator=(ArrowSchemaWrapper&&) noexcept = delete;
    };
    static_assert( sizeof(ArrowSchemaWrapper) ==  sizeof(ArrowSchema), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");
    static_assert(alignof(ArrowSchemaWrapper) >= alignof(ArrowSchema), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");

    // Provides a C++-friendly wrapper around Apache Arrow's `ArrowArray`.
    struct ArrowArrayWrapper : ArrowArray {
        explicit ArrowArrayWrapper(const Series& series) : ArrowArray{}
        {
            struct PrivateData final {
                explicit PrivateData(const Series& series) :
                    series_{series}
                {
                    buffers_[0] = nullptr;          // validity bitmap (unused)
                    buffers_[1] = series_.data();  // data pointer (float64)
                }

                Series series_;
                std::unique_ptr<const void*[]> buffers_ = std::make_unique<const void*[]>(2);
            };
            auto pdata = std::make_unique<PrivateData>(series);

            this->length = static_cast<int64_t>(pdata->series_.size());
            this->null_count = 0;
            this->offset = 0;
            this->n_buffers = 2;
            this->buffers = pdata->buffers_.get();
            this->n_children = 0;
            this->children = nullptr;
            this->dictionary = nullptr;
            this->release = arrow_release_callback<ArrowArray, PrivateData>;
            this->private_data = pdata.release();
        }

        explicit ArrowArrayWrapper(const DataFrame& data_frame) : ArrowArray{}
        {
            struct PrivateData final {
                explicit PrivateData(const DataFrame& data_frame) :
                    data_frame_{data_frame}
                {
                    children_.reserve(data_frame_.width());
                    children_pointers_.reserve(data_frame_.width());
                    for (const auto& series : data_frame_) {
                        auto& wrapper = children_.emplace_back(std::make_unique<ArrowArrayWrapper>(series));
                        children_pointers_.push_back(wrapper.get());
                    }
                }

                DataFrame data_frame_;
                std::vector<std::unique_ptr<ArrowArrayWrapper>> children_;
                std::vector<ArrowArray*> children_pointers_;
                std::vector<const void*> buffers_ = {nullptr};  // validity bitmap
            };
            auto pdata = std::make_unique<PrivateData>(data_frame);

            this->length = data_frame.height();
            this->null_count = 0;
            this->offset = 0;
            this->n_buffers = pdata->buffers_.size();
            this->buffers = pdata->buffers_.data();
            this->n_children = static_cast<int64_t>(pdata->children_pointers_.size());
            this->children = pdata->children_pointers_.data();
            this->dictionary = nullptr;
            this->release = arrow_release_callback<ArrowArray, PrivateData>;
            this->private_data = pdata.release();
        }
        ArrowArrayWrapper(const ArrowArrayWrapper&) = delete;
        ArrowArrayWrapper(ArrowArrayWrapper&&) noexcept = delete;
        ~ArrowArrayWrapper() noexcept
        {
            if (this->release) {
                this->release(this);
            }
        }
    };
    static_assert( sizeof(ArrowArrayWrapper) ==  sizeof(ArrowArray), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");
    static_assert(alignof(ArrowArrayWrapper) >= alignof(ArrowArray), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");

    struct ArrowArrayStreamWrapper : ArrowArrayStream {
        explicit ArrowArrayStreamWrapper(const DataFrame& data_frame) : ArrowArrayStream{}
        {
            struct PrivateData final {
                explicit PrivateData(const DataFrame& data_frame) :
                    data_frame_{data_frame}
                {}

                int get_schema(ArrowSchema& out)
                {
                    last_error_.clear();
                    try {
                        new (&out) ArrowSchemaWrapper{data_frame_};
                        return 0;
                    }
                    catch (const std::bad_alloc& ex) {
                        last_error_ = ex.what();
                        return ENOMEM;
                    }
                }

                int get_next(ArrowArray& out)
                {
                    if (std::exchange(array_already_emitted_, true)) {
                        out.release = nullptr;
                        return 0;
                    }

                    last_error_.clear();
                    try {
                        new (&out) ArrowArrayWrapper{data_frame_};
                        OSC_ASSERT(out.release != nullptr);
                        return 0;
                    }
                    catch (const std::bad_alloc& ex) {
                        last_error_ = ex.what();
                        return ENOMEM;
                    }
                }
                const char* get_last_error() const
                {
                    return last_error_.data();
                }

                DataFrame data_frame_;
                bool array_already_emitted_ = false;
                std::string last_error_;
            };

            auto private_data = std::make_unique<PrivateData>(data_frame);
            this->get_schema = [](ArrowArrayStream* self, ArrowSchema* out)
            {
                return static_cast<PrivateData*>(self->private_data)->get_schema(*out);
            };
            this->get_next = [](ArrowArrayStream* self, ArrowArray* out)
            {
                return static_cast<PrivateData*>(self->private_data)->get_next(*out);
            };
            this->get_last_error = [](ArrowArrayStream* self)
            {
                return static_cast<PrivateData*>(self->private_data)->get_last_error();
            };
            this->release = arrow_release_callback<ArrowArrayStream, PrivateData>;
            this->private_data = private_data.release();
        }
        ArrowArrayStreamWrapper(const ArrowArrayStreamWrapper&) = delete;
        ArrowArrayStreamWrapper(ArrowArrayStreamWrapper&&) noexcept = delete;
        ~ArrowArrayStreamWrapper() noexcept
        {
            if (this->release) {
                this->release(this);
            }
        }
        ArrowArrayStreamWrapper& operator=(const ArrowArrayStreamWrapper&) = delete;
        ArrowArrayStreamWrapper& operator=(ArrowArrayStreamWrapper&&) noexcept = delete;
    };
    static_assert( sizeof(ArrowArrayStreamWrapper) ==  sizeof(ArrowArrayStream), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");
    static_assert(alignof(ArrowArrayStreamWrapper) >= alignof(ArrowArrayStream), "The wrapper is type-punned: it must be entirely destructible via .release and not add any new data members (add them at runtime into a `PrivateData` struct)");

    void register_dataframe_class(nb::module_& m)
    {
        nb::class_<DataFrame> cls(m, "DataFrame", R"(
            Represents data as a table with rows and columns (:class:`opynsim.Series`).
        )");
        cls.def(nb::init{}, "Default-constructs an empty ``DataFrame``");
        cls.def("__repr__", osc::stream_to_string<DataFrame>);
        cls.def("__str__", osc::stream_to_string<DataFrame>);
        cls.def_prop_ro(
            "attrs",
            &DataFrame::attrs,
            R"(
                Returns the attributes (metadata) associated with this `DataFrame`.

                These entries are nominally metadata, but can affect the behavior of functions that
                read data from :class:`DataFrame`\s. Notably, functions like :meth:`opynsim.Model.states_from_data_frame`
                look for attributes like 'inDegrees' to perform on-the-fly degrees-to-radians conversions on
                legacy data files.
            )"
        );
        cls.def(
            "__arrow_c_stream__",
            [](const DataFrame& data_frame,
               [[maybe_unused]] std::optional<nb::capsule> requested_schema = std::nullopt)
            {
                static_assert(std::derived_from<ArrowArrayStreamWrapper, ArrowArrayStream>);
                auto stream = std::make_unique<ArrowArrayStreamWrapper>(data_frame);
                return nb::capsule{stream.release(), "arrow_array_stream", [](void* ptr) noexcept { delete static_cast<ArrowArrayStreamWrapper*>(ptr); }};
            },
            nb::arg("requested_schema") = std::nullopt,
            R"(
                Exports this ``DataFrame`` as an ``ArrowSchema`` (see: `Apache Arrow PyCapsule Interface <https://arrow.apache.org/docs/dev/format/CDataInterface/PyCapsuleInterface.html>`_).

                This is a low-level interface that other dataframe libraries (e.g. `Pandas <https://pandas.pydata.org/>`_
                and `Polars <https://pola.rs/>`_) can use to natively (i.e. rapidly) read OPynSim's ``DataFrame``.

                Args:
                    requested_schema: An optional Arrow schema capsule (currently ignored).

                Returns:
                    A ``PyCapsule`` called "arrow_array_stream" containing a C ``ArrowArrayStream`` struct.
            )"
        );
        cls.def_prop_ro(
            "shape",
            &DataFrame::shape,
            R"(
                Returns the shape of the ``DataFrame``.
            )"
        );
        cls.def(
            "to_pandas",
            [](const DataFrame& data_frame)
            {
                // Lazily import `pandas` (OPynSim isn't dependent on it) and perform
                // runtime lookups to figure out how to export the data into a
                // `pandas.DataFrame`.

                nb::module_ pd = nb::module_::import_("pandas");
                nb::object DataFrame = pd.attr("DataFrame");
                nb::object from_arrow = DataFrame.attr("from_arrow");
                nb::object data_frame_obj = nb::cast(&data_frame, nb::rv_policy::reference);
                return from_arrow(data_frame_obj);
            }
        );
        cls.def(
            "to_polars",
            [](const DataFrame& data_frame)
            {
                // Lazily import `polars` (OPynSim isn't dependent on it) and perform
                // runtime lookups to figure out how to export the data into a
                // `polars.DataFrame`.

                nb::module_ pd = nb::module_::import_("polars");
                nb::object from_arrow = pd.attr("from_arrow");
                nb::object data_frame_obj = nb::cast(&data_frame, nb::rv_policy::reference);
                return from_arrow(data_frame_obj);
            }
        );
    }

    void register_symbol_class(nb::module_& m)
    {
        nb::class_<Symbol> symbol_class(
            m,
            "Symbol",
            R"(
                Represents an immutable, cheap-to-use, readable symbol.

                Symbols are extensively used by the OPynSim API to accelerate associative lookups. They are the
                middle-ground between fast, but hard to read/introspect, integer handles and slow, simpler string
                handles.

                From Python code's point of view, symbols should be seen as string-like handles that OPynSim
                accepts/emits. You can safely store symbols independently of any larger data structure, and
                interchange them across your entire Python codebase, without having to worry about object
                lifetimes. OPynSim's native code uses runtime-checked associative lookups, rather than pointers, to
                ensure that the Python API is memory-safe and can provide suitable feedback whenever a lookup fails.
            )"
        );
        symbol_class.def(
            nb::init<std::string_view>(),
            nb::arg("id"),
            R"(
                Constructs a symbol from a Python string.
            )"
        );
        symbol_class.def(
            "__str__",
            [](const Symbol& symbol) { return static_cast<std::string>(symbol); },
            "Converts this symbol into a Python :class:`str`"
        );
        symbol_class.def("__repr__", [](const Symbol& self) { return std::string("Symbol(\"") + std::string(self) + "\")"; });
        symbol_class.def("__hash__", [](const Symbol& self) { return std::hash<Symbol>{}(self); });
        symbol_class.def("__eq__",   [](const Symbol& self, const Symbol& other)  { return self == other; });
        symbol_class.def("__eq__",   [](const Symbol& self, std::string_view rhs) { return self == rhs; });
        symbol_class.def("__contains__", [](const Symbol& self, std::string_view rhs) { return static_cast<std::string_view>(self).find(rhs) != std::string_view::npos; });
        nb::implicitly_convertible<std::string_view, Symbol>();
    }

    void register_model_specification_class(nb::module_& m)
    {
        nb::class_<ModelSpecification> model_specification_class(
            m,
            "ModelSpecification",
            R"(
                A high-level specification for a :class:`Model`.

                A :class:`ModelSpecification` is what Python code can manipulate, scale, and customize
                before passing it to :meth:`compile`, which returns a readonly :class:`Model`.

                Notes:
                    OPynSim's API design separates the specification of a model (:class:`ModelSpecification`)
                    from its validated, assembled, and optimized simulation representation (:class:`Model`) to ensure
                    that the compilation process (:meth:`compile`) can freeze and optimize internal
                    datastructures at a single point in the process.
            )"
        );
        model_specification_class.def(nb::init<>());  // Define default constructor
        model_specification_class.def(
            "compile",
            &ModelSpecification::compile,
            R"(
                Compiles this :class:`ModelSpecification` into a :class:`Model`.

                The compilation process:

                - Validates this :class:`ModelSpecification`'s components (properties, subcomponents,
                  and sockets), throwing an exception if the specification is invalid in some way.
                - Assembles a physics system from the validated specification, throwing an exception
                  if the physics system cannot be assembled (e.g. if the contains impossible-to-satisfy
                  joints, or invalid muscle definitions).

                Raises:
                    Exception: If the compilation process failed in some way. It is assumed that
                        the provided :class:`ModelSpecification` is valid.
            )"
        );
    }

    void register_model_class(nb::module_& m)
    {
        nb::class_<Model> model_class(
            m,
            "Model",
            R"(
                A validated, optimized, compiled, and ready-to-simulate model of a physics system.

                A :class:`Model` can only be created from a :class:`ModelSpecification` via the
                :meth:`ModelSpecification.compile` function. Therefore, editing a :class:`Model` requires
                editing its associated :class:`ModelSpecification` and recompiling it to create a
                new :class:`Model`.
            )"
        );
        model_class.def_prop_ro(
            "num_coordinates",
            &Model::num_coordinates,
            R"(
                Returns the number of coordinates in the model.

                A coordinate represents a single degree of freedom (DoF) in the model, such as a joint angle,
                translation, or rotational parameter that contributes to the configuration/pose of a model.
            )"
        );
        model_class.def_prop_ro(
            "coordinates",
            &Model::coordinates,
            R"(
                Returns a list of all the coordinates in the model.
            )"
        );
        model_class.def(
            "initial_state",
            &Model::initial_state,
            nb::kw_only{},
            nb::arg("realized_to") = ModelStateStage::instance,
            R"(
                Returns a :class:`ModelState` that represents the initial (default) state of this :class:`Model`.

                The initial state of a :class:`Model` is dictated by the :class:`ModelSpecification` used to
                compile it. For example, if a translational coordinate in the specification had a ``default_value``
                of ``1.0`` then that would be written into the :class:`ModelState` returned by this function.

                The returned :class:`ModelState` will be realized to at least ``realized_to`` as-if by calling
                ``model.realize(returned_state, realized_to)``.
            )"
        );
        model_class.def(
            "column_to_state_variable_mappings",
            &Model::column_to_state_variable_mappings,
            nb::arg("data_frame"),
            R"(
                Returns associative mappings between the names of columns in
                ``data_frame`` and state variables in this ``Model``, where
                the correspondence can be found.

                This mapping uses a variety of heuristics, including (e.g.)
                accounting for legacy column headers supported by earlier
                files in SIMM and OpenSim. It is how :meth:`states_from_data_frame`
                maps dataframes into :class:`ModelState`\s, so it can be
                useful for debugging why states aren't being read correctly.
            )"
        );
        model_class.def(
            "rotational_columns_in",
            &Model::rotational_columns_in,
            nb::arg("data_frame"),
            R"(
                Returns the names of columns in ``data_frame`` that can be
                mapped to rotational state variables in this ``Model`` in
                the column-order of ``data_frame``.

                This is how :meth:`states_from_data_frame` automatically converts
                degrees to radians when ``data_frame.attrs["inDegrees"] == "yes"``,
                so it can be useful for debugging why states aren't
                being read correctly.
            )"
        );
        model_class.def(
            "states_from_data_frame",
            &Model::states_from_data_frame,
            nb::arg("data_frame"),
            nb::kw_only{},
            nb::arg("realized_to") = ModelStateStage::instance,
            R"(
                Returns :class:`ModelStates` constructed from ``data_frame``.

                Columns in ``data_frame`` will be mapped to state variables in
                this ``Model`` (see: :meth:`column_to_state_variable_mappings`). Each
                row in ``data_frame`` constructs one :class:`ModelState` in the returned
                :class:`ModelStates`, in row-order.

                If ``data_frame.attrs["inDegrees"] == "yes"`` then rotational columns
                in ``data_frame`` will be automatically converted into radians internally
                (see: :meth:`rotational_columns_in`). This is to support :class:`DataFrame`\s
                loaded from legacy data sources.

                Each of the returned :class:`ModelState`\s will be realized to at
                least ``realized_to`` as-if by calling ``model.realize(returned_states[i], realized_to)``
                on each of them.
            )"
        );
        model_class.def(
            "realize",
            &Model::realize,
            nb::arg("model_state"),
            nb::arg("model_state_stage"),
            R"(
                Realizes ``model_state`` to the desired ``model_state_stage``, which modifies
                ``model_state`` in-place.

                "Realization" of the state involves taking a new set of values from the :class:`ModelState`
                and performing computations that those new values enable. Realization is performed
                in-order one :class:`ModelStateStage` at time. For example, :attr:`ModelStateStage.POSITION`
                is realized before :attr:`ModelStateStage.VELOCITY`,then :attr:`ModelStateStage.DYNAMICS`,
                and so on.
            )"
        );
        model_class.def(
            "get_coordinate_value",
            &Model::get_coordinate_value,
            nb::arg("model_state"),
            nb::arg("coordinate"),
            R"(
                Returns the value of the corresponding state variable in ``model_state`` for the
                coordinate identified by ``coordinate``.
            )"
        );
        model_class.def(
            "set_coordinate_value",
            &Model::set_coordinate_value,
            nb::arg("model_state"),
            nb::arg("coordinate"),
            nb::arg("value"),
            R"(
                Sets corresponding state variable in ``model_state`` for the coordinate identified by
                ``coordinate`` to ``value``.

                Changing the value of a coordinate changes ``model_state``'s :class:`ModelStateStage` to
                :attr:`ModelStateStage.POSITION`. Therefore, you may need to use :meth:`realize` to
                re-realize the state to a later stage if you intend on using the state with a method that
                requires a later stage (e.g. rendering).
            )"
        );
        model_class.def_prop_ro(
            "num_outputs",
            &Model::num_outputs,
            R"(
                Returns the number of outputs the model has.
            )"
        );
        model_class.def_prop_ro(
            "outputs",
            &Model::outputs,
            R"(
                Returns a list of all outputs the model has.
            )"
        );
        model_class.def(
            "get_output_value",
            [](const Model& model, const ModelState& model_state, const Symbol& output)
            {
                return to_python_output(model.get_output_value(model_state, output));
            },
            nb::arg("model_state"),
            nb::arg("output"),
            R"(
                Returns the value of ``output`` for the given ``model_state``.
            )"
        );
    }

    void register_model_state_class(nb::module_& m)
    {
        nb::class_<ModelState> model_state_class(
            m,
            "ModelState",
            R"(
                Represents a single state of a :class:`Model`.

                A :class:`ModelState` bundles together the state variables, cache variables, and other information
                necessary to describe a single state of a :class:`Model`. :class:`Model`\s can read/manipulate
                :class:`ModelState`\s in order to :meth:`Model.realize` the state to a later stage
                (e.g. as part of forward integration) or read outputs values. However, :class:`ModelState`\s may
                also be created, read, and manipulated by downstream Python code and other utilities in OPynSim.
            )"
        );
        model_state_class.def_prop_ro(
            "stage",
            &ModelState::stage,
            R"(
                Returns the current :class:`ModelStateStage` of the state.

                Notes:
                    A state may be realized to a later stage with :meth:`Model.realize`.
            )"
        );
    }

    void register_model_states_class(nb::module_& m)
    {
        nb::class_<ModelStates> cls(
            m,
            "ModelStates",
            R"(
                Represents a sequence of :class:`ModelState`\s.

                ``ModelStates`` is typically returned from functions that produce sequences
                of :class:`ModelState`\s, such as :meth:`Model.states_from_data_frame`. The
                API of ``ModelStates`` is list-like, meaning downstream code can iterate over
                each ``ModelState``, use ``len`` with it, randomly access a state with ``model_states[idx]``
                and so on.
            )"
        );
        cls.def(nb::init<>());
        cls.def("__len__", &ModelStates::size);
        cls.def("__getitem__", [](ModelStates& states, ptrdiff_t idx)
        {
            if (idx < 0) {
                idx += static_cast<ptrdiff_t>(states.size());
            }
            return states.handle_at(idx);
        });
        cls.def("__getitem__", [](ModelStates& states, const nb::slice& slice)
        {
            const auto [start, stop, step, slice_length] = slice.compute(states.size());
            ModelStates rv;
            rv.reserve(slice_length);
            for (size_t i = 0; i < slice_length; ++i) {
                const auto cur = static_cast<size_t>(start + (static_cast<decltype(step)>(i)*step));
                rv.handle_push_back(states.handle_at(cur));
            }
            return rv;
        });
        cls.def("to_list", &ModelStates::to_handle_list);
    }

    void register_model_state_stage_class(nb::module_& m)
    {
        static_assert(osc::num_options<ModelStateStage>() == 9);
        nb::enum_<ModelStateStage> model_state_stage_class(
            m,
            "ModelStateStage",
            R"(
                Represents a stage of state realization (computation).

                When calling methods like :meth:`Model.realize`, a :class:`ModelState` is
                realized in-order through each :class:`ModelStateStage`, starting at the lowest
                stage and ending at the highest stage.

                Each time a :class:`ModelState` advances through a :class:`ModelStateStage`, more
                information is available in the state. For example, after a :class:`ModelState` is
                realized to :attr:`ModelStateStage.POSITION`, positional quantities such as the positions
                of bodies and offset frames are known, and any positional output on the associated
                :class:`Model` can then extract that information from the :class:`ModelState`.

                For convenience, the :mod:`opynsim` module defines aliases for each :class:`ModelStateStage`:

                - :attr:`opynsim.STAGE_TOPOLOGY` -> :attr:`opynsim.ModelStateStage.TOPOLOGY`
                - :attr:`opynsim.STAGE_MODEL` -> :attr:`opynsim.ModelStateStage.MODEL`
                - :attr:`opynsim.STAGE_INSTANCE` -> :attr:`opynsim.ModelStateStage.INSTANCE`
                - :attr:`opynsim.STAGE_TIME` -> :attr:`opynsim.ModelStateStage.TIME`
                - :attr:`opynsim.STAGE_POSITION` -> :attr:`opynsim.ModelStateStage.POSITION`
                - :attr:`opynsim.STAGE_VELOCITY` -> :attr:`opynsim.ModelStateStage.VELOCITY`
                - :attr:`opynsim.STAGE_DYNAMICS` -> :attr:`opynsim.ModelStateStage.DYNAMICS`
                - :attr:`opynsim.STAGE_ACCELERATION` -> :attr:`opynsim.ModelStateStage.ACCELERATION`
                - :attr:`opynsim.STAGE_REPORT` -> :attr:`opynsim.ModelStateStage.REPORT`
            )"
        );
        model_state_stage_class.value("TOPOLOGY",     ModelStateStage::topology,     "System topology known.");
        model_state_stage_class.value("MODEL",        ModelStateStage::model,        "Modelling choices have been made.");
        model_state_stage_class.value("INSTANCE",     ModelStateStage::instance,     "Physical parameters have been set.");
        model_state_stage_class.value("TIME",         ModelStateStage::time,         "Time has advanced and state variables have new values, but no derived information has been calculated.");
        model_state_stage_class.value("POSITION",     ModelStateStage::position,     "The spatial positions of all bodies are known.");
        model_state_stage_class.value("VELOCITY",     ModelStateStage::velocity,     "The spatial velocities of all bodies are known.");
        model_state_stage_class.value("DYNAMICS",     ModelStateStage::dynamics,     "The force acting on each body is known, along with total kinetic/potential energy.");
        model_state_stage_class.value("ACCELERATION", ModelStateStage::acceleration, "The time derivatives of all continuous state variables are known.");
        model_state_stage_class.value("REPORT",       ModelStateStage::report,       "Additional variables useful for output are known");

        // Define convenience aliases for the enum
        m.attr("STAGE_TOPOLOGY")     = model_state_stage_class.attr("TOPOLOGY");
        m.attr("STAGE_MODEL")        = model_state_stage_class.attr("MODEL");
        m.attr("STAGE_INSTANCE")     = model_state_stage_class.attr("INSTANCE");
        m.attr("STAGE_TIME")         = model_state_stage_class.attr("TIME");
        m.attr("STAGE_POSITION")     = model_state_stage_class.attr("POSITION");
        m.attr("STAGE_VELOCITY")     = model_state_stage_class.attr("VELOCITY");
        m.attr("STAGE_DYNAMICS")     = model_state_stage_class.attr("DYNAMICS");
        m.attr("STAGE_ACCELERATION") = model_state_stage_class.attr("ACCELERATION");
        m.attr("STAGE_REPORT")       = model_state_stage_class.attr("REPORT");
    }

    void register_readers(nb::module_& m)
    {
        m.def(
            "read_osim",
            [](const std::filesystem::path& source) { return opyn::read_osim(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`ModelSpecification` parsed from an `.osim` file on the
                caller's filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_sto",
            [](const std::filesystem::path& source) { return opyn::read_sto(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`DataFrame` parsed from an ``.sto`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_mot",
            [](const std::filesystem::path& source) { return opyn::read_mot(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`DataFrame` parsed from an ``.mot`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_trc",
            [](const std::filesystem::path& source) { return opyn::read_trc(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`DataFrame` parsed from an ``.trc`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_csv",
            [](const std::filesystem::path& source) { return opyn::read_csv(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`DataFrame` parsed from an ``.csv`` file on the caller's
                filesystem.

                The CSV file must have a header section, delimited by 'endheader`. This usually
                necessitates adding an `endheader` entry just above the header row (TODO: this
                limitation was inherited from OpenSim and shouldn't be a thing long-term).

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_vtp",
            [](const std::filesystem::path& source) { return opyn::read_vtp(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`graphics.Mesh` parsed from a ``.vtp`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_obj",
            [](const std::filesystem::path& source) { return opyn::read_obj(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`graphics.Mesh` parsed from a ``.obj`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_stl",
            [](const std::filesystem::path& source) { return opyn::read_obj(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`graphics.Mesh` parsed from a ``.stl`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_png",
            [](const std::filesystem::path& source) { return opyn::read_png(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`graphics.Texture2D` parsed from a ``.png`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_jpeg",
            [](const std::filesystem::path& source) { return opyn::read_jpeg(source); },
            nb::arg("source"),
            R"(
                Returns a :class:`graphics.Texture2D` parsed from a ``.jpeg`` file on the caller's
                filesystem.

                Raises:
                    RuntimeError: If the file cannot be found, read, or is invalid.
            )"
        );

        m.def(
            "read_jpg",
            [](const std::filesystem::path& source) { return opyn::read_jpg(source); },
            nb::arg("source"),
            "An alias for :func:`read_jpeg`"
        );
    }
}

opyn::OPynSimApp& opyn::get_lazy_loaded_opynsim_app()
{
    if (not g_lazy_loaded_app) {
        g_lazy_loaded_app = std::make_unique<OPynSimApp>();
    }
    return *g_lazy_loaded_app;
}

void opyn::destroy_lazy_loaded_opynsim_app()
{
    g_lazy_loaded_app.reset();
}

NB_MODULE(_core, _core_module)  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,misc-use-anonymous-namespace)
{
    // Install an exit handler that cleans up any lazy-loaded application state
    // when the Python interpreter shuts down
    nb::module_::import_("atexit").attr("register")(nb::cpp_function(&destroy_lazy_loaded_opynsim_app));

    // Initialize `config` submodule (also initializes the `opynsim` C++ API, logging, etc.).
    {
        auto config_submodule = _core_module.def_submodule("config");
        init_config_submodule(config_submodule);
    }

    // Initialize `examples` submodule.
    {
        auto examples_submodule = _core_module.def_submodule("examples");
        init_examples_submodule(examples_submodule);
    }

    // Initialize `graphics` submodule.
    {
        auto graphics_submodule = _core_module.def_submodule("graphics");
        init_graphics_submodule(graphics_submodule);
    }

    // Initialize `tps3d` submodule.
    {
        auto tps3d_submodule = _core_module.def_submodule("tps3d");
        init_tps3d_submodule(tps3d_submodule);
    }

    // Initialize `ui` submodule.
    {
        auto ui_submodule = _core_module.def_submodule("ui");
        init_ui_submodule(ui_submodule);
    }

    // Initialize top-level functions/classes
    register_symbol_class(_core_module);
    register_model_specification_class(_core_module);
    register_model_state_stage_class(_core_module);
    register_model_class(_core_module);
    register_model_state_class(_core_module);
    register_model_states_class(_core_module);
    register_dataframe_class(_core_module);
    register_readers(_core_module);
}
