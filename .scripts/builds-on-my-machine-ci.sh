#!/bin/bash -e

verify()
{
    local DIR="$1"
    local LIST=$(find "$DIR" -name Makefile -exec dirname {} \; | grep -v 'TODO')
    for item in ${LIST[*]}; do
	echo "XXX building '$item'..."
	pushd "$item" &> /dev/null
	make clean
	make -j $(nproc)
	make clean
	popd &> /dev/null
	echo "XXX building '$item' - OK"
	echo
    done
}

## main ##
verify "../010__basics"
verify "../030__platform-devices"
verify "../040__simple-busses"
#verify "../050__usb"
verify "../060__memory"
#verify "../070__interrupts"
verify "../080__kthreads"

echo "READY."
