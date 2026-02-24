Quickstart
==========

Import ``opynsim``
------------------

After :doc:`installing OPynSim <installation>`, it may be imported into Python
code like this:

.. code:: python

    import opynsim


Set up Global Environment
-------------------------

.. note::

    This step is **optional**, but makes OPynSim behave similarly to OpenSim, which
    can be important when (e.g.) loading ``.osim`` files that implicitly rely on
    a global geometry fallback.

The ``opynsim`` API exposes functions that globally affect its logging and mesh loading
behavior.

If you are working with OpenSim model files, this can be important because
OpenSim has a noisier logging default and searches through a global geometry fallback
when it encounters ``osim`` files that reference mesh files that aren't available at
``model.osim/../Geometry``.

.. code:: python

    import opynsim
    from pathlib import Path
    import logging

    # Make OPynSim log much more information to the standard output, which is how
    # OpenSim behaves by default. This can be necessary for debugging silent issues
    # in OpenSim (e.g. mesh file not found, muscle exceeds pennation angle, etc.).
    opynsim.set_logging_level(logging.DEBUG)

    # Provide OpenSim with a global geometry fallback directory path that it will
    # use to resolve mesh files that cannot be found next to the model osim
    opynsim.add_geometry_directory(Path("/path/to/geometry/"))

Global initialization only needs to be performed once per Python process.

Import an ``osim`` File
-----------------------

:class:`opynsim.ModelSpecification` is a central part of the ``opynsim``
API. It's a high-level representation that Python code can manipulate before
calling ``opynsim.compile`` to validate, assemble, and yield a readonly physics
:class:`opynsim.Model` and associated :class:`opynsim.ModelState`\s.

:func:`opynsim.import_osim_file` imports an ``.osim`` file on the caller's
filesystem, which can then be used with the rest of the API:

.. code:: python

    import opynsim
    import opynsim.ui
    from pathlib import Path

    # Import an `.osim` file as an `opynsim.ModelSpecification`
    model_specification = opynsim.import_osim_file("arm26.osim")

    # `pathlib.Path`s are also supported
    model_specification2 = opynsim.import_osim_file(Path("/some/path/to/arm26.osim"))

    # (example `ModelSpecification` usage)
    model = opynsim.compile(model_specification)
    state = model.initial_state()
    opynsim.ui.visualize_model_in_state(model, state)
