#include "single_state_simulation.h"

#include <libopensimcreator/documents/model/basic_model_state_pair_with_shared_environment.h>
#include <libopensimcreator/documents/param_block.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/utilities/synchronized_value.h>
#include <liboscar/utilities/synchronized_value_guard.h>

using namespace osc;

class osc::SingleStateSimulation::Impl final {
public:
    explicit Impl(BasicModelStatePairWithSharedEnvironment modelState) :
        m_ModelState{std::move(modelState)}
    {}

    SynchronizedValueGuard<const OpenSim::Model> getModel() const
    {
        return m_ModelState.lock_child<OpenSim::Model>([](const BasicModelStatePairWithSharedEnvironment& ms) -> const OpenSim::Model& { return ms.get_model(); });
    }

    ptrdiff_t getNumReports() const
    {
        return 0;
    }

    SimulationReport getSimulationReport(ptrdiff_t) const
    {
        throw std::runtime_error{"invalid method call on a SingleStateSimulation"};
    }

    std::vector<SimulationReport> getAllSimulationReports() const
    {
        return {};
    }

    SimulationStatus getStatus() const
    {
        return SimulationStatus::Completed;
    }

    SimulationClocks getClocks() const
    {
        return SimulationClocks{SimulationClock::start()};
    }

    const ParamBlock& getParams() const
    {
        return m_Params;
    }

    std::span<const opyn::SharedOutputExtractor> getOutputExtractors() const
    {
        return {};
    }

    float getFixupScaleFactor() const
    {
        return m_ModelState.lock()->get_fixup_scale_factor();
    }

    void setFixupScaleFactor(float v)
    {
        m_ModelState.lock()->set_fixup_scale_factor(v);
    }

    std::shared_ptr<Environment> implUpdAssociatedEnvironment()
    {
        return m_ModelState.lock()->try_upd_environment();
    }

private:
    SynchronizedValue<BasicModelStatePairWithSharedEnvironment> m_ModelState;
    ParamBlock m_Params;
};


osc::SingleStateSimulation::SingleStateSimulation(BasicModelStatePairWithSharedEnvironment modelState) :
    impl_{std::make_unique<Impl>(std::move(modelState))}
{}
osc::SingleStateSimulation::SingleStateSimulation(SingleStateSimulation&&) noexcept = default;
osc::SingleStateSimulation& osc::SingleStateSimulation::operator=(SingleStateSimulation&&) noexcept = default;
osc::SingleStateSimulation::~SingleStateSimulation() noexcept = default;

SynchronizedValueGuard<const OpenSim::Model> osc::SingleStateSimulation::impl_get_model() const
{
    return impl_->getModel();
}

ptrdiff_t osc::SingleStateSimulation::impl_get_num_reports() const
{
    return impl_->getNumReports();
}

SimulationReport osc::SingleStateSimulation::impl_get_simulation_report(ptrdiff_t reportIndex) const
{
    return impl_->getSimulationReport(reportIndex);
}

std::vector<SimulationReport> osc::SingleStateSimulation::impl_get_all_simulation_reports() const
{
    return impl_->getAllSimulationReports();
}

SimulationStatus osc::SingleStateSimulation::impl_get_status() const
{
    return impl_->getStatus();
}

SimulationClocks osc::SingleStateSimulation::impl_get_clocks() const
{
    return impl_->getClocks();
}

const ParamBlock& osc::SingleStateSimulation::impl_get_params() const
{
    return impl_->getParams();
}

std::span<const opyn::SharedOutputExtractor> osc::SingleStateSimulation::impl_get_output_extractors() const
{
    return impl_->getOutputExtractors();
}

float osc::SingleStateSimulation::impl_get_fixup_scale_factor() const
{
    return impl_->getFixupScaleFactor();
}

void osc::SingleStateSimulation::impl_set_fixup_scale_factor(float v)
{
    impl_->setFixupScaleFactor(v);
}

std::shared_ptr<Environment> osc::SingleStateSimulation::impl_upd_associated_environment()
{
    return impl_->implUpdAssociatedEnvironment();
}
