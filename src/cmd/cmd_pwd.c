#include "cmd.h"

void print_path(minode *mip) {
  if (mip == global_root)
    printf("/");
  else {
    char buf[BLKSIZE_1024];
    char name[256];
    get_block(mip->dev, mip->inode.i_block[0], buf);
    dir_entry *this_dir = (dir_entry *)buf;
    snprintf(name, this_dir->name_len, "%s", this_dir->name);
    dir_entry *daddy_dir = (dir_entry *)buf + this_dir->rec_len;
    mip = get_inode(mip->dev, daddy_dir->inode);
    print_path(mip); // recursive call
    printf("/%s", name);
  }
}

bool do_pwd(cmd *c) {
  print_path(running->cwd);
  printf("\n");
  return true;
}
