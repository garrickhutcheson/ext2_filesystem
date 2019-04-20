#include "cmd.h"

bool do_cat(cmd *c) {

  if (c->argc != 2) {
    printf("Usage: cat <filename>\n");
    return false;
  }

  char mybuf[1024], dummy = 0; // a null char at end of mybuf[ ]
  int n, fd;

  if (fd = open_file(c->argv[1], 0) == -1) {
    printf("can't open file for read\n");
    return false;
  }
  while ((n = read_file(fd, mybuf, 1024))) {
    mybuf[n] = 0;        // as a null terminated string
    printf("%s", mybuf); // <=== THIS works but not good
  }
  printf("\n");
  if (close_file(fd) < 0) {
    printf("fail to close file\n");
    return false;
  }
  return true;
}
