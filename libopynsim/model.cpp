#include "model.h"

#include <libopynsim/model_state.h>
#include <OpenSim/Simulation/Model/Model.h>

#include <liboscar/utils/copy_on_upd_ptr.h>

class opyn::Model::Impl final {
public:
    explicit Impl(const OpenSim::Model& model) :
        model_{model}
    {
        // This is effectively what converting a "model specification" to
        // a "model" is, in `opynsim`'s world.
        model_.buildSystem();

        // This is a quirk of OpenSim, because it mixes the state and system
        // into one class, but it must be done here because `initial_state()`
        // is `const` in `opynsim`'s design.
        model_.initializeState();
    }

    ModelState initial_state() const
    {
        // Copy the working state out of the model, so that the caller gets
        // an independent state.
        return ModelState{SimTK::State{model_.getWorkingState()}};
    }
private:
    OpenSim::Model model_;
};

opyn::Model::Model(const OpenSim::Model& model) : impl_{osc::make_cow<Impl>(model)} {}
opyn::ModelState opyn::Model::initial_state() const { return impl_->initial_state(); }
