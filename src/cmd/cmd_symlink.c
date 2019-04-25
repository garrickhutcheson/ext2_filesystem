#include "cmd.h"

bool do_symlink(cmd *c) {
  // check enough args
  if (c->argc != 3) {
    printf("Usage: symlink <src> <dest>\n");
    return false;
  }
  return _symlink(c->argv[1], c->argv[2]);
}

int _symlink(char *src, char *dest) {

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

  if (S_ISLNK(mip->inode.i_mode)) {
    printf("cannot link a link because then you link to the link to the "
           "link...\n");
    return 0;
  }
  int ino = alloc_inode(dest_parent->dev);
  minode *child = get_minode(dest_parent->dev, ino);

  child->inode.i_mode = 0120000;     // LNK type and permissions
  child->inode.i_uid = running->uid; // Owner uid
  child->inode.i_gid = running->gid; // Group Id
  child->inode.i_size = 0;           // Size in bytes nothing
  child->inode.i_links_count = 1;    // Links count=1
  child->inode.i_atime = child->inode.i_ctime = child->inode.i_mtime = time(0L);
  child->inode.i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
  // copy path into i_block
  strcpy((char *)child->inode.i_block, src);
  child->dirty = true;

  // add child to parent
  dir_entry pcd, *parent_child_dir = &pcd;
  parent_child_dir->inode = child->ino;
  strcpy(parent_child_dir->name, bname);
  parent_child_dir->name_len = strlen(parent_child_dir->name);
  add_dir_entry(dest_parent, parent_child_dir);

  DEBUG_PRINT("symlink file with ino %d\n", child->ino);
  // write back to disk / put
  put_minode(dest_parent);
  put_minode(child);
  return ino;
}