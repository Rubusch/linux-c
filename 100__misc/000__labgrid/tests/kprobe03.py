PROJECT = r"kprobe03"
MODULES = ["probe.ko"]
MODARGS = 'symbol="kernel_clone"'
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

def test_050_load_lkm(cmd):
    do_load_lkm_and_args(cmd, MODULES[0], MODARGS)

def test_075_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["XXX planted kprobe at",
                                "XXX <kernel_clone> p->addr ="])
