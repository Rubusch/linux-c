PROJECT = r"30_click_led_demo"
MODULES = ["irq_click.ko"]
DTBO = "irq_click_overlay.dtbo"
KERNELVERSION = r"6.6.21"


import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)
    do_stat(PROJECT, [DTBO])

## reboot

def test_010_login(reboot_then_cmd):
    cmd = reboot_then_cmd
    do_login_check(cmd, KERNELVERSION)

def test_020_copy_dt(target, cmd):
    do_copy_dtbo(cmd, target, [DTBO], PROJECT)

def test_030_modify_config(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_040_copy_lkm(target, reboot_then_cmd):
    cmd = reboot_then_cmd
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_050_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_075_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["ledpwm soc:ledpwm: lothars_probe() - called",
                                "ledpwm soc:ledpwm: lothars_probe() - there are 5 nodes",
                                "ledpwm soc:ledpwm: lothars_probe() - priv data structure allocated",
                                "ledpwm soc:ledpwm: lothars_probe() - the major number is",
                                "keyled blue: led_device_register() - major: ",
                                "keyled blue: led_device_register() - minor: 0",
                                "keyled blue: led_device_register() - led blue added",
                                "keyled green: led_device_register() - major: ",
                                "keyled green: led_device_register() - minor: 1",
                                "keyled green: led_device_register() - led green added",
                                "keyled red: led_device_register() - major: ",
                                "keyled red: led_device_register() - minor: 2",
                                "keyled red: led_device_register() - led red added",
                                "ledpwm soc:ledpwm: lothars_probe() - irq number: ",
                                "ledpwm soc:ledpwm: lothars_probe() - out of device-tree",
                                "ledpwm soc:ledpwm: lothars_probe() - the led period is: 10",
                                "ledpwm soc:ledpwm: lothars_probe() - done",
                                "ledpwm soc:ledpwm: lothars_remove() - called",
                                "ledpwm soc:ledpwm: lothars_remove() - done"])

## reboot

def test_090_revert_config(reboot_then_cmd):
    cmd = reboot_then_cmd
    undo_register_dtbo(cmd)

def test_100_cleanup_overlays(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO])

