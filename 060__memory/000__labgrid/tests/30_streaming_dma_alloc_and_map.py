PROJECT = r"30_streaming_dma_alloc_and_map"
MODULES = ["dma_demo.ko"]
DTBO = "dma_demo_overlay.dtbo"
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

def test_050_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_060_cleanup_lkm(cmd): ## in case already loaded, unload them first...
    undo_load_lkms(cmd, MODULES)

def test_070_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_071_sdma_test(cmd):
    do_cmd(cmd, "echo 123 | sudo tee -a /dev/sdma_test")

def test_075_cleanup_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_080_logs(cmd):
    do_dmesg_verification(cmd, ["lothars_probe() - called",
                                "lothars_probe() - 0. preparation: allocation for DMA buffers",
                                "lothars_probe() - 0. preparation: specify DMA channel caps",
                                "lothars_probe() - 1. request DMA channel",
                                "sdma_write() - called",
                                "sdma_write() - the wbuf string is '123",
                                "sdma_write() - the rbuf string is '' (initially)",
                                "sdma_write() - 2. DMA mapping",
                                "sdma_write() - dma_src map optained: 0x",
                                "sdma_write() - dma_dst map obtained: 0x",
                                "sdma_write() - 3. DMA transaction memcpy()",
                                "sdma_write() - 4. setup a DMA completion",
                                "sdma_write() - 5. submit the DMA transaction",
                                "sdma_write() - 6. start DMA transaction",
                                "sdma_m2m soc:sdma_m2m: dma_m2m_callback() - called",
                                "sdma_m2m soc:sdma_m2m: dma_m2m_callback() - wbuf = '123",
                                "sdma_m2m soc:sdma_m2m: dma_m2m_callback() - rbuf = '123",
                                "sdma_write() - dma transaction has completed: DMA_COMPLETE",
                                "sdma_write() - wbuf = '123",
                                "sdma_write() - rbuf = '123",
                                "sdma_write() - 7. unmap DMA chunks",
                                "lothars_remove() - called"])

## reboot

def test_090_revert_config(reboot_then_cmd):
    cmd = reboot_then_cmd
    undo_register_dtbo(cmd)

def test_100_cleanup_overlays(cmd): ## cleanup
    undo_copy_dtbo(cmd, [DTBO])
