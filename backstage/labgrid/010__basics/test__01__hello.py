def test_shell_command(shell):
    #shell.run_check("insmod /home/pi/*.ko")

    stdout, stderr, returncode = shell.run('sudo insmod /home/pi/gpioirq.ko')
    assert returncode == 0
    assert not stdout
    assert not stderr
    #assert 'Linux' in stdout[0]

    stdout, stderr, returncode = shell.run('sudo rmmod gpioirq.ko')
    assert returncode == 0
    assert not stdout
    assert not stderr
