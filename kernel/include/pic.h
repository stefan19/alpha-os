#ifndef PIC_H
#define PIC_H

#include "stdint.h"

void picInit(uint8_t port1, uint8_t port2);
void picMask(uint8_t intLine);
void picUnmask(uint8_t intLine);
void picAck(uint8_t intLine);

#endif