PROJECT = r"i2c__dc749a-multi-led"
MODULES = ["i2c_ltc3206euf.ko"]
DTBO = "i2c_ltc3206euf_overlay.dtbo"
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

def test_010_login(shell_cmd):
    do_login_check(shell_cmd, KERNELVERSION)

def test_020_copy_dt(target, cmd):
    do_copy_dtbo(cmd, target, [DTBO], PROJECT)

def test_030_modify_config(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_040_copy_lkm(target, shell_cmd):
    do_copy_files(shell_cmd, target, MODULES, PROJECT)

def test_050_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_051_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_052_sudo_modprobe_dev(cmd):
    cmd.run_check("sudo modprobe i2c-dev")

def test_053_sudo_modprobe_regmap(cmd):
    cmd.run_check("sudo modprobe regmap_i2c")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_071_logs(cmd):
    do_dmesg_verification(cmd, ["ltc3206 1-001b: platform_probe enter",
                                "ltc3206 1-001b: led BLUE is ON",
                                "ltc3206 1-001b: there are 5 nodes",
                                "leds sub: the subsystem is sub and num is 0",
                                "leds main: the subsystem is main and num is 0",
                                "leds green: the subsystem is green and num is 0",
                                "leds blue: the subsystem is blue and num is 0",
                                "leds red: the subsystem is red and num is 0",
                                "ltc3206 1-001b: ltc3206_probe() done"])

def test_080_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

## reboot

def test_090_revert_config(shell_cmd):
    undo_register_dtbo(shell_cmd)

def test_100_cleanup_overlays(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO])
