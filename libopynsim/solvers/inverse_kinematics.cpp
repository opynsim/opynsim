#include "inverse_kinematics.h"

#include <libopynsim/model.h>
#include <OpenSim/Simulation/CoordinateReference.h>
#include <OpenSim/Simulation/InverseKinematicsSolver.h>
#include <OpenSim/Simulation/MarkersReference.h>
#include <OpenSim/Simulation/OrientationsReference.h>

using namespace opyn;

namespace
{
    OpenSim::InverseKinematicsSolver create_ik_solver(
        const OpenSim::Model& model,
        std::optional<OpenSim::MarkersReference>& markers_reference,
        std::optional<OpenSim::OrientationsReference>& orientations_reference,
        SimTK::Array_<OpenSim::CoordinateReference>& coordinates_reference)
    {
        if (markers_reference and orientations_reference) {
            return OpenSim::InverseKinematicsSolver{model, *markers_reference, *orientations_reference, coordinates_reference};
        }
        if (markers_reference) {
            return OpenSim::InverseKinematicsSolver{model, *markers_reference, coordinates_reference};
        }
        throw std::runtime_error{"Cannot construct an `OpenSim::InverseKinematicsSolver`"};
    }
}

IKResult opyn::inverse_kinematics_solve(
    const Model& model,
    const std::optional<IKStationMeasurements>& station_measurements,
    const std::optional<IKOrientationMeasurements>& orientation_measurements,
    const std::optional<IKCoordinateMeasurements>& coordinate_measurements)
{
    OSC_ASSERT_ALWAYS((station_measurements or orientation_measurements or coordinate_measurements) and "Must provide at least some kind of measurement to the IK solver");

    std::optional<OpenSim::MarkersReference> markers_reference;
    if (station_measurements) {
        // TODO: build markers ref
    }

    std::optional<OpenSim::OrientationsReference> orientations_reference;
    if (orientation_measurements) {
        // TODO: build orientations ref
    }

    SimTK::Array_<OpenSim::CoordinateReference> coordinates_reference;
    if (coordinate_measurements) {
        // TODO: buid coordinate ref
    }

    OpenSim::InverseKinematicsSolver ik_solver = create_ik_solver(
        model.open_sim_model(),
        markers_reference,
        orientations_reference,
        coordinates_reference
    );

    return IKResult{};  // TODO
}
