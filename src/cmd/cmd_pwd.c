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

    get_block(mip->dev, mip->inode.i_block[0], buf1);
    this_dir = (dir_entry *)buf1;
    parent_dir = (dir_entry *)(buf1 + this_dir->rec_len);
    minode *pip = get_minode(mip->dev, parent_dir->inode);

    if (mip->ino == pip->ino) {
      minode *newpip = mip->dev->mnt_pnt;
      put_minode(pip);
      pip = newpip;
    } else {

      for (int i = 0; i < 12 && !(*name); i++) { // search direct blocks only
        if (pip->inode.i_block[i] == 0)
          break;
        get_block(pip->dev, pip->inode.i_block[i], buf2);
        dirp = (dir_entry *)buf2;
        buf2p = buf2;
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
      put_minode(pip);
    }
    _pwd(pip); // recursive call
    printf("/%s", name);
  }
}
