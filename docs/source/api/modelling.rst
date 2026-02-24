Modelling
=========

At a high-level, OPynSim's modelling API is designed around three classes with
distinct roles:

- :class:`opynsim.ModelSpecification`: A high-level specification of the
  model. This is what Python code manipulates before calling :func:`opynsim.compile`
  to yield...
- :class:`opynsim.Model`: A compiled readonly physics model, which can
  create/read/manipulate...
- :class:`opynsim.ModelState`: A single state of an :class:`opynsim.Model`. This
  can be manipulated with Python code to do something useful with the model.

Here's how the design interplays with common use-cases:

- **Output Extraction**: :class:`opynsim.ModelState`\s can be imported from lab data.
  An :class:`opynsim.Model` can then produce output values---for example, body ``center_of_mass``---from
  the state with :meth:`opynsim.Model.output_value`.


.. automodule:: opynsim
   :members:
   :imported-members:
   :undoc-members:
   :show-inheritance:


.. code:: python

    import opynsim
    from pathlib import Path
    import logging

    ########################################################################
    # Initialization
    ########################################################################

    # Globally set `opynsim`'s log using a Python `logging.LEVEL`
    opynsim.set_logging_level(logging.DEBUG)

    # Globally set where `opynsim` should look for mesh files that cannot
    # be found next to a model file.
    #
    # `opynsim` supports both `pathlib.Path` and `str` for paths.
    opynsim.add_geometry_directory(Path("/path/to/geometry/"))


    ########################################################################
    # Modelling
    ########################################################################

    # A `ModelSpecification` specifies how OPynSim should build the model.
    model_specification = opynsim.import_osim_file("/path/to/model.osim")

    # A `ModelSpecification` can be compiled into a `Model`. Compilation
    # validates the specification and assembles a physics system from it.
    #
    # Callers can use a `Model` to ask questions about the physics system
    # (What inputs/outputs does it have? What coordinates does it have?) and
    # produce/edit `ModelState`s.
    model = model_specification.compile()

    # A `ModelState` is one state of a `Model`. All `Model`s can produce an
    # initial state.
    #
    # Callers can manipulate `ModelState`s directly (e.g. manipulate state
    # vectors "in the raw"), or in tandem with a `Model`.
    state = model.initial_state()

    # The physics system of a `Model` can be used to realize a `ModelState`
    # to a later computational stage (e.g. compute accelerations from
    # dynamics).
    #
    # Different systems require `ModelState`s at different stages. E.g. a
    # visualizer usually wants everything, so `REPORT` (the final stage)
    # is the safest bet.
    model.realize(state, opynsim.ModelStateStage.REPORT)