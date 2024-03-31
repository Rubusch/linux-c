PROJECT = r"34__kthread"
MODULES = [r"hello.ko"]
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

def test_030_copy(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

## no hardware connected

def test_060_unload_lkm(cmd):
    import time
    time.sleep(5)
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, ["mod_init() started",
                              "mod_init() kernelthread initialized",
                              "kernelthread_routine() counting: 0",
                              "kernelthread_routine() counting: 1",
                              "kernelthread_routine() counting: 2",
                              "kernelthread_routine() counting: 3",
                              "kernelthread_routine() counting: 4",
                              "kernelthread_routine() counting: 5",
                              "mod_exit() READY."])
