PROJECT = r"dt_platform_device_leds"
MODULES = [r"leddriver.ko"]
DTBO = "leddriver_overlay.dtbo"
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(f"{PROJECT}", MODULES)
    do_stat(f"{PROJECT}", [DTBO])

def test_010_login(reboot_then_cmd): ## reboot
    do_login_check(reboot_then_cmd, KERNELVERSION)

def test_020_copy_dtbo(cmd, target):
    do_copy_dtbo(cmd, target, [DTBO], f"{PROJECT}")

def test_025_regster_dtbo(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_030_turn_off_wifi(reboot_then_cmd): ## reduce log noise
    cmd = reboot_then_cmd
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_035_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, f"{PROJECT}")

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_command(cmd):
    do_cmd_expect(cmd, r'sudo find /sys -name \*lothar\*', ["/sys/class/misc/lothars_device",
                                                            "/sys/devices/virtual/misc/lothars_device"])

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs_unload(cmd):
    do_log_verification(cmd, ["led_init started",
                              "led_probe(): called",
                              "led_probe(): done",
                              "led_init() done"])

def test_095_revert_dtbo(cmd):
    undo_register_dtbo(cmd)

def test_100_cleanup(cmd):
    undo_copy_dtbo(cmd, [DTBO])

