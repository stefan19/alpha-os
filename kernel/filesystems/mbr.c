#include "mbr.h"
#include "string.h"

mbr_partition partitions[16];

int mbrReadPartition(const generic_device_t* dev, size_t offset, size_t len, void* buffer)
{
    const generic_device_t* drive = partitions[dev->id].dev;

    if(offset+len > partitions[dev->id].sectors * 512)
    {
        printf("Read exception at sector: %x", (offset+len-1)/512 );
        return -1;
    }

    return drive->read(drive, partitions[dev->id].start_lba*512 + offset, len, buffer);
}

void mbrParseSector(const generic_device_t* dev, uint8_t* sector)
{
    uint8_t* off = sector + 0x1BE;
    uint8_t idx = dev->name[3] - '0';

    for(int i=0;i<4;i++)
    {
        uint8_t id = off[4];
        partitions[4*idx + i].present = false;

        //Check if this entry is empty
        if(id != 0)
        {
            partitions[4*idx + i].present = true;
            partitions[4*idx + i].start_lba = *((uint32_t*)(off + 8));
            partitions[4*idx + i].sectors = *((uint32_t*)(off + 12));
            partitions[4*idx + i].dev = dev;
        }

        off += 16;
    }
}

void mbrInit()
{
    uint8_t first_sector[512];

    size_t i = 0;
    const generic_device_t* dev = devmgrRetrieve(DEVICE_STORAGE, 0);
    while(dev != NULL)
    {
        int ret = dev->read(dev, 0, 512, first_sector);
        if(ret == 0)
        {
            //Parse partition table
            mbrParseSector(dev, first_sector);
        }
       dev = devmgrRetrieve(DEVICE_STORAGE, ++i);
    }

    char name[10] = {0};
    generic_device_info_t part_info;
    for(int i=0;i<16;i++)
    {
        if(!partitions[i].present)
            continue;

        char drvidx = i / 4 + '0';
        name[0] = 'a'; name[1] = 't'; name[2] = 'a';
        name[3] = drvidx; name[4] = 'p'; name[5] = i%4 + '0';
        name[6] = '\0';

        strcpy(part_info.name, name);
        part_info.device_type = DEVICE_DISKPART;
        part_info.id = i;
        part_info.read = mbrReadPartition;

        devmgrAddDevice(&part_info);
    }
}

void mbrList()
{
    for(int i=0;i<16;i++)
    {
        if(partitions[i].present)
        {
            printf("\nPartition %u, start: %x, size: %x", i, partitions[i].start_lba, partitions[i].sectors);
        }
    }
}