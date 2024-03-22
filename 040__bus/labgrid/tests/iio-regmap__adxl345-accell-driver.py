PROJECT = r"iio-regmap__adxl345-accell-driver"
MODULES = ["adxl345_core.ko", "adxl345_spi.ko", "adxl345_i2c.ko"]
IDX_MODS_CORE = 0
IDX_MODS_SPI = 1
IDX_MODS_I2C = 2
MODULES_SPI = [MODULES[IDX_MODS_CORE], MODULES[IDX_MODS_SPI]]
MODULES_I2C = [MODULES[IDX_MODS_CORE], MODULES[IDX_MODS_I2C]]
DTBOS = [r"adxl345_spi_overlay.dtbo", r"adxl345_i2c_overlay.dtbo", r"adxl345_spi-3wire_overlay.dtbo"]
IDX_DTBOS_SPI = 0
IDX_DTBOS_I2C = 1
IDX_DTBOS_SPI3WIRE = 2
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

## spi

def test100_modify_config_spi(cmd):
    do_register_dtbo(cmd, DTBOS[IDX_DTBOS_SPI])

def test_110_copy_lkm_spi(target, shell_cmd): ## reboot
    do_copy_files(shell_cmd, target, MODULES_SPI, PROJECT)

def test_120_turn_off_wifi_spi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_130_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_140_sudo_modprobe_regmap_spi(cmd):
    cmd.run_check("sudo modprobe regmap_spi")

## in case some or all modules are already loaded, unload them first...
def test_150_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_160_load_spi(cmd):
    do_load_lkms(cmd, MODULES_SPI)

def test_170_probe_spi(cmd):
    do_log_verification(cmd, [r"adxl345_core_probe(): called",
                              r"adxl345_setup(): called",
                              r"adxl345_setup(): calling setup()",
                              r"adxl345_spi_setup(): called",
                              r"adxl345_setup(): retrieving DEVID"])

def test_180_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_190_revert_config(shell_cmd): ## reboot
    undo_register_dtbo(shell_cmd)

## i2c

def test_200_modify_config_i2c(cmd):
    do_register_dtbo(cmd, DTBOS[IDX_DTBOS_I2C])

def test_210_copy_lkm_i2c(target, shell_cmd): ## reboot
    do_copy_files(shell_cmd, target, MODULES_I2C, PROJECT)

def test_220_turn_off_wifi_i2c(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_230_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_235_sudo_modprobe_i2c_dev(cmd):
    cmd.run_check("sudo modprobe i2c-dev")

def test_240_sudo_modprobe_regmap_i2c(cmd):
    cmd.run_check("sudo modprobe regmap_i2c")

## in case some or all modules are already loaded, unload them first...
def test_250_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_260_load_lkm_i2c(cmd):
    do_load_lkms(cmd, [MODULES[0], MODULES[2]])

def test_270_probe_i2c(cmd):
    do_log_verification(cmd, [r"adxl345_i2c_probe(): called",
                              r"adxl345_core_probe(): called",
                              r"adxl345_setup(): called",
                              r"adxl345_setup(): calling setup()",
                              r"adxl345_setup(): retrieving DEVID"])

def test_280_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_290_revert_config(shell_cmd): ## reboot
    undo_register_dtbo(shell_cmd)

## spi-3wire
def test_300_modify_config_spi3wire(cmd):
    do_register_dtbo(cmd, DTBOS[IDX_DTBOS_SPI3WIRE])

def test_310_copy_lkm_spi3wire(target, shell_cmd): ## reboot
    do_copy_files(shell_cmd, target, MODULES_SPI, PROJECT)

def test_320_turn_off_wifi_spi3wire(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_330_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_340_sudo_modprobe_regmap_spi(cmd):
    cmd.run_check("sudo modprobe regmap_spi")

## in case some or all modules are already loaded, unload them first...
def test_350_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_360_load_spi3wire(cmd):
    do_load_lkms(cmd, MODULES_SPI)

def test_370_probe_spi3wire(cmd):
    do_log_verification(cmd, [r"adxl345_core_probe(): called",
                              r"adxl345_setup(): called",
                              r"adxl345_setup(): calling setup()",
                              r"adxl345_spi_setup(): called",
                              r"adxl345_setup(): retrieving DEVID"])

def test_380_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_390_revert_config(shell_cmd): ## reboot
    undo_register_dtbo(shell_cmd)

## cleanup
def test_400_cleanup_overlays(cmd):
    undo_copy_dtbo(cmd, DTBOS)
