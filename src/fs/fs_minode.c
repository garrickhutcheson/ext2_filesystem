#include "fs.h"

// allocate a free minode for use
minode *alloc_minode() {
  for (int i = 0; i < NUM_MINODES; i++) {
    minode *mp = &minode_arr[i];
    if (mp->ref_count == 0) {
      mp->ref_count = 1;
      return mp;
    }
  }
  printf("panic:out of minodes\n");
  return NULL;
}

// release a used minode. However, root is always referenced.
bool free_minode(minode *mip) {
  mip->ref_count = mip == global_root_inode ? 1 : 0;
  return true;
}

// Returns a pointer to the in-memory minode containing the INODE of (me, ino).
minode *get_minode(mount_entry *dev, int ino) {
  minode *mip;
  inode *ip;
  int i, block, offset;
  char buf[BLKSIZE_1024];
  // search in-memory minodes first
  for (i = 0; i < NUM_MINODES; i++) {
    minode *mip = &minode_arr[i];
    if (mip->ref_count && (mip->dev == dev) && (mip->ino == ino)) {
      mip->ref_count++;
      return mip;
    }
  }
  mip = alloc_minode();
  block = (ino - 1) / 8 + dev->group_desc.bg_inode_table;
  offset = (ino - 1) % 8;
  get_block(dev, block, buf);
  ip = (inode *)buf + offset;
  // initialize minode
  *mip = (minode){
      .inode = *ip,
      .ino = ino,
      .ref_count = 1,
      .dirty = 0,
      .mnt = NULL,
      .dev = dev,
  };

  return mip;
}

// Decrements the ref_count by 1. If the refCount is zero then
// inode is written back to disk if it is modified (dirty).
bool put_minode(minode *mip) {
  inode *ip;
  int i, block, offset;
  char buf[BLKSIZE_1024];
  if (mip == 0)
    return false;
  mip->ref_count--;
  if (mip->ref_count > 0)
    return false;
  // ROOT NEVER FREE. ROOT STAY FOREVER
  if (global_root_inode->ref_count < 1)
    global_root_inode->ref_count = 1;
  if (mip->dirty == 0)
    return false;
  block = (mip->ino - 1) / 8 + mip->dev->group_desc.bg_inode_table;
  offset = (mip->ino - 1) % 8;
  // get block containing this inode
  get_block(mip->dev, block, buf);
  ip = (inode *)buf + offset;
  *ip = mip->inode;
  put_block(mip->dev, block, buf);
  free_minode(mip);
  return true;
}
