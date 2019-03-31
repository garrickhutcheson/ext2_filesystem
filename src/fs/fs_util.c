#include "fs.h"
#include "string.h"
#include <unistd.h>

// returns the ino of the next available inode in inode_bitmap
// returns 0 if outta room
// modifies inode_bitmap
int alloc_inode(mount_entry *me) {
  char buf[BLKSIZE_1024];
  // read inode_bitmap block
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

// read block to buffer
bool get_block(mount_entry *me, int blk, char *buf) {
  lseek(me->fd, blk * BLKSIZE_1024, SEEK_SET);
  int n = read(me->fd, buf, BLKSIZE_1024);
  if (n < 0) {
    printf("get_block[% d % d] error \n", me->fd, blk);
    return false;
  }
  return true;
}

// write block from buffer
bool put_block(mount_entry *me, int blk, char *buf) {
  lseek(me->fd, blk * BLKSIZE_1024, SEEK_SET);
  int n = write(me->fd, buf, BLKSIZE_1024);
  if (n != BLKSIZE_1024) {
    printf("put_block [%d %d] error\n", me->fd, blk);
    return false;
  }
  return true;
}

// does what its name suggests
// returns integer of next available block no
// returns 0 if no more room
// modifies block_bitmap
int alloc_block(mount_entry *me) {
  char buf[BLKSIZE_1024];

  // read inode_bitmap block
  get_block(me, me->group_desc.bg_block_bitmap, buf);

  for (int i = 0; i < me->super_block.s_blocks_count; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      me->group_desc.bg_free_blocks_count--;
      put_block(me, me->group_desc.bg_inode_bitmap, buf);
      return i + 1;
    }
  }
  return 0;
}

// check if inode has the given file mode
bool check_mode(inode *file, int mode) {
  if (((int)(file->i_mode & 0xF000)) == mode)
    return true;
  else
    return false;
}

// tests the nth bit of a buffer
// return value of bit
// buf - buffer to test
// bit - which bit to test
bool tst_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  if (buf[i] & (1 << j))
    return true;
  return false;
}

// sets the nth bit of a buffer to 1
// return bool indicating success
// buf - buffer to test
// bit - which bit to test
bool set_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] |= (1 << j);
  return true;
}

// sets the nth bit of a buffer to 0
// return bool indicating success
// buf - buffer to test
// bit - which bit to test
bool clr_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] &= ~(1 << j);
  return true;
}

int ideal_len(dir_entry *dirp) { return 4 * ((8 + dirp->name_len + 3) / 4); }

// mip - minode * to have entry added to it
// dirp - dir_entry * to be added to *mip
// dirp must have name, name_len, and inode set
int add_dir_entry(minode *mip, dir_entry *new_dirp) {
  char buf[BLKSIZE_1024], *bufp = buf;
  dir_entry *cur_dirp;
  int free_space;

  // check if dir
  if (!check_mode(&mip->inode, DIR_FILE)) {
    printf("cannot mkdir in a file\n");
    return 0;
  }

  // set dirp rec_len to ideal
  new_dirp->rec_len = ideal_len(new_dirp);

  // iterate through blocks
  for (int i = 0; i < 12; i++) { // search direct blocks only
    if (mip->inode.i_block[i] == 0)
      return 0; // todo: alloc a new block if none available
    get_block(mip->mount_entry, mip->inode.i_block[i], buf);
    bufp = buf;
    // iterate through dir_entries to find space
    while (bufp < buf + BLKSIZE_1024) {
      cur_dirp = (dir_entry *)bufp;

      // check if space to insert then break
      free_space = cur_dirp->rec_len - ideal_len(cur_dirp);
      if (free_space > new_dirp->rec_len) {
        cur_dirp->rec_len = ideal_len(cur_dirp);
        new_dirp->rec_len = free_space;
        bufp += cur_dirp->rec_len;
        *(dir_entry *)bufp = *new_dirp;
        // write buffer back to block
        put_block(mip->mount_entry, mip->inode.i_block[i], buf);
        return new_dirp->rec_len;
      }

      bufp += cur_dirp->rec_len;
    }
  }
  return 0;
}
