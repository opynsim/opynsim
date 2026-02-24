> [!CAUTION]
> This is currently **ALPHA** software. You can (of course) use it, but major architectural
> pieces are still being moved around.

<h1 align="center">
    <img src="docs/source/_static/opynsim_banner_horizontal.svg" alt="OPynSim banner" />
</h1>

OPynSim is a python-native API for musculoskeletal modeling that doesn't compromise on nearly
20 years of research, feature development, and UI development from [Simbody](https://github.com/simbody/simbody),
[OpenSim](https://simtk.org/projects/opensim), and [OpenSim Creator](https://www.opensimcreator.com/).

- **Documentation**: [https://docs.opynsim.eu](https://docs.opynsim.eu)
- **Source code**: [https://github.com/opynsim/opynsim](https://docs.opynsim.eu)

OPynSim provides:

- A pythonic interface for building and manipulating musculoskeletal models.
- A visualization API that supports both 2D widgets (plots, buttons, text) and
  real-time 3D rendering.
- High-performance, low-overhead native bindings, implemented with
  [nanobind](https://github.com/wjakob/nanobind).
- Almost zero runtime dependencies. The entire OPynSim C++ stack
  is built into a single native extension module that only depends on common
  system libraries. The Python stack only depends on `numpy` (unpinned).
- Stable Python ABI implementation. Each release of OPynSim works on any
  Python version >= 3.12 on Windows, macOS, and major Linux distributions.
- Strong compatibility with OpenSim data files.
