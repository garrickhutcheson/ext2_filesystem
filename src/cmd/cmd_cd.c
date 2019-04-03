#include "cmd.h"

bool do_cd(cmd *c) {
  minode *dest;
  if (c->argc < 2)
    dest = global_root_inode;
  else {
    path p, *in_path = &p;
    parse_path(c->argv[1], in_path);
    if ((dest = search_path(in_path)) == NULL)
      return false;
  }
  if (!S_ISDIR(dest->inode.i_mode))
    return false;
  put_minode(running->cwd);
  running->cwd = dest;
  return true;
}