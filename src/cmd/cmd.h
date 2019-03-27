#pragma once

#include "../fs/fs.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct cmd {
  int argc;       // count of strings
  char *argv[64]; // array of strings
} cmd;

// specific commands
bool do_blocks(cmd *);
bool do_cd(cmd *);
bool do_chmod(cmd *);
bool do_creat(cmd *);
bool do_link(cmd *);
bool do_ls(cmd *);
bool do_mkdir(cmd *);
bool do_pwd(cmd *);
bool do_rmdir(cmd *);
bool do_stat(cmd *);
bool do_symlink(cmd *);
bool do_touch(cmd *);
bool do_unlink(cmd *);

// utility
bool do_cmd(cmd *c);
int parse_cmd(char *, cmd *);
int spawn_proc(int, int, cmd *);