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

/* Serial port functions for Windows */

#include <windows.h>
#include <stdio.h>

static HANDLE serial_hnd;

// Get a string with the last error message.
static const char *get_system_error()
{
    static char buffer[512];
    int e = GetLastError();
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      buffer, sizeof(buffer), NULL))
        return buffer;
    else
        return "unknown system error";
}

static int serialport_config(int baud)
{
    DCB serialParameters;
    COMMTIMEOUTS timeOut;

    ZeroMemory(&serialParameters, sizeof(DCB));
    serialParameters.DCBlength         = sizeof(DCB);
    serialParameters.BaudRate          = baud;
    serialParameters.Parity            = NOPARITY;
    serialParameters.fBinary           = 1;
    serialParameters.fParity           = 0;
    serialParameters.fOutxCtsFlow      = 0;
    serialParameters.fOutxDsrFlow      = 0;
    serialParameters.fDtrControl       = DTR_CONTROL_DISABLE;
    serialParameters.fDsrSensitivity   = 0;
    serialParameters.fTXContinueOnXoff = 0;
    serialParameters.fOutX             = 0;
    serialParameters.fInX              = 0;
    serialParameters.fErrorChar        = 0;
    serialParameters.fNull             = 0;
    serialParameters.fRtsControl       = RTS_CONTROL_DISABLE;
    serialParameters.fAbortOnError     = 0;
    serialParameters.fDummy2           = 0;
    serialParameters.wReserved         = 0;
    serialParameters.XonLim            = 0;
    serialParameters.XoffLim           = 0;
    serialParameters.ByteSize          = 8;
    serialParameters.StopBits          = ONESTOPBIT;
    serialParameters.XonChar           = 0;
    serialParameters.XoffChar          = 0;
    serialParameters.ErrorChar         = 0;
    serialParameters.EofChar           = 0;
    serialParameters.EvtChar           = 0;
    serialParameters.wReserved1        = 0;
    if( 0 == SetCommState(serial_hnd, &serialParameters) )
    {
        fprintf(stderr, "Can't set serial port parameters: %s\n", get_system_error());
        return 1;
    }

    ZeroMemory(&serialParameters, sizeof(COMMTIMEOUTS));
    timeOut.ReadIntervalTimeout         = 10;
    timeOut.ReadTotalTimeoutMultiplier  = 1;
    timeOut.ReadTotalTimeoutConstant    = 400;
    timeOut.WriteTotalTimeoutMultiplier = 1;
    timeOut.WriteTotalTimeoutConstant   = 100;
    if( 0 == SetCommTimeouts(serial_hnd, &timeOut) )
    {
        fprintf(stderr, "Can't set serial port timeouts: %s\n", get_system_error());
        return 1;
    }

    return 0;
}

int serialport_read(unsigned char *buf, unsigned int readcnt)
{
    unsigned int empty_reads = 10; /* results in a 5s timeout */
    DWORD tmp;
    while(readcnt > 0)
    {
        if (!ReadFile(serial_hnd, buf, readcnt, &tmp, 0))
        {
            fprintf(stderr, "Serial port read error: %s\n", get_system_error());
            return 1;
        }
        if (!tmp)
        {
            empty_reads--;
            if (empty_reads == 0) {
                fprintf(stderr,"Serial port is unresponsive!\n");
                return 1;
            }
            Sleep(100);
        }
        readcnt -= tmp;
        buf += tmp;
    }
    return 0;
}


int serialport_write(const unsigned char *buf, unsigned int writecnt)
{
    DWORD tmp;
    unsigned int empty_writes = 10; /* results in a ca. 10s timeout */

    while (writecnt > 0) {
        if (!WriteFile(serial_hnd, buf, writecnt, &tmp, 0))
        {
            fprintf(stderr, "Serial port write error: %s\n", get_system_error());
            return 1;
        }
        if (!tmp) {
            fprintf(stderr, "Empty write\n");
            empty_writes--;
            Sleep(100);
            if (empty_writes == 0) {
                fprintf(stderr,"Serial port is unresponsive!\n");
                return 1;
            }
        }
        writecnt -= tmp;
        buf += tmp;
    }

    if( !FlushFileBuffers(serial_hnd) )
        fprintf(stderr, "Serial port flush error: %s\n", get_system_error());

    return 0;
}


int serialport_open(const char *dev, int baud)
{
    // Open the device
    serial_hnd = CreateFile(TEXT(dev), GENERIC_READ | GENERIC_WRITE,
                            0,    // exclusive access
                            NULL, // default security attributes
                            OPEN_EXISTING, 0 /*FILE_FLAG_OVERLAPPED*/, NULL);
    if (serial_hnd == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error: cannot open serial port: %s\n", get_system_error());
        serial_hnd = 0;
        return 1;
    }

    if (serialport_config(baud) != 0)
    {
        CloseHandle(serial_hnd);
        return 1;
    }
    return 0;
}

void serialport_close(void)
{
    CloseHandle(serial_hnd);
}

const char *serialport_get_default_device(void)
{
    static char dev_default[64] = "COM1";
    int current_score = 0;
    char dev_path[4096];

    // Try all numbered serial ports and return the most appropriate name.
    for (int i=0; i<256; i++)
    {
        char dev_name[64];
        sprintf(dev_name, "COM%d", i);
        if( 0 != QueryDosDevice(dev_name, dev_path, sizeof(dev_path)) )
        {
            int score = 2;
            // Get score based on the device string
            if( !strncasecmp(dev_path, "\\device\\usbser", 14) )
                score = 4;
            else if( !strncasecmp(dev_path, "\\device\\usb", 11) )
                score = 3;
            else if( !strncasecmp(dev_path, "\\device\\serial", 14) )
                score = 1;
            if( current_score < score )
            {
                current_score = score;
                strcpy(dev_default, dev_name);
                fprintf(stderr,"%s: %s [%d]\n", dev_name, dev_path, score);
            }
        }
    }
    fprintf(stderr,"Selected device: %s\n", dev_default);
    return dev_default;
}
