#pragma once

#include "../fs/fs.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct cmd {
  int argc;       // count of strings
  char *argv[64]; // array of strings
} cmd;

// specific commands
bool do_blocks(cmd *);
bool do_cat(cmd *);
bool do_cd(cmd *);
bool do_chmod(cmd *);
bool do_close(cmd *);
bool do_cp(cmd *);
bool do_creat(cmd *);
bool do_link(cmd *);
bool do_ls(cmd *);
bool do_lseek(cmd *);
bool do_mkdir(cmd *);
bool do_mv(cmd *);
bool do_open(cmd *);
bool do_pwd(cmd *);
bool do_read(cmd *);
bool do_rmdir(cmd *);
bool do_stat(cmd *);
bool do_symlink(cmd *);
bool do_touch(cmd *);
bool do_unlink(cmd *);
bool do_write(cmd *);

// utility
bool do_cmd(cmd *c);
int parse_cmd(char *, cmd *);
void print_path(minode *mip);