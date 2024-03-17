project = r"iio-regmap__adxl345-accell-driver"
module = ["adxl345_core.ko", "adxl345_i2c.ko", "adxl345_spi.ko"]
dtbo = r"input_demo_overlay.dtbo"

#SRC = r"../" + project + "/" + module
DST = r":/tmp"
#DST = r":/home/pi"
KERNELVERSION = r"6.6.21"

def test_revert_config(shell):
    shell.run(r"sudo cp /boot/config.txt.orig /boot/config.txt")
    ## ignore not keeping permissions error due to FAT filesystem

#def test_copy_lkm(sshdriver):
def test_copy_lkm(target):
    drv = target.get_driver("SSHDriver")
    dst = DST

    #dst = ":/home/pi"
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

def test_turn_off_wifi(command):
    command.run_check("sudo killall wpa_supplicant")
    command.run_check("sudo ip link set wlan0 down")
    command.run_check("sudo systemctl stop dnsmasq")

#def test_sudo_modprobe_industrialio(shell):
#    shell.run_check("sudo modprobe industrialio")
#
#def test_sudo_modprobe_regmap_spi(shell):
#    shell.run_check("sudo modprobe regmap_spi")
#
#def test_load_lkm(shell):
#    modules = iter(module)
#    #shell.run_check(r"sudo insmod /home/pi/" + next(modules))
#    #shell.run_check(r"sudo insmod /home/pi/" + next(modules))
#    #shell.run_check(r"sudo insmod /home/pi/" + next(modules))
#    shell.run_check(r"sudo insmod /tmp/" + next(modules))
#    shell.run_check(r"sudo insmod /tmp/" + next(modules))
#    shell.run_check(r"sudo insmod /tmp/" + next(modules))
#
#def test_probe(shell):
#    stdout, stderr, ret = shell.run(r"sudo tail -n 50 /var/log/messages")
#    assert 0 == ret
#    #assert 1 == len([m for m in stdout if r"init_hello_arguments(): initializing..." in m])
#
#def test_unload_lkm(shell):
#    modules = iter(module.reverse())
#    shell.run_check(r"sudo rmmod " + next(modules))
#    shell.run_check(r"sudo rmmod " + next(modules))
#    shell.run_check(r"sudo rmmod " + next(modules))
