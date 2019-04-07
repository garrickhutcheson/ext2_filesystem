#include "cmd.h"

bool do_chmod(cmd *c) {
  path in_path;
  if (c->argc < 3) {
    printf("chmod requires: chmod mode filename\n");
    return false;
  }
  if (!parse_path(c->argv[2], &in_path)) {
    printf("bad path");
    return false;
  }
  minode *mip = search_path(&in_path);
  // long int strtol (const char* str, char** endptr, int base);
  // if given base == 0 then base is determined by +, -,O, OX/Ox prefix
  unsigned int mode = strtol(c->argv[1], NULL, 0);
  mip->inode.i_mode |= mode;
  mip->dirty = true;
  put_minode(mip);
  return true;
}

// 2. chmod filename mode: (mode = |rwx|rwx|rwx|, e.g. 0644 in octal)
//          get INODE of pathname into memroy:
//              ino = getino(pathname);
//              mip = iget(dev, ino);
//              mip->INODE.i_mode |= mode;
//          mip->dirty = 1;
//          iput(mip);