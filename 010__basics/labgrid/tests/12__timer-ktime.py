PROJECT = r"12__timer-ktime"
MODULES = [r"hello.ko"]
MODARGS = r"num=3"
KERNELVERSION = r"6.6.21"

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

def test_030_copy_lkm(cmd, target):
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms_and_args(cmd, MODULES, MODARGS)

def test_050_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_060_delay(cmd):
    import time
    time.sleep(3)

def test_070_logs_load(cmd):
    do_log_verification(cmd, [r"hello_init(): called",
                              r"say(): [1/3] hello again",
                              r"say(): [2/3] hello again",
                              r"say(): [3/3] hello again",
                              r"hello_exit(): called",
                              r"hello_exit(): unloading module after",
                              r"say(): [1/3] hello again",
                              r"say(): [2/3] hello again",
                              r"say(): [3/3] hello again"])
