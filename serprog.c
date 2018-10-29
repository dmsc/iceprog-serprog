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
 * serprog.c: Flashrom "serprog" protocol.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "serial.h"
#include "serprog.h"

/* According to Serial Flasher Protocol Specification - version 1 */
#define S_ACK 0x06
#define S_NAK 0x15
#define S_CMD_NOP		0x00	/* No operation					*/
#define S_CMD_Q_IFACE		0x01	/* Query interface version			*/
#define S_CMD_Q_CMDMAP		0x02	/* Query supported commands bitmap		*/
#define S_CMD_Q_PGMNAME		0x03	/* Query programmer name			*/
#define S_CMD_Q_SERBUF		0x04	/* Query Serial Buffer Size			*/
#define S_CMD_Q_BUSTYPE		0x05	/* Query supported bustypes			*/
#define S_CMD_Q_CHIPSIZE	0x06	/* Query supported chipsize (2^n format)	*/
#define S_CMD_Q_OPBUF		0x07	/* Query operation buffer size			*/
#define S_CMD_Q_WRNMAXLEN	0x08	/* Query Write to opbuf: Write-N maximum length */
#define S_CMD_R_BYTE		0x09	/* Read a single byte				*/
#define S_CMD_R_NBYTES		0x0A	/* Read n bytes					*/
#define S_CMD_O_INIT		0x0B	/* Initialize operation buffer			*/
#define S_CMD_O_WRITEB		0x0C	/* Write opbuf: Write byte with address		*/
#define S_CMD_O_WRITEN		0x0D	/* Write to opbuf: Write-N			*/
#define S_CMD_O_DELAY		0x0E	/* Write opbuf: udelay				*/
#define S_CMD_O_EXEC		0x0F	/* Execute operation buffer			*/
#define S_CMD_SYNCNOP		0x10	/* Special no-operation that returns NAK+ACK	*/
#define S_CMD_Q_RDNMAXLEN	0x11	/* Query read-n maximum length			*/
#define S_CMD_S_BUSTYPE		0x12	/* Set used bustype(s).				*/
#define S_CMD_O_SPIOP		0x13	/* Perform SPI operation.			*/
#define S_CMD_S_SPI_FREQ	0x14	/* Set SPI clock frequency			*/
#define S_CMD_S_PIN_STATE	0x15	/* Enable/disable output drivers		*/

static int sp_docommand(uint8_t command, uint32_t parmlen,
                        uint8_t *params, uint32_t retlen, void *retparms)
{
        unsigned char c;
        if (serialport_write(&command, 1) != 0) {
                fprintf(stderr, "Error: cannot write op code: %s\n", strerror(errno));
                return 1;
        }
        if (serialport_write(params, parmlen) != 0) {
                fprintf(stderr, "Error: cannot write parameters: %s\n", strerror(errno));
                return 1;
        }
        if (serialport_read(&c, 1) != 0) {
                fprintf(stderr, "Error: cannot read from device: %s\n", strerror(errno));
                return 1;
        }
        if (c == S_NAK)
                return 1;
        if (c != S_ACK) {
                fprintf(stderr, "Error: invalid response 0x%02X from device (to command 0x%02X)\n", c, command);
                return 1;
        }
        if (retlen) {
                if (serialport_read(retparms, retlen) != 0) {
                        fprintf(stderr, "Error: cannot read return parameters: %s\n", strerror(errno));
                        return 1;
                }
        }
        return 0;
}

int serprog_spi_send_command(unsigned int writecnt, unsigned int readcnt,
                             const unsigned char *writearr, unsigned char *readarr)
{
        unsigned char *parmbuf;
        int ret;

        parmbuf = malloc(writecnt + 6);
        if (!parmbuf) {
                fprintf(stderr, "Error: could not allocate SPI send param buffer.\n");
                return 1;
        }
        parmbuf[0] = (writecnt >> 0) & 0xFF;
        parmbuf[1] = (writecnt >> 8) & 0xFF;
        parmbuf[2] = (writecnt >> 16) & 0xFF;
        parmbuf[3] = (readcnt >> 0) & 0xFF;
        parmbuf[4] = (readcnt >> 8) & 0xFF;
        parmbuf[5] = (readcnt >> 16) & 0xFF;
        memcpy(parmbuf + 6, writearr, writecnt);
        ret = sp_docommand(S_CMD_O_SPIOP, writecnt + 6, parmbuf, readcnt, readarr);
        free(parmbuf);
        return ret;
}

unsigned serprog_spi_set_clock(unsigned clock_hz)
{
    uint8_t buf[4];
    buf[0] = clock_hz;
    buf[1] = clock_hz >> 8;
    buf[2] = clock_hz >> 16;
    buf[3] = clock_hz >> 24;
    if( !sp_docommand(S_CMD_S_SPI_FREQ, 4, buf, 4, buf) )
        return buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24);

    fprintf(stderr,"Error, can't set SPI frequency\n");
    return 0;
}

void enable_prog()
{
    uint8_t c = 1;
    if( sp_docommand(S_CMD_S_PIN_STATE, 1, &c, 0, 0) )
        fprintf(stderr,"Error, can't enable prog\n");
}

void disable_prog()
{
    uint8_t c = 0;
    if( sp_docommand(S_CMD_S_PIN_STATE, 1, &c, 0, 0) )
        fprintf(stderr,"Error, can't disable prog\n");
}
