#include "mw_document.h"

#include <liboscar/graphics/geometries/cylinder_geometry.h>
#include <liboscar/graphics/geometries/sphere_geometry.h>

osc::MwDocument::MwDocument() :
    source_mesh{SphereGeometry{{.num_width_segments = 16, .num_height_segments = 16}}.mesh()},
    destination_mesh{CylinderGeometry{{.height = 2.0f, .num_radial_segments = 16}}.mesh()},
    blending_factor{1.0f},
    recalculate_normals{false},
    source_landmarks_prescale{1.0f},
    destination_landmarks_prescale{1.0f},
    warping_penalty{},

    // note: These should ideally match the model warper's defaults (#1122).
    apply_affine_translation{false},
    apply_affine_scale{true},
    apply_affine_rotation{false},
    apply_non_affine_warp{true}
{}
