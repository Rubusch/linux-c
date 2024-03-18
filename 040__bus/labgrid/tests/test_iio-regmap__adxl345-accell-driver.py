project = r"iio-regmap__adxl345-accell-driver"
module = ["adxl345_core.ko", "adxl345_spi.ko", "adxl345_i2c.ko"]
dtbo = [r"adxl345_spi_overlay.dtbo", r"adxl345_i2c_overlay.dtbo"]
idx_spi = 0
idx_i2c = 1
## SRC depends on list item
DST = r":/tmp"
KERNELVERSION = r"6.6.21"

def _unload_lkms(cmd):
    modules = iter(list(reversed(module)))
    mod = next(modules)
    mod.split(".")[0]
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if 0 < len(stdout):
        if mod in stdout[0]:
            cmd.run_check(r"sudo rmmod " + mod)
    mod = next(modules)
    mod.split(".")[0]
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if 0 < len(stdout):
        if mod in stdout[0]:
            cmd.run_check(r"sudo rmmod " + mod)
    mod = next(modules)
    mod.split(".")[0]
    stdout, stderr, ret = cmd.run(r"sudo lsmod | grep " + mod)
    if 0 < len(stdout):
        if mod in stdout[0]:
            cmd.run_check(r"sudo rmmod " + mod)


## start #######################################################################

def test_login(shell_cmd):
    stdout, stderr, ret = shell_cmd.run("uname -r")
    assert 0 == ret
    assert KERNELVERSION in stdout[0]

def test_copy_dt(target, cmd):
    drv = target.get_driver("SSHDriver")

    for idx in range(len(dtbo)):
        src = f"../{project}/{dtbo[idx]}"
        dst = DST
        ret = drv.scp(src=src, dst=dst)
        assert 0 == ret
        cmd.run_check(f"sudo mv /tmp/{dtbo[idx]} /boot/overlays/")
        cmd.run_check("sync")

def test_modify_config_spi(cmd):
    cmd.run_check(r"sudo cp /boot/config.txt /boot/config.txt.orig")
    dtfrag = dtbo[idx_spi]
    dtfrag = dtfrag.split(".")[0]
    cmd.run_check(f"echo 'dtoverlay={dtfrag}' |sudo tee -a /boot/config.txt")
    ## sd card write delay...
    cmd.run_check(r"sync")
    import time
    time.sleep(3)

## reboot

def test_copy_lkm_spi(target, shell_cmd):
    drv = target.get_driver("SSHDriver")
    dst = DST

    assert None != drv
    assert ":/tmp" == dst

    src = f"../{project}/{module[0]}"
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_core.ko' == src
    assert 0 == ret

    src = f"../{project}/{module[1]}"
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_spi.ko' == src
    assert 0 == ret

def test_turn_off_wifi_spi(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_sudo_modprobe_industrialio01(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_sudo_modprobe_regmap_spi01(cmd):
    cmd.run_check("sudo modprobe regmap_spi")

## in case some or all modules are already loaded, unload them first...
def test_cleanup_lkm01(cmd):
    _unload_lkms(cmd)

def test_load_spi(cmd):
    modules = iter(module)
    cmd.run_check(f"sudo insmod /tmp/{module[0]}")
    cmd.run_check(f"sudo insmod /tmp/{module[1]}") ## spi

def test_probe_spi(cmd):
    stdout, stderr, ret = cmd.run(r"sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    ## output: (with hardware attached)
    # [ 1539.449651] adxl345_core_probe(): called
    # [ 1539.449704] adxl345_setup(): called
    # [ 1539.449713] adxl345_setup(): calling setup()
    # [ 1539.449722] adxl345_spi_setup(): called
    # [ 1539.449730] adxl345_setup(): retrieving DEVID
    # [ 1539.449788] adxl345_setup(): setting full range  ## with hardware
    # [ 1539.449816] adxl345_setup(): enable measurement
    assert 0 < len([m for m in stdout if r"adxl345_core_probe(): called" in m])
    assert 0 < len([m for m in stdout if r"adxl345_setup(): called" in m])
    assert 0 < len([m for m in stdout if r"adxl345_setup(): calling setup()" in m])
    assert 0 < len([m for m in stdout if r"adxl345_spi_setup(): called" in m])
    assert 0 < len([m for m in stdout if r"adxl345_setup(): retrieving DEVID" in m])
    ## if no hardware is attached, reading DEVID will result in an error
    #assert 0 < len([m for m in stdout if r"adxl345_setup(): setting full range" in m])
    #assert 0 < len([m for m in stdout if r"adxl345_setup(): enable measurement" in m])

def test_cleanup_lkm02(cmd):
    _unload_lkms(cmd)

## i2c

def test_modify_config_i2c(cmd):
    cmd.run_check(r"sudo cp /boot/config.txt /boot/config.txt.orig")
    dtfrag = dtbo[idx_i2c]
    dtfrag = dtfrag.split(".")[0]
    cmd.run_check(f"echo 'dtoverlay={dtfrag}' |sudo tee -a /boot/config.txt")
     ## sd card write delay...
    cmd.run_check(r"sync")
    import time
    time.sleep(3)

## reboot

def test_copy_lkm_i2c(target, shell_cmd):
    drv = target.get_driver("SSHDriver")
    dst = DST
    modules = iter(module)
    assert None != drv
    assert ":/tmp" == dst

    src = f"../{project}/{module[0]}"
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_core.ko' == src
    assert 0 == ret

    src = f"../{project}/{module[2]}"
    ret = drv.scp(src=src, dst=dst)
    assert '../iio-regmap__adxl345-accell-driver/adxl345_i2c.ko' == src
    assert 0 == ret

def test_turn_off_wifi_i2c(cmd):
    cmd.run_check("sudo killall wpa_supplicant")
    cmd.run_check("sudo ip link set wlan0 down")
    cmd.run_check("sudo systemctl stop dnsmasq")

def test_sudo_modprobe_industrialio02(cmd):
    cmd.run_check("sudo modprobe industrialio")

def test_sudo_modprobe_i2c_dev02(cmd):
    cmd.run_check("sudo modprobe i2c-dev")

def test_sudo_modprobe_regmap_i2c02(cmd):
    cmd.run_check("sudo modprobe regmap_i2c")

## in case some or all modules are already loaded, unload them first...
def test_cleanup_lkm03(cmd):
    _unload_lkms(cmd)

def test_load_lkm_i2c(cmd):
    cmd.run_check(f"sudo insmod /tmp/{module[0]}")
    cmd.run_check(f"sudo insmod /tmp/{module[2]}") ## i2c

def test_probe_i2c(cmd):
    stdout, stderr, ret = cmd.run(r"sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    ## output: (with hardware attached)
    # Mar 18 10:26:45 ctrl01 kernel: [  248.224525] adxl345_i2c_probe(): called
    # Mar 18 10:26:45 ctrl01 kernel: [  248.224644] adxl345_core_probe(): called
    # Mar 18 10:26:45 ctrl01 kernel: [  248.224675] adxl345_setup(): called
    # Mar 18 10:26:45 ctrl01 kernel: [  248.224685] adxl345_setup(): calling setup()
    # Mar 18 10:26:45 ctrl01 kernel: [  248.224693] adxl345_setup(): retrieving DEVID
    # Mar 18 10:26:45 ctrl01 kernel: [  248.225217] adxl345_i2c: probe of 1-001d failed with error -121
    assert 1 <= len([m for m in stdout if r"adxl345_i2c_probe(): called" in m])
    assert 1 <= len([m for m in stdout if r"adxl345_core_probe(): called" in m])
    assert 1 <= len([m for m in stdout if r"adxl345_setup(): called" in m])
    assert 1 <= len([m for m in stdout if r"adxl345_setup(): calling setup()" in m])
    assert 1 <= len([m for m in stdout if r"adxl345_setup(): retrieving DEVID" in m])
    ## if no hardware is attached, the read of DEVID will result in an error on the bus

def test_revert_config(shell_cmd):
    shell_cmd.run(r"sudo cp /boot/config.txt.orig /boot/config.txt")
    ## ignore not keeping permissions error due to FAT filesystem

def test_cleanup_overlays(cmd):
    for idx in range(len(dtbo)):
        cmd.run_check(f"sudo rm /boot/overlays/{dtbo[idx]}")

def test_cleanup_lkm04(cmd):
    _unload_lkms(cmd)
