#include "fs.h"
#include "string.h"
#include <unistd.h>

//// ALLOC AND FREE

// returns the ino of the next available inode in inode_bitmap
// returns 0 if no more inodes, modifies inode_bitmap
int alloc_inode(mount_entry *me) {
  char buf[BLKSIZE_1024];
  get_block(me, me->group_desc.bg_inode_bitmap, buf);
  for (int i = 0; i < me->super_block.s_inodes_count; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      me->group_desc.bg_free_inodes_count--;
      put_block(me, me->group_desc.bg_inode_bitmap, buf);
      return i + 1;
    }
  }
  return 0;
}

// Marks the given ino in inode_bitmap as available
// returns 1
int free_inode(mount_entry *me, int ino) {
  char buf[BLKSIZE_1024];
  get_block(me, me->group_desc.bg_inode_bitmap, buf);
  clr_bit(buf, ino - 1);
  me->group_desc.bg_free_inodes_count++;
  put_block(me, me->group_desc.bg_inode_bitmap, buf);
  return 1;
}

// returns bno of next available block in block_bitmap
// returns 0 if out of blocks, modifies block_bitmap
int alloc_block(mount_entry *me) {
  char buf[BLKSIZE_1024];

  // read block_bitmap block
  get_block(me, me->group_desc.bg_block_bitmap, buf);

  for (int i = 0; i < me->super_block.s_blocks_count; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      me->group_desc.bg_free_blocks_count--;
      put_block(me, me->group_desc.bg_block_bitmap, buf);
      return i + 1;
    }
  }
  return 0;
}

// Marks the given bno in block_bitmap as available
int free_block(mount_entry *me, int bno) {
  char buf[BLKSIZE_1024];
  get_block(me, me->group_desc.bg_block_bitmap, buf);
  clr_bit(buf, bno - 1);
  me->group_desc.bg_free_blocks_count++;
  put_block(me, me->group_desc.bg_block_bitmap, buf);
  return 1;
}

// GET PUT BLOCK

// read block to buf from disk
// return 1 on success, 0 on failure
int get_block(mount_entry *me, int bno, char *buf) {
  lseek(me->fd, bno * BLKSIZE_1024, SEEK_SET);
  int n = read(me->fd, buf, BLKSIZE_1024);
  if (n < 0) {
    printf("get_block[% d % d] error \n", me->fd, bno);
    return 0;
  }
  return 1;
}

// write block from buf to disk
// return 1 on success, 0 on failure
int put_block(mount_entry *me, int bno, char *buf) {
  lseek(me->fd, bno * BLKSIZE_1024, SEEK_SET);
  int n = write(me->fd, buf, BLKSIZE_1024);
  if (n != BLKSIZE_1024) {
    printf("put_block [%d %d] error\n", me->fd, bno);
    return 0;
  }
  return 1;
}

// BIT OPERATIONS

// tests the nth bit of buf
// return value of bit
int tst_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  if (buf[i] & (1 << j))
    return 1;
  return 0;
}

// sets the nth bit of buf to 1
// returns 1
int set_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] |= (1 << j);
  return 1;
}

// sets the nth bit of buf to 0
// returns 1
int clr_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] &= ~(1 << j);
  return 1;
}

//// ADD REMOVE DIR

// returns the closest size as a multiple of 4 which can contain *dirp
int ideal_len(dir_entry *dirp) {
  int ideal = 4 * ((8 + dirp->name_len + 3) / 4);
  return ideal;
}

// creates new_dirp in mip's i_block[]
// new_dirp must have name, name_len, and inode set
// return rec_len on success, 0 on failure
// increments mip link count, but does not put inode
int add_dir_entry(minode *mip, dir_entry *new_dirp) {
  char buf[BLKSIZE_1024], *bufp = buf, name[256];
  dir_entry *cur_dirp;
  int free_space;
  snprintf(name, new_dirp->name_len + 1, "%s", new_dirp->name);
  if (search_dir(mip, name)) {
    printf("dir_entry by name of %s already exists\n", name);
    return 0;
  }

  // check if dir
  if (!S_ISDIR(mip->inode.i_mode)) {
    printf("cannot mkdir in a file\n");
    return 0;
  }

  // set new_dirp rec_len to ideal
  new_dirp->rec_len = ideal_len(new_dirp);

  // iterate through direct blocks
  for (int i = 0; i < 12; i++) {
    // if allocating a new block insert record as first entry
    if (mip->inode.i_block[i] == 0) {
      mip->inode.i_block[i] = alloc_block(mip->dev);
      get_block(mip->dev, mip->inode.i_block[i], buf);
      cur_dirp = (dir_entry *)buf;
      *cur_dirp = *new_dirp;
      cur_dirp->rec_len = BLKSIZE_1024;
      put_block(mip->dev, mip->inode.i_block[i], buf);
      mip->inode.i_links_count++;
      return cur_dirp->rec_len;
    }
    // else
    get_block(mip->dev, mip->inode.i_block[i], buf);
    bufp = buf;
    // iterate through dir_entries to find space
    while (bufp < buf + BLKSIZE_1024) {
      cur_dirp = (dir_entry *)bufp;

      // check if space to insert then break
      free_space = cur_dirp->rec_len - ideal_len(cur_dirp);
      if (free_space > new_dirp->rec_len) {
        cur_dirp->rec_len = ideal_len(cur_dirp);
        bufp += cur_dirp->rec_len;
        int new_dirp_size = new_dirp->rec_len;
        new_dirp->rec_len = free_space;
        // using a memcpy here avoids over-writing the end of the buffer
        memcpy(bufp, new_dirp, new_dirp->rec_len);
        // write buffer back to block
        put_block(mip->dev, mip->inode.i_block[i], buf);
        mip->inode.i_links_count++;
        return new_dirp->rec_len;
      }

      bufp += cur_dirp->rec_len;
    }
  }
  return 0;
}

// removes dir with dir.name equal dir_name from mip
// return rec_len of last dir on success, 0 on failure
// decrements mip link_count, does not put
int rm_dir_entry(minode *mip, char *dir_name) {
  int i;
  char buf[BLKSIZE_1024], *bufp, *prev;
  char str[256];
  dir_entry *dep;
  int freed_space;
  if (!S_ISDIR(mip->inode.i_mode)) {
    DEBUG_PRINT("attempt to remove non-dir");
    return 0;
  }
  // search dir_entry direct blocks only
  for (i = 0; i < 12; i++) {
    if (mip->inode.i_block[i] == 0) {
      DEBUG_PRINT("dir_entry not found");
      return 0;
    }
    get_block(mip->dev, mip->inode.i_block[i], buf);
    dep = (dir_entry *)buf;
    bufp = buf;
    while (bufp < buf + BLKSIZE_1024) {
      snprintf(str, dep->name_len + 1, "%s", dep->name);
      if (strcmp(dir_name, str) == 0) {
        // if it's the only entry
        if (bufp == buf) {
          free_block(mip->dev, mip->inode.i_block[i]);
          mip->inode.i_block[i] = 0;
          // if last entry
        } else if (bufp + dep->rec_len >= buf + BLKSIZE_1024) {
          ((dir_entry *)prev)->rec_len += dep->rec_len;
          // if middle entry
        } else {
          dep = (dir_entry *)bufp;
          freed_space = dep->rec_len;
          // copy everything in block after current record onto current record
          memcpy(bufp, bufp + dep->rec_len,
                 (buf + BLKSIZE_1024) - (bufp + dep->rec_len));
          // find last record
          while (bufp + dep->rec_len < buf + BLKSIZE_1024 - freed_space) {
            bufp += dep->rec_len;
            dep = (dir_entry *)bufp;
          }
          // give him some extra room
          dep->rec_len += freed_space;
        }
        put_block(mip->dev, mip->inode.i_block[i], buf);
        mip->inode.i_links_count--;
        mip->inode.i_atime = mip->inode.i_ctime = mip->inode.i_mtime = time(0L);
        mip->dirty = true;
        return dep->rec_len;
      }
      prev = bufp;
      bufp += dep->rec_len;
      dep = (dir_entry *)bufp;
    }
  }
  return 0;
}

//// MISC

// frees all blocks (direct,indirect, etc..) from mip->inode.i_block[]
// return num blocks freed
int free_i_block(minode *mip) {
  char buf1[BLKSIZE_1024], buf2[BLKSIZE_1024], buf3[BLKSIZE_1024];
  int *fs_p1, *fs_p2, *fs_p3, freed_blocks = 0;
  path in_path;
  // direct blocks
  for (int i = 0; i < 12 && mip->inode.i_block[i]; i++)
    freed_blocks += free_block(mip->dev, mip->inode.i_block[i]);

  // indirect blocks
  if (!mip->inode.i_block[12])
    return freed_blocks;
  get_block(mip->dev, mip->inode.i_block[12], buf1);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024)) {
    freed_blocks += free_block(mip->dev, *fs_p1);
    fs_p1++;
  }

  // double indirect blocks
  if (!mip->inode.i_block[13])
    return freed_blocks;
  get_block(mip->dev, mip->inode.i_block[13], buf1);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024)) {
    get_block(mip->dev, *fs_p1, buf2);
    fs_p2 = (int *)buf2;
    while (*fs_p2 && ((char *)fs_p2 < buf2 + BLKSIZE_1024))
      freed_blocks += free_block(mip->dev, *fs_p2);
    fs_p1++;
  }

  // triple indirect blocks
  if (!mip->inode.i_block[14])
    return freed_blocks;
  get_block(mip->dev, mip->inode.i_block[14], buf1);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024)) {
    get_block(mip->dev, *fs_p1, buf2);
    fs_p2 = (int *)buf2;
    while (*fs_p2 && ((char *)fs_p2 < buf2 + BLKSIZE_1024)) {
      get_block(mip->dev, *fs_p2, buf3);
      fs_p3 = (int *)buf3;
      while (*fs_p3 && ((char *)fs_p3 < buf3 + BLKSIZE_1024))
        freed_blocks += free_block(mip->dev, *fs_p3);
      fs_p2++;
    }
    fs_p1++;
  }

  put_minode(mip);
  return freed_blocks;
}
