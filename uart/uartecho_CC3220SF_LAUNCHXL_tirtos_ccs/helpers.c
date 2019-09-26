/*
 * helpers.c
 *
 *  Created on: 2019?6?25?
 *      Author: JUNHU2
 */

#include <helpers.h>
#include <stdint.h>
#include <stddef.h>

unsigned char translateHex2Text(unsigned char data);
unsigned char translateHex2Text(unsigned char data) {
    if (data <= 9) {
        return '0'+data;
    }
    if (data <= 15 && data>9) {
        return 'A'+(data-10);
    }
    return '?';
}


unsigned char *translateBin2Text(unsigned char data, unsigned char *buffer);
unsigned char *translateBin2Text(unsigned char data, unsigned char *buffer) {
    *buffer++ = '0';
    *buffer++ = 'x';
    *buffer++ = translateHex2Text((data>>4)&0x0F);
    *buffer++ = translateHex2Text(data&0x0F);
    *buffer++ = '<';
    if ((data >= 32) && (data <= 127)) {
        *buffer++ = data;
    } else {
        *buffer++ = '.';
    }
    *buffer++ = '>';
    *buffer++ = ' ';
    return buffer;
}

