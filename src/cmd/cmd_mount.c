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
  minode *mnt_dir;
  path dir_path;
  parse_path(dir, &dir_path);
  if (!(mnt_dir = search_path(dir_path))) {
    printf("bad path given for mount point\n");
    put_minode(mnt_dir);
    return false;
  }

  // make mount entry for device
  mount_entry *me = make_me(dev, dir);

  // set device mount point to minode
  me->mnt_pnt = mnt_dir;

  minode *root = get_minode(me, 2);
  mnt_dir->inode = root->inode;

  // shadow new dir's ino with mnt point
  mnt_dir->mnt = me;
  mnt_dir->dirty = false;

  return mnt_dir->ino;
}
