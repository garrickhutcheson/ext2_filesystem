#include "cmd.h"

bool do_lseek(cmd *c) {

  if (c->argc != 4) {
    printf("Usage: lseek <fd> <offset> <position 0|1|2 = "
           "SEET_SET|SEET_CUR|SEEK_END>\n");
    return false;
  }
  return lseek_file(atoi(c->argv[1]), atoi(c->argv[2]), atoi(c->argv[3])) == -1
             ? false
             : true;
}