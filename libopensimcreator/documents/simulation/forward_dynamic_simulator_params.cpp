#include "forward_dynamic_simulator_params.h"

#include <libopensimcreator/documents/simulation/simulation_clock.h>

#include <liboscar/utilities/c_string_view.h>

#include <chrono>
#include <optional>
#include <variant>

using namespace osc;

namespace
{
    constexpr CStringView c_FinalTimeTitle = "Final Time (sec)";
    constexpr CStringView c_FinalTimeDesc = "The final time, in seconds, that the forward dynamic simulation should integrate up to";
    constexpr CStringView c_IntegratorMethodUsedTitle = "Integrator Method";
    constexpr CStringView c_IntegratorMethodUsedDesc = "The integrator that the forward dynamic simulator should use. OpenSim's default integrator is a good choice if you aren't familiar with the other integrators. Changing the integrator can have a large impact on the performance and accuracy of the simulation.";
    constexpr CStringView c_ReportingIntervalTitle = "Reporting Interval (sec)";
    constexpr CStringView c_ReportingIntervalDesc = "How often the simulator should emit a simulation report. This affects how many datapoints are collected for the animation, output values, etc.";
    constexpr CStringView c_IntegratorStepLimitTitle = "Integrator Step Limit";
    constexpr CStringView c_IntegratorStepLimitDesc = "The maximum number of *internal* steps that can be taken within a single call to the integrator's stepTo/stepBy function. This is mostly an internal engine concern, but can occasionally affect how often reports are emitted";
    constexpr CStringView c_IntegratorMinimumStepSizeTitle = "Minimum Step Size (sec)";
    constexpr CStringView c_IntegratorMinimumStepSizeDesc = "The minimum step size, in seconds, that the integrator must take during the simulation. Note: this is mostly only relevant for error-corrected integrators that change their step size dynamically as the simulation runs.";
    constexpr CStringView c_IntegratorMaximumStepSizeTitle = "Maximum step size (sec)";
    constexpr CStringView c_IntegratorMaximumStepSizeDesc = "The maximum step size, in seconds, that the integrator must take during the simulation. Note: this is mostly only relevant for error-correct integrators that change their step size dynamically as the simulation runs";
    constexpr CStringView c_IntegratorAccuracyTitle = "Accuracy";
    constexpr CStringView c_IntegratorAccuracyDesc = "Target accuracy for the integrator. Mostly only relevant for error-controlled integrators that change their step size by comparing this accuracy value to measured integration error";
}


osc::ForwardDynamicSimulatorParams::ForwardDynamicSimulatorParams() :
    final_time{SimulationClock::start() + SimulationClock::duration{10.0}},
    integrator_method_used{opyn::IntegratorMethod::Default},
    reporting_interval{1.0/100.0},
    integrator_step_limit{20000},
    integrator_minimum_step_size{1.0e-8},
    integrator_maximum_step_size{1.0},
    integrator_accuracy{1.0e-5}
{}

ParamBlock osc::to_param_block(const ForwardDynamicSimulatorParams& p)
{
    ParamBlock rv;
    rv.push_param(c_FinalTimeTitle, c_FinalTimeDesc, (p.final_time - SimulationClock::start()).count());
    rv.push_param(c_IntegratorMethodUsedTitle, c_IntegratorMethodUsedDesc, p.integrator_method_used);
    rv.push_param(c_ReportingIntervalTitle, c_ReportingIntervalDesc, p.reporting_interval.count());
    rv.push_param(c_IntegratorStepLimitTitle, c_IntegratorStepLimitDesc, p.integrator_step_limit);
    rv.push_param(c_IntegratorMinimumStepSizeTitle, c_IntegratorMinimumStepSizeDesc, p.integrator_minimum_step_size.count());
    rv.push_param(c_IntegratorMaximumStepSizeTitle, c_IntegratorMaximumStepSizeDesc, p.integrator_maximum_step_size.count());
    rv.push_param(c_IntegratorAccuracyTitle, c_IntegratorAccuracyDesc, p.integrator_accuracy);
    return rv;
}

ForwardDynamicSimulatorParams osc::from_param_block(const ParamBlock& b)
{
    ForwardDynamicSimulatorParams rv;
    if (auto finalTime = b.find_value(c_FinalTimeTitle); finalTime && std::holds_alternative<double>(*finalTime))
    {
        rv.final_time = SimulationClock::start() + SimulationClock::duration{std::get<double>(*finalTime)};
    }
    if (auto integMethod = b.find_value(c_IntegratorMethodUsedTitle); integMethod && std::holds_alternative<opyn::IntegratorMethod>(*integMethod))
    {
        rv.integrator_method_used = std::get<opyn::IntegratorMethod>(*integMethod);
    }
    if (auto repInterv = b.find_value(c_ReportingIntervalTitle); repInterv && std::holds_alternative<double>(*repInterv))
    {
        rv.reporting_interval = SimulationClock::duration{std::get<double>(*repInterv)};
    }
    if (auto stepLim = b.find_value(c_IntegratorStepLimitTitle); stepLim && std::holds_alternative<int>(*stepLim))
    {
        rv.integrator_step_limit = std::get<int>(*stepLim);
    }
    if (auto minStep = b.find_value(c_IntegratorMinimumStepSizeTitle); minStep && std::holds_alternative<double>(*minStep))
    {
        rv.integrator_minimum_step_size = SimulationClock::duration{std::get<double>(*minStep)};
    }
    if (auto maxStep = b.find_value(c_IntegratorMaximumStepSizeTitle); maxStep && std::holds_alternative<double>(*maxStep))
    {
        rv.integrator_maximum_step_size = SimulationClock::duration{std::get<double>(*maxStep)};
    }
    if (auto acc = b.find_value(c_IntegratorAccuracyTitle); acc && std::holds_alternative<double>(*acc))
    {
        rv.integrator_accuracy = std::get<double>(*acc);
    }
    return rv;
}
