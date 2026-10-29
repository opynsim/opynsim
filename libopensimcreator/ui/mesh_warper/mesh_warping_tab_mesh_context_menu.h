#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_input_identifier.h>
#include <libopensimcreator/documents/mesh_warper/mw_undoable_actions.h>
#include <libopensimcreator/ui/mesh_warper/mesh_warping_tab_shared_state.h>
#include <libopensimcreator/ui/shared/basic_widgets.h>

#include <liboscar/maths/ray_collision.h>
#include <liboscar/ui/popups/popup.h>

#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace osc
{
    /// The context menu that's shown when a user right-clicks a landmark
    /// in the scene.
    class MeshWarpingTabMeshContextMenu final : public Popup {
    public:
        explicit MeshWarpingTabMeshContextMenu(
            Widget* parent,
            std::string_view label_,
            std::shared_ptr<MeshWarpingTabSharedState> shared_,
            MiDocumentInputIdentifier mesh_identifier_,
            RayCollision mesh_collision_) :

            Popup{parent, label_},
            m_State{std::move(shared_)},
            m_DocumentIdentifier{mesh_identifier_},
            m_MeshCollision{mesh_collision_}
        {
            set_modal(false);
        }
    private:
        void impl_draw_content() final
        {
            DrawContextMenuHeader("Input Mesh", "Mesh");
            DrawContextMenuSeparator();

            if (ui::draw_menu_item("Place landmark", "LeftClick")) {
                ActionAddLandmark(m_State->updUndoable(), m_DocumentIdentifier, m_MeshCollision.position);
            }
            if (ui::draw_menu_item("Place Non-Participating Landmark", "Ctrl+LeftClick", false, m_DocumentIdentifier == MiDocumentInputIdentifier::Source)) {
                ActionAddNonParticipatingLandmark(m_State->updUndoable(), m_MeshCollision.position);
            }
            if (ui::draw_menu_item("Move Selection Here", std::nullopt, false, m_State->hasSelection(m_DocumentIdentifier))) {
                ActionSetLandmarkLocations(m_State->updUndoable(), m_State->getSelected(m_DocumentIdentifier), m_MeshCollision.position);
            }
        }

        std::shared_ptr<MeshWarpingTabSharedState> m_State;
        MiDocumentInputIdentifier m_DocumentIdentifier;
        RayCollision m_MeshCollision;
    };
}
