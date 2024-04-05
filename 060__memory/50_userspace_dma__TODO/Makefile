SOURCE = sdma.c
OBJECTS = $(SOURCE:.c=.o)
TARGET = $(SOURCE:.c=.elf)

.PREFIXES = .c .o
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall
            
KERNELDIR ?= /usr/src/linux
obj-m += dma_demo.o

## dtbo-y seems to be broken (again...?)
#dtbo-y += dma_demo_overlay.dtbo
DTSFILES += dma_demo_overlay.dts
DTBOFILES := $(DTSFILES:.dts=.dtbo)
DTC ?= dtc

%.dtbo: %.dts
	$(DTC) -O dtb -o $@ $<

%.c%.o:
	$(CC) $(CFLAGS) -c $<

all: $(DTBOFILES) $(TARGET)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	find $(SRC) -type f -name \*~ -delete
	rm -f $(TARGET) $(OBJECTS) *~

.PHONY: all clean
