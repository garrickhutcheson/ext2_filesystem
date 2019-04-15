#include "cmd.h"

bool do_close(cmd *c) {
  if (c->argc < 2) {
    printf("Usage: close <path/to/file>\n");
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

  for (fd = 0; fd < NUM_OFT_PER || running->oft_arr[fd] == NULL; fd++) {
    if (running->oft_arr[fd]->minode->ino == mip->ino) {
      return close_file(fd);
    }
  }

  //   int close_file(int fd)
  // {
  //   1. verify fd is within range.

  //   2. verify running->fd[fd] is pointing at a OFT entry

  //   3. The following code segments should be fairly obvious:
  //      oftp = running->fd[fd];
  //      running->fd[fd] = 0;
  //      oftp->refCount--;
  //      if (oftp->refCount > 0) return 0;

  //      // last user of this OFT entry ==> dispose of the Minode[]
  //      mip = oftp->inodeptr;
  //      iput(mip);

  //      return 0;
  // }
  return true;
}