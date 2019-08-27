#include "ll_io.h"
#include "console.h"
#include "pci.h"

static uint32_t pciDeviceNumber = 0;
static pci_device_t pciDevices[40];

uint32_t pciConfigReadDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus = (uint32_t) bus;
    uint32_t lslot = (uint32_t) slot;
    uint32_t lfunc = (uint32_t) func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | 
                (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    outd(0xCF8, address);

    return ind(0xCFC);
}

uint32_t pciConfigWriteDWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address;
    uint32_t lbus = (uint32_t) bus;
    uint32_t lslot = (uint32_t) slot;
    uint32_t lfunc = (uint32_t) func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | 
                (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    outd(0xCF8, address);
    outd(0xCFC, value);
}

uint16_t pciGetVendorID(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0) & 0xFFFF;
}

uint8_t pciGetHeaderType(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 0xC) >> 16) & 0xFF;
}

uint8_t pciGetBaseClass(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 8) >> 24) & 0xFF;
}

uint8_t pciGetSubClass(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 8) >> 16) & 0xFF;
}

uint8_t pciGetProgIF(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 8) >> 8) & 0xFF;
}

uint16_t pciGetDeviceStatus(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 4) >> 16) & 0xFFFF;
}

uint8_t pciGetCommand(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 4) & 0xFF;
}

void pciWriteCommand(uint8_t bus, uint8_t slot, uint8_t func, uint8_t value)
{
    uint32_t dword = (pciConfigReadDWord(bus, slot, func, 4) & (~0xFF)) + value;
    pciConfigWriteDWord(bus, slot, func, 4, dword);
}

uint32_t pciGetBAR0(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x10);
}

uint32_t pciGetBAR1(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x14);
}

uint32_t pciGetBAR2(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x18);
}

uint32_t pciGetBAR3(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x1C);
}

uint32_t pciGetBAR4(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x20);
}

uint32_t pciGetBAR5(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x24);
}

uint8_t pciGetInterruptLine(uint8_t bus, uint8_t slot, uint8_t func)
{
    return pciConfigReadDWord(bus, slot, func, 0x3C) & 0xFF;
}

void pciWriteInterruptLine(uint8_t bus, uint8_t slot, uint8_t func, uint8_t irq)
{
    outd(0xCF8, (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | 0x3C);
    outb(0xCFC, irq);
}

uint8_t pciGetSecondaryBus(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (pciConfigReadDWord(bus, slot, func, 0x18) >> 8) & 0xFF;
}

void pciCheckFunction(uint8_t bus, uint8_t device, uint8_t function)
{
    uint8_t baseClass;
    uint8_t subClass;
    uint8_t secondaryBus;

    baseClass = pciGetBaseClass(bus, device, function);
    subClass = pciGetSubClass(bus, device, function);

    if(baseClass == 0x06 && subClass == 0x04)
    {
        secondaryBus = pciGetSecondaryBus(bus, device, function);
        pciCheckBus(secondaryBus);
    }
    if(baseClass == 0x01 && subClass == 0x01)
    {
        pciDevices[pciDeviceNumber].baseClass = baseClass;
        pciDevices[pciDeviceNumber].subClass = subClass;
        pciDevices[pciDeviceNumber].progIF = pciGetProgIF(bus, device, function);
        pciDevices[pciDeviceNumber].bus = bus;
        pciDevices[pciDeviceNumber].slot = device;
        pciDevices[pciDeviceNumber].func = function;
        pciDeviceNumber++;
    }
}

void pciCheckDevice(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;
    
    uint16_t vendorID = pciGetVendorID(bus, device, function);
    if(vendorID == 0xFFFF) return;          //Device doesn't exist
    pciCheckFunction(bus, device, function);
    uint8_t headerType = pciGetHeaderType(bus, device, function);

    if((headerType & 0x80) != 0)
    {
        for(function = 1; function < 8; function++) {
            if(pciGetVendorID(bus, device, function) != 0xFFFF)
                pciCheckFunction(bus, device, function);
        }
    }
}

void pciCheckBus(uint8_t bus)
{
    uint8_t device;

    for(device=0; device < 32; device++)
    {
        pciCheckDevice(bus, device);
    }
}

void pciCheckAllBuses(void)
{
    uint8_t function;
    uint8_t bus;

    uint8_t headerType = pciGetHeaderType(0, 0, 0);
    if( (headerType & 0x80) == 0)
        pciCheckBus(0);
    else
    {
        for ( function = 0 ; function < 8; function++)
        {
            if(pciGetVendorID(0, 0, function) != 0xFFFF) break;
            bus = function;
            pciCheckBus(bus);
        }
    }
}

uint32_t pciRetrieveDeviceNumber()
{
    return pciDeviceNumber;
}

pci_device_t* pciRetrieveDevice(uint32_t i)
{
    if(i < pciDeviceNumber)
        return &pciDevices[i];
    
    return 0;
}