""" ref: docs.pytest.org/en/6.2.x/fixture.html """
import pytest


@pytest.fixture(scope="function")
def cmd(strategy):
	strategy.transition("shell")
	return strategy.shell

## reboots and reinits a "shell" state
@pytest.fixture(scope="function")
def reboot_then_cmd(strategy, target):
    strategy.transition("off")
    strategy.transition("shell")
    sshdrv = target.get_driver("SSHDriver")
    target.deactivate(sshdrv)
    target.activate(sshdrv)
    return strategy.shell
