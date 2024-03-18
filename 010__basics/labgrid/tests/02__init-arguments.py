PROJECT = r"02__init-arguments"
MODULES = [r"hello.ko"]
MODARGS = r'mystring="foobar" myshort=255 myintArray=1'
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_turn_off_wifi_spi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_copy_lkm(cmd, target):
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_load_lkm_with_args(cmd):
    do_load_lkms_and_args(cmd, MODULES, MODARGS)

def test_logs_load(cmd):
    do_log_verification(cmd, [r"myshort is a short integer: 255",
                              r"myint is a integer: 420",
                              r"mylong is a long integer: 9999",
                              r"mystring is a string: foobar",
                              r"myintArray[0] = 1",
                              r"myintArray[1] = -1",
                              r"got 1 arguments for myintArray."])

def test_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)
