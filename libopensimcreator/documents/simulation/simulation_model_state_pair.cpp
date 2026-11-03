#include "simulation_model_state_pair.h"

#include <libopensimcreator/documents/model/basic_model_state_pair_with_shared_environment.h>
#include <libopensimcreator/documents/simulation/simulation.h>
#include <libopensimcreator/documents/simulation/simulation_report.h>
#include <libopensimcreator/documents/simulation/single_state_simulation.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/utilities/uid.h>
#include <OpenSim/Common/Component.h>
#include <OpenSim/Common/ComponentPath.h>
#include <OpenSim/Simulation/Model/Model.h>

#include <memory>
#include <utility>

using namespace osc;

class osc::SimulationModelStatePair::Impl final {
public:
    Impl() :
        m_Simulation{std::make_shared<Simulation>(SingleStateSimulation{BasicModelStatePairWithSharedEnvironment{}})}
    {}

    Impl(std::shared_ptr<Simulation> simulation, SimulationReport simulationReport) :
        m_Simulation{std::move(simulation)},
        m_SimulationReport{std::move(simulationReport)}
    {}

    const OpenSim::Model& getModel() const
    {
        return *m_Simulation->get_model();  // TODO: UH OH - lock leak (#707 - are locks necessary?)
    }

    UID getModelVersion() const
    {
        return m_ModelVersion;
    }

    const SimTK::State& getState() const
    {
        return m_SimulationReport.state();
    }

    UID getStateVersion() const
    {
        return m_StateVersion;
    }

    const OpenSim::Component* getSelected() const
    {
        return opyn::find_component(getModel(), m_Selected);
    }

    void setSelected(const OpenSim::Component* c)
    {
        m_Selected = opyn::get_absolute_path_or_empty(c);
    }

    const OpenSim::Component* getHovered() const
    {
        return opyn::find_component(getModel(), m_Hovered);
    }

    void setHovered(const OpenSim::Component* c)
    {
        m_Hovered = opyn::get_absolute_path_or_empty(c);
    }

    float getFixupScaleFactor() const
    {
        return m_Simulation->get_fixup_scale_factor();
    }

    void setFixupScaleFactor(float v)
    {
        m_Simulation->set_fixup_scale_factor(v);
    }

    std::shared_ptr<Environment> implUpdAssociatedEnvironment() const
    {
        return m_Simulation->try_upd_environment();
    }

    std::shared_ptr<Simulation> updSimulation()
    {
        return m_Simulation;
    }

    void setSimulation(std::shared_ptr<Simulation> s)
    {
        if (s != m_Simulation)
        {
            m_Simulation = std::move(s);
            m_ModelVersion = UID{};
        }
    }

    SimulationReport getSimulationReport() const
    {
        return m_SimulationReport;
    }

    void setSimulationReport(SimulationReport r)
    {
        if (r != m_SimulationReport)
        {
            m_SimulationReport = std::move(r);
            m_StateVersion = UID{};
        }
    }

private:
    UID m_ModelVersion;
    UID m_StateVersion;
    OpenSim::ComponentPath m_Selected;
    OpenSim::ComponentPath m_Hovered;
    std::shared_ptr<Simulation> m_Simulation;
    SimulationReport m_SimulationReport;
};


// public API (PIMPL)

osc::SimulationModelStatePair::SimulationModelStatePair() :
    impl_{std::make_unique<Impl>()}
{
}

osc::SimulationModelStatePair::SimulationModelStatePair(std::shared_ptr<Simulation> simulation, SimulationReport report) :
    impl_{std::make_unique<Impl>(std::move(simulation), std::move(report))}
{
}

osc::SimulationModelStatePair::SimulationModelStatePair(SimulationModelStatePair&&) noexcept = default;
osc::SimulationModelStatePair& osc::SimulationModelStatePair::operator=(SimulationModelStatePair&&) noexcept = default;
osc::SimulationModelStatePair::~SimulationModelStatePair() noexcept = default;

std::shared_ptr<Simulation> osc::SimulationModelStatePair::upd_simulation()
{
    return impl_->updSimulation();
}

void osc::SimulationModelStatePair::set_simulation(std::shared_ptr<Simulation> sim)
{
    impl_->setSimulation(std::move(sim));
}

SimulationReport osc::SimulationModelStatePair::get_simulation_report() const
{
    return impl_->getSimulationReport();
}

void osc::SimulationModelStatePair::set_simulation_report(SimulationReport report)
{
    impl_->setSimulationReport(std::move(report));
}

const OpenSim::Model& osc::SimulationModelStatePair::impl_get_model() const
{
    return impl_->getModel();
}

UID osc::SimulationModelStatePair::impl_get_model_version() const
{
    return impl_->getModelVersion();
}

const SimTK::State& osc::SimulationModelStatePair::impl_get_state() const
{
    return impl_->getState();
}

UID osc::SimulationModelStatePair::impl_get_state_version() const
{
    return impl_->getStateVersion();
}

const OpenSim::Component* osc::SimulationModelStatePair::impl_get_selected() const
{
    return impl_->getSelected();
}

void osc::SimulationModelStatePair::impl_set_selected(const OpenSim::Component* c)
{
    impl_->setSelected(c);
}

const OpenSim::Component* osc::SimulationModelStatePair::impl_get_hovered() const
{
    return impl_->getHovered();
}

void osc::SimulationModelStatePair::impl_set_hovered(const OpenSim::Component* c)
{
    impl_->setHovered(c);
}

float osc::SimulationModelStatePair::impl_get_fixup_scale_factor() const
{
    return impl_->getFixupScaleFactor();
}

void osc::SimulationModelStatePair::impl_set_fixup_scale_factor(float v)
{
    impl_->setFixupScaleFactor(v);
}

std::shared_ptr<Environment> osc::SimulationModelStatePair::impl_upd_associated_environment() const
{
    return impl_->implUpdAssociatedEnvironment();
}
