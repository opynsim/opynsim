#pragma once

#include <libopensimcreator/documents/mesh_warper/mw_document_landmark_pair.h>
#include <libopensimcreator/documents/mesh_warper/mw_document_non_participating_landmark.h>

#include <liboscar/graphics/mesh.h>

#include <vector>

namespace osc
{
    /// Represents the document that the mesh warper UI edits in-place.
    struct MwDocument final {
        MwDocument();

        Mesh source_mesh;
        Mesh destination_mesh;
        std::vector<MwDocumentLandmarkPair> landmark_pairs;
        std::vector<MwDocumentNonParticipatingLandmark> non_participating_landmarks;
        float blending_factor;
        bool recalculate_normals;
        float source_landmarks_prescale;
        float destination_landmarks_prescale;
        float warping_penalty;
        bool apply_affine_translation;
        bool apply_affine_scale;
        bool apply_affine_rotation;
        bool apply_non_affine_warp;
    };
}
