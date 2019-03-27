#include "fs.h"
#include "string.h"
#include <unistd.h>

int fs_init() {
  int i, j;
  // initialize all minodes
  for (i = 0; i < NUM_MINODES; i++)
    minode_arr[i].refCount = 0;
  // initialize mount entries
  for (i = 0; i < NUM_MOUNT_ENTRIES; i++)
    mount_entry_arr[i].fd = 0;
  // initialize PROCs
  for (i = 0; i < NUM_PROCS; i++) {
    proc_arr[i].status = PROC_FREE;
    proc_arr[i].pid = i;
    // P0 is a superuser process
    proc_arr[i].uid = i;
    // initialize PROC file descriptors to NULL
    for (j = 0; j < NUM_FD; j++)
      proc_arr[i].oft_arr[j] = 0;
    proc_arr[i].next = &proc_arr[i + 1];
  }
  // circular list
  proc_arr[NUM_PROCS - 1].next = &proc_arr[0];
  // P0 runs first
  running = &proc_arr[0];
  return 0;
}

minode *alloc_minode()
// allocate a free minode for use
{
  int i;
  for (i = 0; i < NUM_MINODES; i++) {
    minode *mp = &minode_arr[i];
    if (mp->refCount == 0) {
      mp->refCount = 1;
      return mp;
    }
  }
  printf("FS panic : out of minodes\n");
  return 0;
}

int free_minode(minode *mip) // release a used minode
{
  mip->refCount = 0;
  return 0;
}

// get_block/put_block functions:
// We assume that a block device, e.g. a real or virtual disk, can
// only be read or written in unit of block size. For real disks, this is due to
// hardware constraints. For virtual disks, we assume that read/write is
// by block size, so that the code can be ported to real disks if desired. For
// a virtual disk, we first open it for R|W mode and use the file descriptor
// as the device number. The following functions read/write a virtual disk
// block into/from a buffer area in memory.

int get_block(int dev, int blk, char *buf) {
  lseek(dev, blk * BLKSIZE_1024, SEEK_SET);
  int n = read(dev, buf, BLKSIZE_1024);
  if (n < 0)
    printf("get_block[% d % d] error \n", dev, blk);
  return 0;
}

int put_block(int dev, int blk, char *buf) {
  lseek(dev, blk * BLKSIZE_1024, SEEK_SET);
  int n = write(dev, buf, BLKSIZE_1024);
  if (n != BLKSIZE_1024)
    printf("put_block [%d %d] error\n", dev, blk);
  return 0;
}

// get_inode(dev, ino) function:
// This function returns a pointer to the in-memory minode containing the
// INODE of (dev, ino). The returned minode is unique, i.e. only one copy of the
// INODE exists in memory. In a real file system, the returned minode is locked
// for exclusive use until it is either released or unlocked. For
// simplicity,we
// shall assume that minode locking is unnecessary, which will
// be explained later.

// todo: rename to get_minode?
minode *get_inode(int dev, int ino) {
  minode *mip;
  mount_entry *me = &mount_entry_arr[0];
  inode *ip;
  int i, block, offset;
  char buf[BLKSIZE_1024];
  // search in-memory minodes first
  for (i = 0; i < NUM_MINODES; i++) {
    minode *mip = &minode_arr[i];
    if (mip->refCount && (mip->dev == dev) && (mip->ino == ino)) {
      mip->refCount++;
      return mip;
    }
  }
  mip = alloc_minode();
  block = (ino - 1) / 8 + me->group_desc.bg_inode_table;
  offset = (ino - 1) % 8;
  get_block(dev, block, buf);
  ip = (inode *)buf + offset;
  // initialize minode
  // todo: check if mount_entry is set correctly
  *mip = (minode){
      .inode = *ip,
      .dev = dev,
      .ino = ino,
      .refCount = 1,
      .dirty = 0,
      .mounted = 0,
      .mount_entry = 0,
  };
  return mip;
}

// put_inode(INODE *mip) function:
// This function releases a used minode pointed by mip. Each
// minode has a refCount, which represents the number of users that are using
// the minode. put_inode() decrements the refCount by 1. If the refCount is non
// - zero, meaning that the minode still has other users, the caller simply
// returns. If the caller is the last user of the minode (refCount 1⁄4 0), the
// INODE is written back to disk if it is modified (dirty).

// todo: rename to put_minode?
int put_inode(minode *mip) {
  inode *ip;
  int i, block, offset;
  char buf[BLKSIZE_1024];
  if (mip == 0)
    return 0;
  mip->refCount--;
  if (mip->refCount > 0)
    return 0;
  if (mip->dirty == 0)
    return 0;
  block = (mip->ino - 1) / 8 + mount_entry_arr[0].group_desc.bg_inode_table;
  offset = (mip->ino - 1) % 8;
  // get block containing this inode
  get_block(mip->dev, block, buf);
  ip = (inode *)buf + offset;
  *ip = mip->inode;
  put_block(mip->dev, block, buf);
  free_minode(mip);
  return 0;
}
