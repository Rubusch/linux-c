PROJECT = r"22__waitqueue"
MODULES = [r"waitqueue.ko"]
KERNELVERSION = r"6.6.21"
DEVNODE = "/dev/lothars_hello_device"

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
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_write(cmd):
    import time
    time.sleep(3)
    do_echo_write(cmd, DEVNODE, "1")

def test_054_write(cmd):
    import time
    time.sleep(3)
    do_echo_write(cmd, DEVNODE, "2")
    time.sleep(3)

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, ["mod_init() called",
                              "mod_init(): both threads up and running",
                              "hello_write(): called",
                              "hello_write(): waitqueue_flag is 1",
                              "thread_routine(): waitqueue_flag is pending.. timeout",
                              "hello_write(): called",
                              "hello_write(): waitqueue_flag is 2",
                              "mod_exit(): called",
                              "thread_routine(): waitqueue_flag is 11",
                              "thread_routine(): waitqueue_flag is pending.. timeout"])
