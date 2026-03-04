import opynsim

spec = opynsim.example_specification_double_pendulum()
model = opynsim.compile_specification(spec)
state = model.initial_state()
model.realize(state, opynsim.ModelStateStage.REPORT)

import opynsim.ui

opynsim.ui.show_model_in_state(model, state)
