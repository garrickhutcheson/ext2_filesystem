#include "cmd.h"

bool do_link(cmd *c) {
  if (c->argc != 3) {
    printf("Usage: link <src filename> <dest filename>\n");
    return false;
  }
  return _link(c->argv[1], c->argv[2]);
}

int _link(char *src, char *dest) {
  path src_path, dest_path;
  parse_path(src, &src_path);
  parse_path(dest, &dest_path);

  char *bname = dest_path.argv[dest_path.argc - 1];

  dest_path.argc--;
  minode *dest_parent = search_path(dest_path);
  minode *mip = search_path(src_path);
  if (!mip || !dest_parent) {
    printf("bad path\n");
    return 0;
  }

  if (!(S_ISREG(mip->inode.i_mode) || S_ISLNK(mip->inode.i_mode))) {
    printf("cannot link this type of file\n");
    return false;
  }

  // add child to parent
  dir_entry de, *dep = &de;
  dep->inode = mip->ino;
  strcpy(dep->name, bname);
  dep->name_len = strlen(dep->name);
  add_dir_entry(dest_parent, dep);
  mip->inode.i_links_count++;

  DEBUG_PRINT("ino %d link count %d\n", mip->ino, mip->inode.i_links_count);
  // write back to disk / put
  mip->dirty = true;
  put_minode(mip);
  dest_parent->dirty = true;
  put_minode(dest_parent);
  return mip->inode.i_links_count;
}
