/*
 *  iceprog -- simple programming tool for Lattice iCE FPGA
 *
 *  Copyright (C) 2015  Clifford Wolf <clifford@clifford.at>
 *  Copyright (C) 2018  Piotr Esden-Tempski <piotr@esden.net>
 *  Copyright (C) 2018  Daniel Serpell <daniel.serpell@gmail.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>

static int serial_fd;

#ifdef __linux

#include <sys/ioctl.h>
// Copied from Linux kernel headers, to support missing
// arbitrary baud rates in GLIBC.
#define LINUX_NCCS 19

struct linux_termios2
{
  tcflag_t c_iflag;           /* input mode flags */
  tcflag_t c_oflag;           /* output mode flags */
  tcflag_t c_cflag;           /* control mode flags */
  tcflag_t c_lflag;           /* local mode flags */
  cc_t c_line;                /* line discipline */
  cc_t c_cc[LINUX_NCCS];      /* control characters */
  speed_t c_ispeed;           /* input speed */
  speed_t c_ospeed;           /* output speed */
};

#define termios2 linux_termios2

// Simple wrappers
static int linux_tcsetattr(int fd, int optional_actions, const struct linux_termios2 *t)
{
 return ioctl(fd, TCSETS2, t);
}

#if 0
static int linux_tcgetattr(int fd, const struct linux_termios2 *t)
{
 return ioctl(fd, TCGETS2, t);
}
#endif

static int serialport_config(int baud)
{
    struct termios2 tty;
    memset(&tty, 0, sizeof tty);

    tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
    tty.c_cflag |= (CS8 | CLOCAL | CREAD);
    tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = -1;

    tty.c_cflag |= CBAUDEX;
    tty.c_ispeed = tty.c_ospeed = baud;

    if (linux_tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

#endif // __linux

int serialport_read(unsigned char *buf, unsigned int readcnt)
{
        ssize_t tmp = 0;

        while (readcnt > 0) {
                tmp = read(serial_fd, buf, readcnt);
                if (tmp == -1) {
                        fprintf(stderr, "Serial port read error!\n");
                        return 1;
                }
                if (!tmp)
                        fprintf(stderr, "Empty read\n");
                readcnt -= tmp;
                buf += tmp;
        }

        return 0;
}


int serialport_write(const unsigned char *buf, unsigned int writecnt)
{
        ssize_t tmp = 0;
        unsigned int empty_writes = 250; /* results in a ca. 125ms timeout */

        while (writecnt > 0) {
                tmp = write(serial_fd, buf, writecnt);
                if (tmp == -1) {
                        fprintf(stderr, "Serial port write error!\n");
                        return 1;
                }
                if (!tmp) {
                        fprintf(stderr, "Empty write\n");
                        empty_writes--;
                        usleep(500);
                        if (empty_writes == 0) {
                                fprintf(stderr,"Serial port is unresponsive!\n");
                                return 1;
                        }
                }
                writecnt -= tmp;
                buf += tmp;
        }

        return 0;
}


int serialport_open(const char *dev, int baud)
{
        serial_fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY); // Use O_NDELAY to ignore DCD state
        if (serial_fd < 0) {
                fprintf(stderr, "Error: cannot open serial port: %s\n", strerror(errno));
                return 1;
        }

        /* Ensure that we use blocking I/O */
        const int flags = fcntl(serial_fd, F_GETFL);
        if (flags == -1) {
                fprintf(stderr, "Error: cannot set serial port mode: %s\n", strerror(errno));
                goto err;
        }
        if (fcntl(serial_fd, F_SETFL, flags & ~O_NONBLOCK) != 0) {
                fprintf(stderr, "Error: cannot set serial port to blocking: %s\n", strerror(errno));
                goto err;
        }
        if (serialport_config(baud) != 0) {
                goto err;
        }
        return 0;
err:
        close(serial_fd);
        return 1;
}

void serialport_close(void)
{
    close( serial_fd );
}

const char *serialport_get_default_device(void)
{
    static char dev_default[64] = "/dev/ttyACM0";
    return dev_default;
}
