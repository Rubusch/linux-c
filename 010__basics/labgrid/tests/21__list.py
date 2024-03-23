PROJECT = r"21__list"
MODULES = [r"list.ko"]
KERNELVERSION = r"6.6.21"
DEVNODE = "/dev/lothars_device"

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
    do_echo_write(cmd, DEVNODE, "7")

def test_052_cat(cmd):
    do_cat_verification(cmd, DEVNODE, [])

def test_054_write(cmd):
    do_echo_write(cmd, DEVNODE, "456")

def test_056_cat(cmd):
    do_cat_verification(cmd, DEVNODE, [])

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, [r"mod_init(): called",
                              r"hello_linkedlist_write(): called",
                              r"hello_linkedlist_write(): received 7",
                              r"hello_linkedlist_read(): called",
                              r"hello_linkedlist_read(): node 0 data = 7",
                              r"hello_linkedlist_read(): total nodes: 1",
                              r"hello_linkedlist_write(): called",
                              r"hello_linkedlist_write(): received 456",
                              r"hello_linkedlist_read(): called",
                              r"hello_linkedlist_read(): node 0 data = 7",
                              r"hello_linkedlist_read(): node 1 data = 456",
                              r"hello_linkedlist_read(): total nodes: 2",
                              r"mod_exit(): called"])
