from opynsim._core import *

import sys

# Legacy shim: pre-alpha versions of OPynSim had a separate `graphics`
# submodule for graphics-related classes (e.g. `Camera`) which was
# subsequently merged into the top-level `opynsim` module for ease-of-use.
#
# These lines make downstream code like `import opynsim.graphics` or
# `from opynsim.graphics import Camera` just import `opynsim`.
sys.modules[f"{__name__}.graphics"] = sys.modules[__name__]
graphics = sys.modules[__name__]
