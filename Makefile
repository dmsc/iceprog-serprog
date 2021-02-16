TARGET      = iceprog
PKG         = $(TARGET)

CC         ?= gcc
STRIP      ?= strip
INSTALL    ?= install
PREFIX     ?= /usr
BINDIR     ?= $(PREFIX)/bin
CFLAGS     ?= -O2 -Wall
LDFLAGS    ?= 

SRCS = serial.c serprog.c iceprog.c

ifdef LIBS_BASE
CFLAGS += -I$(LIBS_BASE)/include
LDFLAGS += -L$(LIBS_BASE)/lib -Wl,-rpath -Wl,$(LIBS_BASE)/lib
endif

ifeq ($(CONFIG_STATIC), yes)
LDFLAGS += -static
endif

$(PKG): $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET)

strip: $(TARGET)
	$(STRIP) $(TARGET)

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
