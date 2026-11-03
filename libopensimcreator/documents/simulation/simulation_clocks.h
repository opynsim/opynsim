#pragma once

#include <libopensimcreator/documents/simulation/simulation_clock.h>

#include <liboscar/maths/closed_interval.h>
#include <liboscar/maths/normalized.h>
#include <liboscar/utilities/chrono_helpers.h>

namespace osc
{
    // clocks/progress associated with a single simulation
    class SimulationClocks final {
    public:
        constexpr SimulationClocks() = default;

        constexpr SimulationClocks(
            SimulationClock::time_point singlePoint_,
            bool completed = true) :

            SimulationClocks{{singlePoint_, singlePoint_}, completed ? 1.0f : 0.0f}
        {}

        constexpr SimulationClocks(
            ClosedInterval<SimulationClock::time_point> timeRange_,
            Normalized<float> progress_ = 1.0f) :

            time_range_{timeRange_},
            progress_{progress_}
        {}

        constexpr SimulationClocks(
            ClosedInterval<SimulationClock::time_point> timeRange_,
            SimulationClock::time_point current_) :

            time_range_{timeRange_},
            progress_{static_cast<float>((current_ - time_range_.lower)/(time_range_.upper - time_range_.lower))}
        {
            // TODO: provide invlerp (inverse LERP), rather than calculating the above
        }

        constexpr SimulationClock::time_point start() const { return time_range_.lower; }
        constexpr SimulationClock::time_point current() const
        {
            return lerp(start(), end(), progress_.get());
        }
        constexpr SimulationClock::time_point end() const { return time_range_.upper; }
        constexpr float progress() const { return progress_; }
    private:
        ClosedInterval<SimulationClock::time_point> time_range_{SimulationClock::start(), SimulationClock::start()};
        Normalized<float> progress_ = 1.0f;
    };
}
