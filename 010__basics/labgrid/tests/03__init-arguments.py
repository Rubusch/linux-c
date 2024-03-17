project = r"03__init-arguments"
module = r"hello.ko"
mod_arguments = r'hello_int_arg=76 hello_int_array=1,2,3 hello_string_arg="Hello"'

SRC = r"../" + project + "/" + module
DST = r":/tmp"
#KERNELVERSION = r"6.3.13" # TODO rm
KERNELVERSION = r"6.6.21"

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
    shell.run_check(r"sudo insmod /tmp/" + module + " " + mod_arguments)
    stdout, stderr, ret = shell.run(r"sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): initializing..." in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_int_arg = 76" in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_int_arg_cb = 0" in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_int_array[0] = 1" in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_int_array[1] = 2" in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_int_array[2] = 3" in m])
    assert 1 == len([m for m in stdout if r"init_hello_arguments(): hello_string_arg = 'Hello'" in m])

def test_unload_lkm(shell):
    shell.run_check(r"sudo rmmod " + module)
