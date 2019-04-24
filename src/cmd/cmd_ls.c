#include "cmd.h"

bool do_ls(cmd *c) {
  dir_entry dep[4096];
  minode *cur_dir = NULL;
  int entryc;

  if (c->argc < 2) {
    cur_dir = running->cwd;
    cur_dir->ref_count++;
  } else {
    path in_path;
    parse_path(c->argv[1], &in_path);
    cur_dir = search_path(in_path);
    if (!cur_dir) {
      printf("invalid path\n");
      return false;
    }
  }
  entryc = list_dir(cur_dir, dep);
  for (int i = 0; i < entryc; i++) {
    minode *file = get_minode(cur_dir->dev, dep[i].inode);
    // printf("%s\n", dep[i].name);
    char filename[256] = {0};
    strncpy(filename, dep[i].name, dep[i].name_len);
    _ls_file(file, filename);

    put_minode(file);
  }
  put_minode(cur_dir);
  return true;
}

int _ls_file(minode *file, char *fname) {
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";
  char ftime[256], buf[256] = {0};
  int r, i;
  if (S_ISREG(file->inode.i_mode))
    printf("-");
  else if (S_ISDIR(file->inode.i_mode))
    printf("d");
  else if (S_ISLNK(file->inode.i_mode))
    printf("l");
  for (i = 8; i >= 0; i--) {
    if ((file->inode.i_mode & (1 << i))) // print r|w|x
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }
  printf("%4d %4d %4d %8d ", (int)file->inode.i_links_count, file->inode.i_gid,
         file->inode.i_uid, (int)file->inode.i_size);
  printf("%s %s", strtok(ctime((long *)&file->inode.i_ctime), "\n"), fname);
  if ((file->inode.i_mode & 0xF000) == 0xA000) {
    // use readlink() to read linkname
    char linkname[256] = {0};
    readlink(fname, linkname, 256);
    printf(" -> %s", linkname);
  }
  printf("\n");
  return 0;
}
