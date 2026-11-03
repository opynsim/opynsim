#include "sto_file_simulation.h"

#include <libopensimcreator/documents/simulation/simulation_clock.h>
#include <libopensimcreator/documents/simulation/simulation_report.h>
#include <libopensimcreator/documents/simulation/simulation_status.h>
#include <libopensimcreator/documents/param_block.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <OpenSim/Common/Component.h>
#include <OpenSim/Common/StateVector.h>
#include <OpenSim/Common/Storage.h>
#include <OpenSim/Simulation/Model/Model.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace osc;

namespace
{
    SimTK::State CreateStateFromStorageRow(
        OpenSim::Model& model,
        const OpenSim::Storage& storage,
        const std::unordered_map<int, int>& lut,
        int row)
    {
        SimTK::State state = model.getWorkingState();
        opyn::update_state_variables_from_storage_row(model, state, lut, storage, row);
        model.assemble(state);
        model.realizeReport(state);
        return state;
    }

    std::vector<SimulationReport> ExtractReports(
        OpenSim::Model& model,
        const std::filesystem::path& stoFilePath)
    {
        // load+condition the underlying `OpenSim::Storage`
        const auto storage = opyn::load_storage(model, stoFilePath, opyn::StorageLoadingParameters{
            .resample_to_frequency = 1.0/100.0,  // resample the state trajectory at 100FPS (#708)
        });

        // map column header indices in the `OpenSim::Storage` to the `OpenSim::Model`'s state variables' indices
        const auto lut = opyn::create_storage_index_to_model_statevar_mapping_with_warnings(model, *storage);

        // init the model+state with unlocked coordinates
        opyn::initialize_model(model);
        opyn::initialize_state(model);

        // convert each timestep in the `OpenSim::Storage` as a `SimulationReport`
        std::vector<SimulationReport> rv;
        rv.reserve(storage->getSize());
        for (int row = 0; row < storage->getSize(); ++row) {
            rv.emplace_back(CreateStateFromStorageRow(model, *storage, lut, row));
        }
        return rv;
    }
}

class osc::StoFileSimulation::Impl final {
public:
    Impl(
        std::unique_ptr<OpenSim::Model> model,
        const std::filesystem::path& stoFilePath,
        float fixupScaleFactor,
        std::shared_ptr<Environment> environment) :

        m_Model{std::move(model)},
        m_SimulationReports{ExtractReports(*m_Model, stoFilePath)},
        m_FixupScaleFactor{fixupScaleFactor},
        m_Environment{std::move(environment)}
    {}

    SynchronizedValueGuard<const OpenSim::Model> getModel() const
    {
        return {m_ModelMutex, *m_Model};
    }

    size_t getNumReports() const
    {
        return m_SimulationReports.size();
    }

    SimulationReport getSimulationReport(ptrdiff_t reportIndex) const
    {
        return m_SimulationReports.at(reportIndex);
    }

    std::vector<SimulationReport> getAllSimulationReports() const
    {
        return m_SimulationReports;
    }

    SimulationStatus getStatus() const
    {
        return SimulationStatus::Completed;
    }

    SimulationClocks getClocks() const
    {
        return SimulationClocks{{m_Start, m_End}};
    }

    const ParamBlock& getParams() const
    {
        return m_ParamBlock;
    }

    std::span<const opyn::SharedOutputExtractor> getOutputExtractors() const
    {
        return {};
    }

    float getFixupScaleFactor() const
    {
        return m_FixupScaleFactor;
    }

    void setFixupScaleFactor(float v)
    {
        m_FixupScaleFactor = v;
    }

    std::shared_ptr<Environment> implUpdAssociatedEnvironment()
    {
        return m_Environment;
    }

private:
    mutable std::mutex m_ModelMutex;
    std::unique_ptr<OpenSim::Model> m_Model;
    std::vector<SimulationReport> m_SimulationReports;
    SimulationClock::time_point m_Start = m_SimulationReports.empty() ? SimulationClock::start() : m_SimulationReports.front().get_time();
    SimulationClock::time_point m_End = m_SimulationReports.empty() ? SimulationClock::start() : m_SimulationReports.back().get_time();
    ParamBlock m_ParamBlock;
    float m_FixupScaleFactor = 1.0f;
    std::shared_ptr<Environment> m_Environment;
};

osc::StoFileSimulation::StoFileSimulation(
    std::unique_ptr<OpenSim::Model> model,
    const std::filesystem::path& stoFilePath,
    float fixupScaleFactor,
    std::shared_ptr<Environment> environment) :
    impl_{std::make_unique<Impl>(std::move(model), stoFilePath, fixupScaleFactor, std::move(environment))}
{}
osc::StoFileSimulation::StoFileSimulation(StoFileSimulation&&) noexcept = default;
osc::StoFileSimulation& osc::StoFileSimulation::operator=(StoFileSimulation&&) noexcept = default;
osc::StoFileSimulation::~StoFileSimulation() noexcept = default;

SynchronizedValueGuard<const OpenSim::Model> osc::StoFileSimulation::impl_get_model() const
{
    return impl_->getModel();
}

ptrdiff_t osc::StoFileSimulation::impl_get_num_reports() const
{
    return impl_->getNumReports();
}

SimulationReport osc::StoFileSimulation::impl_get_simulation_report(ptrdiff_t reportIndex) const
{
    return impl_->getSimulationReport(reportIndex);
}

std::vector<SimulationReport> osc::StoFileSimulation::impl_get_all_simulation_reports() const
{
    return impl_->getAllSimulationReports();
}

SimulationStatus osc::StoFileSimulation::impl_get_status() const
{
    return impl_->getStatus();
}

SimulationClocks osc::StoFileSimulation::impl_get_clocks() const
{
    return impl_->getClocks();
}

const ParamBlock& osc::StoFileSimulation::impl_get_params() const
{
    return impl_->getParams();
}

std::span<const opyn::SharedOutputExtractor> osc::StoFileSimulation::impl_get_output_extractors() const
{
    return impl_->getOutputExtractors();
}

float osc::StoFileSimulation::impl_get_fixup_scale_factor() const
{
    return impl_->getFixupScaleFactor();
}

void osc::StoFileSimulation::impl_set_fixup_scale_factor(float v)
{
    impl_->setFixupScaleFactor(v);
}

std::shared_ptr<Environment> osc::StoFileSimulation::impl_upd_associated_environment()
{
    return impl_->implUpdAssociatedEnvironment();
}
