PROJECT = r"timer_std"
MODULES = ["timer.ko"]
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

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_delay(cmd):
    import time
    time.sleep(30)

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["mod_init(): called",
                                "timer_callback(): called",
                                "timer_callback(): 0",
                                "timer_callback(): called",
                                "timer_callback(): 1"])
