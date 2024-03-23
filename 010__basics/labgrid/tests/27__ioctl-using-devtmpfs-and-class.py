PROJECT = r"27__ioctl-using-devtmpfs-and-class"
MODULES = [r"happy_ioctl.ko"]
KERNELVERSION = r"6.6.21"
APPFILE = "ioctl_test.elf"

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

def test_050_app(cmd):
    do_cmd(cmd, f"sudo /tmp/{APPFILE}")

def test_055_ls(cmd):
    do_cmd_expect(cmd, "ls -l /sys/class/lothars_class/lothars_device",
                  [r"/sys/class/lothars_class/lothars_device -> ../../devices/virtual/lothars_class/lothars_device"])

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_load(cmd):
    do_log_verification(cmd, ["chardev_init(): hello chardev init",
                              "chardev_init(): allocated correctly with major number",
                              "chardev_init(): device class registered correctly",
                              "chardev_init(): the device is created correctly",
                              "chardev_open(): called",
                              "chardev_ioctl(): called, cmd = ",
                              "chardev_close(): called",
                              "chardev_exit(): hello chardev exit"])
