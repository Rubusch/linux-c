def test_copy_adxl345_spi(shell, target):
    drv = target.get_driver("SSHDriver")
    src = '../../../../040__bus/iio-regmap__adxl345-accell-driver/adxl345_spi.ko'
    dst = ':/home/pi'

    res = drv.scp(src=src, dst=dst)
    if res:
        exc = InteractiveCommandError("scp error: adxl345_spi.ko")
        exc.exitcode = res
        raise exc

