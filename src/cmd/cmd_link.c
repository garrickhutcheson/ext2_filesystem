#include "cmd.h"

bool do_link(cmd *c) {
  // check enough args
  if (c->argc < 3) {
    printf("link requires two paths\n");
    return false;
  }
  path src_path, dest_path;
  parse_path(c->argv[1], &src_path);
  parse_path(c->argv[2], &dest_path);

  char *bname = dest_path.argv[dest_path.argc - 1];

  dest_path.argc--;
  minode *dest_parent = search_path(&dest_path);
  minode *src = search_path(&src_path);
  if (!(S_ISREG(src->inode.i_mode) || S_ISLNK(src->inode.i_mode))) {
    printf("cannot link this type of file\n");
    return false;
  }

  // add child to parent
  dir_entry de, *dep = &de;
  dep->inode = src->ino;
  strcpy(dep->name, bname);
  dep->name_len = strlen(dep->name);
  add_dir_entry(dest_parent, dep);
  src->inode.i_links_count++;

  DEBUG_PRINT("link file with ino %d", src->ino);
  // write back to disk / put
  put_minode(src);
  put_minode(dest_parent);
  return true;
}
