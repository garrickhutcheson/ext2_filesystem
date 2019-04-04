#include "cmd.h"

bool do_chmod(cmd *c) {
  printf("command not yet implemented\n");
  return 0;
}

// 2. chmod filename mode: (mode = |rwx|rwx|rwx|, e.g. 0644 in octal)
//          get INODE of pathname into memroy:
//              ino = getino(pathname);
//              mip = iget(dev, ino);
//              mip->INODE.i_mode |= mode;
//          mip->dirty = 1;
//          iput(mip);