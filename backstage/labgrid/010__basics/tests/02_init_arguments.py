project = r"02__init-arguments"
module = r"hello.ko"
mod_arguments = r'mystring="foobar" myshort=255 myintArray=1'

def test_preparation(shell):
    stdout, stderr, ret = shell.run("uname -r")
    assert 0 == ret
    assert "6.3.13" in stdout[0]

def test_copy_lkm(target):
    drv = target.get_driver("SSHDriver")
    src = r"../../../010__basics/" + project + "/" + module
    dst = r":/tmp"
    ret = drv.scp(src=src, dst=dst)
    assert 0 == ret

def test_load_lkm(shell):
    shell.run_check(r"sudo insmod /tmp/" + module + " " + mod_arguments)
    stdout, stderr, ret = shell.run(r"sudo tail -n 50 /var/log/messages")
    assert ret == 0
    assert 1 == len([m for m in stdout if r"myshort is a short integer: 255" in m])
    assert 1 == len([m for m in stdout if r"myint is a integer: 420" in m])
    assert 1 == len([m for m in stdout if r"mylong is a long integer: 9999" in m])
    assert 1 == len([m for m in stdout if r"mystring is a string: foobar" in m])
    assert 1 == len([m for m in stdout if r"myintArray[0] = 1" in m])
    assert 1 == len([m for m in stdout if r"myintArray[1] = -1" in m])
    assert 1 == len([m for m in stdout if r"got 1 arguments for myintArray." in m])

def test_unload_lkm(shell):
    shell.run_check(r"sudo rmmod " + module)
