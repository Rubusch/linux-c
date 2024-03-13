project = r"01__hello"
module = r"hello.ko"
pattern1 = r"Hello World!"
pattern2 = r"Goodbye World!"

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
    shell.run_check(r"sudo insmod /tmp/" + module)
    stdout, stderr, ret = shell.run(r"sudo tail /var/log/messages")
    assert ret == 0
    assert 1 == len([m for m in stdout if pattern1 in m])

def test_unload_lkm(shell):
    shell.run_check(r"sudo rmmod " + module)
    stdout, stderr, ret = shell.run(r"sudo tail /var/log/messages")
    assert ret == 0
    assert 1 == len([m for m in stdout if pattern2 in m])
