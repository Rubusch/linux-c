project = r"iio-regmap__adxl345-accell-driver"
module = ["adxl345_core.ko", "adxl345_i2c.ko", "adxl345_spi.ko"]
dtbo = r"input_demo_overlay.dtbo"
## SRC depends on list item
DST = r":/tmp"
KERNELVERSION = r"6.6.21"

def test_preparation(shell_cmd):
    stdout, stderr, ret = shell_cmd.run("uname -r")
    assert 0 == ret
    assert KERNELVERSION in stdout[0]

def test_copy_dt(target, cmd):
    drv = target.get_driver("SSHDriver")
    src = "../" + project + "/" + dtbo
    dst = DST
    ret = drv.scp(src=src, dst=dst)
    assert 0 == ret
    cmd.run_check(r"sudo mv /tmp/" + dtbo + " /boot/overlays/")
    cmd.run_check("sync")

def test_modify_config(cmd):
    cmd.run_check(r"sudo cp /boot/config.txt /boot/config.txt.orig")
    cmd.run_check(r"sudo echo 'dtoverlay=" + dtbo.split(".")[0] + "'")

## reboot

def test_revert_config(shell_cmd):
    shell_cmd.run(r"sudo cp /boot/config.txt.orig /boot/config.txt")
    ## ignore not keeping permissions error due to FAT filesystem

def test_copy_lkm(target):
    drv = target.get_driver("SSHDriver")
    dst = DST
    modules = iter(module)
    assert None != drv
    assert ":/tmp" == dst

    src = "../" + project + "/" + next(modules)
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_core.ko' == src
    assert 0 == ret

    src = "../" + project + "/" + next(modules)
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_i2c.ko' == src
    assert 0 == ret

    src = "../" + project + "/" + next(modules)
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_spi.ko' == src
    assert 0 == ret

def test_turn_off_wifi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_sudo_modprobe_industrialio(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_sudo_modprobe_regmap_spi(cmd):
    cmd.run_check("sudo modprobe regmap_spi")

## in case some or all modules are already loaded, unload them first...
def test_cleanup_lkm(cmd):
    modules = iter(module.reverse())
    mod = next(modules)
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if mod in stdout[0]:
        cmd.run_check(r"sudo rmmod " + mod)
    mod = next(modules)
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if mod in stdout[0]:
        cmd.run_check(r"sudo rmmod " + mod)
    mod = next(modules)
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if mod in stdout[0]:
        cmd.run_check(r"sudo rmmod " + mod)

def test_load_lkm(cmd):
    modules = iter(module)
    cmd.run_check(r"sudo insmod /tmp/" + next(modules))
    cmd.run_check(r"sudo insmod /tmp/" + next(modules))
    cmd.run_check(r"sudo insmod /tmp/" + next(modules))

def test_probe(cmd):
    stdout, stderr, ret = cmd.run(r"sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    #assert 1 == len([m for m in stdout if r"init_hello_arguments(): initializing..." in m])

def test_unload_lkm(cmd):
    modules = iter(module.reverse())
    cmd.run_check(r"sudo rmmod " + next(modules))
    cmd.run_check(r"sudo rmmod " + next(modules))
    cmd.run_check(r"sudo rmmod " + next(modules))
