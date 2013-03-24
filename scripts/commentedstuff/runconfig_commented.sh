#=+cch========================================================================#
#  NAME     : runconfig.sh                                                    #
#  FUNCTION :                                                                 #
#-----------------------------------------------------------------------------#
#  (C) Copyright 2005 Nokia Siemens Networks                                  #
#-----------------------------------------------------------------------------#
#  AUTHOR   : pt101493                                                        #
#  CREATED  : 2005-11-03                                                      #
#========================================================================cch+=#
# vob directory
export VOB_cots_linux_appl=/vobs/cots/linux_appl
export VOB_cxu=/hix/gpon_cxu_f
export VOB_ramdisk=/hix/gpon_ramdisk
export VOB_kernel=/vobs/cots/linux-2.4
export VOB_gpon_kernel=/hix/gpon_kernel-2.4
export VOB_dlv=/hix/gpon_cxu_bld
export VOB_uboot=/vobs/cots/uboot
export VOB_sdk=/vobs/cots/broadcom/sdk-5.4
export VOB_gpon_bld=/hix/gpon_cxu_bld

# directory for load output
export DLV_LOAD=${VOB_dlv}/cxu_f/load
export DLV_TMP=${VOB_gpon_bld}/cxu_f/tmp

# eldk environment
export ELDK_ARCH=ppc_82xx
export ELDK_VER=2.1
export ELDK_ROOT=/vobs/cots/eldk/eldk${ELDK_VER}
export ELDK_KERNEL_INCLUDE=2.95.4

# cross compiler: gcc-(?) ppc_82xx-
export CROSS_COMPILE=$ELDK_ARCH-

# uboot settings
export UBOOT_VER=u-boot-1.1

# load settings
export THIS_ROUTER_OS_VERSION=gpon-r2.0.3-cxu_f-o

# settings kernel image
export KERNEL_VER=linux-2.4
export KERNEL_RD_IMG=${DLV_TMP}/ramdisk.image.gz
export SRCDIR_linux=${VOB_cxu}/os/${KERNEL_VER}
export KERNEL_TARGET_TYPE=nos

# settings ramdisk image
export RAMDISK_SIZE=30000
export RAMDISK=${DLV_TMP}/ramdisk
#export RAMDISK_FREE_BLOCKS=2000
export RAMDISK2=${DLV_TMP}/ramdisk2
export RAMDISK3=${DLV_TMP}/ramdisk3
export RAMDISK_FREE_BLOCKS=200


