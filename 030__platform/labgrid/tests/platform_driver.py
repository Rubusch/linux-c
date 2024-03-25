PROJECT = r"platform_driver"
MODULES = [r"platform.ko", r"start.ko"]
MODARGS = r'PROBED_MODULE_NAME="lothars-platform-dummy"'
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)

def test_010_login(reboot_then_cmd): ## reboot
    do_login_check(reboot_then_cmd, KERNELVERSION)

## reboot

def test_020_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, [MODULES[0]])

def test_045_load_lkm_starter(cmd):
    do_load_lkm_and_args(cmd, MODULES[1], MODARGS)

def test_050_cat(cmd):
    do_cat_verification(cmd, "/dev/lothars_device", [])

def test_055_echo(cmd):
    do_cmd(cmd, 'echo "foo" |sudo tee -a /dev/lothars_device')

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs(cmd):
    do_log_verification(cmd, ["pdrv_probe(): called",
                              "dummy_open(): called",
                              "dummy_read(): called",
                              "dummy_release(): called",
                              "dummy_open(): called",
                              "dummy_write(): called",
                              "dummy_release(): called",
                              "pdrv_remove(): called"])
