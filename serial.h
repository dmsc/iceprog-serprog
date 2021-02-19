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

/*
 * serial.h: Linux serial port handling.
 */

#pragma once

/*  Reads "readcnt" bytes from serial port. */
int serialport_read(unsigned char *buf, unsigned int readcnt);

/*  Writes "writecnt" bytes from serial port. */
int serialport_write(const unsigned char *buf, unsigned int writecnt);

/*  Opens serial device and sets baud rate. */
int serialport_open(const char *dev, int baud);

/*  Closes serial port. */
void serialport_close(void);

/* Returns default serial device */
const char *serialport_get_default_device(void);
