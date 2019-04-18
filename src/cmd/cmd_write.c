#include "cmd.h"

bool do_write(cmd *c) {
  char buf[4096];
  int wrote;
  if (c->argc < 3) {
    printf("Usage: write <fd> <message>\n");
  }
  strcpy(buf, c->argv[2]);
  wrote = write_file(atoi(c->argv[1]), buf, strlen(buf));
  printf("wrote %d bytes\n", wrote);
  return true;
}