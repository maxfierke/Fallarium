/**************************************\
* crc.h                                *
* CRC functions from the 05 Jul 1997   *
* release of SNIPPETS                  *
\**************************************/

#ifndef CRC__H
#define CRC__H


#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 *  File: CRC_32.C
 */
#define UPDC32(octet,crc) (crc_32_tab[((crc) ^ ((uint8_t)octet)) & 0xff] ^ ((crc) >> 8))

uint32_t updateCRC32(unsigned char ch, uint32_t crc);
bool crc32file(const char *name, uint32_t *crc, long *charcnt);
uint32_t crc32buf(const char *buf, size_t len);

#endif /* CRC__H */
