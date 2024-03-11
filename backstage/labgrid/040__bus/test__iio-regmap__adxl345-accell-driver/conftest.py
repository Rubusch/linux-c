""" ref: docs.pytest.org/en/6.2.x/fixture.html """
import pytest

#import labgrid
#from labgrid import Environment, Target
#from labgrid.driver import Driver
#from labgrid.resource import Resource

@pytest.fixture
def shell(strategy):
    strategy.transition("shell")
    return strategy.shell

@pytest.fixture
def strategy(strategy):
    return strategy

#class Error(Exception):
#    pass
#
#class InteractiveCommandError(Error):
#    pass
#
### TODO rename "shell()" to "shell_with_copy()"     
#@pytest.fixture
#def shell(strategy):
#    #env = Environment('../../rpi5__shell__env.yaml')
#    #target = env.get_target('lab')
#    ## target = Target('rpi')
#    ##TODO no valid target found based on the environment            
#    resource = target.get_resource("NetworkService")
#    drv = target.get_driver("SSHDriver", name=resource.name)
#
#    src = ':../../../../040__bus/iio-regmap__adxl345-accell-driver/adxl345_core.ko'
#    dst = ':/home/pi'
#    res = drv.scp(src, dst)
#    if res:
#        exc = InteractiveCommandError("scp error: adxl345_core.ko")
#        exc.exitcode = res
#        raise exc
#
#    src = ':../../../../040__bus/iio-regmap__adxl345-accell-driver/adxl345_spi.ko'
#    dst = ':/home/pi'
#    res = drv.scp(src, dst)
#    if res:
#        exc = InteractiveCommandError("scp error: adxl345_spi.ko")
#        exc.exitcode = res
#        raise exc
#
#    strategy.transition("shell")
#    return strategy.shell
