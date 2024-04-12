PROJECT = r"sysfs-descriptors"
MODULES = ["sysfs-demo.ko"]
APP = "sysfs-select.elf"
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)

def test_001_stat(cmd):
    do_stat(PROJECT, [APP])

def test_010_login(cmd):
    do_login_check(cmd, KERNELVERSION)

def test_020_copy_lkm(target, cmd):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_021_copy_lkm(target, cmd):
    do_copy_files(cmd, target, [APP], PROJECT)

def test_030_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_app(cmd):
    do_cmd(cmd, f"sudo /tmp/{APP} &")

def test_060_app(cmd):
    do_cmd(cmd, "echo foo | sudo tee -a /sys/lothars-sysfs/trigger")

def test_075_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["mod_init(): called",
                                "sysfs_show(): called - (notify)",
                                "sysfs_show(): called - (trigger)"])

