PROJECT = r"20__type-sizes"
MODULES = [r"print_my_types.ko"]
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
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_060_logs_load(cmd):
    do_log_verification(cmd, [r"--- types ---",
                              r"unsigned short [2]: 0 -> 65535",
                              r"short [2]: -32768 -> 32767",
                              r"unsigned int [4]: 0 -> 4294967295",
                              r"int [4]: -2147483648 -> 2147483647",
                              r"unsigned long [8]: 0 -> 18446744073709551615",
                              r"long [8]: -9223372036854775808 -> 9223372036854775807",
                              r"unsigned long long [8]: 0 -> 18446744073709551615",
                              r"long long [8]: -9223372036854775808 -> 9223372036854775807",
                              r"unsigned int pointer max: 18446744073709551615"])
