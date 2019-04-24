#include "cmd.h"

bool do_umount(cmd *c) {
  if (c->argc != 2) {
    printf("Usage: umount <dir>\n");
    return false;
  }
  return _umount(c->argv[1]);
}

int _umount(char *dir) {
  printf("not implemented\n");
  return 0;
}