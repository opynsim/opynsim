#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_element_id.h>

#include <unordered_set>
#include <utility>

namespace osc
{
    /// Represents the user's current selection.
    struct MwUiSelection final {

        size_t size() const { return selected_scene_elements_.size(); }
        auto begin() const { return selected_scene_elements_.begin(); }
        auto end() const { return selected_scene_elements_.end(); }

        void clear() { selected_scene_elements_.clear(); }
        void select(MwDocumentElementID el) { selected_scene_elements_.insert(std::move(el)); }
        bool contains(const MwDocumentElementID& el) const { return selected_scene_elements_.contains(el); }
    private:
        std::unordered_set<MwDocumentElementID> selected_scene_elements_;
    };
}
