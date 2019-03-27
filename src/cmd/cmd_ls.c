#include "cmd.h"

bool do_ls(cmd *c) {
  dir_entry dep[4096];
  int entryc;
  if (c->argc < 2)
    entryc = list_dir(global_root, dep);
  else {
    path in_path;
    parse_path(c->argv[1], &in_path);
    minode m, *found = &m;
    found = search_path(&in_path);
    if (!found)
      return 1;
    entryc = list_dir(found, dep);
    put_inode(found);
  }
  for (int i = 0; i < entryc; i++)
    printf("%s\n", dep[i].name);

  return 0;
}
