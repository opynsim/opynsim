Building From Source
====================

Overview
--------

Building OPynSim typically has the following high-level requirements/steps, regardless of
platform/toolchain/editor.

**Before building**, you set up your development environment (see :ref:`development-environment-setup`). At
a high-level:

- You need Python 3.12. This dictates :ref:`supported-platforms`. Tools like
  ``py`` (Windows + `python.org <https://www.python.org>`_), or
  `uv <https://docs.astral.sh/uv/>`_ enable installing/using multiple Python versions.
- Python dependencies must be installed from ``requirements/all_requirements.txt`` inside a
  virtual environment called  ``.venv`` in the OPynSim source folder.
- You need C++ development toolchains: i.e. cmake, C/C++ compiler, linker, etc.
- You may need additional system-level C/C++ libraries to build the C++ dependencies.
- You need to build and install the vendored C++ dependencies from ``third_party/`` into a
  configuration-specific installation folder (e.g. ``build/third_party/Release-Linux-amd64``). Described
  in :ref:`development-environment-setup`.

**To build OPynSim**, you configure (see :ref:`build-configurations`) and build the main project. You
can do this from the command line or directly inside an IDE with built-in CMake support, such as
`CLion <https://www.jetbrains.com/clion/>`_ or `Visual Studio <https://visualstudio.microsoft.com/>`_. See
:ref:`build-and-run-project` for specific explanations.

**As a high-level command-line example**, here is how OPynSim could be built, tested, and
packaged via the command-line on an Ubuntu desktop once the development environment is
set up:

.. code:: bash

    #!/usr/bin/env bash

    BUILD_CONFIGURATION="Development"  # or "ErrorCheck", "Release-Linux-amd64", etc.

    cd opynsim/third_party/
    cmake --workflow --preset "${BUILD_CONFIGURATION}"
    cd ..
    uv venv --python 3.12
    uv pip install requirements/all_requirements.txt
    cmake --workflow --preset "${BUILD_CONFIGURATION}"

.. _build-configurations:

Build Configurations
--------------------

**Before building**, you must choose a build configuration. This dictates how OPynSim will be
built. OPynSim uses `CMakePresets.json <https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html>`_
to implement configurations. These presets are available:

- ``Development``: Default preset for day-to-day development work. ✅ Debug
  symbols, ✅ assertions, ✅ Sphinx, ❌ linters, ❌ libASAN, ❌ optimization, ❌ warnings as
  errors, ❌ ABI checks.
- ``Development-RapidLinuxLinking``: Same as ``Development``, but uses ``mold`` and separates
  debug files to speed up linking.
- ``ErrorCheck`` (Linux only): Preset that builds slowly, but ensures code quality. ✅ Debug
  symbols, ✅ assertions, ❌ Sphinx, ✅ linters, ❌ libASAN, ❌ optimization, ✅ warnings as
  errors, ❌ ABI checks.
- ``ErrorCheck-LibASAN`` (Linux only): Same as ``ErrorCheck``, but compiled with ``libASAN``
  for additional runtime correctness checks.
- ``Release-$OS-$ARCH`` (e.g. ``Release-Windows-amd64``): Preset used by software releases. ✅ Debug
  symbols (stripped by ``CPack``), ❌ assertions, ✅ Sphinx, ❌ linters, ❌ libASAN, ✅ optimization, ✅ warnings as errors, ✅ ABI checks.

Alternatively, you may also choose to not use presets at all, or override parts of a preset. This is also
possible, but it's your responsibility to set the appropriate cmake cache variables. Read
``CMakePresets.json`` and ``third_party/CMakePresets.json`` for inspiration.


.. _development-environment-setup:

Development Environment Setup
-----------------------------

**Before building**, you must set up a development environment. This depends on your chosen
platform/tools, because OPynSim can be built in a variety of ways. Below are specific guides
for common combinations of operating systems, compilers, and editors.

Windows
~~~~~~~

On Windows, the most straightforward setup is to install Git, Visual Studio, and Python with
their installers, followed by setting everything else up in a terminal.

1. Install Git:

   - Download and install Git from `https://git-scm.com/downloads <https://git-scm.com/downloads>`_.
   - Make sure Git is added to your ``PATH`` during installation so that you can run ``git`` in a terminal
     without manual path hunting.

2. Install Visual Studio:

   - Download and install Visual Studio 2026 from `https://visualstudio.microsoft.com/downloads/ <https://visualstudio.microsoft.com/downloads/>`_.
     **Note**: "Visual Studio Code" ≠ "Visual Studio".
   - Make sure you select the C/C++ development workload in the installer (when you can choose which features to add),
     so that you get the MSVC compiler toolchain and CMake integration.

3. Install Python:

   - Download and install Python from `https://www.python.org/downloads/ <https://www.python.org/downloads/>`_ and
     run the installer.
   - Make sure it adds Python to ``PATH``, so that ``py``, ``python``, and ``pip`` are available in your shell.
   - You don't necessarily need to install Python 3.12. Modern installations include ``py``, which lets you select
     a Python version when setting up the virtual environment (later).

4. Clone OPynSim's source code:

   - Open a terminal in your workspace directory (for example, ``Desktop``) and clone the OPynSim
     repository:

   .. code-block:: console

      git clone https://github.com/opynsim/opynsim

5. Create a Python 3.12 virtual environment:

   - Open a terminal in the cloned ``opynsim/`` directory, or navigate to it with ``cd``.

   - Create a virtual environment with ``py``, activate it in the terminal session, then install
     all necessary Python dependencies:

   .. code-block:: console

      py -3.12 -m venv .venv
      call .\.venv\Scripts\activate
      pip install -r requirements/all_requirements.txt

6. Build vendored third-party dependencies:

   - Open a terminal in the ``opynsim/third_party/`` directory, or navigate to it with ``cd``.
   - Activate the Visual Studio x64 development environment for the terminal, then build
     ``third_party`` with your chosen build configuration (see :ref:`build-configurations`). Here,
     we build it with a ``Development`` configuration:

   .. code-block:: console

      call ../scripts/env_vs-x64.bat
      cmake --workflow --preset Development

   - **Alternatively**, you can open ``opynsim/third_party`` in Visual Studio as a directory (i.e. as-if
     you were developing it) and build it from Visual Studio.


macOS
~~~~~

On macOS, the most straightforward setup is to install Git, XCode Command Line Tools, and
Python with their installers, followed by setting everything else up in a terminal.

1. Install Xcode Command Line Tools:

   - Install Xcode Command Line Tools from the terminal (or install full Xcode from the Mac App
     Store if you want the full IDE and SDKs alongside it). Note that full Xcode or a current
     compiler toolchain is needed if your default compiler lacks support for the required C++
     standard:

   .. code-block:: bash

      xcode-select --install

2. Install Git:

   - Xcode Command Line Tools provides ``git``, or you can download and run the official macOS Git
     installer from `https://git-scm.com/downloads <https://git-scm.com/downloads>`_.
   - Verify it's installed in a terminal with ``git --version``.

3. Install CMake:

   - Download the macOS installer or binary distribution from `https://cmake.org/download/ <https://cmake.org/download/>`_
     and install it (adding the CMake ``bin`` directory to your ``PATH`` if it is not already available in your shell).
   - Verify it's installed in a terminal with ``cmake --version``.

4. Install ``uv`` and Python 3.12:

   - Install ``uv`` using its standalone installer script (see `https://docs.astral.sh/uv/#installation <https://docs.astral.sh/uv/#installation>`_),
     then use ``uv`` to fetch and install Python 3.12:

   .. code-block:: console

      curl -LsSf https://astral.sh/uv/install.sh | sh
      uv python install 3.12

   - Restart your shell or source your shell profile if ``uv`` is not yet on your ``PATH``.

5. Clone OPynSim's source code:

   - Change to your workspace directory and clone the repository:

   .. code-block:: console

      cd ~/Desktop
      git clone https://github.com/opynsim/opynsim

6. Create a Python 3.12 virtual environment:

   - Create a Python 3.12 virtual environment with ``uv`` and install dependencies
     into it:

   .. code-block:: console

      uv venv --python 3.12 .venv
      uv pip install -r requirements/all_requirements.txt

7. Build vendored third-party dependencies:

   - Open a terminal in the ``opynsim/third_party/`` directory, or navigate to it with ``cd``.
   - Build ``third_party`` with your chosen build configuration (see :ref:`build-configurations`). Here,
     we build it with a ``Development`` configuration:

   .. code-block:: console

       cd third_party
       cmake --workflow --preset Development
       cd ..

   - If you have multiple C++ compilers installed, set ``CC`` and ``CXX`` to your chosen C++
     compiler before running CMake, then build ``third_party``.


Ubuntu (≥ 24.04)
~~~~~~~~~~~~~~~~

On Ubuntu, the most straightforward setup is to install all dependencies in a terminal
with ``apt``, followed by setting everything else up in the terminal.

1. Install system dependencies:

   - Use a terminal to install all system dependencies with ``apt``. There's many on Ubuntu because
     OPynSim's build also builds `SDL <https://www.libsdl.org/>`_ from source:

   .. code-block:: console

      sudo apt-get update
      sudo apt-get install -y git cmake ninja-build build-essential python3.12 python3.12-dev python3.12-venv pkg-config libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev libxfixes-dev libxss-dev xvfb libwayland-dev libxkbcommon-dev libdecor-0-dev cage libopengl-dev libgl1-mesa-dev libgl1-mesa-dri libglx-mesa0 libgles2-mesa-dev libegl-dev

2. Clone the source code repository:

   .. code-block:: console

      git clone https://github.com/opynsim/opynsim

3. Create Python 3.12 virtual environment:

   .. code-block:: console

      python3.12 -m venv .venv
      source .venv/bin/activate
      pip install -r requirements/all_requirements.txt

4. Build vendored third-party dependencies:

   - Open a terminal in the ``opynsim/third_party/`` directory, or navigate to it with ``cd``.
   - Build ``third_party`` with your chosen build configuration (see :ref:`build-configurations`). Here,
     we build it with a ``Development`` configuration:

   .. code-block:: console

      cd third_party
      cmake --workflow --preset Development


.. _build-and-run-project:

Build/Run Project
-----------------

Once your development environment is set up and ``third_party/`` has been built and
installed, you can configure and build OPynSim using your preferred editor or from
the command line.

Windows (Visual Studio)
~~~~~~~~~~~~~~~~~~~~~~~

- Open the repository folder directly in Visual Studio.
- Visual Studio should detect the available configurations in ``CMakePresets.json``. Select
  your chosen configuration, which must match a third-party dependencies build
  configuration; otherwise, it won't find necessary libraries.
- Let CMake (via Visual Studio) configure the project
- Once the project is configured, you can then start building/running build targets in the project. For example, you
  could build/run `osc.exe`.
- Build artifacts are written to ``build/$BUILD_CONFIGURATION``.

Windows (cmd.exe)
~~~~~~~~~~~~~~~~~

- Open a ``cmd.exe`` terminal.
- Navigate to the OPynSim source code directory root with ``cd`` (if necessary).
- Activate your Visual Studio toolchain. OPynSim includes ``env_vs-x64.bat`` for this.
- Run CMake workflow for your chosen configuration, which must match a third-party
  dependencies build configuration; otherwise, cmake won't find necessary libraries.
- Build artifacts are written to ``build/$BUILD_CONFIGURATION``.

**Example**:

.. code-block:: bat

   cd path\to\opynsim
   call .\scripts\env_vs-x64.bat
   cmake --workflow --preset Development

CLion
~~~~~

- Launch CLion
- Open the OPynSim source code root directory
- CLion should detect all possible configurations in ``CMakePresets.json``. Enable your chosen
  configuration, which must match a third-party dependencies build configuration; otherwise,
  cmake won't find necessary libraries. Disable CLion's default configuration, which it sometimes
  generates (called ``Debug``).
- Once configured, use CLion's build/run targets (dropdown at the top) to choose a target and
  build/run it.
- Build artifacts are written to ``build/$BUILD_CONFIGURATION``.

Bash
~~~~

- ``cd`` to the OPynSim source code directory
- Run cmake build workflow for your chosen configuration,  which must match a third-party
  dependencies build configuration; otherwise, cmake won't find necessary libraries.
- Build artifacts are written to ``build/$BUILD_CONFIGURATION``.

**Example**:

.. code-block:: console

    cd /path/to/opynsim
    cmake --workflow --preset Development
