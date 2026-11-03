#include "mi_joint.h"

#include <libopensimcreator/documents/mesh_importer/mi_crossref_descriptor.h>
#include <libopensimcreator/documents/mesh_importer/mi_crossref_direction.h>
#include <libopensimcreator/documents/mesh_importer/mi_strings.h>
#include <libopensimcreator/platform/msmicons.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/maths/transform.h>
#include <liboscar/utilities/c_string_view.h>
#include <liboscar/utilities/uid.h>

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

using osc::MiCrossrefDescriptor;
using osc::MiClass;

osc::MiJoint::MiJoint(
    UID id,
    std::string jointTypeName,
    const std::string& userAssignedName,  // can be empty
    UID parent,
    UID child,
    const Transform& xform) :

    id_{id},
    joint_type_name_{std::move(jointTypeName)},
    user_assigned_name_{opyn::sanitize_to_open_sim_component_name(userAssignedName)},
    parent_{parent},
    child_{child},
    xform_{xform}
{}

MiClass osc::MiJoint::create_class()
{
    return
    {
        MiStrings::c_JointLabel,
        MiStrings::c_JointLabelPluralized,
        MiStrings::c_JointLabelOptionallyPluralized,
        MSMICONS_LINK,
        MiStrings::c_JointDescription,
    };
}

std::vector<MiCrossrefDescriptor> osc::MiJoint::impl_get_cross_references() const
{
    return
    {
        {parent_, MiStrings::c_JointParentCrossrefName, MiCrossrefDirection::ToParent},
        {child_,  MiStrings::c_JointChildCrossrefName,  MiCrossrefDirection::ToChild },
    };
}

std::ostream& osc::MiJoint::impl_write_to_stream(std::ostream& o) const
{
    return o << "MiJoint(ID = " << id_
        << ", JointTypeName = " << joint_type_name_
        << ", UserAssignedName = " << user_assigned_name_
        << ", Parent = " << parent_
        << ", Child = " << child_
        << ", Transform = " << xform_
        << ')';
}

void osc::MiJoint::impl_set_label(std::string_view sv)
{
    user_assigned_name_ = opyn::sanitize_to_open_sim_component_name(sv);
}
