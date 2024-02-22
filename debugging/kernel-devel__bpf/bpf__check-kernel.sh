#!/bin/sh
## reference:
## https://amedee.me/2020/06/17/bpf-on-ubuntu-20/
bootconfig="/boot/config-$(uname -r)"
while read -r flag;
do
  if ! grep -q "$flag" $bootconfig; then
      echo "Kernel flag \"$flag\" is missing in $bootconfig"
	    fi 
		done << __EOM
		CONFIG_BPF=y
		CONFIG_BPF_SYSCALL=y
		CONFIG_BPF_JIT=y
		CONFIG_HAVE_EBPF_JIT=y
		CONFIG_BPF_EVENTS=y
		CONFIG_FTRACE_SYSCALLS=y
		CONFIG_BPF=y
		CONFIG_HAVE_EBPF_JIT=y
		CONFIG_BPF_EVENTS=y
		CONFIG_UPROBE_EVENTS=y
		CONFIG_KPROBE_EVENTS=y
		__EOM
echo "READY."
