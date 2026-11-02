#pragma once

#include <libopensimcreator/documents/mesh_importer/mi_ids.h>

#include <liboscar/maths/vector.h>
#include <liboscar/utilities/uid.h>

namespace osc
{
    class MeshImporterHover final {
    public:
        MeshImporterHover() :
            id{MiIDs::Empty()},
            pos{}
        {
        }

        MeshImporterHover(UID id_, Vector3 pos_) :
            id{id_},
            pos{pos_}
        {
        }

        explicit operator bool () const
        {
            return id != MiIDs::Empty();
        }

        void reset()
        {
            *this = MeshImporterHover{};
        }

        UID id;
        Vector3 pos;
    };
}
