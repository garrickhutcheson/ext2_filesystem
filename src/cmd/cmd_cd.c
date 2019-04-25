#include "cmd.h"

bool do_cd(cmd *c) {
  if (c->argc < 2)
    return _cd("/");
  else
    return _cd(c->argv[1]);
}

int _cd(char *dest) {
  path in_path;
  minode *mip;
  parse_path(dest, &in_path);
  if (!(mip = search_path(in_path))) {
    printf("path not found\n");
    return 0;
  }
  // if we got back a symlink
  if (S_ISLNK(mip->inode.i_mode)) {
    path sym_path;
    parse_path((char *)mip->inode.i_block, &sym_path);
    if (!sym_path.is_absolute) {
      // replace link name with link contents and search again
      memcpy(&(in_path.argv[--(in_path.argc)]), sym_path.argv,
             sizeof(char *) * sym_path.argc);
      in_path.argc += sym_path.argc;
      mip = search_path(in_path);
    } else // search absolute path
      mip = search_path(sym_path);
  }
  if (!S_ISDIR(mip->inode.i_mode)) {
    printf("cannot cd to non-dir\n");
    return 0;
  }

  if (mip == NULL) {
    printf("path not found\n");
  }

  put_minode(running->cwd);
  running->cwd = mip;
  DEBUG_PRINT("cwd is now %d\n", mip->ino);
  return mip->ino;
}