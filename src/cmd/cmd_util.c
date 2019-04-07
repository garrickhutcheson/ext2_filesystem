#include "../debug/debug.h"
#include "cmd.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

bool do_cmd(cmd *c) {
  if (!c)
    DEBUG_PRINT("cmd was null\n");
  if (!strcmp(c->argv[0], "blocks")) {
    do_blocks(c);
  } else if (!strcmp(c->argv[0], "cd")) {
    do_cd(c);
  } else if (!strcmp(c->argv[0], "chmod")) {
    do_chmod(c);
  } else if (!strcmp(c->argv[0], "creat")) {
    do_creat(c);
  } else if (!strcmp(c->argv[0], "link")) {
    do_link(c);
  } else if (!strcmp(c->argv[0], "ls")) {
    do_ls(c);
  } else if (!strcmp(c->argv[0], "mkdir")) {
    do_mkdir(c);
  } else if (!strcmp(c->argv[0], "pwd")) {
    do_pwd(c);
  } else if (!strcmp(c->argv[0], "rmdir")) {
    do_rmdir(c);
  } else if (!strcmp(c->argv[0], "stat")) {
    do_stat(c);
  } else if (!strcmp(c->argv[0], "symlink")) {
    do_symlink(c);
  } else if (!strcmp(c->argv[0], "touch")) {
    do_touch(c);
  } else if (!strcmp(c->argv[0], "unlink")) {
    do_unlink(c);
  } else if (!strcmp(c->argv[0], "entries")) {
    do_entries(c);
  } else if (!strcmp(c->argv[0], "quit")) {
    exit(0);
  } else {
    printf("command not recognized: %s\n", c->argv[0]);
  }
  return 0;
}

int parse_cmd(char *line, cmd *c) {
  // split by whitespace into cmd struct
  int i = 0;
  char *s = strtok(line, " ");
  for (; s; i++) {
    c->argv[i] = s;
    s = strtok(NULL, " ");
  }
  c->argc = i;
  // NULL terminate argv
  c->argv[i] = NULL;
  return i;
}

void print_path(minode *mip) {
  if (mip == global_root_inode)
    ;
  else {
    char buf1[BLKSIZE_1024], *buf1p;
    char buf2[BLKSIZE_1024], *buf2p;
    char name[256] = {0};
    dir_entry *this_dir, *parent_dir, *dirp;

    get_block(mip->mount_entry, mip->inode.i_block[0], buf1);
    this_dir = (dir_entry *)buf1;
    parent_dir = (dir_entry *)(buf1 + this_dir->rec_len);
    mip = get_minode(mip->mount_entry, parent_dir->inode);

    for (int i = 0; i < 12 && !(*name); i++) { // search direct blocks only
      if (mip->inode.i_block[i] == 0)
        break;
      get_block(mip->mount_entry, mip->inode.i_block[i], buf2);
      dirp = (dir_entry *)buf2;
      buf2p = buf2;
      // todo: double check this condition
      while (buf2p < buf2 + BLKSIZE_1024) {
        dirp = (dir_entry *)buf2p;
        buf2p += dirp->rec_len;
        if (dirp->inode == this_dir->inode) {
          strncpy(name, dirp->name, dirp->name_len);
          name[dirp->name_len] = '\0';
          break;
        }
      }
    }
    put_minode(mip);
    print_path(mip); // recursive call
    printf("/%s", name);
  }
}