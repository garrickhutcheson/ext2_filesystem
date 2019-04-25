#include "cmd.h"

bool do_rmdir(cmd *c) {

  if (c->argc != 2) {
    printf("Usage: rmdir <path>\n");
    return false;
  }
  return _rmdir(c->argv[1]);
}

int _rmdir(char *dest) {
  path in_path;
  parse_path(dest, &in_path);
  minode *mip = search_path(in_path);
  if (!S_ISDIR(mip->inode.i_mode)) {
    printf("Can't rm non-directory\n");
    put_minode(mip);
    return 0;
  }
  char *bname = in_path.argv[in_path.argc - 1];
  in_path.argc--;
  minode *parent = search_path(in_path);

  // Just like in real filesystems
  if (running->uid != 0 && mip->inode.i_uid != running->uid) {
    printf("you do not have permission\n");
    put_minode(mip);
    return 0;
  }
  if (mip->ref_count > 1) {
    printf("DIR is in use\n");
    put_minode(mip);
    return 0;
  }
  if (!(mip->inode.i_links_count > 2))
    if (count_dir(mip) > 2) {
      printf("DIR is not empty\n");
      put_minode(mip);
      return 0;
    }
  int ino = mip->ino;
  // Deallocate its block and inode
  for (int i = 0; i < 12; i++) {
    if (mip->inode.i_block[i] == 0)
      continue;
    free_block(mip->dev, mip->inode.i_block[i]);
  }
  free_minode(mip);
  put_minode(mip);
  rm_dir_entry(parent, bname);
  put_minode(parent);
  return ino;
}