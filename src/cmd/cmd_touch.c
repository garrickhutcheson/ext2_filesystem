#include "cmd.h"

bool do_touch(cmd *c) {
  path in_path;
  if (c->argc < 2) {
    printf("touch requires path\n");
    return false;
  }
  parse_path(c->argv[1], &in_path);
  minode *mip = search_path(in_path);
  if (!mip)
    return do_creat(c);
  else
    mip->inode.i_atime = time(0L);
  return true;
}