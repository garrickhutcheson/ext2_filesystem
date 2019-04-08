#include "cmd.h"

bool do_symlink(cmd *c) {
  // check enough args
  if (c->argc < 3) {
    printf("symlink requires two paths\n");
    return false;
  }
  path src_path, dest_path;
  parse_path(c->argv[1], &src_path);
  parse_path(c->argv[2], &dest_path);

  char *bname = dest_path.argv[dest_path.argc - 1];

  dest_path.argc--;
  minode *dest_parent = search_path(&dest_path);
  minode *src = search_path(&src_path);
  if (!src || !dest_parent) {
    printf("bad path\n");
    return false;
  }

  if (S_ISLNK(src->inode.i_mode)) {
    printf(
        "cannot link a link because then you link to the link to the link\n");
    return false;
  }
  int ino = alloc_inode(dest_parent->mount_entry);
  minode *child = get_minode(dest_parent->mount_entry, ino);

  child->inode.i_mode = 0120000;     // LNK type and permissions
  child->inode.i_uid = running->uid; // Owner uid
  child->inode.i_gid = running->gid; // Group Id
  child->inode.i_size = 0;           // Size in bytes nothing
  child->inode.i_links_count = 1;    // Links count=1
  child->inode.i_atime = child->inode.i_ctime = child->inode.i_mtime = time(0L);
  child->inode.i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
  // copy path into i_block
  strcpy((char *)child->inode.i_block, c->argv[1]);
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
  return true;
}
//    symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

//    ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

// (1). verify oldNAME exists (either a DIR or a REG file)
// (2). creat a FILE /x/y/z
// (3). change /x/y/z's type to LNK (0120000)=(1010.....)=0xA...
// (4). write the string oldNAME into the i_block[ ], which has room for 60
// chars.
//     (INODE has 24 unused bytes after i_block[]. So, up to 84 bytes for
//     oldNAME)

//      set /x/y/z file size = number of chars in oldName

// (5). write the INODE of /x/y/z back to disk.

// 4. readlink pathname: return the contents of a symLink file

// (1). get INODE of pathname into a minode[ ].
// (2). check INODE is a symbolic Link file.
// (3). return its string contents in INODE.i_block[ ].