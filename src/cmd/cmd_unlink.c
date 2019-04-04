#include "cmd.h"

bool do_unlink(cmd *c) {
  printf("command not yet implemented\n");
  return 0;
}
// todo
// 2.                     HOW TO unlink

//      unlink pathname

// (1). get pathname's INODE into memory

// (2). verify it's a FILE (REG or LNK), can not be a DIR;

// (3). decrement INODE's i_links_count by 1;

// (4). if i_links_count == 0 ==> rm pathname by

//         deallocate its data blocks by:

//      Write a truncate(INODE) function, which deallocates ALL the data blocks
//      of INODE. This is similar to printing the data blocks of INODE.

//         deallocate its INODE;

// (5). Remove childName = basename(pathname) from the parent directory by

//         rm_child(parentInodePtr, childName)

//      which is the SAME as that in rmdir or unlink file operations.