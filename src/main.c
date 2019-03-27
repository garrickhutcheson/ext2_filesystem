#include "cmd/cmd.h"
#include "fs/fs.h"

int main(int argc, char const *argv[]) {
  char line[128], *root_dev;
  cmd c, *user_cmd = &c;
  if (argc > 1)
    root_dev = (char *)argv[1];
  fs_init();
  mount_root(root_dev);
  for (;;) {
    DEBUG_PRINT("running->pid == %d\n", running->pid);
    printf("input command: ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;
    if (!line[0])
      continue;
    parse_cmd(line, user_cmd);
    for (int i = 0; i < user_cmd->argc; i++)
      DEBUG_PRINT("user_cmd->argv[%d] == %s\n", i, user_cmd->argv[i]);
    do_cmd(user_cmd);
  }
}
