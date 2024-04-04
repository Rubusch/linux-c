PROJECT = r"usb__hid02-led-urb"
MODULES = ["pic32mx470-led.ko"]
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)

## reboot

def test_010_login(shell_cmd):
    do_login_check(shell_cmd, KERNELVERSION)

def test_040_copy_lkm(target, shell_cmd):
    cmd = shell_cmd
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_050_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_080_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

## NB: for practical reasons, this and all tests here
## do not take the particular extra-hardware setup into account
def test_090_logs(cmd):
    do_dmesg_verification(cmd, ["usbcore: registered new interface driver lothars_usb",
                                "usbcore: deregistering interface driver lothars_usb"])

