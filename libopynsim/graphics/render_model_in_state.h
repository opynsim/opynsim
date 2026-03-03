#pragma once

#include <liboscar/graphics/texture2d.h>

namespace opyn { class Model; }
namespace opyn { class ModelState; }

namespace opyn
{
    osc::Texture2D render_model_in_state(const Model&, const ModelState&);
}
