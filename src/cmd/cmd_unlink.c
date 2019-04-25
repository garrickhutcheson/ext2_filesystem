#include "cmd.h"

bool do_unlink(cmd *c) {
  if (c->argc != 2) {
    printf("Usage: unlink <filename>\n");
    return false;
  }
  return _unlink(c->argv[1]);
}

int _unlink(char *dest) {
  path in_path;
  minode *mip, *parent;
  if (!parse_path(dest, &in_path) || !(mip = search_path(in_path))) {
    printf("bad path");
    return false;
  }

  if (running->uid != 0 && mip->inode.i_uid != running->uid) {
    printf("you do not have permission\n");
    put_minode(mip);
    return 0;
  }

  if (S_ISDIR(mip->inode.i_mode)) {
    printf("Can't unlink directory\n");
    put_minode(mip);
    return false;
  }
  char *bname = in_path.argv[in_path.argc - 1];
  in_path.argc--;
  parent = search_path(in_path);
  mip->inode.i_links_count--;
  if (!mip->inode.i_links_count) {
    DEBUG_PRINT("freeing ino %d\n", mip->ino);
    free_i_block(mip);
    free_inode(mip->dev, mip->ino);
  } else {
    DEBUG_PRINT("ino %d link count now %d\n", mip->ino,
                mip->inode.i_links_count);
  }
  rm_dir_entry(parent, bname);
  mip->dirty = true;
  put_minode(mip);
  parent->dirty = true;
  put_minode(parent);
}
