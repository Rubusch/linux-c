PROJECT = r"iio"
MODULES = ["iio-dummy.ko", "start.ko"]
MODARGS = 'PROBED_MODULE_NAME="lothars-iio-dummy"'
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)

def test_010_login(cmd):
    do_login_check(cmd, KERNELVERSION)

## reboot

def test_040_copy_lkm(target, shell_cmd):
    do_copy_files(shell_cmd, target, MODULES, PROJECT)

def test_050_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_051_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, [MODULES[0]])

def test_071_load_lkm(cmd):
    do_load_lkm_and_args(cmd, MODULES[1], MODARGS)

def test_072_sysfs(cmd):
    do_cmd_expect(cmd, r"ls -l /sys/bus/iio/devices/iio\:device0/", ["in_voltage0_raw",
                                                                    "in_voltage1_raw",
                                                                    "in_voltage2_raw",
                                                                    "in_voltage3_raw",
                                                                    "in_voltage_scale"])

def test_080_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_085_logs(cmd):
    do_dmesg_verification(cmd, ["pdrv_probe(): called",
                                "pdrv_remove(): called"])
