TARGET      = iceprog
PKG         = $(TARGET)

CC         ?= gcc
STRIP      ?= strip
INSTALL    ?= install
PREFIX     ?= /usr
BINDIR     ?= $(PREFIX)/bin
CFLAGS     ?= -O2 -Wall
LDFLAGS    ?= 
WARNERROR  ?= no

ifeq ($(WARNERROR), yes)
CFLAGS += -Werror
endif

ifdef LIBS_BASE
CFLAGS += -I$(LIBS_BASE)/include
LDFLAGS += -L$(LIBS_BASE)/lib -Wl,-rpath -Wl,$(LIBS_BASE)/lib
endif

ifeq ($(CONFIG_STATIC),yes)
LDFLAGS += -static
endif

ifeq ($(TARGET_OS), MinGW)
EXEC_SUFFIX := .exe
CFLAGS += -posix
CFLAGS += -Dffs=__builtin_ffs
CFLAGS += -D__USE_MINGW_ANSI_STDIO=1
LDFLAGS += -ltermiwin
endif

SRCS = serial.c serprog.c iceprog.c

$(PKG): $(TARGET)$(EXEC_SUFFIX)

$(TARGET)$(EXEC_SUFFIX): $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)$(EXEC_SUFFIX)

strip: $(TARGET)$(EXEC_SUFFIX)
	$(STRIP) $(TARGET)$(EXEC_SUFFIX)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)/etc/udev/rules.d
	$(INSTALL) -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0664 40-persistent-serprog.rules $(DESTDIR)/etc/udev/rules.d/40-persistent-serprog.rules

install-udev-rule:
	cp 40-persistent-serprog.rules /etc/udev/rules.d/
	udevadm control --reload-rules
.PHONY: clean install-udev-rule

debian/changelog:
	dch --create -v 0.1-1 --package $(PKG)

deb:
	dpkg-buildpackage -b -us -uc
.PHONY: install debian/changelog deb
