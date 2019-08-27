#include "fat.h"
#include "string.h"
#include "devmgr.h"
#include "vfs.h"
#include "kheap.h"

BPB_t BPB;
BPB_fat32_t BPB_fat32;
FAT_volume_t volume; 

const generic_device_t* dev;

uint8_t* dirBuffer;

int fat32Close(vnode* node);
int fat32Read(vnode* node, size_t offset, size_t length, void* buffer);
vnode* fat32Open(vnode* node, const char* name);

int fat32ReadCluster(uint32_t cluster_no, void* buffer)
{
    uint32_t first_sector_in_cluster = (cluster_no - 2) * BPB.BPB_SecPerClus + volume.first_data_sector;
    return dev->read(dev, first_sector_in_cluster * 512, BPB.BPB_SecPerClus * 512, buffer);
}

uint32_t fat32NextCluster(uint32_t cluster_no)
{
    uint8_t FAT_table[512];
    uint32_t fat_offset = cluster_no * 4;
    uint32_t fat_sector = BPB.BPB_RsvdSecCnt + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;

    if(dev->read(dev, fat_sector * 512, 512, FAT_table) != 0)
        return -1;

    uint32_t table_value = *(uint32_t*)(FAT_table + ent_offset) & 0x0FFFFFFF;

    if(table_value >= 0x0FFFFFF8)
        return 0;

    return table_value;
}

void fat32ParseLFN(char* filename, FAT_LFN_ent* entry)
{
    uint8_t order = entry->order & (~0x40);

    filename += (order - 1) * 13;
    size_t i = 0, j = 0;
    while(i < 5 && entry->name1[i] != 0xFFFF)
        filename[j++] = entry->name1[i++] & 0xFF;

    i = 0;
    while(i < 6 && entry->name2[i] != 0xFFFF)
        filename[j++] = entry->name2[i++] & 0xFF;

    i = 0;
    while(i < 2 && entry->name3[i] != 0xFFFF)
        filename[j++] = entry->name3[i++] & 0xFF;

    //If this the last entry, end the string
    if( (entry->order & 0x40) && filename[j-1] != '\0')
    {
        filename[j] = '\0';
    }
}

void fat32FetchStdName(FAT_dirent* entry, char* filename)
{
    size_t j = 0, k = 0;
    while(k < 8 && entry->name[k] != ' ')
        filename[j++] = entry->name[k++];

    if(entry->name[8] != ' ')
        filename[j++] = '.';
    k = 8;
    while(k < 11 && entry->name[k] != ' ')
        filename[j++] = entry->name[k++];
    
    filename[j] = '\0';
}

dirent* fat32Readdir(vnode* node, uint32_t idx)
{
    if(fat32ReadCluster(node->id, dirBuffer) != 0)
        return NULL;

    size_t i = 0;
    char filename[256];
    uint32_t active_cluster = node->id;

    while(dirBuffer[i] != 0 && idx > 0)
    {
        FAT_dirent* fat_entry = (FAT_dirent*) (dirBuffer + i);

        if(dirBuffer[i] != 0xE5 && (fat_entry->attr == FAT_LFN || !(fat_entry->attr & FAT_VOLUME_ID)) )
        {    
            FAT_LFN_ent* lfn_entry = (FAT_LFN_ent*) fat_entry;

            if(fat_entry->attr == FAT_LFN && (lfn_entry->order & 0x40))
            {
                //Long file names follow (skip)
                i += (lfn_entry->order & (~0x40)) * 32;
            }

            idx--;
        }
        
        i += 32;
        if(i >= BPB.BPB_SecPerClus * 512)
        {
            //The end of the cluster has been reached
            size_t j = i / (BPB.BPB_SecPerClus * 512);
            while(j--)
            {
                if(active_cluster == 0)
                    return NULL;
                active_cluster = fat32NextCluster(active_cluster);
            }

            if(active_cluster == 0)
                return NULL;
            else if(fat32ReadCluster(active_cluster, dirBuffer) != 0)
                return NULL;

            i = i % (BPB.BPB_SecPerClus * 512);
        }
    }

    if(idx != 0 || dirBuffer[i] == 0)
        return NULL;

    while(dirBuffer[i] == 0xE5)
    {
        i += 32;
        if(i >= BPB.BPB_SecPerClus * 512)
        {
            //The end of the cluster has been reached
            active_cluster = fat32NextCluster(active_cluster);

            if(active_cluster == 0)
                return NULL;
            else if(fat32ReadCluster(active_cluster, dirBuffer) != 0)
                return NULL;

            i -= BPB.BPB_SecPerClus * 512;
        }
    }

    if(dirBuffer[i] == 0)
        return NULL;

    FAT_dirent* fat_entry = (FAT_dirent*) (dirBuffer + i);

    //Parse the filename and create the directory entry
    bool long_name = false;

    while(fat_entry->attr == FAT_LFN)
    {
        long_name = true;

        FAT_LFN_ent* lfn_entry = (FAT_LFN_ent*)fat_entry;
        fat32ParseLFN(filename, lfn_entry);

        i += 32;
        if(i >= BPB.BPB_SecPerClus * 512)
        {
            //The end of the cluster has been reached
            active_cluster = fat32NextCluster(active_cluster);

            if(active_cluster == 0)
                return NULL;
            else if(fat32ReadCluster(active_cluster, dirBuffer) != 0)
                return NULL;

            i -= BPB.BPB_SecPerClus * 512;
        }

        fat_entry = (FAT_dirent*) (dirBuffer + i);
    }

    if(!long_name)
        fat32FetchStdName(fat_entry, filename);
    
    vnode* nnode = (vnode*) kheapAlloc(sizeof(vnode));
    nnode->id = (uint32_t)((fat_entry->hi_cluster << 16) + fat_entry->lo_cluster);
    nnode->length = fat_entry->size;
    if(fat_entry->attr & FAT_DIRECTORY)
        nnode->type = FS_DIR;
    else
        nnode->type = FS_FILE;
    nnode->read = fat32Read;

    if(nnode->type == FS_DIR)
    {
        nnode->open = fat32Open;
        nnode->readdir = fat32Readdir;
    }
    else
    {
        nnode->open = NULL;
        nnode->readdir = NULL;
    }
    nnode->close = fat32Close;

    dirent* new = (dirent*) kheapAlloc(sizeof(dirent));
    strcpy(new->name, filename);
    new->node = nnode;

    return new;
}

int fat32Close(vnode* node)
{
    kheapFree(node);
    return 0;
}

int fat32Read(vnode* node, size_t offset, size_t length, void* buffer)
{
    if(offset + length > node->length)
    {
        printf("Cannot read file (out of bounds)");
        return -1;
    }

    uint32_t start = offset / (BPB.BPB_SecPerClus * 512);
    uint32_t end = (offset + length - 1) / (BPB.BPB_SecPerClus * 512);

    uint32_t start_off = offset % (BPB.BPB_SecPerClus * 512);
    uint32_t end_off = (offset + length - 1) % (BPB.BPB_SecPerClus * 512);

    int result;

    uint32_t active_cluster = node->id;
    uint32_t i = start;
    while(i-- > 0)
    {
        if(active_cluster == 0)
        {
            printf("FAT_FILE_SYSTEM: corrupted cluster chain");
            return -1;
        }
        active_cluster = fat32NextCluster(active_cluster);
    }

    if(start == end)
    {
        result = fat32ReadCluster(active_cluster, dirBuffer);
        if(result != 0) return result;

        memcpy(buffer, dirBuffer + start_off, length);
        return 0;
    }

    uint32_t first_whocls, last_whocls;
    if(start_off == 0) first_whocls = start;
    else 
    {
        printf("Reading from offset %u, %u bytes", start_off, BPB.BPB_SecPerClus*512 - start_off);
        first_whocls = start + 1;

        //Copy bytes from start_off
        result = fat32ReadCluster(active_cluster, dirBuffer);
        if(result != 0) return result;

        memcpy(buffer, dirBuffer + start_off, BPB.BPB_SecPerClus*512 - start_off);
        active_cluster = fat32NextCluster(active_cluster);

        buffer = (void*)((uint32_t)buffer + BPB.BPB_SecPerClus*512 - start_off);
    }

    if(end_off + 1 == BPB.BPB_SecPerClus*512) last_whocls = end;
    else last_whocls = end - 1;
    
    //Copy whole clusters if there are any
    if(last_whocls >= first_whocls)
    {
        uint32_t n = last_whocls - first_whocls + 1;          

        //Read n chained clusters
        while(n > 0)
        {
            if(active_cluster == 0)
            {
                printf("FAT_FILE_SYSTEM: corrupted cluster chain");
                return -1;
            }
                
            //Read bytes directly in caller's buffer
            result = fat32ReadCluster(active_cluster, buffer);
            if(result != 0) return result;

            buffer += BPB.BPB_SecPerClus * 512;
            active_cluster = fat32NextCluster(active_cluster);

            n--;
        }
    }

    //Finally, read last cluster
    result = fat32ReadCluster(active_cluster, dirBuffer);
    if(result != 0) return result;

    memcpy(buffer, dirBuffer, end_off+1);

    return 0;
}

vnode* fat32Open(vnode* node, const char* name)
{
    if(fat32ReadCluster(node->id, dirBuffer) != 0)
        return NULL;
    
    size_t i = 0;
    char filename[256];
    uint32_t active_cluster = node->id;

    bool long_name = false;

    while(dirBuffer[i] != 0)
    {
        if(dirBuffer[i] != 0xE5)
        {
            FAT_dirent* fat_entry = (FAT_dirent*) (dirBuffer + i);

            if(fat_entry->attr == FAT_LFN)
            {
                FAT_LFN_ent* lfn_entry = (FAT_LFN_ent*) (dirBuffer + i);
                if(lfn_entry->order & 0x40)
                    long_name = true;

                fat32ParseLFN(filename, lfn_entry);
            }
            else
            {
                if(long_name == false)
                    fat32FetchStdName(fat_entry, filename);
                else
                    long_name = false;
                    
                if(strcmp(name, filename) == 0 && fat_entry->attr != FAT_VOLUME_ID)
                {
                    vnode* new = (vnode*) kheapAlloc(sizeof(vnode));
                    new->id = (uint32_t)((fat_entry->hi_cluster << 16) + fat_entry->lo_cluster);
                    new->length = fat_entry->size;
                    if(fat_entry->attr & FAT_DIRECTORY)
                        new->type = FS_DIR;
                    else
                        new->type = FS_FILE;
                    new->read = fat32Read;

                    if(new->type == FS_DIR)
                    {
                        new->open = fat32Open;
                        new->readdir = fat32Readdir;
                    }
                    else
                    {
                        new->open = NULL;
                        new->readdir = NULL;
                    }
                        
                    new->close = fat32Close;
                    return new;
                }
            }
        }
        
        i += 32;
        if(i >= BPB.BPB_SecPerClus * 512)
        {
            //The end of the cluster has been reached
            active_cluster = fat32NextCluster(active_cluster);
            if(active_cluster == 0)
                break;
            else if(fat32ReadCluster(active_cluster, dirBuffer) != 0)
                break;

            i = 0;
        }
    }

    return NULL;
}

int fat32InitVolume(const char* part_name)
{
    uint8_t first_sector[512];

    dev = devmgrFind(part_name);

    if(dev == NULL)
    {
        printf("Partition not found");
        return -1;
    }
    
    if(dev->read(dev, 0, 512, first_sector) != 0)
    {
        printf("Error reading boot sector");
        return -1;
    }
        
    memcpy(&BPB, first_sector, sizeof(BPB_t));
    memcpy(&BPB_fat32, first_sector + sizeof(BPB_t), sizeof(BPB_fat32_t));

    if(BPB_fat32.BS_BootSig != 0x29 && BPB_fat32.BS_BootSig != 0x28)
    {
        printf("Volume on partition %s is not FAT 32", part_name);
        return -1;
    }

    if(BPB.BPB_FATSz16 != 0)
        volume.FAT_size = BPB.BPB_FATSz16;
    else
        volume.FAT_size = BPB_fat32.BPB_FATSz32;

    volume.first_data_sector = BPB.BPB_RsvdSecCnt + BPB.BPB_NumFATs * volume.FAT_size;

    dirBuffer = (uint8_t*)kheapAlloc_a((BPB.BPB_SecPerClus * 512 - 1) / 4096 + 1);

    vnode root;
    root.id = BPB_fat32.BPB_RootClus;
    root.length = 0;
    root.type = FS_DIR;
    root.read = NULL;
    root.readdir = fat32Readdir;
    root.open = fat32Open;
    root.close = NULL;

    vfsMountRoot(&root);

    return 0;
}