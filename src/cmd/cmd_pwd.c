#include "cmd.h"

bool do_pwd(cmd *c) {
  if (running->cwd == global_root_inode)
    printf("/\n");
  else {
    print_path(running->cwd);
    printf("\n");
  }
  return true;
}
