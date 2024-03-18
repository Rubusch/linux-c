PROJECT = r"03__init-arguments"
MODULES = [r"hello.ko"]
MODARGS = r'hello_int_arg=76 hello_int_array=1,2,3 hello_string_arg="Hello"'
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_turn_off_wifi_spi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_copy_lkm(cmd, target):
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_load_lkm_with_args(cmd):
    do_load_lkms_and_args(cmd, MODULES, MODARGS)

def test_logs_load(cmd):
    do_log_verification(cmd, [r"init_hello_arguments(): initializing...",
                              r"init_hello_arguments(): hello_int_arg = 76",
                              r"init_hello_arguments(): hello_int_arg_cb = 0",
                              r"init_hello_arguments(): hello_int_array[0] = 1",
                              r"init_hello_arguments(): hello_int_array[1] = 2",
                              r"init_hello_arguments(): hello_int_array[2] = 3",
                              r"init_hello_arguments(): hello_string_arg = 'Hello'"])

def test_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)
