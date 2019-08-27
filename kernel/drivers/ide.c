#include "ide.h"
#include "ll_io.h"
#include "console.h"
#include "pci.h"
#include "paging.h"
#include "kheap.h"
#include "string.h"
#include "isr.h"
#include "timer.h"
#include "pic.h"
#include "vfs.h"
#include "devmgr.h"

uint16_t identifyBuffer[256];

ide_drive_info ideDrives[4];
pci_device_t ideController;

prd_entry* prdt;
uint8_t* buffer;

void ideIdentifyDevice(uint16_t bus, uint8_t drive, uint8_t i)
{
    ideDrives[i].type = IDE_NONE;

    outb(bus + ATA_DRIVESEL, drive);
    outb(bus + ATA_SECTORCNT, 0);
    outb(bus + ATA_LBA_LO, 0);
    outb(bus + ATA_LBA_MID, 0);
    outb(bus + ATA_LBA_HIGH, 0);

    //Send IDENTIFY command
    outb(bus + ATA_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = inb(bus + ATA_STATUS);
    if(status == 0)
        return;

    while((status & 0x80) != 0)           //Wait until BSY bit clears
        status = inb(bus + ATA_STATUS);
    
    //Check if drive is ATA (it might be ATAPI)
    if(inb(bus + ATA_LBA_MID) != 0 || inb(bus + ATA_LBA_HIGH) != 0)
    {
        if(inb(bus + ATA_LBA_MID) == 0x14 && inb(bus + ATA_LBA_HIGH) == 0xEB)
            ideDrives[i].type = IDE_ATAPI;
        else if(inb(bus + ATA_LBA_MID) == 0x3C && inb(bus + ATA_LBA_HIGH) == 0xC3)
            ideDrives[i].type = IDE_SATA;
        return;
    }

    while( ((status & 0x8) == 0) && ((status & 0x1) == 0) )   //Continue polling until DRQ or ERR sets
        status = inb(bus + ATA_STATUS);

    if((status & 0x1) != 0)
        return;

    //Selected drive is certainly ATA, transfer data
    ideDrives[i].type = IDE_ATA;
    for(int i=0;i<256;i++)
        identifyBuffer[i] = inw(bus + ATA_DATA);

    //Determine UDMA capabilities
    if(identifyBuffer[53] & 0x4)
    {
        if((identifyBuffer[88] >> 8) & 0xFF)
            ideDrives[i].udmaActive = true;
        else
            ideDrives[i].udmaActive = false;        
    }
    else
    {
        ideDrives[i].udmaActive = false;
    }

    //Determine DMA Single/Multi-Word capabilities
    if(identifyBuffer[53] & 0x2)
    {
        if((identifyBuffer[62] >> 8) & 0xFF || (identifyBuffer[63] >> 8) & 0xFF)
            ideDrives[i].dmaActive = true;
        else
            ideDrives[i].dmaActive = false;
    }
    else
    {
        ideDrives[i].dmaActive = false;
    }

    //Determine addressing mode
    ideDrives[i].lba28Supported = false;
    ideDrives[i].lba48Supported = false;

    if(identifyBuffer[83] & (1 << 10))
    {
        ideDrives[i].lba48Supported = true;
        ideDrives[i].maxSector = *((uint64_t*)(identifyBuffer + 100));
    }

    uint32_t maxLba28 = *((uint32_t*)(identifyBuffer + 60));
    if(maxLba28 != 0)
    {
        ideDrives[i].lba28Supported = true;
        ideDrives[i].maxSector = (uint64_t) maxLba28;
    }
}

void idePrintType(uint8_t type)
{
    switch (type)
    {
    case IDE_NONE:
        consoleWriteStr("IDE_NONE");
        break;
    case IDE_ATA:
        consoleWriteStr("IDE_ATA");
        break;
    case IDE_ATAPI:
        consoleWriteStr("IDE_ATAPI");
        break;
    case IDE_SATA:
        consoleWriteStr("IDE_SATA");
        break;
    default:
        consoleWriteStr("not a type");
        break;
    }
}

volatile bool transfered;

void idePrimaryIntHandler(const interrupt_frame_t* regs)
{
    uint8_t status = inb(ideDrives[0].bmBase + ATA_BMSTATUS);
    inb(ideDrives[0].ioBase + ATA_STATUS);          //Read drive status!
    if(status & 0x4)
    {
        outb(ideDrives[0].bmBase + ATA_BMCOMMAND, 0);
        outb(ideDrives[0].bmBase + ATA_BMSTATUS, 4);
        transfered = true;
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void ideSecondaryIntHandler(const interrupt_frame_t* regs)
{
    uint8_t status = inb(ideDrives[2].bmBase + ATA_BMSTATUS);
    inb(ideDrives[2].ioBase + ATA_STATUS);          //Read drive status!
    if(status & 0x4)
    {
        outb(ideDrives[2].bmBase + ATA_BMCOMMAND, 0);
        outb(ideDrives[2].bmBase + ATA_BMSTATUS, 4);
        transfered = true;
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

static void ideInitInternalBuffers()
{
    buffer = (uint8_t*)kheapAlloc_a(1);
    prdt = (prd_entry*)kheapAlloc_a(1);
}

int ideFsRead(vnode* node, uint32_t offset, uint32_t length, void* ptr)
{
    return ideDMARead(node->id, offset, length, ptr);
}

int ideDevmgrRead(const generic_device_t* dev, uint32_t offset, uint32_t length, void* ptr)
{
    if(dev->device_type != DEVICE_STORAGE)
    {
        printf("Fatal: device type mismatch for %s", dev->name);
        return -1;
    }
    return ideDMARead(dev->id, offset, length, ptr);
}

void ideEnumerateDrives()
{
    for(uint8_t i=0;i<4;i++)
        ideDrives[i].type = IDE_NONE;

    uint8_t ok = 0;
    for(uint8_t i=0;i<pciRetrieveDeviceNumber() && !ok;i++)
    {
        pci_device_t* dev = pciRetrieveDevice(i);
        if(dev->baseClass == 0x01 && dev->subClass == 0x01)
        {
            ok = 1;
            memcpy(&ideController, dev, sizeof(pci_device_t));
        }
    }

    if(ok)
    {
        //Set up CORRECT interrupt lines
        uint8_t progIF = pciGetProgIF(ideController.bus, ideController.slot, ideController.func);

        if( (progIF & 0x80) == 0 )
        {
            consoleWriteStr("\nWarning: This IDE controller doesn't support DMA and won't be able to complete transfers");
            return;
        }

        //This controller supports Busmastering
        if( ((progIF & 0x1) == 0) && ((progIF & 0x4) == 0) )
        {
            registerInterruptHandler(46, idePrimaryIntHandler);
            registerInterruptHandler(47, ideSecondaryIntHandler);
        }

        //Enable busmastering DMA
        uint8_t cmd = pciGetCommand(ideController.bus, ideController.slot, ideController.func);
        cmd |= 0x4;
        pciWriteCommand(ideController.bus, ideController.slot, ideController.func, cmd);

        uint16_t bar4 = pciGetBAR4(ideController.bus, ideController.slot, ideController.func) & (~3);

        ideDrives[1].bmBase = ideDrives[0].bmBase = bar4;
        ideDrives[3].bmBase = ideDrives[2].bmBase = bar4 + 0x8;

        ideDrives[1].ioBase = ideDrives[0].ioBase = ATA_PRIMARY_BASE;
        ideDrives[3].ioBase = ideDrives[2].ioBase = ATA_SECONDARY_BASE;

        ideDrives[1].ctrlBase = ideDrives[0].ctrlBase = ATA_PRIMARY_CONTROL;
        ideDrives[3].ctrlBase = ideDrives[2].ctrlBase = ATA_SECONDARY_CONTROL;

        uint8_t status = inb(ATA_PRIMARY_BASE + ATA_STATUS);
        //Check if primary bus exists
        if(status != 0xFF)
        {
            ideIdentifyDevice(ATA_PRIMARY_BASE, ATA_MASTER, 0);
            ideIdentifyDevice(ATA_PRIMARY_BASE, ATA_SLAVE, 1);
        }

        status = inb(ATA_SECONDARY_BASE + ATA_STATUS);
        //Check if secondary bus exists
        if(status != 0xFF)
        {
            ideIdentifyDevice(ATA_SECONDARY_BASE, ATA_MASTER, 2);
            ideIdentifyDevice(ATA_SECONDARY_BASE, ATA_SLAVE, 3);
        }

        ideInitInternalBuffers();

        picUnmask(14);
        picUnmask(15);
    }
    
    consoleWriteStr("\nEnumerating IDE drives: ");
    generic_device_info_t devinfo;
    for(uint8_t i=0;i<4;i++)
    {
        if(ideDrives[i].type == IDE_ATA)
        {
            char name[5];
            strcpy(name, "ata");
            name[3] = i + '0';
            name[4] = '\0';

            /* devfsAddDevice(name, ideFsRead, i); */

            strcpy(devinfo.name, name);
            devinfo.device_type = DEVICE_STORAGE;
            devinfo.id = i;
            devinfo.read = ideDevmgrRead;
            devmgrAddDevice(&devinfo);
        }
        if(ideDrives[i].type != IDE_NONE)
        {
            printf("\nIDE drive %u type: ", i);
            idePrintType(ideDrives[i].type);
            if(ideDrives[i].type != IDE_NONE)
                printf(" size: %u sectors", ideDrives[i].maxSector);
        }
        
    }
}

//n should not be larger than 8 (buffer is 4kb)
int ideDMAReadSectors(uint32_t idx, uint32_t lba, uint32_t n)
{
    if(idx > 4)
        return -1;
    if(ideDrives[idx].type != IDE_ATA)
    {
        printf("\nDrive connected to %u is not ATA", idx);
        return -1;
    }
    if(ideDrives[idx].udmaActive == false && ideDrives[idx].dmaActive == false)
    {
        printf("\nDMA not active for %u", idx);
        return -1;
    }
    if(ideDrives[idx].lba28Supported == false || lba > 0xFFFFFFF || lba > ideDrives[idx].maxSector)
    {
        printf("\nLba 28 not supported/sector address incorrect");
        return -1;
    }

    prdt->base = getPhysAddress((uint32_t)buffer);
    prdt->byteCount = n * 512;
    prdt->reserved = 0;
    prdt->EOT = 1;

    //Stop any DMA transfer
    outb(ideDrives[idx].bmBase + ATA_BMCOMMAND, 0);

    //Load the PRDT
    outd(ideDrives[idx].bmBase + ATA_BMPRDT, getPhysAddress((uint32_t)prdt));
    
    //Clear Interrupt and Error bits
    outb(ideDrives[idx].bmBase + ATA_BMSTATUS, 6);

    uint8_t driveNo = 0xE0;
    if(idx % 2 != 0)
        driveNo = 0xF0;

    //Send READ_DMA Command
    outb(ideDrives[idx].ioBase + ATA_DRIVESEL, driveNo | ((lba >> 24) & 0x0F) );
    outb(ideDrives[idx].ioBase + ATA_SECTORCNT, n);
    outb(ideDrives[idx].ioBase + ATA_LBA_LO, lba & 0xFF);
    outb(ideDrives[idx].ioBase + ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ideDrives[idx].ioBase + ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ideDrives[idx].ioBase + ATA_COMMAND, ATA_CMD_READ_DMA28);

    //Engage
    outb(ideDrives[idx].bmBase + ATA_BMCOMMAND, 9);

    transfered = false;
    uint32_t timeoutTime = timerGetNano() + 50000000;
    while(!transfered && timerGetNano() < timeoutTime);

    if(!transfered)
    {
        consoleWriteStr("Transfer timed out");
        return -1;
    }
    
    return 0;
}

int ideDMARead(uint32_t idx, uint32_t offset, uint32_t len, void *ptr)
{
    int result;

    uint32_t first_sector = offset / 512;
    uint32_t last_sector = (offset + len - 1) / 512;

    uint32_t first_offset = offset % 512;
    uint32_t last_offset = (offset + len - 1) % 512;

    //len < 512
    if(first_sector == last_sector)
    {
        result = ideDMAReadSectors(idx, first_sector, 1);
        if(result != 0) return result;

        memcpy(ptr, buffer + first_offset, len);
        return 0;
    }

    uint32_t first_wholesec, last_wholesec;
    if(first_offset == 0) first_wholesec = first_sector;
    else 
    {
        first_wholesec = first_sector + 1;
        //Copy bytes from first_offset
        result = ideDMAReadSectors(idx, first_sector, 1);
        if(result != 0) return result;

        memcpy(ptr, buffer + first_offset, 512 - first_offset);
    }

    if(last_offset == 511) last_wholesec = last_sector;
    else
    {
        last_wholesec = last_sector - 1;
        //Copy bytes from 0 to last_offset
        result = ideDMAReadSectors(idx, last_sector, 1);
        if(result != 0) return result;

        memcpy(ptr+len-last_offset-1, buffer, last_offset+1);
    }

    //Finally, copy the whole sectors if there are any
    if(last_wholesec >= first_wholesec)
    {
        uint32_t n = last_wholesec - first_wholesec + 1;
        uint32_t lba = first_wholesec;
        if(first_offset != 0)
            ptr = (void*)((uint32_t)ptr + 512 - first_offset);

        while(n > 8)
        {
            result = ideDMAReadSectors(idx, lba, 8);
            if(result != 0) return result;

            //Copy 4 KiB of data
            memcpy(ptr, buffer, 0x1000);    
            ptr += 0x1000;
            n -= 8;
            lba += 8;
        }

        result = ideDMAReadSectors(idx, lba, n);
        if(result != 0) return result;
        memcpy(ptr, buffer, n*512);
    }

    return 0;
}

////**** LEGACY *****////
/* void idePIORead(uint32_t lba)
{
    uint16_t buffer[256];

    outb(ATA_PRIMARY_BASE + ATA_DRIVESEL, 0xE0 | ((lba >> 24) & 0x0F) );
    outb(ATA_PRIMARY_BASE + ATA_SECTORCNT, 1);
    outb(ATA_PRIMARY_BASE + ATA_LBA_LO, lba & 0xFF);
    outb(ATA_PRIMARY_BASE + ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_BASE + ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_BASE + ATA_COMMAND, ATA_CMD_READ_PIO28);

    uint8_t status = inb(ATA_PRIMARY_BASE + ATA_STATUS);

    //Wait for BSY to clear
    while(status & 0x80) status = inb(ATA_PRIMARY_BASE + ATA_STATUS);

    //Wait for DRQ or ERR or DF to set
    while( ((status & 0x8) == 0) && ((status & 0x1) == 0) && ((status & 0x20) == 0) )
        status = inb(ATA_PRIMARY_BASE + ATA_STATUS);
    
    if( (status & 0x1) || (status & 0x20) )
    {
        consoleWriteStr("Read failure");
        return;
    }

    printf("\nstatus = %x", status);

    for(size_t i=0;i<256;i++)
    {
        buffer[i] = inw(ATA_PRIMARY_BASE + ATA_DATA);
    }

    printf("Read successful: %x", buffer[0]);
} */