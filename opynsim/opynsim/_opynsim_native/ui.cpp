#include "ui.h"

#include <libopynsim/ui/hello_ui.h>
#include <nanobind/nanobind.h>

void opyn::init_ui_submodule(nanobind::module_& ui_module)
{
    ui_module.def("hello_ui", show_hello_ui);
}
