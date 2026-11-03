#include "mi_undoable_actions.h"

#include <libopensimcreator/documents/mesh_importer/mi_body.h>
#include <libopensimcreator/documents/mesh_importer/mi_document.h>
#include <libopensimcreator/documents/mesh_importer/mi_document_helpers.h>
#include <libopensimcreator/documents/mesh_importer/mi_joint.h>
#include <libopensimcreator/documents/mesh_importer/mi_ids.h>
#include <libopensimcreator/documents/mesh_importer/mi_object.h>
#include <libopensimcreator/documents/mesh_importer/mi_object_helpers.h>
#include <libopensimcreator/documents/mesh_importer/mi_mesh.h>
#include <libopensimcreator/documents/mesh_importer/mi_station.h>

#include <libopynsim/utilities/open_sim_helpers.h>
#include <liboscar/maths/angle.h>
#include <liboscar/maths/math_helpers.h>
#include <liboscar/maths/transform.h>
#include <liboscar/maths/vector.h>
#include <liboscar/utilities/uid.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_set>

using osc::UID;

bool osc::point_axis_towards(
    MiUndoableDocument& udoc,
    UID id,
    int axis,
    UID other)
{
    point_axis_towards(udoc.upd_scratch(), id, axis, other);
    udoc.commit_scratch("reoriented " + udoc.scratch().get_label_by_id(id));
    return true;
}

bool osc::try_assign_mesh_attachments(
    MiUndoableDocument& udoc,
    const std::unordered_set<UID>& meshIDs,
    UID newAttachment)
{
    MiDocument& doc = udoc.upd_scratch();

    if (newAttachment != MiIDs::ground() && !doc.contains<MiBody>(newAttachment))
    {
        return false;  // bogus ID passed
    }

    for (const UID id : meshIDs)
    {
        auto* const ptr = doc.try_upd_by_id<MiMesh>(id);
        if (!ptr)
        {
            continue;  // hardening: ignore invalid assignments
        }

        ptr->set_parent_id(newAttachment);
    }

    std::stringstream commitMsg;
    commitMsg << "assigned mesh";
    if (meshIDs.size() > 1)
    {
        commitMsg << "es";
    }
    commitMsg << " to " << doc.get_by_id(newAttachment).get_label();


    udoc.commit_scratch(std::move(commitMsg).str());

    return true;
}

bool osc::try_create_joint(
    MiUndoableDocument& udoc,
    UID childID,
    UID parentID)
{
    MiDocument& doc = udoc.upd_scratch();

    const Vector3 parentPos = doc.get_pos_by_id(parentID);
    const Vector3 childPos = doc.get_pos_by_id(childID);
    const Vector3 midPoint = midpoint(parentPos, childPos);

    const auto& joint = doc.emplace<MiJoint>(
        UID{},
        "WeldJoint",
        std::string{},
        parentID,
        childID,
        Transform{.translation = midPoint}
    );
    doc.select_only(joint);

    udoc.commit_scratch("added " + joint.get_label());

    return true;
}

bool osc::try_orient_object_axis_along_two_points(
    MiUndoableDocument& udoc,
    UID id,
    int axis,
    Vector3 p1,
    Vector3 p2)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const Vector3 direction = normalize(p2 - p1);
    const Transform t = obj->get_x_form(doc);

    obj->set_xform(doc, point_axis_along(t, axis, direction));
    udoc.commit_scratch("reoriented " + obj->get_label());

    return true;
}

bool osc::try_orient_object_axis_along_two_objects(
    MiUndoableDocument& udoc,
    UID id,
    int axis,
    UID obj1,
    UID obj2)
{
    return try_orient_object_axis_along_two_points(
        udoc,
        id,
        axis,
        udoc.scratch().get_pos_by_id(obj1),
        udoc.scratch().get_pos_by_id(obj2)
    );
}

bool osc::try_translate_object_between_two_points(
    MiUndoableDocument& udoc,
    UID id,
    const Vector3& a,
    const Vector3& b)
{
    MiDocument& doc = udoc.upd_scratch();
    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    obj->set_pos(doc, midpoint(a, b));
    udoc.commit_scratch("translated " + obj->get_label());

    return true;
}

bool osc::try_translate_between_two_objects(
    MiUndoableDocument& udoc,
    UID id,
    UID a,
    UID b)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const MiObject* const objA = doc.try_get_by_id(a);
    if (!objA)
    {
        return false;
    }

    const MiObject* const objB = doc.try_get_by_id(b);
    if (!objB)
    {
        return false;
    }

    obj->set_pos(doc, midpoint(objA->get_pos(doc), objB->get_pos(doc)));
    udoc.commit_scratch("translated " + obj->get_label());

    return true;
}

bool osc::try_translate_object_to_another_object(
    MiUndoableDocument& udoc,
    UID id,
    UID other)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const MiObject* const otherObj = doc.try_get_by_id(other);
    if (!otherObj)
    {
        return false;
    }

    obj->set_pos(doc, otherObj->get_pos(doc));
    udoc.commit_scratch("moved " + obj->get_label());

    return true;
}

bool osc::try_translate_to_mesh_average_center(
    MiUndoableDocument& udoc,
    UID id,
    UID meshID)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const auto* const mesh = doc.try_get_by_id<MiMesh>(meshID);
    if (!mesh)
    {
        return false;
    }

    obj->set_pos(doc, average_center(*mesh));
    udoc.commit_scratch("moved " + obj->get_label());

    return true;
}

bool osc::try_translate_to_mesh_bounds_center(
    MiUndoableDocument& udoc,
    UID id,
    UID meshID)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const auto* const mesh = doc.try_get_by_id<MiMesh>(meshID);
    if (!mesh)
    {
        return false;
    }

    const std::optional<AABB> bounds = mesh->calc_bounds();
    if (!bounds)
    {
        return false;
    }

    const Vector3 boundsMidpoint = centroid_of(*bounds);

    obj->set_pos(doc, boundsMidpoint);
    udoc.commit_scratch("moved " + obj->get_label());

    return true;
}

bool osc::try_translate_to_mesh_mass_center(
    MiUndoableDocument& udoc,
    UID id,
    UID meshID)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const auto* const mesh = doc.try_get_by_id<MiMesh>(meshID);
    if (!mesh)
    {
        return false;
    }

    obj->set_pos(doc, mass_center_of(*mesh));
    udoc.commit_scratch("moved " + obj->get_label());

    return true;
}

bool osc::try_reassign_crossref(
    MiUndoableDocument& udoc,
    UID id,
    int crossref,
    UID other)
{
    if (other == id)
    {
        return false;
    }

    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    if (!doc.contains(other))
    {
        return false;
    }

    obj->set_cross_reference_connectee_id(crossref, other);
    udoc.commit_scratch("reassigned " + obj->get_label() + " " + obj->get_cross_reference_label(crossref));

    return true;
}

bool osc::delete_selected(MiUndoableDocument& udoc)
{
    MiDocument& doc = udoc.upd_scratch();
    if (!doc.has_selection())
    {
        return false;
    }
    doc.delete_selected();
    udoc.commit_scratch("deleted selection");
    return true;
}

bool osc::delete_object(MiUndoableDocument& udoc, UID id)
{
    MiDocument& doc = udoc.upd_scratch();

    const MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const std::string label = to_string(obj->get_label());

    if (!doc.delete_by_id(obj->get_id()))
    {
        return false;
    }

    udoc.commit_scratch("deleted " + label);
    return true;
}

void osc::rotate_axis(
    MiUndoableDocument& udoc,
    MiObject& el,
    int axis,
    Radians radians)
{
    const MiDocument& doc = udoc.upd_scratch();
    el.set_xform(doc, rotate_axis(el.get_x_form(doc), axis, radians));
    udoc.commit_scratch("reoriented " + el.get_label());
}

bool osc::try_copy_orientation(
    MiUndoableDocument& udoc,
    UID id,
    UID other)
{
    MiDocument& doc = udoc.upd_scratch();

    MiObject* const obj = doc.try_upd_by_id(id);
    if (!obj)
    {
        return false;
    }

    const MiObject* const otherObj = doc.try_get_by_id(other);
    if (!otherObj)
    {
        return false;
    }

    obj->set_rotation(doc, otherObj->rotation(doc));
    udoc.commit_scratch("reoriented " + obj->get_label());

    return true;
}


UID osc::add_body(
    MiUndoableDocument& udoc,
    const Vector3& position,
    UID andTryAttach)
{
    MiDocument& doc = udoc.upd_scratch();

    const auto& b = doc.emplace<MiBody>(UID{}, MiBody::get_class().generate_name(), Transform{.translation = position});
    doc.de_select_all();
    doc.select(b.get_id());

    auto* const obj = doc.try_upd_by_id<MiMesh>(andTryAttach);
    if (obj)
    {
        if (obj->get_parent_id() == MiIDs::ground() || obj->get_parent_id() == MiIDs::empty())
        {
            obj->set_parent_id(b.get_id());
            doc.select(*obj);
        }
    }

    udoc.commit_scratch(std::string{"added "} + b.get_label());

    return b.get_id();
}

UID osc::add_body(MiUndoableDocument& udoc)
{
    return add_body(udoc, {}, MiIDs::empty());
}

bool osc::add_station_at_location(
    MiUndoableDocument& udoc,
    const MiObject& obj,
    const Vector3& loc)
{
    MiDocument& doc = udoc.upd_scratch();

    if (!can_attach_station_to(obj))
    {
        return false;
    }

    const auto& station = doc.emplace<MiStation>(
        UID{},
        get_station_attachment_parent(doc, obj),
        loc,
        MiStation::get_class().generate_name()
    );
    doc.select_only(station);
    udoc.commit_scratch("added station " + station.get_label());
    return true;
}

bool osc::add_station_at_location(
    MiUndoableDocument& udoc,
    UID attachment,
    const Vector3& loc)
{
    const MiDocument& doc = udoc.upd_scratch();

    const auto* const obj = doc.try_get_by_id(attachment);
    if (!obj)
    {
        return false;
    }

    return add_station_at_location(udoc, *obj, loc);
}

void osc::action_import_landmarks(
    MiUndoableDocument& udoc,
    std::span<const opyn::NamedLandmark> landmarks,
    std::optional<std::string> maybeName)
{
    MiDocument& doc = udoc.upd_scratch();
    for (const auto& lm : landmarks)
    {
        doc.emplace<MiStation>(
            UID{},
            MiIDs::ground(),
            lm.position,
            lm.name
        );
    }

    std::stringstream ss;
    ss << "imported " << std::move(maybeName).value_or("landmarks");
    udoc.commit_scratch(std::move(ss).str());
}
