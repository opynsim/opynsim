from typing import Annotated

import numpy
from numpy.typing import NDArray


class TPSCoefficients3D:
    def __repr__(self) -> str: ...

    @property
    def a1(self) -> Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu')]: ...

    @property
    def a2(self) -> Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu')]: ...

    @property
    def a3(self) -> Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu')]: ...

    @property
    def a4(self) -> Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu')]: ...

    def warp_point(self, point: Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu', writable=False)]) -> Annotated[NDArray[numpy.float64], dict(shape=(3), device='cpu')]:
        """Warps a single 3D point"""

def solve_coefficients(source_landmarks: Annotated[NDArray[numpy.float64], dict(shape=(None, 3), device='cpu', writable=False)], destination_landmarks: Annotated[NDArray[numpy.float64], dict(shape=(None, 3), device='cpu', writable=False)]) -> TPSCoefficients3D:
    """
    Pairs `source_landmarks` with `destination_landmarks` and uses the pairing to compute the Thin-Plate Spline (coefficients) of the pairing
    """
