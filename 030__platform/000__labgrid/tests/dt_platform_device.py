PROJECT = r"dt_platform_device"
MODULES = [r"platform.ko"]
APPFILE = "ioctl_test.elf"
DTBO = "platform_overlay.dtbo"
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(f"{PROJECT}/module", MODULES)
    do_stat(f"{PROJECT}/module", [DTBO])
    do_stat(f"{PROJECT}/userspace", [APPFILE])

def test_010_login(reboot_then_cmd): ## reboot
    do_login_check(reboot_then_cmd, KERNELVERSION)

def test_020_copy_dtbo(cmd, target):
    do_copy_dtbo(cmd, target, [DTBO], f"{PROJECT}/module")

def test_025_regster_dtbo(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_030_turn_off_wifi(reboot_then_cmd): ## reduce log noise
    cmd = reboot_then_cmd
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_040_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, f"{PROJECT}/module")

def test_042_copy_app(cmd, target):
    do_copy_files(cmd, target, [APPFILE], f"{PROJECT}/userspace")

def test_045_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_command(cmd):
    do_cmd_expect(cmd, r'sudo find /sys -name \*lothar\*', ["/sys/class/misc/lothars_device",
                                                            "/sys/devices/virtual/misc/lothars_device",
                                                            "/sys/bus/platform/drivers/lotharskeys",
                                                            "/sys/module/platform/drivers/platform:lotharskeys"])
def test_055_command(cmd):
    do_cmd(cmd, f"sudo /tmp/{APPFILE}")

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_unload(cmd):
    do_log_verification(cmd, ["chardev_probe(): called",
                              "chardev_probe(): got minor 121",
                              "chardev_open(): called",
                              "chardev_ioctl(): called, cmd = 100, arg = 110",
                              "chardev_close(): called"])

def test_095_revert_dtbo(cmd):
    undo_register_dtbo(cmd)

def test_100_cleanup(cmd):
    undo_copy_dtbo(cmd, [DTBO])
