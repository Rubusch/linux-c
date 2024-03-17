""" ref: docs.pytest.org/en/6.2.x/fixture.html """
import pytest

## just activates "shell" (assumed already we're already in "shell")
# TODO merge both and do a check    
@pytest.fixture(scope="function")
def cmd(target):
    shell = target.get_driver('CommandProtocol')
    target.activate(shell)
    return shell

## reboots and reinits a "shell" state
@pytest.fixture(scope="function")
def shell_cmd(strategy, target):
    sshdrv = target.get_driver("SSHDriver")
    strategy.transition("off")
    sshdrv.on_deactivate()
    strategy.transition("shell")
    sshdrv.on_activate()
    return strategy.shell
