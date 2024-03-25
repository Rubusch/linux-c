PROJECT = r"dt_platform_device_leds_and_class"
MODULES = [r"leddriver.ko"]
DTBO = "leddriver_overlay.dtbo"
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)
    do_stat(PROJECT, [DTBO])

def test_010_login(reboot_then_cmd): ## reboot
    do_login_check(reboot_then_cmd, KERNELVERSION)

def test_031_copy_dtbo(cmd, target):
    do_copy_dtbo(cmd, target, [DTBO], PROJECT)

def test_035_regster_dtbo(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_020_turn_off_wifi(reboot_then_cmd): ## reduce log noise
    cmd = reboot_then_cmd
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy_lkm(cmd, target):
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_060_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_070_logs(cmd):
    do_log_verification(cmd, ["led_init(): started",
                              "ledclass_probe(): started",
                              "RGBclassleds 3f200000.ledclassRGB: res->start = 0x3f200000",
                              "RGBclassleds 3f200000.ledclassRGB: res->end = 0x3f2000b3",
                              "RGBclassleds 3f200000.ledclassRGB: there are 3 nodes",
                              "RGBclassleds 3f200000.ledclassRGB: ledclass_probe(): done",
                              "led_init(): done",
                              "led_exit(): started",
                              "RGBclassleds 3f200000.ledclassRGB: ledclass_remove() started",
                              "RGBclassleds 3f200000.ledclassRGB: ledclass_remove() done",
                              "led_exit(): done"])

def test_095_revert_dtbo(cmd):
    undo_register_dtbo(cmd)

def test_100_cleanup(cmd):
    undo_copy_dtbo(cmd, [DTBO])
