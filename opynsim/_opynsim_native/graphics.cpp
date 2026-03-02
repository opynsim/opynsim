#include "graphics.h"

#include <libopynsim/graphics/render_model_in_state.h>
#include <libopynsim/model.h>
#include <libopynsim/model_state.h>
#include <nanobind/nanobind.h>

namespace nb = nanobind;

void opyn::init_graphics_submodule(nanobind::module_& graphics_module)
{
    graphics_module.def(
        "render_model_in_state",
        render_model_in_state,
        nb::arg("model"),
        nb::arg("state"),
        R"(
            Renders the given :class:`opynsim.Model` + :class:`opynsim.ModelState` to
            a raster image.
        )"
    );
}
