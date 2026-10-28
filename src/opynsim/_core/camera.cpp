#include "camera.h"

#include <opynsim/_core/type_casters/vector.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <liboscar/graphics/camera.h>
#include <liboscar/maths/angle.h>
#include <liboscar/maths/vector.h>

#include <format>

namespace nb = nanobind;

void opyn::python::def_camera(nb::module_& m)
{
    nb::class_<osc::Camera> cls(
        m,
        "Camera",
        "Represents a virtual camera at `position` in world space, looking along\n"
        "a normalized `direction` vector. The `up` vector (also normalized)\n"
        "specifies which direction corresponds to the 'top' of the rendered image.\n"
    );
    cls.def_static(
        "look_at",
        &osc::Camera::look_at,
        nb::arg("position"),
        nb::arg("target"),
        nb::kw_only{},
        nb::arg("up") = osc::Vector3{0.0f, 1.0f, 0.0f},

        "Returns a ``Camera`` located at ``position`` and looking at ``target``,\n"
        "oriented such that ``up`` points toward the top of the camera's image plane.\n"
        "\n"
        "Args:\n"
        "    position: Position of the camera in world-space.\n"
        "    target: A point in world-space that the camera should look at.\n"
        "    up: Direction towards the top of the camera's image plane. Normalized by the \n"
        "        implementation. If it is parallel with ``target`` then the implementation \n"
        "        will select a fallback that is non-parallel.\n"
    );
    cls.def(
        nb::init<>{},
        "Default-constructs a camera with reasonable initial parameters."
    );
    cls.def(
        nb::init<const osc::Vector3&, const osc::Vector3&, const osc::Vector3&>{},
        nb::arg("position"),
        nb::arg("direction"),
        nb::kw_only{},
        nb::arg("up") = osc::Vector3{0.0f, 1.0f, 0.0f},
        "(Constructs a ``Camera`` located at ``position`` in world-space and pointing\n"
        "in a world-space ``direction``, oriented such that ``up`` points toward the\n"
        "top of the camera's image plane.\n"
        "\n"
        "Args:\n"
        "    position: Position of the camera in world-space.\n"
        "    direction: Direction the camera is pointing towards in world-space. Normalized by the implementation.\n"
        "    up: Direction towards the top of the camera's image plane. Normalized by the implementation. If\n"
        "        it is parallel with ``direction`` then the implementation will select a fallback that is non-parallel.\n"
    );
    cls.def_prop_rw(
        "position",
        &osc::Camera::position,
        &osc::Camera::set_position,
        "Get/set the position of the camera in world space."
    );
    cls.def_prop_rw(
        "direction",
        &osc::Camera::direction,
        &osc::Camera::set_direction,
        "Get/set the direction the camera is looking toward. Normalized by the implementation."
    );
    cls.def_prop_rw(
        "up",
        &osc::Camera::up,
        &osc::Camera::set_up,
        "Get/set the direction that corresponds to the 'top' of the rendered image. Normalized by the implementation."
    );
    cls.def_prop_rw(
        "vertical_field_of_view",
        [](const osc::Camera& camera) { return camera.vertical_field_of_view().count(); },
        [](osc::Camera& camera, double fov) { camera.set_vertical_field_of_view(osc::Radians{fov}); },
        "Get/set the vertical field-of-view angle of the camera in radians. Only applicable to perspective cameras."
    );
    cls.def("__repr__",  [](const osc::Camera& camera) { return std::format("{}", camera); });
}
