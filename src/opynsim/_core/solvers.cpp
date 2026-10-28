#include "solvers.h"

#include <opynsim/_core/forward_dynamics_solver.h>
#include <opynsim/_core/model_warper.h>
#include <opynsim/_core/integrator_settings.h>

void opyn::init_solvers_submodule(nanobind::module_& solvers_module)
{
    python::def_model_warper(solvers_module);
    python::def_integrator_settings(solvers_module);
    python::def_forward_dynamics_solver(solvers_module);
}
