#include "mi_object.h"

#include <liboscar/maths/math_helpers.h>

#include <algorithm>

namespace rgs = std::ranges;

void osc::MiObject::apply_rotation(
    const MiObjectFinder& lookup,
    const EulerAngles& eulerAngles,
    const Vector3& rotationCenter)
{
    Transform t = get_x_form(lookup);
    apply_world_space_rotation(t, eulerAngles, rotationCenter);
    set_xform(lookup, t);
}

bool osc::MiObject::is_cross_referencing(
    UID id,
    MiCrossrefDirection direction) const
{
    return rgs::any_of(impl_get_cross_references(), [id, direction](const MiCrossrefDescriptor& desc)
    {
        return desc.get_connectee_id() == id and (desc.get_direction() & direction);
    });
}
