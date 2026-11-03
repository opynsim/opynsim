#include "mi_ground.h"

#include <libopensimcreator/documents/mesh_importer/mi_class.h>
#include <libopensimcreator/documents/mesh_importer/mi_ids.h>
#include <libopensimcreator/documents/mesh_importer/mi_strings.h>
#include <libopensimcreator/platform/msmicons.h>

#include <liboscar/utilities/c_string_view.h>
#include <liboscar/utilities/uid.h>

#include <iostream>

using osc::MiClass;
using osc::CStringView;
using osc::UID;

MiClass osc::MiGround::create_class()
{
    return
    {
        MiStrings::c_GroundLabel,
        MiStrings::c_GroundLabelPluralized,
        MiStrings::c_GroundLabelOptionallyPluralized,
        MSMICONS_DOT_CIRCLE,
        MiStrings::c_GroundDescription,
    };
}

UID osc::MiGround::impl_get_id() const
{
    return MiIDs::ground();
}

std::ostream& osc::MiGround::impl_write_to_stream(std::ostream& o) const
{
    return o << MiStrings::c_GroundLabel << "()";
}

CStringView osc::MiGround::impl_get_label() const
{
    return MiStrings::c_GroundLabel;
}
