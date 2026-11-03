#pragma once

#include <libopensimcreator/ui/mesh_warper/mw_ui_actions_menu.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_edit_menu.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_file_menu.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_shared_state.h>
#include <libopensimcreator/ui/shared/main_menu.h>

#include <liboscar/platform/widget.h>
#include <liboscar/ui/panels/panel_manager.h>
#include <liboscar/ui/widgets/window_menu.h>

#include <memory>

namespace osc
{
    // widget: the main menu (contains multiple submenus: 'file', 'edit', 'about', etc.)
    class MwUiMainMenu final : public Widget {
    public:
        explicit MwUiMainMenu(
            Widget* parent,
            const std::shared_ptr<MwUiSharedState>& tabState_,
            const std::shared_ptr<PanelManager>& panelManager_) :

            Widget{parent},
            file_menu_{tabState_},
            edit_menu_{tabState_},
            actions_menu_{tabState_},
            window_menu_{this, panelManager_}
        {}

        void on_draw()
        {
            file_menu_.on_draw();
            edit_menu_.on_draw();
            actions_menu_.on_draw();
            window_menu_.on_draw();
            about_tab_.onDraw();
        }
    private:
        MwUiFileMenu file_menu_;
        MwUiEditMenu edit_menu_;
        MwUiActionsMenu actions_menu_;
        WindowMenu window_menu_;
        MainMenuAboutTab about_tab_;
    };
}
