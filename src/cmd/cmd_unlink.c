#include "cmd.h"

bool do_unlink(cmd *c) {
  path in_path;
  minode *mip, *parent;
  if (c->argc < 2) {
    printf("unlink requires: unlink filename\n");
    return false;
  }
  if (!parse_path(c->argv[1], &in_path) || !(mip = search_path(in_path))) {
    printf("bad path");
    return false;
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
    free_i_block(mip);
    free_inode(mip->me, mip->ino);
  }
  rm_dir_entry(parent, bname);
  mip->dirty = true;
  put_minode(mip);
  parent->dirty = true;
  put_minode(parent);
  return true;
}
