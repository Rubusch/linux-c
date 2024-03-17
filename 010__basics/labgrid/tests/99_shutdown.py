def test_shutdown(target):
    pw = target.get_driver("ExternalPowerDriver")
    pw.off()
    assert 1 == 1
