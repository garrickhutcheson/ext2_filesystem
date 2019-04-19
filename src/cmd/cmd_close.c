#include "cmd.h"

bool do_close(cmd *c) {
  if (c->argc != 2) {
    printf("Usage: close <filename>\n");
    return false;
  }

  int fd;
  struct path p;
  parse_path(c->argv[1], &p);
  minode *mip = NULL;
  if (!(mip = search_path(p)) || !S_ISREG(mip->inode.i_mode)) {
    put_minode(mip);
    return 0;
  }

  for (fd = 0; fd < NUM_OFT_PER && !(running->oft_arr[fd] == NULL); fd++) {
    if (running->oft_arr[fd]->minode->ino == mip->ino) {
      put_minode(mip);
      return close_file(fd);
    }
  }

  return true;
}
