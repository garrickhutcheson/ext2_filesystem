#include "cmd.h"

bool do_cp(cmd *c) {
  if (c->argc != 3) {
    printf("Usage: cp <src filename> <dest filename>\n");
    return false;
  }
  return _cp(c->argv[1], c->argv[2]);
}

int _cp(char *src, char *dest) {
  minode *src_mip, *dest_mip;
  int src_fd, dest_fd, n, copied = 0;
  char buf[BLKSIZE_1024];
  // open src for READ;
  if ((src_fd = open_file(src, 0)) < 0) {
    printf("failed to open src for read\n");
    return 0;
  }
  // creat dst if not exist
  if (_creat(dest))
    DEBUG_PRINT("creat %s", dest);
  // open dst for WR;
  if ((dest_fd = open_file(dest, 2)) < 0) {
    printf("failed to open dest for read\n");
    return 0;
  }
  // copy data
  while (n = read_file(src_fd, buf, BLKSIZE_1024)) {
    int written = write_file(dest_fd, buf, n);
    DEBUG_PRINT("wrote %d\n", written);
    copied += n;
  }

  // close src;
  if ((close_file(src_fd)) < 0) {
    printf("failed to close src\n");
    return 0;
  }
  // close dest;
  if ((close_file(dest_fd)) < 0) {
    printf("failed to close dest\n");
    return 0;
  }
  return copied;
}