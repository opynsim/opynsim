#pragma once

#include <libopynsim/model_state.h>
#include <libopynsim/symbol.h>

#include <liboscar/maths/quaternion.h>
#include <liboscar/maths/vector.h>
#include <liboscar/utilities/assertions.h>

#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

namespace opyn
{
    /// Represents the output of the inverse kinematics (IK) solver.
    class IKResult final {
    public:
        /// Returns the number of states in this result. This is equal to
        /// the number of measurement time points provided to the solver.
        size_t num_states() const;

        /// Returns a reference to the `i`th `ModelState` in this result.
        const ModelState& model_state(size_t i) const;

    private:
        std::vector<ModelState> model_states_;
    };

    /// Represents a sequence of IK measurements associated to one component
    /// in the model.
    template<typename T>
    class IKMeasurements final {
    public:

        /// Constructs a single-element measurement sequence associated with
        /// `model_component` with given `value`, `time`, and `weight`.
        template<typename U>
        requires std::constructible_from<T, U>
        explicit IKMeasurements(
            const Symbol& model_component,
            U&& value,
            double time = 0.0,
            double weight = 1.0) :

            model_component_{model_component},
            times_{time},
            values_{std::forward<U>(value)},
            weights_{weight}
        {}

        /// Constructs a measurement sequence associated with `model_component` of
        /// `values` at `times`, where all values have equal weight (1.0).
        ///
        /// The number of `times` must be equal to the number of `values`.
        explicit IKMeasurements(
            const Symbol& model_component,
            std::vector<double> times,
            std::vector<T> values) :

            model_component_{model_component},
            times_{std::move(times)},
            values_{std::move(values)}
        {
            OSC_ASSERT_ALWAYS(times_.size() == values_.size() && "The number of times/weights/values in an IK measurement sequence must be equal");
            weights_.resize(times_.size(), 1.0);
        }

        /// Constructs a measurement sequence associated with `model_component` of
        /// `values` at `times` with `weights`.
        ///
        /// The number of `times`, `values`, and `weights` must be equal.
        explicit IKMeasurements(
            const Symbol& model_component,
            std::vector<double> times,
            std::vector<T> values,
            std::vector<double> weights) :

            model_component_{model_component},
            times_{std::move(times)},
            values_{std::move(values)},
            weights_{std::move(weights)}
        {
            OSC_ASSERT_ALWAYS(times_.size() == values_.size() && values_.size() == weights_.size() && "The number of times/weights/values in an IK measurement sequence must be equal");
        }

        /// Returns a `Symbol` that can be used to look up the component in the model
        /// that's associated with these measurements.
        const Symbol& model_component() const { return model_component_; }

        /// Returns the number of measurements in the sequence.
        size_t size() const { return times_.size(); }

        /// Returns the measurement time of the `i`th measurement in the sequence.
        double time(size_t i) const { return times_.at(i); }

        /// Returns the value of the `i`th measurement in the sequence.
        const T& value(size_t i) const { return values_.at(i); }

        /// Returns the weight of the `i`th measurement in the sequence.
        double weight(size_t i) const { return weights_.at(i); }

    private:
        Symbol model_component_;
        std::vector<double> times_;
        std::vector<T> values_;
        std::vector<double> weights_;
    };

    /// Represents a sequence of experimental measurements of locations in
    /// ground for a station (e.g. the locations of a motion capture marker
    /// over time).
    using IKStationMeasurements = IKMeasurements<osc::Vector3d>;

    /// Represents a sequence of experimental measurements of orientations
    /// in ground for a frame (e.g. the orientations of an IMU over time).
    using IKOrientationMeasurements = IKMeasurements<osc::Quaterniond>;

    /// Represents a sequence of experimental measurements of a coordinate
    /// in the model (e.g. the angle of a joint over time).
    using IKCoordinateMeasurements = IKMeasurements<double>;

    /// Returns the result of running an IK solver on `model` for the provided measurements.
    IKResult inverse_kinematics_solve(
        const Model& model,
        const std::optional<IKStationMeasurements>& station_measurements = {},
        const std::optional<IKOrientationMeasurements>& orientation_measurements = {},
        const std::optional<IKCoordinateMeasurements>& coordinate_measurements = {}
    );
}
