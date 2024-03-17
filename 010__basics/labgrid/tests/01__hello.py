project = r"01__hello"
module = r"hello.ko"

#KERNELVERSION = r"6.3.13" # TODO rm
KERNELVERSION = r"6.6.21"
SRC = r"../" + project + "/" + module
DST = r":/tmp/"

def test_preparation(shell):
    stdout, stderr, ret = shell.run("uname -r")
    assert 0 == ret
    assert KERNELVERSION in stdout[0]

def test_copy_lkm(target):
    drv = target.get_driver("SSHDriver")
    src = SRC
    dst = DST
    ret = drv.scp(src=src, dst=dst)
    assert 0 == ret

def test_load_lkm(shell):
    shell.run_check(r"sudo insmod /tmp/" + module)
    stdout, stderr, ret = shell.run(r"sudo tail -n 50 /var/log/messages")
    assert ret == 0
    assert 1 <= len([m for m in stdout if r"Hello World!" in m])

def test_unload_lkm(shell):
    shell.run_check(r"sudo rmmod " + module)
    stdout, stderr, ret = shell.run(r"sudo tail -n 50 /var/log/messages")
    assert ret == 0
    assert 1 <= len([m for m in stdout if r"Goodbye World!" in m])
