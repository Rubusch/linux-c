PROJECT = r"workqueue_shared"
MODULES = [r"shared-workqueue.ko"]
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_files(cmd):
    import subprocess
    subprocess.run(["make", "-C", f"../{PROJECT}", "clean"])
    subprocess.run(["make", "-C", f"../{PROJECT}"])
    do_stat(PROJECT, MODULES)

def test_010_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_020_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)
    import time
    time.sleep(10)

## TODO

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_unload(cmd):
    do_log_verification(cmd, ["mod_init(): called",
                              "mod_init(): going to sleep...",
                              "work_handler(): called",
                              "work_handler(): data is 47",
                              "mod_init(): wake up!"])
