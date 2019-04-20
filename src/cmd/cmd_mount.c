#include "cmd.h"

bool do_mount(cmd *c) {
  if (c->argc != 3) {
    printf("Usage: umount <device> <dir>\n");
    return false;
  }
  return _mount(c->argv[1], c->argv[2]);
}

int _mount(char *dev, char *dir) {
  path mnt_path;
  minode *exists;
  parse_path(dir, &mnt_path);
  // get basename
  char *bname = mnt_path.argv[mnt_path.argc - 1];
  // check if dir already exists
  if ((exists = search_path(mnt_path))) {
    printf("%s already exists\n", dir);
    put_minode(exists);
    return 0;
  }
  mnt_path.argc--;
  // get parent directory
  minode *parent = search_path(mnt_path);
  if (!S_ISDIR(parent->inode.i_mode)) {
    printf("Can't add file to non-directory\n");
    return 0;
  }
  // make mount entry for device
  mount_entry *me;
  me = make_me(dev, dir);

  // make custom minode for mnt point
  minode *mip, *dev_mip, *dev_root;
  int mnt_ino, dev_ino;
  mnt_ino = alloc_inode(global_root_mount);
  dev_mip = alloc_minode();
  dev_mip->ino = mnt_ino;
  dev_mip->me = me;
  dev_mip->ref_count = 2;
  dev_root = get_minode(me, 2);
  dev_mip->inode = dev_root->inode;

  // add dir entry with ino == mnt point ino
  dir_entry dir, *dirp = &dir;
  dirp->inode = mnt_ino;
  strcpy(dirp->name, bname);
  dirp->name_len = strlen(dirp->name);
  add_dir_entry(parent, dirp);

  return mnt_ino;
}