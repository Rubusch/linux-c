PROJECT = r"25__ioctl-fops-demo"
MODULES = [r"happy_ioctl.ko"]
KERNELVERSION = r"6.6.21"
APPFILE = "ioctl.elf"

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
    do_copy_files(cmd, target, [APPFILE], PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, [r"init_happy_ioctl() - major =",
                              r"init_happy_ioctl() device driver init - OK",
                              "If you want to talk to the device driver,",
                              r"you'll have to create a device file, do a:",
                              r"$ sudo mknod lothars_chardev",
                              "the device file name is important, because",
                              "the ioctl program assumes that's the",
                              r"file you'll use.",
                              "character device unregistered",
                              r"READY."])

