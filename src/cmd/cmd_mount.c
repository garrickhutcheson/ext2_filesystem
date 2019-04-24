#include "cmd.h"

bool do_mount(cmd *c) {
  if (c->argc != 3) {
    printf("Usage: mount <device> <dir>\n");
    return false;
  }
  return _mount(c->argv[1], c->argv[2]);
}

int _mount(char *dev, char *dir) {

  // make a directory at dir
  _mkdir(dir);
  // search for the directory we just made
  minode *mip;
  path dir_path;
  parse_path(dir, &dir_path);
  if (!(mip = search_path(dir_path))) {
    printf("bad path given for mount point\n");
    put_minode(mip);
    return false;
  }
  // make mount entry for device
  mount_entry *me = make_me(dev, dir);

  // set device mount point to minode
  inode root = me->mnt_pnt->inode;
  put_minode(me->mnt_pnt);
  me->mnt_pnt = mip;

  // shadow new dir's ino with mnt point
  mip->mnt = me;
  mip->inode = root;
  mip->dirty = false;

  return mip->ino;
}
