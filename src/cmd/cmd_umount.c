#include "cmd.h"

bool do_umount(cmd *c) {
  if (c->argc != 2) {
    printf("Usage: umount <dir>\n");
    return false;
  }
  return _umount(c->argv[1]);
}

int _umount(char *dir) {
  // find mount minode
  path mnt_path;
  parse_path(dir, &mnt_path);
  minode *mip;
  if (!(mip = search_path(mnt_path)) || !(mip->ino == 2)) {
    printf("not a mount point\n");
    return 0;
  }

  minode *newmip = mip->dev->mnt_pnt;
  put_minode(mip);
  mip = newmip;

  // get mount entry
  mount_entry *me = mip->mnt;

  // check if dev in use
  for (int i = 0; i < NUM_MINODES; i++) {
    if ((minode_arr[i].ref_count) && (minode_arr[i].dev == me)) {
      printf("cannot umount, device in use");
      return 0;
    }
  }

  // free me
  for (int i = 0; i < NUM_MOUNT_ENTRIES + 1; i++) {
    if (i == NUM_MOUNT_ENTRIES) {
      printf("could not locate mount entry\n");
      return 0;
    }
    if (&mount_entry_arr[i] == me) {
      DEBUG_PRINT("closing fd %d\n", mount_entry_arr[i].fd);
      close(mount_entry_arr[i].fd);
      mount_entry_arr[i].fd = 0;
      break;
    }
  }

  // set ref count zero and not dirty
  mip->ref_count = 1;
  mip->dirty = false;
  // put
  put_minode(mip);
  return 0;
}