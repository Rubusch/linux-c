PROJECT = r"gpio-interrupt__rpi4"
MODULES = [r"hello_gpio.ko"]
KERNELVERSION = r"6.6.21"
DTBO = "gpio-interrupt__rpi4.dtbo"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)
    do_stat(PROJECT, [DTBO])

 ## reboot

def test_010_login(reboot_then_cmd):
    do_login_check(reboot_then_cmd, KERNELVERSION)

def test_020_copy_dtbo(cmd, target):
    do_copy_dtbo(cmd, target, [DTBO], PROJECT)

def test_025_regster_dtbo(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_020_turn_off_wifi(reboot_then_cmd): ## reduce log noise
    cmd = reboot_then_cmd
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, [MODULES[0]])

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs(cmd):
    do_log_verification(cmd, ["mindblowing_probe(): called",
                              "mindblowing_probe(): got minor",
                              "mindblowing_probe(): ok",
                              "mindblowing_remove(): called"])

 ## reboot

def test_095_revert_dtbo(reboot_then_cmd):
    cmd = reboot_then_cmd
    undo_register_dtbo(cmd)

def test_100_cleanup(cmd):
    undo_copy_dtbo(cmd, [DTBO])
