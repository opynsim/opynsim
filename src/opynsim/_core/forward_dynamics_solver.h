#pragma once

namespace nanobind { class module_; }

namespace opyn::python
{
    void def_forward_dynamics_solver(nanobind::module_&);
}
