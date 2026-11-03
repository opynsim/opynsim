#pragma once

#include <libopensimcreator/documents/simulation/simulation_clock.h>
#include <libopensimcreator/documents/param_block.h>

#include <libopynsim/solvers/integrator_method.h>

namespace osc
{
    // simulation parameters
    struct ForwardDynamicSimulatorParams final {

        ForwardDynamicSimulatorParams();

        // final time for the simulation
        SimulationClock::time_point final_time;

        // which integration method to use for the simulation
        opyn::IntegratorMethod integrator_method_used;

        // the time interval, in simulation time, between report updates
        SimulationClock::duration reporting_interval;

        // max number of *internal* steps that may be taken within a single call
        // to the integrator's stepTo or stepBy function
        //
        // this is mostly an internal concern, but can affect how regularly the
        // simulator reports updates (e.g. a lower number here *may* mean more
        // frequent per-significant-step updates)
        int integrator_step_limit;

        // minimum step, in time, that the integrator should attempt
        //
        // some integrators just ignore this
        SimulationClock::duration integrator_minimum_step_size;

        // maximum step, in time, that an integrator can attempt
        //
        // e.g. even if the integrator *thinks* it can skip 10s of simulation time
        // it still *must* integrate to this size and return to the caller (i.e. the
        // simulator) to report the state at this maximum time
        SimulationClock::duration integrator_maximum_step_size;

        // accuracy of the integrator
        //
        // this only does something if the integrator is error-controlled and able
        // to improve accuracy (e.g. by taking many more steps)
        double integrator_accuracy;

        friend bool operator==(const ForwardDynamicSimulatorParams&, const ForwardDynamicSimulatorParams&) = default;
    };

    // convert to a generic parameter block (for UI binding)
    ParamBlock to_param_block(const ForwardDynamicSimulatorParams&);
    ForwardDynamicSimulatorParams from_param_block(const ParamBlock&);
}
