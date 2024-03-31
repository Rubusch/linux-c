PROJECT = r"30__gpio"
MODULES = [r"gpioled.ko"]
KERNELVERSION = r"6.6.21"
#DEVNODE = "/dev/lothars_gpio_driver"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_files(cmd):
    do_stat(PROJECT, MODULES)

def test_010_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_020_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

## w/o hardware, this makes the system freeze
#def test_050_echo_on(cmd):
#    do_cmd(cmd, f"echo 1| sudo tee -a {DEVNODE}")
#
#def test_052_echo_off(cmd):
#    do_cmd(cmd, f"echo 0| sudo tee -a {DEVNODE}")
#
#def test_054_echo_on(cmd):
#    do_cmd(cmd, f"echo 1| sudo tee -a {DEVNODE}")
#
#def test_056_echo_off(cmd):
#    do_cmd(cmd, f"echo 0| sudo tee -a {DEVNODE}")

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, ["mod_init(): called",
                              "mod_exit(): called"])
