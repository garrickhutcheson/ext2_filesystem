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

int spawn_proc(int in_fd, int out_fd, cmd *c) {
  pid_t pid = fork();
  if (pid == 0) { // child
    if (in_fd != 0) {
      dup2(in_fd, 0);
      close(in_fd);
    }
    if (out_fd != 1) {
      dup2(out_fd, 1);
      close(out_fd);
    }
    execvp(c->argv[0], (char *const *)c->argv);
  }
  return pid; // parent
}