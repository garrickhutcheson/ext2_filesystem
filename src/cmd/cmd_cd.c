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
  if (S_ISLNK(dest->inode.i_mode)) {
    path sym_path;
    parse_path((char *)dest->inode.i_block, &sym_path);
    dest = search_path(&sym_path);
  }
  if (!S_ISDIR(dest->inode.i_mode)) {
    printf("cannot cd to non-dir\n");
    return false;
  }
  put_minode(running->cwd);
  running->cwd = dest;
  DEBUG_PRINT("cwd is now %d\n", dest->ino);
  return true;
}