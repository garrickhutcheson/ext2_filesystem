#include "fs.h"

// allocate a free minode for use
minode *alloc_minode() {
  for (int i = 0; i < NUM_MINODES; i++) {
    minode *mp = &minode_arr[i];
    if (mp->refCount == 0) {
      mp->refCount = 1;
      return mp;
    }
  }
  printf("panic:out of minodes\n");
  return 1;
}

// release a used minode
int free_minode(minode *mip) {
  mip->refCount = 0;
  return 0;
}

// Returns a pointer to the in-memory minode containing the INODE of (dev, ino).
minode *get_minode(int dev, int ino) {
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

// Decrements the refCount by 1. If the refCount is zero then
// inode is written back to disk if it is modified (dirty).
int put_minode(minode *mip) {
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
