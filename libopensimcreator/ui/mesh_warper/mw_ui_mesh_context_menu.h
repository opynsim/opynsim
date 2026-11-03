#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_input_identifier.h>
#include <libopensimcreator/documents/mesh_warper/mw_undoable_actions.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_shared_state.h>
#include <libopensimcreator/ui/shared/basic_widgets.h>

#include <liboscar/maths/ray_collision.h>
#include <liboscar/platform/modifier_key.h>
#include <liboscar/platform/mouse_combination.h>
#include <liboscar/platform/mouse_button.h>
#include <liboscar/ui/popups/popup.h>

#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace osc
{
    /// The context menu that's shown when a user right-clicks a landmark
    /// in the scene.
    class MwUiMeshContextMenu final : public Popup {
    public:
        explicit MwUiMeshContextMenu(
            Widget* parent,
            std::string_view label_,
            std::shared_ptr<MwUiSharedState> shared_,
            MiDocumentInputIdentifier mesh_identifier_,
            RayCollision mesh_collision_) :

            Popup{parent, label_},
            state_{std::move(shared_)},
            document_identifier_{mesh_identifier_},
            mesh_collision_{mesh_collision_}
        {
            set_modal(false);
        }
    private:
        void impl_draw_content() final
        {
            DrawContextMenuHeader("Input Mesh", "Mesh");
            DrawContextMenuSeparator();

            if (ui::draw_menu_item("Place landmark", MouseButton::Left)) {
                ActionAddLandmark(state_->upd_undoable(), document_identifier_, mesh_collision_.position);
            }
            if (ui::draw_menu_item("Place Non-Participating Landmark",  ModifierKey::Ctrl | MouseButton::Left, false, document_identifier_ == MiDocumentInputIdentifier::Source)) {
                ActionAddNonParticipatingLandmark(state_->upd_undoable(), mesh_collision_.position);
            }
            if (ui::draw_menu_item("Move Selection Here", std::nullopt, false, state_->has_selection(document_identifier_))) {
                ActionSetLandmarkLocations(state_->upd_undoable(), state_->get_selected(document_identifier_), mesh_collision_.position);
            }
        }

        std::shared_ptr<MwUiSharedState> state_;
        MiDocumentInputIdentifier document_identifier_;
        RayCollision mesh_collision_;
    };
}
