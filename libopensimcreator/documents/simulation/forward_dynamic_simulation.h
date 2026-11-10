#pragma once

#include <libopensimcreator/documents/model/basic_model_state_pair_with_shared_environment.h>
#include <libopensimcreator/documents/simulation/abstract_simulation.h>
#include <libopensimcreator/documents/simulation/simulation_clock.h>
#include <libopensimcreator/documents/simulation/simulation_report.h>
#include <libopensimcreator/documents/simulation/simulation_status.h>

#include <liboscar/utilities/synchronized_value_guard.h>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace opyn
{
    class SharedOutputExtractor;
}

namespace OpenSim
{
    class Model;
}

namespace osc
{
    struct ForwardDynamicSimulatorParams;
    class ParamBlock;

    // an `AbstractSimulation` that represents a live forward-dynamic simulation
    // that `osc` is running
    class ForwardDynamicSimulation final : public AbstractSimulation {
    public:
        ForwardDynamicSimulation(BasicModelStatePairWithSharedEnvironment, const ForwardDynamicSimulatorParams&);
        ForwardDynamicSimulation(const ForwardDynamicSimulation&) = delete;
        ForwardDynamicSimulation(ForwardDynamicSimulation&&) noexcept;
        ForwardDynamicSimulation& operator=(const ForwardDynamicSimulation&) = delete;
        ForwardDynamicSimulation& operator=(ForwardDynamicSimulation&&) noexcept;
        ~ForwardDynamicSimulation() noexcept override;

        // blocks the current thread until the simulator thread finishes its execution
        void join();

    private:
        SynchronizedValueGuard<const OpenSim::Model> impl_get_model() const final;

        ptrdiff_t impl_get_num_reports() const final;
        SimulationReport impl_get_simulation_report(ptrdiff_t) const final;
        std::vector<SimulationReport> impl_get_all_simulation_reports() const final;

        SimulationStatus impl_get_status() const final;
        SimulationClocks impl_get_clocks() const final;
        const ParamBlock& impl_get_params() const final;
        std::span<const opyn::SharedOutputExtractor> impl_get_output_extractors() const final;

        bool impl_can_change_end_time() const final { return true; }
        void impl_request_new_end_time(SimulationClock::time_point) final;

        void impl_request_stop() final;
        void impl_stop() final;

        float impl_get_fixup_scale_factor() const final;
        void impl_set_fixup_scale_factor(float) final;

        std::shared_ptr<Environment> impl_upd_associated_environment() final;

        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
