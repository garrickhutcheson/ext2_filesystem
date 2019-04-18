#include "cmd.h"

bool do_cat(cmd *c) {

  char mybuf[1024], dummy = 0; // a null char at end of mybuf[ ]
  int n;

  int fd = open_file(c->argv[1], 0);
  while ((n = read_file(fd, mybuf, 1024))) {
    mybuf[n] = 0;        // as a null terminated string
    printf("%s", mybuf); // <=== THIS works but not good
  }
  close_file(fd);
  return true;
}
