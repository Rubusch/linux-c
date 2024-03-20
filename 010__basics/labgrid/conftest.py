""" ref: docs.pytest.org/en/6.2.x/fixture.html """
import pytest

## just activates "shell" (assumed already we're already in "shell")
# TODO merge both and do a check    
@pytest.fixture(scope="function")
def cmd(strategy):
	strategy.transition("shell")
	return strategy.shell

#def cmd(target):
#    shell = target.get_driver('CommandProtocol')
#    target.activate(shell)
#    return shell

## reboots and reinits a "shell" state
@pytest.fixture(scope="function")
def shell_cmd(strategy, target):
    strategy.transition("off")
    strategy.transition("shell")
    sshdrv = target.get_driver("SSHDriver")
    sshdrv.on_deactivate()
    sshdrv.on_activate()
    return strategy.shell
