PROJECT = r"08__procfs-seq-operations"
MODULES = [r"hello.ko"]
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_turn_off_wifi_spi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_copy_lkm(cmd, target):
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_procfs(cmd):
    stdout, stderr, ret = cmd.run("cat /proc/lothars_procfs_entry")
    assert 0 == ret
    assert "Hello ProcFS!" in stdout[0]

def test_logs_load(cmd):
    do_log_verification(cmd, [r"start_procfs()",
                              r"/proc/lothars_procfs_entry created",
                              r"open_procfs()",
                              r"read_procfs()",
                              r"READ"])

def test_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)

def test_logs_unload(cmd):
    do_log_verification(cmd, [r"stop_procfs()",
                              r"/proc/lothars_procfs_entry removed"])
