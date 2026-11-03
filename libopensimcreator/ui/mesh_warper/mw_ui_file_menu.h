#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_input_identifier.h>
#include <libopensimcreator/documents/mesh_warper/mw_undoable_actions.h>
#include <libopensimcreator/platform/msmicons.h>
#include <libopensimcreator/ui/mesh_warper/mw_ui_shared_state.h>

#include <libopynsim/documents/landmarks/landmark_csv_flags.h>
#include <liboscar/platform/app.h>
#include <liboscar/ui/oscimgui.h>

#include <memory>
#include <utility>

namespace osc
{
    // widget: the 'file' menu (a sub menu of the main menu)
    class MwUiFileMenu final {
    public:
        explicit MwUiFileMenu(
            std::shared_ptr<MwUiSharedState> tabState_) :
            state_{std::move(tabState_)}
        {}

        void on_draw()
        {
            if (ui::begin_menu("File"))
            {
                draw_content();
                ui::end_menu();
            }
        }
    private:
        void draw_content()
        {
            if (ui::draw_menu_item(MSMICONS_FILE " New", ModifierKey::Ctrl | Key::N))
            {
                ActionCreateNewDocument(state_->upd_undoable());
            }

            if (ui::begin_menu(MSMICONS_FILE_IMPORT " Import"))
            {
                draw_import_menu_content();
                ui::end_menu();
            }

            if (ui::begin_menu(MSMICONS_FILE_EXPORT " Export"))
            {
                draw_export_menu_content();
                ui::end_menu();
            }

            if (ui::draw_menu_item(MSMICONS_TIMES " Close", ModifierKey::Ctrl | Key::W))
            {
                state_->close_tab();
            }

            if (ui::draw_menu_item(MSMICONS_TIMES_CIRCLE " Quit", ModifierKey::Ctrl | Key::Q))
            {
                App::upd().request_quit();
            }
        }

        void draw_import_menu_content()
        {
            if (ui::draw_menu_item("Source Mesh"))
            {
                ActionPromptUserToLoadMeshFile(state_->get_undoable_shared_ptr(), MiDocumentInputIdentifier::Source);
            }
            if (ui::draw_menu_item("Destination Mesh"))
            {
                ActionPromptUserToLoadMeshFile(state_->get_undoable_shared_ptr(), MiDocumentInputIdentifier::Destination);
            }
            if (ui::draw_menu_item("Source Landmarks from CSV"))
            {
                ActionPromptUserToLoadLandmarksFromCSV(state_->get_undoable_shared_ptr(), MiDocumentInputIdentifier::Source);
            }
            if (ui::draw_menu_item("Destination Landmarks from CSV"))
            {
                ActionPromptUserToLoadLandmarksFromCSV(state_->get_undoable_shared_ptr(), MiDocumentInputIdentifier::Destination);
            }
            if (ui::draw_menu_item("Non-Participating Landmarks from CSV"))
            {
                ActionPromptUserToLoadNonParticipatingLandmarksFromCSV(state_->get_undoable_shared_ptr());
            }
        }

        void draw_export_menu_content()
        {
            if (ui::draw_menu_item("Source Landmarks to CSV"))
            {
                ActionPromptUserToSaveLandmarksToCSV(state_->get_scratch(), MiDocumentInputIdentifier::Source);
            }
            if (ui::draw_menu_item("Destination Landmarks to CSV"))
            {
                ActionPromptUserToSaveLandmarksToCSV(state_->get_scratch(), MiDocumentInputIdentifier::Destination);
            }
            if (ui::draw_menu_item("Landmark Pairs to CSV"))
            {
                ActionPromptUserToSavePairedLandmarksToCSV(state_->get_scratch());
            }
            if (ui::draw_menu_item("Landmark Pairs to CSV (no names)"))
            {
                ActionPromptUserToSavePairedLandmarksToCSV(state_->get_scratch(), opyn::LandmarkCSVFlags::NoNames);
            }
            if (ui::draw_menu_item("Non-Participating Landmarks to CSV"))
            {
                ActionPromptUserToSaveNonParticipatingLandmarksToCSV(state_->get_scratch());
            }
        }

        std::shared_ptr<MwUiSharedState> state_;
    };
}
