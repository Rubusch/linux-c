PROJECT = r"06__fops"
MODULES = [r"hello.ko"]
KERNELVERSION = r"6.6.21"

import sys
## NB: this is from where pytest is called!
sys.path.insert(0, '../../backstage/labgrid/labgrid_lib')
import common_kernel
from common_kernel import *


def test_000_files(cmd):
    do_stat(PROJECT, MODULES)

def test_010_login(shell_cmd): ## reboot
    do_login_check(shell_cmd, KERNELVERSION)

def test_020_turn_off_wifi(cmd): ## reduce log noise
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_030_copy_lkm(cmd, target):
    do_copy_lkms(cmd, target, MODULES, PROJECT)

def test_040_load_lkm(cmd):
    do_load_lkms(cmd, MODULES)

def test_050_logs_load(cmd):
    do_log_verification(cmd, [r"I was assigned major number ",
                              r"To talk to the device, create a dev file with",
                              r"'sudo mknod /dev/lothars_char_dev c"])

def test_060_device_node(cmd):
    import re
    stdout, stderr, ret = cmd.run("sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    line = [m for m in stdout if r"sudo mknod /dev/lothars_char_dev c" in m]
    assert 0 < len(line)
    devnode = re.search(r"sudo mknod /dev/lothars_char_dev c (.+) 0", line[0])
    cmd.run_check(f"{devnode.group(0)}")
    stdout, stderr, ret = cmd.run("sudo cat /dev/lothars_char_dev")
    assert 0 == ret
    assert "already told you 1 times Hello World!" in stdout[0]

def test_070_unload_lkm(cmd):
    undo_load_lkms(cmd, MODULES)
