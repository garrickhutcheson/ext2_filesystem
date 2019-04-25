#pragma once

#include "../fs/fs.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct cmd {
  int argc;       // count of strings
  char *argv[64]; // array of strings
} cmd;

// command handlers
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
bool do_mount(cmd *);
bool do_mv(cmd *);
bool do_open(cmd *);
bool do_pwd(cmd *);
bool do_read(cmd *);
bool do_rmdir(cmd *);
bool do_stat(cmd *);
bool do_su(cmd *c);
bool do_symlink(cmd *);
bool do_touch(cmd *);
bool do_umount(cmd *);
bool do_unlink(cmd *);
bool do_write(cmd *);

// command implementations
int _cd(char *);
int _cp(char *, char *);
int _creat(char *);
int _link(char *, char *);
int _ls_file(minode *, char *);
int _mkdir(char *);
int _mount(char *, char *);
int _pwd(minode *);
int _rmdir(char *);
int _symlink(char *, char *);
int _umount(char *);
int _unlink(char *);

// utility
bool do_cmd(cmd *c);
int parse_cmd(char *, cmd *);