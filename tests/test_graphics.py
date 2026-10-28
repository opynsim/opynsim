def test_legacy_graphics_submodule_mostly_works():
    # This is just to ensure legacy code from OPynSim's pre-alpha
    # that used `opynsim.graphics` will still keep chugging along,
    # because we merged it into the top-level `opynsim` module.

    import opynsim.graphics
    import opynsim
    assert opynsim.graphics is opynsim

    # e.g. this should still be valid
    opynsim.graphics.Camera()
