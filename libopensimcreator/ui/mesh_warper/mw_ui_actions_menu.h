#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_helpers.h>
#include <libopensimcreator/documents/mesh_warper/mw_undoable_actions.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_shared_state.h>
#include <libopensimcreator/platform/msmicons.h>

#include <liboscar/ui/oscimgui.h>

#include <memory>
#include <utility>

namespace osc
{
    // widget: the 'edit' menu (a sub menu of the main menu)
    class MwUiActionsMenu final {
    public:
        explicit MwUiActionsMenu(
            std::shared_ptr<MwUiSharedState> tabState_) :
            state_{std::move(tabState_)}
        {
        }

        void on_draw()
        {
            if (ui::begin_menu("Actions"))
            {
                draw_content();
                ui::end_menu();
            }
        }

    private:

        void draw_content()
        {
            draw_clear_landmarks_menu_item();
            draw_clear_non_participating_landmarks_menu_item();
        }

        void draw_clear_landmarks_menu_item()
        {
            const bool hasLandmarks = ContainsLandmarks(state_->get_scratch());

            if (not hasLandmarks) {
                ui::begin_disabled();
            }
            if (ui::draw_menu_item(MSMICONS_ERASER " clear landmarks")) {
                ActionClearAllLandmarks(state_->upd_undoable());
            }
            if (not hasLandmarks) {
                ui::end_disabled();
            }
        }

        void draw_clear_non_participating_landmarks_menu_item()
        {
            const bool hasNonParticipatingLandmarks =
                ContainsNonParticipatingLandmarks(state_->get_scratch());

            if (not hasNonParticipatingLandmarks) {
                ui::begin_disabled();
            }
            if (ui::draw_menu_item(MSMICONS_ERASER " clear non-participating landmarks")) {
                ActionClearAllNonParticipatingLandmarks(state_->upd_undoable());
            }
            if (not hasNonParticipatingLandmarks) {
                ui::end_disabled();
            }
        }

        std::shared_ptr<MwUiSharedState> state_;
    };
}
