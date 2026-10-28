def test_legacy_solvers_submodule_mostly_works():
    # This is just to ensure legacy code from OPynSim's pre-alpha
    # that used `opynsim.solvers` will still keep chugging along,
    # because we merged it into the top-level `opynsim` module.

    import opynsim.solvers
    import opynsim

    assert opynsim.solvers is opynsim
