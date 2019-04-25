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
int *add_lbk(blk_iter *it) {

  mount_entry *me = it->mip->dev;
  int last_bno = it->mip->inode.i_size / BLKSIZE_1024;
  get_lbk(it, last_bno);

  if (it->map1_bno) {
    for (int i = 0; i < BLKSIZE_1024 / sizeof(int); i++) {
      if (!it->map1[i]) {
        it->map1[i] = alloc_block(me);
        if (i + 1 < BLKSIZE_1024 / sizeof(int))
          it->map1[i + 1] = 0;
        put_block(me, it->map1_bno, (char *)it->map1);
        return &it->map1[i];
      }
    }
  }
  if (it->map2_bno) {
    for (int i = 0; i < BLKSIZE_1024 / sizeof(int); i++) {
      if (!it->map2[i]) {
        it->map2[i] = alloc_block(me);
        put_block(me, it->map2_bno, (char *)it->map2);
        it->map1_bno = it->map2[i];
        get_block(me, it->map1_bno, (char *)it->map1);
        it->map1[0] = alloc_block(me);
        it->map1[1] = 0;
        put_block(me, it->map1_bno, (char *)it->map1);
        return &it->map1[0];
      }
    }
  }
  if (it->map3_bno) {
    for (int i = 0; i < BLKSIZE_1024 / sizeof(int); i++) {
      if (!it->map3[i]) {
        it->map3[i] = alloc_block(me);
        put_block(me, it->map3_bno, (char *)it->map3);
        it->map2_bno = it->map3[i];
        get_block(me, it->map2_bno, (char *)it->map2);
        it->map2[0] = alloc_block(me);
        it->map2[1] = 0;
        put_block(me, it->map2_bno, (char *)it->map2);
        it->map1_bno = it->map2[0];
        get_block(me, it->map1_bno, (char *)it->map1);
        it->map1[0] = alloc_block(me);
        it->map1[1] = 0;
        put_block(me, it->map1_bno, (char *)it->map1);
        return &it->map1[0];
      }
    }
  }

  inode *ip = &it->mip->inode;
  for (int i = 0; i < 15; i++) {
    if (!ip->i_block[i]) {
      ip->i_block[i] = alloc_block(me); // always need one
      if (i == 12) {                    // one more blocks
        get_block(me, ip->i_block[12], (char *)it->map1);
        it->map1_bno = ip->i_block[12];
        it->map1[0] = alloc_block(me);
        it->map1[1] = 0;
        put_block(me, ip->i_block[12], (char *)it->map1);
        return &it->map1[0];
      } else if (i == 13) { // two more blocks
        get_block(me, ip->i_block[13], (char *)it->map2);
        it->map2_bno = ip->i_block[13];
        it->map2[0] = alloc_block(me);
        it->map2[1] = 0;
        put_block(me, ip->i_block[13], (char *)it->map2);
        get_block(me, it->map2[0], (char *)it->map1);
        it->map1_bno = it->map2[0];
        it->map1[0] = alloc_block(me);
        it->map1[1] = 0;
        put_block(me, it->map2[0], (char *)it->map1);
        return &it->map1[0];
      } else if (i == 14) { // three more blocks
        get_block(me, ip->i_block[14], (char *)it->map3);
        it->map3_bno = ip->i_block[14];
        it->map3[0] = alloc_block(me);
        it->map3[1] = 0;
        put_block(me, ip->i_block[14], (char *)it->map3);
        get_block(me, it->map3[0], (char *)it->map2);
        it->map2_bno = it->map3[0];
        it->map2[0] = alloc_block(me);
        it->map2[1] = 0;
        put_block(me, it->map3[0], (char *)it->map2);
        get_block(me, it->map2[0], (char *)it->map1);
        it->map1_bno = it->map2[0];
        it->map1[0] = alloc_block(me);
        it->map1[1] = 0;
        put_block(me, it->map2[0], (char *)it->map1);
        return &it->map1[0];
      }
      return &ip->i_block[i];
    }
  }
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
  mount_entry *me = it->mip->dev;
  // null check
  if (!it || !it->mip)
    return 0;

  // get blocks based on target

  // get direct block
  if (target < direct_end) {
    it->map1_bno = it->map2_bno = it->map3_bno = 0;
    bno = &i_block[target];
  }
  // get indirect block
  else if (target < indirect_end) {
    it->map2_bno = it->map3_bno = 0;
    if (!(it->lbkno >= indirect_start && it->lbkno < indirect_end))
      // check if map1 cached
      get_block(me, it->map1_bno = i_block[12], (char *)it->map1);
    bno = &it->map1[target - indirect_start];
  }
  // get double indirect block
  else if (target < double_end) {
    it->map3_bno = 0;
    if (!(it->lbkno >= double_start && it->lbkno < double_end))
      // check if map2 cached
      get_block(me, it->map2_bno = i_block[13], (char *)it->map2);
    if (!((target - double_start) / blks_per ==
          (it->lbkno - double_start) / blks_per))
      // check if map1 cached
      get_block(me, it->map1_bno = it->map2[(target - double_start) / blks_per],
                (char *)it->map1);
    bno = &it->map1[(target - double_start) % blks_per];
  }
  // triple  indirect blocks
  else if (target < triple_end) {
    if (!(it->lbkno >= triple_start && it->lbkno < triple_end))
      // check if map3 cached
      get_block(me, it->map3_bno = i_block[14], (char *)it->map3);
    if (!((target - triple_start) / (blks_per * blks_per) ==
          (it->lbkno - triple_start) / (blks_per * blks_per)))
      // check if map2 cached
      get_block(me,
                it->map2_bno =
                    it->map3[(target - triple_start) / (blks_per * blks_per)],
                (char *)it->map2);
    if (!((target - triple_start) / blks_per ==
          (it->lbkno - triple_start) / blks_per))
      // check if map1 cached
      get_block(me, it->map1_bno = it->map2[(target - triple_start) / blks_per],
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

  oftp->it.lbkno = -1;
  oftp->it.map1_bno = -1;
  oftp->it.map2_bno = -1;
  oftp->it.map3_bno = -1;
  oftp->it.mip = mip;

  // mode = 0|1|2|3 for R|W|RW|APPEND
  if (mode == 0) {
    oftp->minode->inode.i_atime = time(0L);
    oftp->mode = 0;
  } else if (mode == 1) {
    oftp->minode->inode.i_atime = oftp->minode->inode.i_mtime = time(0L);
    oftp->mode = 1;
    free_i_block(mip);
  } else if (mode == 2) {
    oftp->minode->inode.i_atime = oftp->minode->inode.i_mtime = time(0L);
    oftp->mode = 2;
  } else if (mode == 3) {
    oftp->minode->inode.i_atime = oftp->minode->inode.i_mtime = time(0L);
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
    if (!(bno = *get_lbk(&oftp->it, tar_lbk)))
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
  int tar_lbk, tar_byte, *bnop, to_copy, remain = 0, mid;
  while (count) {
    // find logical block
    tar_lbk = oftp->offset / BLKSIZE_1024;
    // find offset from start of block
    tar_byte = oftp->offset % BLKSIZE_1024;
    // find offset from end of block
    remain = BLKSIZE_1024 - tar_byte;
    // get bno and alloc
    bnop = get_lbk(&oftp->it, tar_lbk);
    if (!*bnop) {
      bnop = add_lbk(&oftp->it);
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
    oftp->minode->inode.i_size = ((oftp->offset > oftp->minode->inode.i_size)
                                      ? oftp->offset
                                      : oftp->minode->inode.i_size);
  }
  return src - (char *)buf;
}