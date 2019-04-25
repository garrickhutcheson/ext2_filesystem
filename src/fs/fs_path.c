#include "fs.h"

// set if path relative/absolute/root in buf
// split path_name on "/" into argv and argc of buf
// return argc
int parse_path(char *path_name, path *buf_path) {
  char *s, safe_name[256];
  strcpy(safe_name, path_name);
  buf_path->argc = 0;

  // check if absolute or relative
  if (safe_name[0] == '/')
    buf_path->is_absolute = true;
  else
    buf_path->is_absolute = false;

  // split into components
  s = strtok(safe_name, "/");
  while (s) {
    strcpy(buf_path->argv[buf_path->argc++], s);
    s = strtok(NULL, "/");
  }
  buf_path->argv[buf_path->argc][0] = 0;
  return buf_path->argc;
}

// iterates through i_block of mip
// return ino of dir with dir_name on success
// return 0 on failure
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
    get_block(mip->dev, mip->inode.i_block[i], buf);
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

// iterate through i_block of mip and store in dir_arr
// return dirc on success, return 0 on failure
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
    get_block(mip->dev, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    fs_p = buf;
    while (fs_p < buf + BLKSIZE_1024) {
      dirp = (dir_entry *)fs_p;
      dir_arr[dirc] = *dirp;
      dirc++;
      fs_p += dirp->rec_len;
    }
  }
  return dirc;
}

// returns count of dir entries in mip on success
// returns 0 on failure
int count_dir(minode *mip) {
  char buf[BLKSIZE_1024], *bufp = buf, temp[256];
  dir_entry *dirp;
  int dirc = 0;
  if (!S_ISDIR(mip->inode.i_mode))
    return 0;
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return dirc;
    get_block(mip->dev, mip->inode.i_block[i], buf);
    dirp = (dir_entry *)buf;
    bufp = buf;
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

// returns minode of path on success
// returns NULL on failure
// does not put found minode
minode *search_path(path target_path) {
  minode *prev_mip, *mip = global_root_inode;
  int ino;
  if (!target_path.is_absolute)
    mip = running->cwd; // if relative
  mip->ref_count++;

  // search for each token string
  for (int i = 0; i < target_path.argc; i++) {

    // find component
    ino = search_dir(mip, target_path.argv[i]);

    // special case
    // traverse up to mnt point from mnt device
    if (ino == 2 && mip->ino == 2) {
      minode *newmip = mip->dev->mnt_pnt;
      put_minode(mip);
      mip = newmip;
    }

    // bad path
    if (!ino) {
      DEBUG_PRINT("no such component name %s\n", target_path.argv[i]);
      put_minode(mip);
      return NULL;
    }

    minode *prev_mip = mip;
    mip = get_minode(mip->dev, ino);

    // traverse down to mnt device
    if (mip->mnt) {
      minode *newmip = get_minode(mip->mnt, 2);
      mip = newmip;
    }

    if (S_ISLNK(mip->inode.i_mode)) { // handle symlink
      if (i == target_path.argc - 1)  // if last entry return symlink
        return mip;
      path sym_path;
      parse_path((char *)mip->inode.i_block, &sym_path);
      if (sym_path.is_absolute) { // recurse and continue iteration on new mip
        put_minode(mip);
        mip = search_path(sym_path);
      } else // append sym_path to target_path and continue iteration
      {
        memcpy(&target_path.argv[i + sym_path.argc], &target_path.argv[i + 1],
               sizeof(char *) * sym_path.argc);
        memcpy(&target_path.argv[i], sym_path.argv,
               sizeof(char *) * sym_path.argc);
        target_path.argc += sym_path.argc - 1;
        i--;
        put_minode(mip);
        mip = prev_mip;
        continue;
      }
    }
    put_minode(prev_mip);
  }
  return mip;
}
