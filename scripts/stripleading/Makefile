#
# Makefile for GCOV profiling kernel module
#

O_TARGET			:= gcov.o

export-objs			+= gcov-core.o

obj-$(CONFIG_GCOV_PROFILE)	+= gcov-core.o
obj-$(CONFIG_GCOV_PROC)		+= gcov-proc.o

CFLAGS_gcov-core.o := -DGCOV_SRC_PATH='"$(TOPDIR)"' \
		      -DGCOV_OBJ_PATH='"$(TOPDIR)"'

include $(TOPDIR)/Rules.make
