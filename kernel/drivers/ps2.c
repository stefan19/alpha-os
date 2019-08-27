#include "ps2.h"
#include "ll_io.h"
#include "timer.h"
#include "devmgr.h"

#define TIMEOUT_TIME    5000000

uint8_t ps2ReadRequest(uint8_t cmd);
void ps2WriteRequest(uint8_t cmd, uint8_t aux);

int ps2FirstPortSendData(uint8_t data);
int ps2SecondPortSendData(uint8_t data);

int ps2ResetDevice(uint8_t id);
int ps2Identify(uint8_t id, uint8_t* signature);

uint8_t ps2_first_present = 0;
uint8_t ps2_second_present = 0;

void ps2Initialise()
{
    //Disable devices
    outb(PS2_CMD_PORT, PS2_CMD_DISABLE_FIRST_PORT);
    outb(PS2_CMD_PORT, PS2_CMD_DISABLE_SECOND_PORT);

    //Flush the output buffer
    if(inb(PS2_STATUS_PORT) & 0x1)
    {
        inb(PS2_DATA_PORT);
    }

    //Set the controller configuration byte
    uint8_t cfg = ps2ReadRequest(PS2_CMD_READ_CONFIG_BYTE);

    //Disable interrupts and translation
    cfg &= ~1;
    cfg &= ~2;
    cfg &= ~(1 << 6);

    uint8_t second_port = 1;
    if( (cfg & (1 << 5)) == 0)
        second_port = 0;        //The second port definitely doesn't exist

    ps2WriteRequest(PS2_CMD_WRITE_CONFIG_BYTE, cfg);

    //Perform controller self test
    uint8_t res = ps2ReadRequest(PS2_CMD_TEST_CONTROLLER);
    if(res != 0x55)
    {
        printf("PS/2 Controller error: self-test failed");
        return;
    }
    ps2WriteRequest(PS2_CMD_WRITE_CONFIG_BYTE, cfg);

    //Check second channel
    if(second_port)
    {
        outb(PS2_CMD_PORT, PS2_CMD_ENABLE_SECOND_PORT);

        cfg = ps2ReadRequest(PS2_CMD_READ_CONFIG_BYTE);
        if(cfg & (1 << 5))
            second_port = 0;

        if(second_port)
            outb(PS2_CMD_PORT, PS2_CMD_DISABLE_SECOND_PORT);
    }

    //Perform interface tests
    //Test the first port
    res = ps2ReadRequest(PS2_CMD_TEST_FIRST_PORT);
    if(res != 0)
    {
        printf("PS/2 controller error: first port test failed: %x", res);
        return;
    }

    //Test the second port if it exists
    if(second_port)
    {
        res = ps2ReadRequest(PS2_CMD_TEST_SECOND_PORT);
        if(res != 0)
        {
            printf("PS/2 controller error: second port test failed: %x", res);
            return;
        }
    }

    //Enable existing ports
    outb(PS2_CMD_PORT, PS2_CMD_ENABLE_FIRST_PORT);
    if(second_port)
        outb(PS2_CMD_PORT, PS2_CMD_ENABLE_SECOND_PORT);

    //Reset existing devices
    if(ps2ResetDevice(0) == 0)
        ps2_first_present = 1;
    if(second_port)
    {
        if(ps2ResetDevice(1) == 0)
            ps2_second_present = 2;
    }
    ps2_first_present = ps2_second_present = 1;

    uint8_t signature[2];
    int signatureLength;
    //Identify valid devices
    if(ps2_first_present)
    {
        signatureLength = ps2Identify(0, signature);
        if(signatureLength < 0)
        {
            printf("Identification error");
            return;
        }
        if(signatureLength == 2 && signature[0] == 0xAB && signature[1] == 0x83)
        {
            generic_device_info_t kbd;
            kbd.id = 0;
            kbd.device_type = DEVICE_KEYBOARD;
            strcpy(kbd.name, "ps2kbd0");
            kbd.read = NULL;

            devmgrAddDevice(&kbd);
        }
    }
    if(ps2_second_present)
    {
        signatureLength = ps2Identify(1, signature);
        if(signatureLength < 0)
        {
            printf("Identification error");
            return;
        }
        if(signatureLength == 2 && signature[0] == 0xAB && signature[1] == 0x83)
        {
            generic_device_info_t kbd;
            kbd.id = 0;
            kbd.device_type = DEVICE_KEYBOARD;
            strcpy(kbd.name, "ps2kbd1");
            kbd.read = NULL;

            devmgrAddDevice(&kbd);
        }
    }
}

uint8_t ps2ReadRequest(uint8_t cmd)
{
    outb(PS2_CMD_PORT, cmd);

    //Wait for the response byte
    while( (inb(PS2_STATUS_PORT) & 0x1) == 0);

    return inb(PS2_DATA_PORT);
}

void ps2WriteRequest(uint8_t cmd, uint8_t aux)
{
    outb(PS2_CMD_PORT, cmd);

    //Wait before sending the next byte
    while(inb(PS2_STATUS_PORT) & 0x2);

    outb(PS2_DATA_PORT, aux);
}

int ps2DeviceSendData(uint8_t id, uint8_t data)
{
    if(id == 1)
        outb(PS2_CMD_PORT, PS2_CMD_WRITE_TO_SECOND);

    uint64_t timeoutTime = timerGetNano() + TIMEOUT_TIME;
    while( (inb(PS2_STATUS_PORT) & 0x2) && timerGetNano() < timeoutTime);

    if(timerGetNano() >= timeoutTime)
        return -1;

    outb(PS2_DATA_PORT, data);
    return 0;
}

int ps2ResetDevice(uint8_t id)
{
    //Send RESET command
    ps2DeviceSendData(id, PS2_CMD_DEVICE_RESET);

    uint64_t timeoutTime = timerGetNano() + TIMEOUT_TIME;
    while( (inb(PS2_STATUS_PORT) & 0x1) == 0 && timerGetNano() < timeoutTime);
    if(timerGetNano() >= timeoutTime)
    {
        printf("\nPS/2 Device: not present");
        return -1;
    }

    uint8_t deviceOk = 0;
    uint8_t res = inb(PS2_DATA_PORT);

    uint8_t timeout = 0;
    while(!timeout)
    {
        uint64_t timeoutTime = timerGetNano() + TIMEOUT_TIME;
        while( (inb(PS2_STATUS_PORT) & 0x1) == 0 && timerGetNano() < timeoutTime);
        if(timerGetNano() >= timeoutTime)
        {
            timeout = 1;
        }

        res = inb(PS2_DATA_PORT);
        if(res == 0xAA)
        {
            deviceOk = 1;
        }
    }
    
    if(deviceOk)
    {
        return 0;
    }
    else
        return -1;
}

int ps2DeviceSendCmd(uint8_t id, uint8_t cmd)
{
    uint8_t retries = 0;
    while(retries < 3)
    {
        ps2DeviceSendData(id, cmd);

        while( (inb(PS2_STATUS_PORT) & 0x1) == 0);

        uint8_t res = inb(PS2_DATA_PORT);
        if(res == PS2_ACK)
            return 0;
        else if(res == PS2_RESEND)
        {
            retries++;
        }
        else
        {
            printf("Unknown response: %x, retry: %u", res, retries);
            return -1;
        }
            
    }

    printf("Command timed out");
    return -1;
}

int ps2Identify(uint8_t id, uint8_t* signature)
{
    if(ps2DeviceSendCmd(id, PS2_DEVCMD_DISABLE_SCANNING) != 0)
        return -1;
    if(ps2DeviceSendCmd(id, PS2_DEVCMD_IDENTIFY) != 0)
        return -1;

    uint64_t timeoutTime = timerGetNano() + TIMEOUT_TIME;
    while((inb(PS2_STATUS_PORT) & 0x1) == 0 && timerGetNano() < timeoutTime);
    if(timerGetNano() >= timeoutTime)
    {
        //Identify returned nothing
        return 0;
    }

    signature[0] = inb(PS2_DATA_PORT);

    timeoutTime = timerGetNano() + TIMEOUT_TIME;
    while((inb(PS2_STATUS_PORT) & 0x1) == 0 && timerGetNano() < timeoutTime);
    if(timerGetNano() >= timeoutTime)
    {
        //Identify returned one byte
        return 1;
    }

    signature[1] = inb(PS2_DATA_PORT);
    return 2;
}

void ps2DeviceEnableInterrupts(uint8_t id)
{
    if(id > 1)
        return;
    
    uint8_t cfg = ps2ReadRequest(PS2_CMD_READ_CONFIG_BYTE);
    cfg |= (1 << id);
    ps2WriteRequest(PS2_CMD_WRITE_CONFIG_BYTE, cfg);
}