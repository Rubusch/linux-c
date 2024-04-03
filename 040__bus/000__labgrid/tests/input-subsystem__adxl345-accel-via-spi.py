PROJECT = r"input-subsystem__adxl345-accel-via-spi"
MODULES = ["input_demo.ko"]
KERNELVERSION = r"6.6.21"
DTBO = r"input_demo_overlay.dtbo"

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
    cmd = shell_cmd
    do_login_check(cmd, KERNELVERSION)

def test_020_copy_dt(target, cmd):
    do_copy_dtbo(cmd, target, [DTBO], PROJECT)

def test_030_modify_config(cmd):
    do_register_dtbo(cmd, DTBO)

## reboot

def test_040_copy_lkm(target, shell_cmd):
    cmd = shell_cmd
    do_copy_files(cmd, target, MODULES, PROJECT)

def test_050_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_051_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_080_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_085_logs(cmd):
    do_dmesg_verification(cmd, ["adxl345_spi_probe(): called",
                                "adxl345_probe(): called",
                                "adxl345_spi_read(): called",
                                "adxl345_probe(): DEVID",
                                "adxl345_probe(): dev_name(dev) 'spi0.0'",
                                "adxl345_probe(): ADXL345 is found",
                                "adxl345_probe(): ac->phys 'spi0.0/input0'",
                                "adxl345_probe(): the IRQ number is:",
                                "input: ADXL345 accelerometer as /devices/platform/soc/",
                                "adxl345_spi_write(): called",
                                "adxl345_spi_remove(): called"])

## reboot

def test_090_revert_config(shell_cmd):
    cmd = shell_cmd
    undo_register_dtbo(cmd)

def test_100_cleanup_overlays(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO])

