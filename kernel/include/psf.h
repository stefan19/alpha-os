#ifndef PSF_H
#define PSF_H

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

#include <stdint.h>

typedef struct {
        uint8_t magic[2];     /* Magic number */
        uint8_t mode;         /* PSF font mode */
        uint8_t charsize;     /* Character size */
} psf1_header;

void psfOpen();
void psfRender(uint32_t x, uint32_t y, uint16_t c, uint32_t bg, uint32_t fg);
void psfGetCharSize(uint32_t* width, uint32_t* height);

#endif