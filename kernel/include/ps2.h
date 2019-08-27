#ifndef PS2_H
#define PS2_H

#include <stdint.h>

#define PS2_CMD_PORT    0x64
#define PS2_STATUS_PORT 0x64
#define PS2_DATA_PORT   0x60

#define PS2_CMD_READ_CONFIG_BYTE    0x20
#define PS2_CMD_WRITE_CONFIG_BYTE   0x60

#define PS2_CMD_ENABLE_FIRST_PORT   0xAE
#define PS2_CMD_DISABLE_FIRST_PORT  0xAD
#define PS2_CMD_ENABLE_SECOND_PORT  0xA8
#define PS2_CMD_DISABLE_SECOND_PORT 0xA7

#define PS2_CMD_TEST_CONTROLLER     0xAA
#define PS2_CMD_TEST_FIRST_PORT     0xAB
#define PS2_CMD_TEST_SECOND_PORT    0xA9

#define PS2_CMD_WRITE_TO_SECOND     0xD4
#define PS2_CMD_DEVICE_RESET        0xFF

#define PS2_DEVCMD_DISABLE_SCANNING 0xF5
#define PS2_DEVCMD_ENABLE_SCANNING  0xF4
#define PS2_DEVCMD_IDENTIFY         0xF2

#define PS2_ACK                     0xFA
#define PS2_RESEND                  0xFE

void ps2Initialise();
int ps2DeviceSendData(uint8_t id, uint8_t data);
int ps2DeviceSendCmd(uint8_t id, uint8_t cmd);
void ps2DeviceEnableInterrupts(uint8_t id);

#endif