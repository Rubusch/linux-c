SOURCE = gpio-demo.c
OBJECTS = $(SOURCE:.c=.o)
TARGET = $(SOURCE:.c=.elf)

.PREFIXES = .c .o
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall -lgpiod

%.c%.o:
	$(CC) $(CFLAGS) -c $<

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lgpiod

clean:
	rm -f $(TARGET) $(OBJECTS) *~

.PHONY: all clean
