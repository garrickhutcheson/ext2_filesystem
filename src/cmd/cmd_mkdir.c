#include "cmd.h"

bool do_mkdir(cmd *c) {

  if (c->argc != 2) {
    printf("Usage: mkdir <dest>\n");
    return false;
  }

  return _mkdir(c->argv[1]);
}

int _mkdir(char *dest) {
  minode *exists;
  path in_path;
  parse_path(dest, &in_path);
  char *bname = in_path.argv[in_path.argc - 1];
  if ((exists = search_path(in_path))) {
    printf("%s already exists\n", dest);
    put_minode(exists);
    return 0;
  }
  in_path.argc--;
  minode *parent = search_path(in_path);
  if (!S_ISDIR(parent->inode.i_mode)) {
    printf("Can't add file to non-directory\n");
    return 0;
  }
  int ino = alloc_inode(parent->dev);

  minode *child = get_minode(parent->dev, ino);
  child->inode.i_mode = 0x41ED;       // OR 040755: DIR type and permissions
  child->inode.i_uid = running->uid;  // Owner uid
  child->inode.i_gid = running->gid;  // Group Id
  child->inode.i_size = BLKSIZE_1024; // Size in bytes
  child->inode.i_links_count = 0;     // incremented in add_dir_entry
  child->inode.i_atime = child->inode.i_ctime = child->inode.i_mtime = time(0L);
  child->inode.i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  for (int i = 0; i < 15; i++)
    child->inode.i_block[i] = 0;

  child->dirty = true;

  // make .
  dir_entry cd, *child_dir = &cd;
  child_dir->inode = child->ino;
  strcpy(child_dir->name, ".");
  child_dir->name_len = strlen(child_dir->name);
  add_dir_entry(child, child_dir);
  // make ..
  dir_entry pd, *parent_dir = &pd;
  parent_dir->inode = parent->ino;
  strcpy(parent_dir->name, "..");
  parent_dir->name_len = strlen(parent_dir->name);
  add_dir_entry(child, parent_dir);
  // add child to parent
  dir_entry pcd, *parent_child_dir = &pcd;
  parent_child_dir->inode = child->ino;
  strcpy(parent_child_dir->name, bname);
  parent_child_dir->name_len = strlen(parent_child_dir->name);
  add_dir_entry(parent, parent_child_dir);

  // write back to disk / put
  put_minode(parent);
  put_minode(child);
  return ino;
}