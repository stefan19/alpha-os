#ifndef PCI_H
#define PCI_H

typedef struct 
{
    uint8_t baseClass;
    uint8_t subClass;
    uint8_t progIF;
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
} pci_device_t;

void pciCheckAllBuses(void);
uint32_t pciRetrieveDeviceNumber();
pci_device_t* pciRetrieveDevice(uint32_t i);

uint8_t pciGetBaseClass(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pciGetSubClass(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pciGetProgIF(uint8_t bus, uint8_t slot, uint8_t func);
uint16_t pciGetDeviceStatus(uint8_t bus, uint8_t slot, uint8_t func);
uint8_t pciGetCommand(uint8_t bus, uint8_t slot, uint8_t func);
void pciWriteCommand(uint8_t bus, uint8_t slot, uint8_t func, uint8_t value);
uint32_t pciGetBAR0(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pciGetBAR1(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pciGetBAR2(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pciGetBAR3(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pciGetBAR4(uint8_t bus, uint8_t slot, uint8_t func);
uint32_t pciGetBAR5(uint8_t bus, uint8_t slot, uint8_t func);

uint8_t pciGetInterruptLine(uint8_t bus, uint8_t slot, uint8_t func);
void pciWriteInterruptLine(uint8_t bus, uint8_t slot, uint8_t func, uint8_t irq);

#endif