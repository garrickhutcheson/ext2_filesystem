#include "cmd.h"

bool do_mount(cmd *c) {
  if (c->argc == 1) {
    for (int i = 0; i < NUM_MOUNT_ENTRIES; i++) {
      mount_entry *me = &mount_entry_arr[i];
      if (me->fd)
        printf("me: %s at %s with fd %d\n", me->dev_path, me->mnt_path, me->fd);
    }
    return true;
  } else if (c->argc == 3) {
    return _mount(c->argv[1], c->argv[2]);
  } else {
    printf("Usage: mount <device> <dir>\n");
    return false;
  }
}

int _mount(char *dev, char *dir) {

  // check if already mounted
  for (int i = 0; i < NUM_MOUNT_ENTRIES; i++) {
    if (mount_entry_arr[i].fd && (!strcmp(dev, mount_entry_arr[i].dev_path))) {
      printf("cannot mount, you already mount! why?\n");
      return 0;
    }
  }

  // make a directory at dir
  _mkdir(dir);

  // search for the directory we just made
  minode *mnt_dir;
  path dir_path;
  parse_path(dir, &dir_path);
  if (!(mnt_dir = search_path(dir_path)) || !S_ISDIR(mnt_dir->inode.i_mode)) {
    printf("bad path given for mount point\n");
    put_minode(mnt_dir);
    return false;
  }

  // make mount entry for device
  mount_entry *me = make_me(dev, dir);

  // set device mount point to minode
  me->mnt_pnt = mnt_dir;

  // shadow new dir's ino with mnt point
  mnt_dir->mnt = me;
  mnt_dir->dirty = false;

  return mnt_dir->ino;
}
