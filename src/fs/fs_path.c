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
  } else {
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
  }
  buf_path->argv[buf_path->argc][0] = 0;
  return buf_path->argc;
}

// checks directory for a file
int search_dir(minode *mip, char *dir_name) {
  int i;
  char *fs_p, temp[256], buf[BLKSIZE_1024] = {0}, *b = buf;
  dir_entry *dep;
  if (!S_ISDIR(mip->inode.i_mode))
    return 0;
  // search dir_entry direct blocks only
  for (i = 0; i < 12; i++) {
    // if direct block is null stap
    if (mip->inode.i_block[i] == 0)
      return 0;
    // get next direct block
    get_block(mip->dev, mip->inode.i_block[i], buf);
    dep = (dir_entry *)buf;
    fs_p = buf;
    while (fs_p < buf + BLKSIZE_1024) {
      snprintf(temp, dep->rec_len, "%s", dep->name);
      DEBUG_PRINT("%8d%8d%8u %s\n", dep->inode, dep->rec_len, dep->name_len,
                  temp);
      if (strncmp(dir_name, dep->name, dep->name_len) == 0) {
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
int list_dir(minode *mip, dir_entry *dir_arr) {
  char *fs_p, buf[BLKSIZE_1024];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode))
    return 0;
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->dev, mip->inode.i_block[i], buf);
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

// must put_inode on returned minode when done
minode *search_path(path *target_path) {
  minode *mip;
  int i, ino;
  if (target_path->is_root) {
    return global_root;
  }
  if (target_path->is_absolute)
    mip = global_root; // if absolute
  else
    mip = running->cwd; // if relative
  mip->refCount++;

  // search for each token string
  for (i = 0; i < target_path->argc; i++) {
    // get subdir by token
    ino = search_dir(mip, target_path->argv[i]);
    if (!ino) {
      printf("no such component name %s\n", target_path->argv[i]);
      put_inode(mip);
      return NULL;
    }
    // release current minode
    put_inode(mip);
    // switch to new minode
    mip = get_inode(mount_entry_arr[0].fd, ino);
  }
  return mip;
}
