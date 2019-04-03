#include "cmd.h"

bool do_creat(cmd *c) {
  path in_path;
  parse_path(c->argv[1], &in_path);
  if (c->argc < 2) {
    printf("mkdir requires path\n");
    return false;
  }
  char *bname = in_path.argv[in_path.argc - 1];

  in_path.argc--;
  minode *parent = search_path(&in_path);
  if (!S_ISDIR(parent->inode.i_mode)) {
    printf("Can't add file to non-directory\n");
    return false;
  }
  int ino = alloc_inode(parent->mount_entry);

  minode *child = get_minode(parent->mount_entry, ino);
  child->inode.i_mode = 0x81A4;      // OR 0100644: REG type and permissions
  child->inode.i_uid = running->uid; // Owner uid
  child->inode.i_gid = running->gid; // Group Id
  child->inode.i_size = 0;           // Size in bytes nothing in file
  child->inode.i_links_count = 1;    // Links count=1 because REG
  child->inode.i_atime = child->inode.i_ctime = child->inode.i_mtime = time(0L);
  child->inode.i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
  for (int i = 0; i < 15; i++)
    child->inode.i_block[i] = 0;
  child->dirty = true;

  // add child to parent
  dir_entry pcd, *parent_child_dir = &pcd;
  parent_child_dir->inode = child->ino;
  strcpy(parent_child_dir->name, bname);
  parent_child_dir->name_len = strlen(parent_child_dir->name);
  add_dir_entry(parent, parent_child_dir);

  // write back to disk / put
  put_minode(parent);
  put_minode(child);
  return true;
}
