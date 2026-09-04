import opynsim as opyn
from opynsim.solvers import ModelWarper

from pathlib import Path
import inspect
import pytest

def test_solvers_module_contains_the_model_warper_solver_class():
    assert inspect.isclass(ModelWarper)

def test_model_warper_has_default_constructor():
    model_warper = ModelWarper()  # shouldn't throw

def test_model_warper_from_xml_throws_when_given_invalid_path():
    with pytest.raises(Exception):
        ModelWarper.from_xml("does/not/exist")

def test_model_warper_from_xml_works_when_given_a_valid_scaling_document():
    # Shouldn't throw
    ModelWarper.from_xml(Path(__file__).resolve().parent.parent / "libopynsim/tests/resources/Documents/model_warper/scaling-document.xml")
