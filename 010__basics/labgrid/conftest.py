""" ref: docs.pytest.org/en/6.2.x/fixture.html """
import pytest

@pytest.fixture(scope='session')
def command(target):
    shell = target.get_driver('CommandProtocol')
    target.activate(shell)
    return shell

@pytest.fixture(scope='session')
def shell(strategy):
    strategy.transition("shell")
    return strategy.shell

@pytest.fixture(scope='session')
def strategy(strategy):
    return strategy
