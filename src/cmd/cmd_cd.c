#include "cmd.h"

bool do_cd(cmd *c) {
  minode *dest;
  if (c->argc < 2)
    dest = global_root;
  else {
    path p, *in_path = &p;
    parse_path(c->argv[1], in_path);
    dest = search_path(in_path);
  }
  if (!check_mode(&dest->inode, DIR_FILE))
    return false;
  put_minode(running->cwd);
  running->cwd = dest;
  return true;
}