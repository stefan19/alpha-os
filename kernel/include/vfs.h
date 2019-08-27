#ifndef VFS_H
#define VFS_h

#include <stdint.h>
#include <stddef.h>

#define FS_FILE 0x1
#define FS_DIR  0x2

struct vnode_struct;
struct mount_point_struct;

typedef struct
{
    char name[256];
    struct vnode_struct *node;
} dirent;

struct vnode_struct;

typedef dirent* (*readdir_t)(struct vnode_struct*, size_t);
typedef int (*read_t)(struct vnode_struct*, size_t, size_t, void*);
typedef struct vnode_struct* (*open_t)(struct vnode_struct*, const char*);
typedef int (*close_t)(struct vnode_struct*);

struct vnode_struct
{
    uint8_t type;
    uint32_t length;
    uint32_t id;
    read_t read;
    readdir_t readdir;
    open_t open;
    close_t close;
};
typedef struct vnode_struct vnode;

struct mount_point_struct
{
    char mountPoint[256];
    vnode* root;
    struct mount_point_struct *next;
};
typedef struct mount_point_struct mount_point;

void vfsInit();
void vfsMount(const char* path, vnode* root);
void vfsMountRoot(vnode* root);
int vfsRead(const char* path, size_t offset, size_t length, void* buffer);
dirent* vfsReaddir(const char* path, size_t index);
vnode* vfsOpen(const char* path);

void vfsPrintMountPoints();

#endif
