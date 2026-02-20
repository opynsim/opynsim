import enum

from . import tps3d as tps3d, ui as ui


class ModelSpecification:
    def compile(self) -> Model: ...

class Model:
    def initial_state(self) -> ModelState: ...

    def realize(self, arg0: ModelState, arg1: ModelStateStage, /) -> None: ...

class ModelState:
    pass

class ModelStateStage(enum.Enum):
    TIME = 0

    POSITION = 1

    VELOCITY = 2

    DYNAMICS = 3

    ACCELERATION = 4

    REPORT = 5

def set_logging_level(python_logging_level: int) -> None: ...

def import_osim_file(osim_file_path: str) -> ModelSpecification: ...

def add_geometry_directory(geometry_directory_path: str) -> None: ...
