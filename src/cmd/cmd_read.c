#include "cmd.h"

bool do_read(cmd *c) {
  char buf[4096] = {0};
  if (c->argc < 3) {
    printf("Usage: read <fd> <bytes <= 4096>");
  }
  read_file(atoi(c->argv[1]), buf, atoi(c->argv[2]));
  printf("%s\n", buf);
  return true;
}