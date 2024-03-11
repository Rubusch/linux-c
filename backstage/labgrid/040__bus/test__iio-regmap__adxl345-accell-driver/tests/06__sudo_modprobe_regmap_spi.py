def test_sudo_modprobe_regmap_spi(shell):
    shell.run_check("sudo modprobe regmap_spi")
