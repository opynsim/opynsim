#include <_opynsim_native/tps3d.h>
#include <_opynsim_native/ui.h>

#include <libopynsim/opynsim.h>
#include <nanobind/nanobind.h>

using namespace opyn;

NB_MODULE(_opynsim_native, _opynsim_native_module)  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables,misc-use-anonymous-namespace)
{
    // Libraries should be quiet by default
    opyn::set_log_level(osc::LogLevel::err);

    // Globally initialize the opynsim API (Simbody, OpenSim, oscar)
    opyn::init();

    // Initialize `tps3d` submodule.
    {
        auto tps3d_submodule = _opynsim_native_module.def_submodule("tps3d");
        init_tps3d_submodule(tps3d_submodule);
    }

    // Initialize `ui` submodule.
    {
        auto ui_submodule = _opynsim_native_module.def_submodule("ui");
        init_ui_submodule(ui_submodule);
    }
}
