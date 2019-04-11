#include "cmd.h"

bool do_cd(cmd *c) {

  path p, *in_path = &p;
  minode *dest;
  if (c->argc < 2)
    dest = global_root_inode;
  else {
    parse_path(c->argv[1], in_path);
    if ((dest = search_path(in_path)) == NULL) {
      printf("path not found");
      return false;
    }
  }
  // if we got back a symlink
  if (S_ISLNK(dest->inode.i_mode)) {
    path s, *sym_path = &s;
    parse_path((char *)dest->inode.i_block, sym_path);
    if (!sym_path->is_absolute) {
      // replace link name with link contents and search again
      memcpy(&(in_path->argv[--(in_path->argc)]), sym_path->argv,
             sizeof(char *) * sym_path->argc);
      in_path->argc += sym_path->argc;
      dest = search_path(in_path);
    } else // search absolute path
      dest = search_path(sym_path);
  }
  if (!S_ISDIR(dest->inode.i_mode)) {
    printf("cannot cd to non-dir\n");
    return false;
  }

  if (dest == NULL) {
    printf("path not found");
  }

  put_minode(running->cwd);
  running->cwd = dest;
  DEBUG_PRINT("cwd is now %d\n", dest->ino);
  return true;
}