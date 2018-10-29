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
 * serprog.h: Flashrom "serprog" protocol.
 */

#pragma once

/* Transmit "writecnt" bytes and then receives "readcnt" bytes from SPI */
int serprog_spi_send_command(unsigned int writecnt, unsigned int readcnt,
                             const unsigned char *writearr, unsigned char *readarr);

/* Set SPI clock, in Hz, returns actual speed. */
unsigned serprog_spi_set_clock(unsigned clock_hz);

/* Detect serprog programmer. */
int serprog_detect();

/* Enable SPI programmer. */
void enable_prog();

/* Disable SPI programmer - puts pins in input mode. */
void disable_prog();
