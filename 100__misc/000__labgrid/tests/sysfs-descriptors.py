PROJECT = r"calling-userspace"
MODULES = ["exec-userspace.ko"]
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

def test_020_copy_lkm(target, cmd):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_030_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_040_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_050_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_075_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["mod_init(): called",
                                "delayed_shutdown(): called"])
