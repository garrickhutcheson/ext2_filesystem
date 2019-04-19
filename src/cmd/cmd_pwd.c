#include "cmd.h"

bool do_pwd(cmd *c) {
  if (running->cwd == global_root_inode)
    printf("/\n");
  else {
    _pwd(running->cwd);
    printf("\n");
  }
  return true;
}

int _pwd(minode *mip) {
  if (mip == global_root_inode)
    ;
  else {
    char buf1[BLKSIZE_1024], *buf1p;
    char buf2[BLKSIZE_1024], *buf2p;
    char name[256] = {0};
    dir_entry *this_dir, *parent_dir, *dirp;

    get_block(mip->mount_entry, mip->inode.i_block[0], buf1);
    this_dir = (dir_entry *)buf1;
    parent_dir = (dir_entry *)(buf1 + this_dir->rec_len);
    mip = get_minode(mip->mount_entry, parent_dir->inode);

    for (int i = 0; i < 12 && !(*name); i++) { // search direct blocks only
      if (mip->inode.i_block[i] == 0)
        break;
      get_block(mip->mount_entry, mip->inode.i_block[i], buf2);
      dirp = (dir_entry *)buf2;
      buf2p = buf2;
      // todo: double check this condition
      while (buf2p < buf2 + BLKSIZE_1024) {
        dirp = (dir_entry *)buf2p;
        buf2p += dirp->rec_len;
        if (dirp->inode == this_dir->inode) {
          strncpy(name, dirp->name, dirp->name_len);
          name[dirp->name_len] = '\0';
          break;
        }
      }
    }
    put_minode(mip);
    _pwd(mip); // recursive call
    printf("/%s", name);
  }
}
