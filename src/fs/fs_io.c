
#include "fs.h"

// allocate a free oft for use
oft *alloc_oft() {
  for (int i = 0; i < NUM_OFT; i++) {
    oft *op = &oft_arr[i];
    if (op->ref_count == 0) {
      op->ref_count = 1;
      return op;
    }
  }
  printf("panic:out of fd's\n");
  return NULL;
}

// release a used oft.
bool free_oft(oft *op) {
  op->ref_count = 0;
  return true;
}

// returns block number of logical block
// 0 on failure (nothing more to read)
// start from lbkno = -1
int *get_lbk(blk_iter *it, int target) {
  // calculations for convience, could be macros
  int blks_per = BLKSIZE_1024 / sizeof(int), *bno;
  int direct_start = 0, direct_end = 12, indirect_start = direct_end,
      indirect_end = direct_end + blks_per, double_start = indirect_end,
      double_end = indirect_end + blks_per * blks_per,
      triple_start = double_end,
      triple_end = double_end + blks_per * blks_per * blks_per;
  // pointers for shorter names
  unsigned int *i_block = it->mip->inode.i_block;
  mount_entry *dev = it->mip->dev;
  // null check
  if (!it || !it->mip)
    return 0;
  // get blocks based on target
  if (target < direct_end) {
    it->map1_bno = it->map2_bno = it->map3_bno = 0;
    // get direct block
    bno = &i_block[target];
  } else if (target < indirect_end) {
    it->map2_bno = it->map3_bno = 0;
    // get indirect block
    if (!(it->lbkno >= indirect_start && it->lbkno < indirect_end))
      // check if map1 cached
      get_block(dev, it->map1_bno = i_block[12], (char *)it->map1);
    bno = &it->map1[target - indirect_start];
  } else if (target < double_end) {
    it->map3_bno = 0;
    // get double indirect block
    if (!(it->lbkno >= double_start && it->lbkno < double_end))
      // check if map2 cached
      get_block(dev, it->map2_bno = i_block[13], (char *)it->map2);
    if (!((target - double_start) / blks_per ==
          (it->lbkno - double_start) / blks_per))
      // check if map1 cached
      get_block(dev,
                it->map1_bno = it->map2[(target - double_start) / blks_per],
                (char *)it->map1);
    bno = &it->map1[(target - double_start) % blks_per];
  } else if (target < triple_end) {
    // triple  indirect blocks
    if (!(it->lbkno >= triple_start && it->lbkno < triple_end))
      // check if map3 cached
      get_block(dev, it->map3_bno = i_block[14], (char *)it->map3);
    if (!((target - triple_start) / (blks_per * blks_per) ==
          (it->lbkno - triple_start) / (blks_per * blks_per)))
      // check if map2 cached
      get_block(dev,
                it->map2_bno =
                    it->map3[(target - triple_start) / (blks_per * blks_per)],
                (char *)it->map2);
    if (!((target - triple_start) / blks_per ==
          (it->lbkno - triple_start) / blks_per))
      // check if map1 cached
      get_block(dev,
                it->map1_bno = it->map2[(target - triple_start) / blks_per],
                (char *)it->map1);
    bno = &it->map1[(target - triple_start) % blks_per];
  }
  it->lbkno = target;
  return bno;
}

// returns fd or -1 for fail
int open_file(char *path, int mode) {
  int fd;
  struct path p;
  parse_path(path, &p);
  minode *mip = NULL;
  if (!(mip = search_path(p)) || !S_ISREG(mip->inode.i_mode)) {
    put_minode(mip);
    return 0;
  }

  oft *oftp = alloc_oft();
  for (fd = 0; fd < NUM_OFT_PER; fd++) {
    if (running->oft_arr[fd] == NULL) {
      running->oft_arr[fd] = oftp;
      break;
    }
  }

  oftp->minode = mip;
  oftp->offset = 0;
  // mode = 0|1|2|3 for R|W|RW|APPEND
  // TODO: we aren't actually checking file permissions?
  if (mode == 0)
    // TODO: update atime
    oftp->mode = 0;
  else if (mode == 1) {
    // TODO: update atime and mtime
    oftp->mode = 1;
    free_i_block(mip);
  } else if (mode == 2) {
    // TODO: update atime and mtime
    oftp->mode = 2;
  } else if (mode == 3) {
    // TODO: update atime and mtime
    oftp->mode = 3;
    oftp->offset = mip->inode.i_size;
  } else {
    put_minode(mip);
    free_oft(oftp);
    printf("Invalid file mode given\n");
    return -1;
  }

  for (int i = 0; i < NUM_OFT; i++) {
    if (oft_arr[i].ref_count && oft_arr[i].minode->ino == mip->ino &&
        &oft_arr[i] != oftp && (oft_arr[i].mode || oftp->mode)) {
      printf("Only allowed to write to unopened file\n");
      put_minode(mip);
      free_oft(oftp);
      return -1;
    }
  }
  mip->dirty = true;
  return fd;
}

// return final offset or -1 for failure
int lseek_file(int fd, int offset, int whence) {
  oft *oftp;
  int og_off;
  if (fd < 0 || fd > NUM_OFT_PER)
    return -1;
  if ((oftp = running->oft_arr[fd])) {
    og_off = oftp->offset;
    // 0 	SEEK_SET
    if (whence == 0)
      oftp->offset = offset;
    // 1 	SEEK_CUR
    else if (whence == 1)
      oftp->offset += offset;
    // 2 	SEEK_END
    else if (whence == 2)
      oftp->offset = oftp->minode->inode.i_size + offset;
    else
      return -1;
  }
  if (oftp->offset > oftp->minode->inode.i_size || oftp->offset < 0) {
    oftp->offset = og_off;
    return -1;
  }
  return oftp->offset;
}

// returns fd close or -1 for fail
int close_file(int fd) {
  if (fd > NUM_OFT_PER)
    return -1;
  if (running->oft_arr[fd]) {
    put_minode(running->oft_arr[fd]->minode);
    free_oft(running->oft_arr[fd]);
    running->oft_arr[fd] = NULL;
    return fd;
  }
  printf("fd not open\n");
  return -1;
}

int read_file(int fd, void *buf, unsigned int count) {
  char *dest = (char *)buf;
  char blk_buf[BLKSIZE_1024] = {0};
  oft *oftp = running->oft_arr[fd];
  if (!oftp || !(oftp->mode == 0 || oftp->mode == 2))
    return 0;
  blk_iter it = {.mip = oftp->minode, .lbkno = -1};
  int tar_lbk, avil, tar_byte, bno, to_copy, remain = 0, mid;
  avil = oftp->minode->inode.i_size - oftp->offset;
  while (count && avil) {
    // find logical block
    tar_lbk = oftp->offset / BLKSIZE_1024;
    // find offset from start of block
    tar_byte = oftp->offset % BLKSIZE_1024;
    // find offset from end of block
    remain = BLKSIZE_1024 - tar_byte;
    // get bno
    if (!(bno = *get_lbk(&it, tar_lbk)))
      return 0;
    // get full ass block
    get_block(oftp->minode->dev, bno, blk_buf);

    // figure out how much of block to copy
    to_copy = ((mid = (count > avil ? avil : count)) > remain) ? remain : mid;

    // copy that much of block
    memcpy(dest, blk_buf + tar_byte, to_copy);
    // increment buf pointer  by amount copied
    dest += to_copy;
    // increment offset by amount copied
    oftp->offset += to_copy;
    // decrement count by amount copied
    count -= to_copy;
    // decrement avil by amount copied
    avil -= to_copy;
  }
  return dest - (char *)buf;
}

int write_file(int fd, void *buf, unsigned int count) {
  char *src = (char *)buf;
  char blk_buf[BLKSIZE_1024] = {0};
  oft *oftp = running->oft_arr[fd];
  if (!oftp || !(oftp->mode == 1 || oftp->mode == 2))
    return 0;
  blk_iter it = {.mip = oftp->minode, .lbkno = -1};
  int tar_lbk, tar_byte, *bnop, to_copy, remain = 0, mid;
  while (count) {
    // find logical block
    tar_lbk = oftp->offset / BLKSIZE_1024;
    // find offset from start of block
    tar_byte = oftp->offset % BLKSIZE_1024;
    // find offset from end of block
    remain = BLKSIZE_1024 - tar_byte;
    // get bno
    bnop = get_lbk(&it, tar_lbk);
    *bnop;
    if (!(*bnop)) {
      int new_bno = alloc_block(oftp->minode->dev);
      *bnop = new_bno;
      if (it.map1_bno)
        put_block(oftp->minode->dev, it.map1_bno, (char *)it.map1);
    }

    // figure out how much of block to copy
    to_copy = (count > remain) ? remain : count;

    // read full ass block if needed
    if (to_copy < BLKSIZE_1024)
      get_block(oftp->minode->dev, *bnop, blk_buf);

    // copy that much to block
    memcpy(blk_buf + tar_byte, src, to_copy);
    put_block(oftp->minode->dev, *bnop, blk_buf);
    // increment buf pointer  by amount copied
    src += to_copy;
    // increment offset by amount copied
    oftp->offset += to_copy;
    // decrement count by amount copied
    count -= to_copy;
    // increase file size
    oftp->minode->inode.i_size = (oftp->offset > oftp->minode->inode.i_size)
                                     ? oftp->offset
                                     : oftp->minode->inode.i_size;
  }
  return src - (char *)buf;
}