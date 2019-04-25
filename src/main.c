#include "cmd/cmd.h"
#include "fs/fs.h"

int main(int argc, char const *argv[]) {
  char line[128], *root_dev;
  cmd c, *user_cmd = &c;
  if (argc > 1)
    root_dev = (char *)argv[1];

  // init globals
  fs_init();
  // read device meta data
  mount_entry *me = make_me(root_dev, "/");
  // make root mnt pnt be root
  me->mnt_pnt = get_minode(me, 2);
  global_root_inode = me->mnt_pnt;

  for (int i = 0; i < NUM_PROCS; i++)
    proc_arr[i].cwd = global_root_inode;

  // progam loop
  for (;;) {
    DEBUG_PRINT("running->pid == %d\n", running->pid);

    // prompt and read
    printf("User:%d @ ", running->uid);
    _pwd(running->cwd);
    printf(": ");

    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;
    if (!line[0])
      continue;

    // split into argv argc
    parse_cmd(line, user_cmd);

    for (int i = 0; i < user_cmd->argc; i++)
      DEBUG_PRINT("user_cmd->argv[%d] == %s\n", i, user_cmd->argv[i]);

    // execute use command
    do_cmd(user_cmd);
  }
}
