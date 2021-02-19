CFLAGS=-O2 -Wall

OBJS=\
     	serial.o \
	serprog.o \
	iceprog.o \

all: iceprog

iceprog: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Dependencies
iceprog.o: iceprog.c serprog.h serial.h
serial.o: serial.c serial-lnx.c serial-w32.c
serial-lnx.o: serial-lnx.c
serial-w32.o: serial-w32.c
serprog.o: serprog.c serial.h serprog.h
