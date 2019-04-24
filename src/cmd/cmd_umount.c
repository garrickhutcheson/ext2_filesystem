#include "cmd.h"

bool do_umount(cmd *c) {
  if (c->argc != 2) {
    printf("Usage: umount <dir>\n");
    return false;
  }
  return _umount(c->argv[1]);
}
// TODO: write back to disk
int _umount(char *dir) {
  path mnt_path;
  parse_path(dir, &mnt_path);
  minode *mip = search_path(mnt_path);
  mip->ref_count = 0;
  mip->dirty = false;
  put_minode(mip);
  return 0;
}