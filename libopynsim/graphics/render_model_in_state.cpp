#include "render_model_in_state.h"

#include <libopynsim/graphics/open_sim_decoration_options.h>
#include <libopynsim/graphics/open_sim_decoration_generator.h>
#include <libopynsim/model.h>
#include <libopynsim/model_state.h>

#include <liboscar/graphics/scene/scene_cache.h>
#include <liboscar/graphics/scene/scene_renderer.h>
#include <liboscar/graphics/scene/scene_renderer_params.h>
#include <liboscar/maths/polar_perspective_camera.h>
#include <liboscar/platform/app.h>

#include "liboscar/graphics/graphics.h"

osc::Texture2D opyn::render_model_in_state(const Model& model, const ModelState& model_state)
{
    // Initialize application state
    osc::App app;

    // Generate 3D scene
    osc::SceneCache scene_cache;
    const OpenSimDecorationOptions decoration_options;
    const std::vector<osc::SceneDecoration> decorations = GenerateModelDecorations(
        scene_cache,
        model.opensim_model(),
        model_state.simbody_state(),
        decoration_options
    );

    // Render scene to `RenderTexture` (GPU)
    osc::PolarPerspectiveCamera camera;
    osc::SceneRenderer scene_renderer{scene_cache};
    const osc::Vector2 dimensions = {800.0f, 600.0f};
    osc::SceneRendererParams scene_renderer_params = {
        .dimensions = dimensions,
        .anti_aliasing_level = osc::AntiAliasingLevel{4},
        .view_matrix = camera.view_matrix(),
        .projection_matrix = camera.projection_matrix(osc::aspect_ratio_of(dimensions)),
    };
    scene_renderer.render(decorations, scene_renderer_params);
    const osc::RenderTexture& rendered_scene = scene_renderer.upd_render_texture();

    // Blit `RenderTexture` to `Texture2D` (CPU accessible, for Python)
    osc::Texture2D rv{dimensions};
    osc::graphics::copy_texture(rendered_scene, rv);
    return rv;
}
