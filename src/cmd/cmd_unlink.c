#include "cmd.h"

bool do_unlink(cmd *c) {
  path in_path;
  minode *mip, *parent;
  if (c->argc < 2) {
    printf("unlink requires: unlink filename\n");
    return false;
  }
  if (!parse_path(c->argv[1], &in_path) || !(mip = search_path(in_path))) {
    printf("bad path");
    return false;
  }
  if (S_ISDIR(mip->inode.i_mode)) {
    printf("Can't unlink directory\n");
    put_minode(mip);
    return false;
  }
  char *bname = in_path.argv[in_path.argc - 1];
  in_path.argc--;
  parent = search_path(in_path);
  mip->inode.i_links_count--;
  if (!mip->inode.i_links_count) {
    free_i_block(mip);
    free_inode(mip->mount_entry, mip->ino);
  }
  rm_dir_entry(parent, bname);
  mip->dirty = true;
  put_minode(mip);
  parent->dirty = true;
  put_minode(parent);
  return true;
}
// todo

// (4). if i_links_count == 0 ==> rm pathname by

//         deallocate its data blocks by:

//      Write a truncate(INODE) function, which deallocates ALL the data blocks
//      of INODE. This is similar to printing the data blocks of INODE.

//         deallocate its INODE;

// (5). Remove childName = basename(pathname) from the parent directory by

//         rm_child(parentInodePtr, childName)

//      which is the SAME as that in rmdir or unlink file operations.