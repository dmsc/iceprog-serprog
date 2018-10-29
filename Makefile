CFLAGS=-O2 -Wall

OBJS=\
     	serial.o \
	serprog.o \
	iceprog.o \

all: iceprog

iceprog: $(OBJS)
