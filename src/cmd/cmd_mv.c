#include "cmd.h"

bool do_mv(cmd *c) {

  //                       HOW TO mv (rename)
  // mv src dest:

  // 1. verify src exists; get its INODE in ==> you already know its dev
  // 2. check whether src is on the same dev as src

  //               CASE 1: same dev:
  // 3. Hard link dst with src (i.e. same INODE number)
  // 4. unlink src (i.e. rm src name from its parent directory and reduce
  // INODE's
  //                link count by 1).

  //               CASE 2: not the same dev:
  // 3. cp src to dst
  // 4. unlink src
  return true;
}