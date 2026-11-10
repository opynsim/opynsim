#pragma once

#include <liboscar/ui/tabs/tab.h>

namespace osc
{
    class Widget;

    /// Represents the "mesh warping workflow" UI tab.
    class MwUiTab final : public Tab {
    public:
        static CStringView id();

        explicit MwUiTab(Widget*);

    private:
        void impl_on_mount() final;
        void impl_on_unmount() final;
        bool impl_on_event(Event&) final;
        void impl_on_tick() final;
        void impl_on_draw_main_menu() final;
        void impl_on_draw() final;

        class Impl;
        OSC_WIDGET_DATA_GETTERS(Impl);
    };
}
