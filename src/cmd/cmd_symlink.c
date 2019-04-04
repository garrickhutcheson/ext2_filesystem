#include "cmd.h"

bool do_symlink(cmd *c) {
  printf("command not yet implemented\n");
  return 0;
}
// todo
// 3. ======================== HOW TO symlink ================================
//    symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

//    ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

// (1). verify oldNAME exists (either a DIR or a REG file)
// (2). creat a FILE /x/y/z
// (3). change /x/y/z's type to LNK (0120000)=(1010.....)=0xA...
// (4). write the string oldNAME into the i_block[ ], which has room for 60
// chars.
//     (INODE has 24 unused bytes after i_block[]. So, up to 84 bytes for
//     oldNAME)

//      set /x/y/z file size = number of chars in oldName

// (5). write the INODE of /x/y/z back to disk.

// 4. readlink pathname: return the contents of a symLink file

// (1). get INODE of pathname into a minode[ ].
// (2). check INODE is a symbolic Link file.
// (3). return its string contents in INODE.i_block[ ].