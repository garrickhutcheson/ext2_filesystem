#include "cmd.h"

bool do_open(cmd *c) {
  if (c->argc < 3) {
    printf("Usage: open <path/to/file> <mode: (0|1|2|3 or R|W|RW|APPEND)>\n");
    return false;
  }
  int mode;
  if (!strcmp(c->argv[2], "R") || !strcmp(c->argv[2], "0"))
    mode = 0;
  else if (!strcmp(c->argv[2], "W") || !strcmp(c->argv[2], "1")) {
    mode = 1;
  } else if (!strcmp(c->argv[2], "RW") || !strcmp(c->argv[2], "2"))
    mode = 2;
  else if (!strcmp(c->argv[2], "APPEND") || !strcmp(c->argv[2], "3")) {
    mode = 3;
  } else {
    printf("Usage: open <path/to/file> <mode: (0|1|2|3 or R|W|RW|APPEND)>\n");
    return false;
  }

  int fd = open_file(c->argv[1], mode);
  if (fd == -1)
    printf("Usage: open <path/to/file> <mode: (0|1|2|3 or R|W|RW|APPEND)>\n");
  return (bool)fd;
}