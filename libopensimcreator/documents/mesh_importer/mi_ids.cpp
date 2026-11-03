#include "mi_ids.h"

using osc::MiIDs;

const MiIDs::AllIDs& osc::MiIDs::get_all_ids()
{
    static const AllIDs s_AllIDs;
    return s_AllIDs;
}
