PROJECT = r"09__sysfs"
MODULES = [r"hello.ko"]
KERNELVERSION = r"6.6.21"
SYSFS_FILE = r"/sys/kernel/lothars_sysfs/hello_sysfs_value"

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

def test_050_cat_sysfs(cmd):
    do_cat_verification(cmd, SYSFS_FILE, [r"0"])

def test_060_echo_sysfs(cmd):
    do_echo_write(cmd, SYSFS_FILE, r"7")

def test_070_cat_sysfs(cmd):
    do_cat_verification(cmd, SYSFS_FILE, [r"7"])

def test_080_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_090_logs_load(cmd):
    do_log_verification(cmd, [r"mod_init(): called",
                              r"mod_exit(): called"])
