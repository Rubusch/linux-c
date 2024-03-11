def test_sudo_modprobe_industrialio(shell):
    shell.run_check("sudo modprobe industrialio")
