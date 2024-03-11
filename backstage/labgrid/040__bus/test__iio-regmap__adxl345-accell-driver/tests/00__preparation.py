def test_preparation(shell):
    pass

def test_copy_dt(target):
    drv = target.get_driver("SSHDriver")
    src = '../../../../040__bus/iio-regmap__adxl345-accell-driver/input_demo_overlay.dtbo'
    dst = ':/home/pi'
    res = drv.scp(src=src, dst=dst)
    if res:
        exc = InteractiveCommandError("scp error: input_demo_overlay.dtbo")
        exc.exitcode = res
        raise exc

def test_mv_dt(shell):
    shell.run_check(r"sudo mv input_demo_overlay.dtbo /boot/overlays/input_demo_overlay.dtbo");

def test_sync(shell):
    shell.run_check("sync")
