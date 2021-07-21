# Makefile (simple)
#
# use -O, -O1, -O2 for optimization 


SOURCE = cat_noblock.c
OBJECTS = ${SOURCE:.c=.o}

.PREFIXES = .c .o
CC = gcc
CFLAGS = -g -Wall
RM = rm -f

.c.o:
	${CC} -c ${CFLAGS} $<

cat_noblock: ${OBJECTS}
	${CC} -o $@ ${OBJECTS} -lm

all: cat_noblock

clean: 
	${RM} ${OBJECTS} cat_noblock *~ core


