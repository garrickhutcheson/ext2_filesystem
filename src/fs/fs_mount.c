#include "fs.h"

int mount_root(char *dev_path) {
  int i;
  mount_entry *me;
  super_block *sb;
  group_desc *gd;
  char buf[BLKSIZE_1024];
  int dev = open(dev_path, O_RDWR);
  if (dev < 0) {
    printf("panic : can’t open root device\n");
    exit(1);
  }
  /* get super block of device */
  get_block(dev, 1, buf);
  sb = (super_block *)buf;
  /* check magic number */
  if (sb->s_magic != EXT2_SUPER_MAGIC) {
    printf("super magic=%x : %s is not an EXT2 filesys\n", sb->s_magic,
           dev_path);
    exit(0);
  }
  // fill mount table root with device information
  me = &mount_entry_arr[0];
  me->fd = dev;
  strcpy(me->dev_path, dev_path);
  strcpy(me->mnt_path, "/");
  // copy super block info into mount entry
  me->super_block = *sb;
  // get group descriptor from device
  get_block(dev, 2, buf);
  gd = (group_desc *)buf;
  // copy group descriptor into mount entry
  me->group_desc = *gd;

  DEBUG_PRINT("block_bitmap=%d inode_bitmap=%d inode_table=%d\n",
              me->group_desc.bg_block_bitmap, me->group_desc.bg_inode_bitmap,
              me->group_desc.bg_inode_table);
  // call get_inode(), which inc minode’s refCount
  me->root = get_inode(dev, 2);
  // set global root
  global_root = me->root;
  // double link
  global_root->mount_entry = me;
  // set proc CWDs
  for (i = 0; i < NUM_PROCS; i++)        // set proc’s CWD
    proc_arr[i].cwd = get_inode(dev, 2); // increments refCount
  DEBUG_PRINT("mount : %s mounted on / \n", dev_path);
  return 0;
}
