
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
// start from nth = -1
int get_lbk(blk_iter *it, int lbk) {
  // calculations for convience, could be macros
  int blks_per = BLKSIZE_1024 / sizeof(int), bno;
  int direct_start = 0, direct_end = 12, indirect_start = direct_end,
      indirect_end = direct_end + blks_per, double_start = indirect_end,
      double_end = indirect_end + blks_per * blks_per,
      triple_start = double_end,
      triple_end = double_end + blks_per * blks_per * blks_per;
  // pointers for shorter names
  unsigned int *i_block = it->mip->inode.i_block;
  mount_entry *me = it->mip->mount_entry;
  // null check
  if (!it || !it->mip)
    return 0;
  // get blocks based on lbk
  if (lbk < direct_end) {
    // get direct block
    bno = i_block[lbk];
  } else if (lbk < indirect_end) {
    // get indirect block
    if (!(it->nth >= indirect_start && it->nth < indirect_end))
      // check if map1 cached
      get_block(me, i_block[12], it->map1);
    bno = it->map1[lbk - indirect_start];
  } else if (lbk < double_end) {
    // get double indirect block
    if (!(it->nth >= double_start && it->nth < double_end))
      // check if map2 cached
      get_block(me, i_block[13], it->map2);
    if (!((lbk - double_start) / blks_per ==
          (it->nth - double_start) / blks_per))
      // check if map1 cached
      get_block(me, it->map2[(lbk - double_start) / blks_per], it->map1);
    bno = it->map1[(lbk - double_start) % blks_per];
  } else if (lbk < triple_end) {
    // triple  indirect blocks
    if (!(it->nth >= triple_start && it->nth < triple_end))
      // check if map3 cached
      get_block(me, i_block[12], it->map3);
    if (!((lbk - triple_start) / (blks_per * blks_per) ==
          (it->nth - triple_start) / (blks_per * blks_per)))
      // check if map2 cached
      get_block(me, it->map3[(lbk - triple_start) / (blks_per * blks_per)],
                it->map2);
    if (!((lbk - triple_start) / blks_per ==
          (it->nth - triple_start) / blks_per))
      // check if map1 cached
      get_block(me, it->map2[(lbk - triple_start) / blks_per], it->map1);
    bno = it->map1[(lbk - triple_start) % blks_per];
  }
  it->nth = lbk;
  return bno;
}

// returns fd closed or -1 for fail
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
  // todo: we aren't actually checking file permissions?
  if (mode == 0)
    // todo: update atime
    oftp->mode = 0;
  else if (mode == 1) {
    // todo: update atime and mtime
    oftp->mode = 1;
    free_i_block(mip);
  } else if (mode == 2) {
    // todo: update atime and mtime
    oftp->mode = 2;
  } else if (mode == 3) {
    // todo: update atime and mtime
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
  void *bufp = buf;
  char bbuf[BLKSIZE_1024] = {0};
  oft *oftp = running->oft_arr[fd];
  if (!oftp || !(oftp->mode == 0 || oftp->mode == 2))
    return 0;
  blk_iter it = {.mip = oftp->minode, .nth = -1};
  int lbk, avil, start, bno, to_copy, blk_off = 0, mid;
  avil = oftp->minode->inode.i_size - oftp->offset;
  while (count && avil) {
    // find logical block
    lbk = oftp->offset / BLKSIZE_1024;
    // find offset from start of block
    start = oftp->offset % BLKSIZE_1024;
    // find offset from end of block
    blk_off = BLKSIZE_1024 - start;
    // get bno
    if (!(bno = get_lbk(&it, lbk)))
      return 0;
    // get full ass block
    get_block(oftp->minode->mount_entry, bno, bbuf);

    // figure out how much of block to copy
    to_copy = ((mid = (count > avil ? avil : count)) > blk_off) ? blk_off : mid;

    // copy that much of block
    memcpy(bufp, bbuf + start, to_copy);
    // increment buf pointer  by amount copied
    bufp += to_copy;
    // increment offset by amount copied
    oftp->offset += to_copy;
    // decrement count by amount copied
    count -= to_copy;
    // decrement avil by amount copied
    avil -= to_copy;
  }
  return bufp - buf;
}