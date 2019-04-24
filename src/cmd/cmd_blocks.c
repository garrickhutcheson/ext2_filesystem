#include "cmd.h"
bool do_blocks(cmd *c) {

  if (c->argc != 2) {
    printf("Usage: blocks <filename>\n");
    return false;
  }

  char buf1[BLKSIZE_1024], buf2[BLKSIZE_1024], buf3[BLKSIZE_1024];
  int *fs_p1, *fs_p2, *fs_p3;
  path in_path;
  minode *mip;

  parse_path(c->argv[1], &in_path);
  if (!(mip = search_path(in_path)))
    return false;

  printf("\ndirect blocks:\n");
  for (int i = 0; i < 12 && mip->inode.i_block[i]; i++)
    printf(" %-4u ", mip->inode.i_block[i]);

  get_block(mip->dev, mip->inode.i_block[12], buf1);
  if (mip->inode.i_block[12])
    printf("\nindirect blocks:\n[%u] :\n", mip->inode.i_block[12]);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024))
    printf(" %-4u ", *fs_p1++);

  get_block(mip->dev, mip->inode.i_block[13], buf1);
  if (mip->inode.i_block[13])
    printf("\ndouble indirect blocks:\n[%u] :\n", mip->inode.i_block[13]);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024)) {
    get_block(mip->dev, *fs_p1, buf2);
    printf("[[%u]] :\n", *fs_p1);
    fs_p2 = (int *)buf2;
    while (*fs_p2 && ((char *)fs_p2 < buf2 + BLKSIZE_1024))
      printf(" %-4u ", *fs_p2++);
    printf("\n");
    fs_p1++;
  }

  get_block(mip->dev, mip->inode.i_block[14], buf1);
  if (mip->inode.i_block[14])
    printf("triple indirect blocks:\n[%u] :\n", mip->inode.i_block[14]);
  fs_p1 = (int *)buf1;
  while (*fs_p1 && ((char *)fs_p1 < buf1 + BLKSIZE_1024)) {
    get_block(mip->dev, *fs_p1, buf2);
    printf("[[%u]] :\n", *fs_p1);
    fs_p2 = (int *)buf2;
    while (*fs_p2 && ((char *)fs_p2 < buf2 + BLKSIZE_1024)) {
      get_block(mip->dev, *fs_p2, buf3);
      printf("[[[%u]]] :\n", *fs_p2);
      fs_p3 = (int *)buf3;
      while (*fs_p3 && ((char *)fs_p3 < buf3 + BLKSIZE_1024))
        printf(" %-4u ", *fs_p3++);
      printf("\n");
      fs_p2++;
    }
    printf("\n");
    fs_p1++;
  }
  printf("\n");

  put_minode(mip);
  return true;
}