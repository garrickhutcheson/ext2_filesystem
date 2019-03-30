#include "fs.h"
#include "string.h"
#include <unistd.h>

// check if inode has the given file mode
bool check_mode(inode *file, int mode) {
  if (((int)(file->i_mode & 0xF000)) == mode)
    return true;
  else
    return false;
}

// read block to buffer
int get_block(int dev, int blk, char *buf) {
  lseek(dev, blk * BLKSIZE_1024, SEEK_SET);
  int n = read(dev, buf, BLKSIZE_1024);
  if (n < 0)
    printf("get_block[% d % d] error \n", dev, blk);
  return 0;
}

// write block from buffer
int put_block(int dev, int blk, char *buf) {
  lseek(dev, blk * BLKSIZE_1024, SEEK_SET);
  int n = write(dev, buf, BLKSIZE_1024);
  if (n != BLKSIZE_1024)
    printf("put_block [%d %d] error\n", dev, blk);
  return 0;
}

int tst_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  if (buf[i] & (1 << j))
    return 1;
  return 0;
}

int set_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit) {
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] &= ~(1 << j);
}

/*
int alloc_inode(int dev) {
  int i;
  char buf[BLKSIZE_1024];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i = 0; i < NUM_MINODES; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, imap, buf);
      return i + 1;
    }
  }
  return 0;
}
*/