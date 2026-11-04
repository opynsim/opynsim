#pragma once

#include <liboscar/ui/panels/panel.h>

#include <functional>
#include <memory>
#include <string_view>

namespace OpenSim { class Component; }
namespace OpenSim { class ComponentPath; }
namespace opyn { class ModelStatePair; }

namespace osc
{
    class NavigatorPanel final : public Panel {
    public:
        explicit NavigatorPanel(
            Widget* parent,
            std::string_view panel_name,
            std::shared_ptr<opyn::ModelStatePair>,
            std::function<void(const OpenSim::ComponentPath&)> on_right_click = [](const auto&){}
        );

    private:
        void impl_draw_content() final;

        class Impl;
        OSC_WIDGET_DATA_GETTERS(Impl);
    };
}
