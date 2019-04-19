#include "cmd.h"

bool do_touch(cmd *c) {
  path in_path;
  if (c->argc != 2) {
    printf("Usage: touch <filename>\n");
    return false;
  }
  parse_path(c->argv[1], &in_path);
  minode *mip = search_path(in_path);
  if (!mip)
    return _creat(c->argv[1]);
  else
    return !!(mip->inode.i_atime = time(0L));
}