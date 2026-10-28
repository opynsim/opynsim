#include "graphics.h"

#include <opynsim/_core/type_casters/vector.h>
#include <opynsim/_core/_core.h>
#include <opynsim/_core/camera.h>
#include <opynsim/_core/color.h>
#include <opynsim/_core/mesh.h>
#include <opynsim/_core/scene_cache.h>
#include <opynsim/_core/texture2d.h>

#include <libopynsim/graphics/render_model_in_state.h>
#include <liboscar/graphics/scene/scene_cache.h>
#include <libopynsim/model.h>
#include <libopynsim/model_state.h>
#include <liboscar/graphics/camera.h>
#include <liboscar/graphics/mesh.h>
#include <liboscar/graphics/texture2d.h>
#include <liboscar/maths/geometric_functions.h>
#include <liboscar/utilities/assertions.h>
#include <nanobind/nanobind.h>
#pragma warning(push)
#pragma warning(disable : 4702) // Disable "unreachable code"
#include <nanobind/ndarray.h>
#pragma warning(pop)
#include <nanobind/stl/array.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/unique_ptr.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <new>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace nb = nanobind;
using namespace opyn;

namespace
{
    void def_color(nanobind::module_& m)
    {
        python::def_color(m);
    }

    void def_camera(nanobind::module_& m)
    {
        python::def_camera(m);
    }

    void def_texture2d(nanobind::module_& m)
    {
        python::def_texture2d(m);
    }

    void def_mesh(nanobind::module_& m)
    {
        python::def_mesh(m);
    }

    void def_scene_cache(nanobind::module_& m)
    {
        python::def_scene_cache(m);
    }

    void def_render_model_in_state(nanobind::module_& m)
    {
        m.def(
            "render_model_in_state",
            [](const Model& model,
               const ModelState& model_state,
               std::pair<int, int> dimensions,
               const osc::Color& background_color,
               bool draw_floor,
               osc::SceneCache* scene_cache,
               const osc::Camera* camera)
            {
                return render_model_in_state(
                    get_lazy_loaded_opynsim_app(),
                    model,
                    model_state,
                    osc::Vector2{dimensions.first, dimensions.second},
                    background_color,
                    draw_floor,
                    scene_cache,
                    camera
                );
            },
            nb::arg("model"),
            nb::arg("model_state"),
            nb::kw_only{},
            nb::arg("dimensions") = std::make_pair(640, 480),
            nb::arg("background_color") = osc::Color::clear(),
            nb::arg("draw_floor") = false,
            nb::arg("scene_cache") = nullptr,
            nb::arg("camera") = nullptr,
            R"(
                Renders the given :class:`opynsim.Model` + :class:`opynsim.ModelState` to
                a :class:`opynsim.graphics.Texture2D`.

                Args:
                    model (opynsim.Model): The model to render.
                    model_state (opynsim.ModelState): The state of the model to render. Should be realized to at least :attr:`opynsim.ModelStateStage.REPORT`.
                    dimensions (tuple[int, int]): The desired output resolution (width, height) of the rendered image in pixels.
                    background_color (opynsim.graphics.Color): The desired background color of the rendered scene.
                    draw_floor (bool): Toggles drawing a chequered floor at Y=0 in the scene.
                    scene_cache (opynsim.graphics.SceneCache): A scene cache from which the implementation pulls cached scene elements (shaders, meshes, etc.). Otherwise, the implementation loads all assets every time this function is called (slow).
                    camera (opynsim.graphics.Camera): The camera to use when rendering (default: looks down -Z and pulls back from the origin until the entire model fits in-frame).

                Returns:
                    opynsim.graphics.Texture2D: The rendered image, which will have the specified ``dimensions``.
            )"
        );
    }
}

void opyn::init_graphics_submodule(nanobind::module_& graphics_module)
{
    def_color(graphics_module);
    def_scene_cache(graphics_module);
    def_texture2d(graphics_module);
    def_mesh(graphics_module);
    def_camera(graphics_module);
    def_render_model_in_state(graphics_module);
}
