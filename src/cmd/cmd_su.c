#include "cmd.h"

bool do_su(cmd *c) {
  // check enough args
  if (c->argc != 2) {
    printf("Usage: su <uid>\n");
    return false;
  }
  running->uid = atoi(c->argv[1]);
  return true;
}
