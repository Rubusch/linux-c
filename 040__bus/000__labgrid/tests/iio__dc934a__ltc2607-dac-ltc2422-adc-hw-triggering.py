PROJECT = r"iio__dc934a__ltc2607-dac-ltc2422-adc-hw-triggering"

PROJECT_ADC = f"{PROJECT}/module__ltc2422-hw-trigger"
MODULES_ADC = ["iio-ltc2422-adc.ko"]
DTBO_ADC = "iio-ltc2422-adc_overlay.dtbo"

PROJECT_DAC = f"{PROJECT}/module__ltc2607-dual-dac"
MODULES_DAC = ["iio-ltc2607-dac.ko"]
DTBO_DAC = "iio-ltc2607-dac_overlay.dtbo"

PROJECT_APP = f"{PROJECT}/userspace"
APPLICATION = "iio_app.elf"
## NB: no hardware attached, so minimal test for application

KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_stat(cmd):
    do_stat(PROJECT_ADC, MODULES_ADC)
    do_stat(PROJECT_ADC, [DTBO_ADC])

    do_stat(PROJECT_DAC, MODULES_DAC)
    do_stat(PROJECT_DAC, [DTBO_DAC])

    do_stat(PROJECT_APP, [APPLICATION])

## reboot

def test_010_login(shell_cmd):
    cmd = shell_cmd
    do_login_check(cmd, KERNELVERSION)

def test_020_copy_dt_adc(target, cmd):
    do_copy_dtbo(cmd, target, [DTBO_ADC], PROJECT_ADC)

def test_021_copy_dt_dac(target, cmd):
    do_copy_dtbo(cmd, target, [DTBO_DAC], PROJECT_DAC)

def test_030_modify_config_adc(cmd):
    do_register_dtbo(cmd, DTBO_ADC)

def test_031_modify_config_dac(cmd):
    do_register_dtbo(cmd, DTBO_DAC)

## reboot

def test_040_turn_off_wifi(shell_cmd):
    cmd = shell_cmd
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_041_copy_lkm_adc(target, cmd):
    do_copy_files(cmd, target, MODULES_ADC, PROJECT_ADC)

def test_042_copy_lkm_dac(target, cmd):
    do_copy_files(cmd, target, MODULES_DAC, PROJECT_DAC)

def test_043_copy_app(target, cmd):
    do_copy_files(cmd, target, [APPLICATION], PROJECT_APP)

def test_045_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_046_sudo_modprobe_dev(cmd):
    cmd.run_check("sudo modprobe i2c-dev")

def test_070_load_lkm_dac(cmd):
    do_load_lkms(cmd, MODULES_DAC)

def test_071_load_lkm_adc(cmd):
    do_load_lkms(cmd, MODULES_ADC)

def test_075_cleanup_lkm_adc(cmd):
    undo_load_lkms(cmd, MODULES_ADC)

def test_076_cleanup_lkm_dac(cmd):
    undo_load_lkms(cmd, MODULES_DAC)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["ltc2607_probe() - called",
                                "ltc2607_probe() - was called from DAC00",
                                "ltc2607_probe() - called",
                                "ltc2607_probe() - was called from DAC01",
                                "ltc2422_probe() - called",
                                "ltc2422_probe() - the irq number is '",
                                "ltc2422_remove() - called"])

## reboot

def test_090_revert_config(shell_cmd):
    cmd = shell_cmd
    undo_register_dtbo(cmd)

def test_095_cleanup_overlays_adc(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO_ADC])

def test_096_cleanup_overlays_dac(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO_DAC])

