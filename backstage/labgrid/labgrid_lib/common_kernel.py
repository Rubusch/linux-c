def announce():
    print("Imported!")

def do_stat(arg_project, arg_files):
    import errno, os, sys
    import os.path
    for filename in arg_files:
        ret = os.path.isfile(f"../{arg_project}/{filename}")
        if not ret:
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), filename)

def do_login_check(cmd, arg_kernelversion):
    stdout, stderr, ret = cmd.run("uname -r")
    assert 0 == ret
    assert arg_kernelversion in stdout[0]

def do_copy_lkms(cmd, target, arg_mods, arg_project):
    drv = target.get_driver("SSHDriver")
    assert None != drv
    dst = r":/tmp"
    for mod in arg_mods:
        src = f"../{arg_project}/{mod}"
        ret = drv.scp(src=src, dst=dst)
        assert 0 == ret
    cmd.run_check("sync")
    import time
    time.sleep(3)

def do_load_lkms(cmd, arg_mods):
    for mod in arg_mods:
        cmd.run_check(f"sudo insmod /tmp/{mod}")

def do_load_lkms_and_args(cmd, arg_mods, arg_modargs):
    for mod in arg_mods:
        cmd.run_check(f"sudo insmod /tmp/{mod} {arg_modargs}")

def undo_load_lkms(cmd, arg_mods):
    for mod in list(reversed(arg_mods)):
        mod = mod.split(".")[0]
        cmd.run(f"sudo rmmod {mod}")

def do_copy_dtbo(target, cmd, arg_dtbos, arg_project):
    drv = target.get_driver("SSHDriver")
    for dtbo in arg_dtbos:
        src = f"../{arg_project}/{dtbo}"
        dst = f":/tmp"
        ret = drv.scp(src=src, dst=dst)
        assert 0 == ret
        cmd.run_check(f"sudo mv /tmp/{dtbo} /boot/overlays/")
    cmd.run_check("sync")
    import time
    time.sleep(3)

def undo_copy_dtbo(cmd, arg_dtbos):
    for dtbo in arg_dtbos:
        cmd.run_check(f"sudo rm /boot/overlays/{dtbo}")

def do_register_dtbo(cmd, arg_dtbo):
    cmd.run_check(r"sudo cp /boot/config.txt /boot/config.txt.orig")
    dtfrag = arg_dtbo
    dtfrag = dtfrag.split(".")[0]
    cmd.run_check(f"echo 'dtoverlay={dtfrag}' |sudo tee -a /boot/config.txt")
    ## sd card write delay...
    cmd.run_check(r"sync")
    import time
    time.sleep(3)

def undo_register_dtbo(cmd):
    cmd.run(r"sudo cp /boot/config.txt.orig /boot/config.txt")
    ## sd card write delay...
    cmd.run_check(r"sync")
    import time
    time.sleep(3)

def do_log_verification(cmd, arg_patterns):
    stdout, stderr, ret = cmd.run(r"sudo tail -n 50 /var/log/messages")
    assert 0 == ret
    for pattern in arg_patterns:
        assert 0 < len([m for m in stdout if pattern in m])
