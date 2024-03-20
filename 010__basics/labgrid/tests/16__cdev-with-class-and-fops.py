PROJECT = r"16__cdev-with-class-and-fops"
MODULES = [r"hello.ko"]
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
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_write(cmd):
    do_echo_write(cmd, DEVNODE, "1")

def test_060_cat(cmd):
    do_cat_verification(cmd, DEVNODE, [])

def test_070_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs_load(cmd):
    do_log_verification(cmd, ["init_hello_chardev(): called",
                              "init_hello_chardev(): major = ",
                              "init_hello_chardev(): done",
                              "hello_open(): called",
                              "hello_write(): called",
                              "hello_release(): called",
                              "hello_open(): called",
                              "hello_read(): called",
                              "hello_release(): called",
                              "cleanup_hello_chardev(): READY."])
