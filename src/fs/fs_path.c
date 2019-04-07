#include "fs.h"

// search_inode() function:
// The search_inode() function implements the file system tree traversal
// algorithm. It returns the INODE number (ino) of a specified pathname. To
// begin with, we assume that in the level-1 file system implementation, the
// file system resides on a single root device, so that there are no mounted
// devices and mounting point crossings. Mounted file systems and mounting point
// crossing will be considered later in level-3 of the file system
// implementation Thus,
// the search_inode() function essentially returns the (dev, ino) of a pathname.
// The function first uses the tokenize() function to break up pathname into
// component strings. We assume that the tokenized strings are in a
// global data
// area, each pointed by a name[i] pointer and the number of
// token strings is nname. Then it calls the search() function
// to search for the token strings in
// successive directories. The following shows the tokenize() and search()
// functions.

int parse_path(char *path_name, path *buf_path) {
  char *s, safe_path[256];
  buf_path->argc = 0;

  // check if root
  if (strcmp(path_name, "/") == 0) {
    buf_path->is_root = true;
    buf_path->argc = 1;
  } else
    buf_path->is_root = false;

  // check if absolute or relative
  if (path_name[0] == '/')
    buf_path->is_absolute = true;
  else
    buf_path->is_absolute = false;

  strcpy(safe_path, path_name);
  // split into components
  s = strtok(safe_path, "/");
  while (s) {
    strcpy(buf_path->argv[buf_path->argc++], s);
    s = strtok(NULL, "/");
  }
  buf_path->argv[buf_path->argc][0] = 0;
  return buf_path->argc;
}

// todo: refactor search_dir, list_dir, count_dir

int search_dir(minode *mip, char *dir_name) {
  int i;
  char *fs_p, temp[256], buf[BLKSIZE_1024] = {0}, *b = buf;
  dir_entry *dep;
  DEBUG_PRINT("search for %s\n", dir_name);
  if (!S_ISDIR(mip->inode.i_mode)) {
    DEBUG_PRINT("search fail %s is not a dir\n", dir_name);
    return 0;
  }
  // search dir_entry direct blocks only
  for (i = 0; i < 12; i++) {
    // if direct block is null stap
    if (mip->inode.i_block[i] == 0)
      return 0;
    // get next direct block
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dep = (dir_entry *)buf;
    fs_p = buf;
    while (fs_p < buf + BLKSIZE_1024) {
      snprintf(temp, dep->name_len + 1, "%s", dep->name);
      DEBUG_PRINT("ino:%d rec_len:%d name_len:%u name:%s\n", dep->inode,
                  dep->rec_len, dep->name_len, temp);
      if (strcmp(dir_name, temp) == 0) {
        DEBUG_PRINT("found %s : inumber = %d\n", dir_name, dep->inode);
        return dep->inode;
      }
      fs_p += dep->rec_len;
      dep = (dir_entry *)fs_p;
    }
  }
  return 0;
}

// returns an array of dir_entry from a dir minode
// only supports direct blocks
// write dir entries
int list_dir(minode *mip, dir_entry *dir_arr) {
  char *fs_p, buf[BLKSIZE_1024];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode)) {
    DEBUG_PRINT("list fail ino %d is not a dir\n", mip->ino);
    return 0;
  }
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    fs_p = buf;
    // todo: double check this condition
    while (fs_p < buf + BLKSIZE_1024) {
      dirp = (dir_entry *)fs_p;
      dir_arr[dirc] = *dirp;
      dirc++;
      fs_p += dirp->rec_len;
    }
  }
  // todo: indirect and double indirect
  return dirc;
}

// returns number of dir entries in mip
int count_dir(minode *mip) {
  char buf[BLKSIZE_1024], *bufp = buf, temp[256];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode))
    return 0;
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    bufp = buf;
    // todo: double check this condition
    while (bufp < buf + BLKSIZE_1024) {
      snprintf(temp, dirp->name_len + 1, "%s", dirp->name);
      DEBUG_PRINT("ino:%d rec_len:%d name_len:%u name:%s\n", dirp->inode,
                  dirp->rec_len, dirp->name_len, temp);
      dirc++;
      bufp += dirp->rec_len;
      dirp = (dir_entry *)bufp;
    }
  }
  return dirc;
}

// must put_minode on returned minode when done
// returns found minode on success
// returns NULL on failure
minode *search_path(path *target_path) {
  minode *mip;
  int i, ino;
  if (target_path->is_root)
    return global_root_inode;
  if (target_path->is_absolute)
    mip = global_root_inode; // if absolute
  else
    mip = running->cwd; // if relative
  mip->ref_count++;

  // search for each token string
  for (i = 0; i < target_path->argc; i++) {
    // get subdir by token
    ino = search_dir(mip, target_path->argv[i]);
    if (!ino) {
      printf("no such component name %s\n", target_path->argv[i]);
      put_minode(mip);
      return NULL;
    }
    put_minode(mip);
    // switch to new minode
    mip = get_minode(mip->mount_entry, ino);
  }
  return mip;
}
