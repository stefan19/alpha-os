#include "vfs.h"
#include "string.h"
#include "stdio.h"
#include "kheap.h"

#include <stdbool.h>

vnode root_node;
mount_point *mountpointList;

void vfsInit()
{
    memset(&root_node, 0, sizeof(vnode));
    root_node.type = FS_DIR;
}

vnode* vfsTraversePath(const char* path)
{
    mount_point* k = mountpointList;
    while(k != NULL)
    {
        if(strncmp(k->mountPoint, path, strlen(k->mountPoint)) == 0 )
        {
            break;
        }

        k = k->next;        
    }
    //k contains the volume on which the requested file resides
    //if k = NULL, the file resides on the root volume
    vnode* root;
    if(k == NULL) root = &root_node;
    else
    {
        root = k->root;
        path += strlen(k->mountPoint);
    }
    
    char path_cp[100];
    strcpy(path_cp, path+1);
    char* name = strtok(path_cp, '/');
    vnode* node = root;

    while(name != NULL && strlen(name) != 0)
    {
        if(node->type != FS_DIR)
        {
            printf("Item %s does not exist", path);
            return NULL;
        }

        vnode* tmp = node;
        node = node->open(node, name);
        if(node == NULL)
        {
            printf("Item %s does not exist", path);
            return NULL;
        }

        if(tmp->close != NULL) tmp->close(tmp);

        name = strtok(NULL, '/');
        /* dirent* entry = node->readdir(node, 0);
        size_t i = 0;
        while(entry != NULL && !ok)
        {
            if(strcmp(name, entry->name) == 0)
            {
                node = entry->node;
                ok = true;
            }
            else
            {
                entry = node->readdir(node, ++i);
            }
        }

        if(!ok)
        {
            printf("Item %s does not exist", path);
            return NULL;
        } */
    }

    return node;
}

void vfsMount(const char *path, vnode *root)
{
    mount_point *k = mountpointList;
    bool ok = true;
    while (k != NULL && ok)
    {
        if (strcmp(k->mountPoint, path) == 0)
            ok = false;
        k = k->next;
    }
    if (!ok)
    {
        printf("Filesystem already mounted on %s", path);
        return;
    }

    mount_point *prev = NULL;
    k = mountpointList;
    while (strlen(path) < strlen(k->mountPoint) && k != NULL)
    {
        prev = k;
        k = k->next;
    }

    //Insert new mount point before k
    mount_point *new = (mount_point *)kheapAlloc(sizeof(mount_point));
    strcpy(new->mountPoint, path);
    new->next = k;
    new->root = root;

    if (prev == NULL)
    {
        mountpointList = new;
    }
    else
    {
        prev->next = new;
    }
    
}

void vfsPrintMountPoints()
{
    mount_point *k = mountpointList;
    while (k != NULL)
    {
        printf("\n%s", k->mountPoint);
        k = k->next;
    }
}

int vfsRead(const char* path, size_t offset, size_t length, void* buffer)
{
    vnode* node = vfsTraversePath(path);
    if(node != NULL && node->read != NULL)
        return node->read(node, offset, length, buffer);
    else
        return -1;
}

dirent* vfsReaddir(const char* path, size_t index)
{
    vnode* node = vfsTraversePath(path);
    if(node != NULL && node->readdir != NULL)
        return node->readdir(node, index);
    else
        return NULL;
}

vnode* vfsOpen(const char* path)
{
    return vfsTraversePath(path);
}

void vfsMountRoot(vnode* root)
{
    memcpy(&root_node, root, sizeof(vnode));
}

/* int vfsMount(const char* path, folder_inode* superblock)
{
    char path_cp[40];
    strcpy(path_cp, path+1);
    char* name = strtok(path_cp, '/');
    folder_inode* node = &root_node;

    while(name != NULL)
    {
        bool ok = false;
        for(size_t i=0;i < node->num_subfolders && !ok;i++)
        {
            if(strcmp(name, node->folders[i].name) == 0)
            {
                node = node->folders[i].inode;
                ok = true;
            }
        }
        if(!ok)
        {
            printf("Cannot mount: %s does not exist", path);
            return -1;
        }

        name = strtok(NULL, '/');
    }

    node = superblock;
    printf("Mounted successfully on %s", path);

    return 0;
} */
