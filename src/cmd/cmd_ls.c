#include "cmd.h"

int ls_file(minode *file, char *fname) {
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";
  char ftime[256], buf[256] = {0}, *p_buf = buf, bufbuf[256] = {0};
  int r, i;
  if (check_mode(&file->inode, REG_FILE)) // if (S_ISREG())
    p_buf = stpcpy(p_buf, "-");
  if (check_mode(&file->inode, DIR_FILE)) // if (S_ISDIR())
    p_buf = stpcpy(p_buf, "d");
  if (check_mode(&file->inode, LNK_FILE)) // if (S_ISLNK())
    p_buf = stpcpy(p_buf, "l");
  for (i = 8; i >= 0; i--) {
    if ((file->inode.i_mode & (1 << i))) { // print r|w|x
      sprintf(bufbuf, "%c", t1[i]);
      p_buf = stpcpy(p_buf, bufbuf);
    } else {
      sprintf(bufbuf, "%c", t2[i]);
      p_buf = stpcpy(p_buf, bufbuf);
    }
  }
  sprintf(bufbuf, "%4d %4d %4d %8d", (int)file->inode.i_links_count,
          file->inode.i_gid, file->inode.i_uid, (int)file->inode.i_size);
  p_buf = stpcpy(p_buf, bufbuf);
  const time_t *tp = (time_t *)file->inode.i_ctime;
  strcpy(ftime, ctime(tp));
  ftime[strlen(ftime) - 1] = 0;
  sprintf(bufbuf, " %s %s", ftime, basename(fname));
  p_buf = stpcpy(p_buf, bufbuf);
  if ((file->inode.i_mode & 0xF000) == 0xA000) {
    // use readlink() to read linkname
    char linkname[256] = {0};
    readlink(fname, linkname, 256);
    sprintf(bufbuf, " -> %s", linkname);
    p_buf = stpcpy(p_buf, bufbuf); // print linked name
  }
  p_buf = stpcpy(p_buf, "\n");
  printf("%s", buf);
  return 0;
}

bool do_ls(cmd *c) {
  dir_entry dep[4096];
  minode *cur_dir = global_root;
  int entryc;

  if (c->argc < 2)
    entryc = list_dir(cur_dir, dep);
  else {
    path in_path;
    parse_path(c->argv[1], &in_path);
    minode m;
    cur_dir = search_path(&in_path);
    if (!cur_dir)
      return 1;
    entryc = list_dir(cur_dir, dep);
    // put_inode(cur_dir);
  }
  for (int i = 0; i < entryc; i++) {
    minode *file = get_inode(cur_dir->dev, dep[i].inode);
    // printf("%s\n", dep[i].name);
    ls_file(file, dep[i].name);
    put_inode(file);
  }
  if (cur_dir)
    put_inode(cur_dir);
  return 0;
}
