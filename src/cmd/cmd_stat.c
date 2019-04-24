#include "cmd.h"

bool do_stat(cmd *c) {
  path in_path;
  minode *mip;
  if (c->argc < 2) {
    printf("stat requires: stat filename\n");
    return false;
  }
  if (!parse_path(c->argv[1], &in_path) || !(mip = search_path(in_path))) {
    printf("bad path");
    return false;
  }
  inode *i = &mip->inode;
  printf("File: %s\n"
         "Size: %u\t Blocks: %u\t Mode: %o\n"
         "Device: %s\t Ino: %u\t Links: %u\t \n"
         "Uid: %u\t Gid: %u\t \n"
         "Access: %s"
         "Modify: %s"
         "Change: %s",
         c->argv[1], i->i_size, i->i_blocks, i->i_mode, mip->dev->dev_path,
         mip->ino, i->i_links_count, i->i_uid, i->i_gid,
         ctime((long *)&i->i_atime), ctime((long *)&i->i_mtime),
         ctime((long *)&i->i_ctime));

  return true;
}