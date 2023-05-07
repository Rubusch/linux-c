SOURCE = ioctl_test.c
OBJECTS = $(SOURCE:.c=.o)
TARGET = $(SOURCE:.c=.elf)

.PREFIXES = .c .o
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall

%.c%.o:
	$(CC) $(CFLAGS) -c $<

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm $(TARGET) $(OBJECTS) *~

.PHONY: all clean
