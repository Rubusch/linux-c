PROJECT = r"iio-regmap__adxl345-accell-driver"
MODULES = ["adxl345_core.ko", "adxl345_spi.ko", "adxl345_i2c.ko"]
DTBOS = [r"adxl345_spi_overlay.dtbo", r"adxl345_i2c_overlay.dtbo"]
IDX_SPI = 0
IDX_I2C = 1
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT, MODULES)
    do_stat(PROJECT, DTBOS)

def test_010_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_020_copy_dt(target, cmd):
    do_copy_dtbo(target, cmd, DTBOS, PROJECT)

def test_030_modify_config_spi(cmd):
    do_register_dtbo(cmd, DTBOS[IDX_SPI])

def test_040_copy_lkm_spi(target, shell_cmd): ## reboot
    do_copy_lkms(shell_cmd, target, [MODULES[0], MODULES[1]], PROJECT)

def test_050_turn_off_wifi_spi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_060_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_070_sudo_modprobe_regmap_spi(cmd):
    cmd.run_check("sudo modprobe regmap_spi")

## in case some or all modules are already loaded, unload them first...
def test_080_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_090_load_spi(cmd):
    do_load_lkms(cmd, [MODULES[0], MODULES[1]]) ## spi

def test_100_probe_spi(cmd):
    do_log_verification(cmd, [r"adxl345_core_probe(): called",
                              r"adxl345_setup(): called",
                              r"adxl345_setup(): calling setup()",
                              r"adxl345_spi_setup(): called",
                              r"adxl345_setup(): retrieving DEVID"])

def test_110_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_120_revert_config(shell_cmd): ## reboot
    undo_register_dtbo(shell_cmd)

## i2c

def test_130_modify_config_i2c(cmd):
    do_register_dtbo(cmd, DTBOS[IDX_I2C])

def test_140_copy_lkm_i2c(target, shell_cmd): ## reboot
    do_copy_lkms(shell_cmd, target, [MODULES[0], MODULES[2]], PROJECT)

def test_150_turn_off_wifi_i2c(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_160_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_170_sudo_modprobe_i2c_dev(cmd):
    cmd.run_check("sudo modprobe i2c-dev")

def test_180_sudo_modprobe_regmap_i2c(cmd):
    cmd.run_check("sudo modprobe regmap_i2c")

## in case some or all modules are already loaded, unload them first...
def test_190_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_200_load_lkm_i2c(cmd):
    do_load_lkms(cmd, [MODULES[0], MODULES[2]]) ## i2c

def test_210_probe_i2c(cmd):
    do_log_verification(cmd, [r"adxl345_i2c_probe(): called",
                              r"adxl345_core_probe(): called",
                              r"adxl345_setup(): called",
                              r"adxl345_setup(): calling setup()",
                              r"adxl345_setup(): retrieving DEVID"])

def test_220_revert_config(shell_cmd): ## reboot
    undo_register_dtbo(shell_cmd)

def test_230_cleanup_overlays(cmd):
    undo_copy_dtbo(cmd, DTBOS)

def test_240_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)
