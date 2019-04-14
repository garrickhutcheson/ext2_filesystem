#include "cmd.h"

bool do_close() {
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
}