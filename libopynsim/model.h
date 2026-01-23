#pragma once

#include <libopynsim/model_state.h>

#include <liboscar/utils/copy_on_upd_ptr.h>

namespace OpenSim { class Model; }

namespace opyn
{
    class Model final {
    private:
        friend class ModelSpecification;
        explicit Model(const OpenSim::Model&);

    public:
        ModelState initial_state() const;

    private:
        class Impl;
        osc::CopyOnUpdPtr<Impl> impl_;
    };
}
