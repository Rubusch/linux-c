PROJECT = r"18__miscdevice"
MODULES = [r"miscdevice.ko"]
KERNELVERSION = r"6.6.21"
USERSPACE = ["ioctl_app.elf"]

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
    do_copy_lkms(cmd, target, USERSPACE, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_app(cmd):
    do_cmd(cmd, f"sudo /tmp/{USERSPACE[0]}")

def test_060_cmd_expect(cmd):
    do_cmd_expect(cmd, f"sudo ls -l /dev/lothars_device", ["/dev/lothars_device"])

def test_070_cmd(cmd):
    do_cmd(cmd, "sudo cat /sys/class/misc/lothars_device/dev")

def test_080_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_090_logs_load(cmd):
    do_log_verification(cmd, [r"mod_init(): hello chardev init",
                              r"mod_init(): got minor ",
                              r"chardev_open(): called",
                              r"chardev_ioctl(): called, cmd = ",
                              r"chardev_close(): called",
                              r"mod_exit(): hello chardev exit"])
