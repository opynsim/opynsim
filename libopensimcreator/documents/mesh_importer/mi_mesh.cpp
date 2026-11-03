#include "mi_mesh.h"

#include <libopensimcreator/documents/mesh_importer/mi_crossref_descriptor.h>
#include <libopensimcreator/documents/mesh_importer/mi_crossref_direction.h>
#include <libopensimcreator/documents/mesh_importer/mi_class.h>
#include <libopensimcreator/documents/mesh_importer/mi_strings.h>
#include <libopensimcreator/platform/msmicons.h>

#include <libopynsim/graphics/simbody_mesh_loader.h>
#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/maths/aabb.h>
#include <liboscar/maths/aabb_functions.h>
#include <liboscar/utilities/uid.h>

#include <filesystem>
#include <ostream>
#include <utility>
#include <vector>

using osc::MiCrossrefDescriptor;
using osc::MiClass;
using osc::AABB;

osc::MiMesh::MiMesh(
    UID id,
    UID attachment,
    osc::Mesh meshData,
    std::filesystem::path path) :

    id_{id},
    attachment_{attachment},
    mesh_data_{std::move(meshData)},
    path_{std::move(path)},
    name_{opyn::sanitize_to_open_sim_component_name(path_.filename().replace_extension().string())}
{}

std::optional<AABB> osc::MiMesh::calc_bounds() const
{
    return mesh_data_.bounds().transform([this](const AABB& localBounds)
    {
        return transform_aabb(transform_, localBounds);
    });
}

void osc::MiMesh::reload_mesh_data_from_disk()
{
    mesh_data_ = opyn::load_mesh_via_simbody(get_path());
}

MiClass osc::MiMesh::create_class()
{
    return
    {
        MiStrings::c_MeshLabel,
        MiStrings::c_MeshLabelPluralized,
        MiStrings::c_MeshLabelOptionallyPluralized,
        MSMICONS_CUBE,
        MiStrings::c_MeshDescription,
    };
}

std::vector<MiCrossrefDescriptor> osc::MiMesh::impl_get_cross_references() const
{
    return
    {
        {attachment_, MiStrings::c_MeshAttachmentCrossrefName, MiCrossrefDirection::ToParent},
    };
}

std::ostream& osc::MiMesh::impl_write_to_stream(std::ostream& o) const
{
    return o << "Mesh("
        << "ID = " << id_
        << ", Attachment = " << attachment_
        << ", m_Transform = " << transform_
        << ", MeshData = " << &mesh_data_
        << ", Path = " << path_
        << ", Name = " << name_
        << ')';
}

void osc::MiMesh::impl_set_label(std::string_view sv)
{
    name_ = opyn::sanitize_to_open_sim_component_name(sv);
}
