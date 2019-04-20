#include "fs.h"

int fs_init() {
  int i, j;
  // initialize all minodes
  for (i = 0; i < NUM_MINODES; i++)
    minode_arr[i].ref_count = 0;
  // initialize mount entries
  for (i = 0; i < NUM_MOUNT_ENTRIES; i++)
    mount_entry_arr[i].mounted = false;
  // initialize PROCs
  for (i = 0; i < NUM_PROCS; i++) {
    proc_arr[i].status = PROC_FREE;
    proc_arr[i].pid = i;
    // P0 is a superuser process
    proc_arr[i].uid = i;
    // initialize PROC file descriptors to NULL
    for (j = 0; j < NUM_OFT_PER; j++)
      proc_arr[i].oft_arr[j] = 0;
    proc_arr[i].next = &proc_arr[i + 1];
  }
  // circular list
  proc_arr[NUM_PROCS - 1].next = &proc_arr[0];
  // P0 runs first
  running = &proc_arr[0];
  return 0;
}

mount_entry *make_me(char *dev_path, char *mnt_path) {
  mount_entry *me;
  int meno;
  char buf[BLKSIZE_1024];
  // open 'device'
  int dev = open(dev_path, O_RDWR);
  if (dev < 0) {
    printf("panic : can’t open device\n");
    exit(1);
  }
  // alloc mount entry
  for (meno = 0; meno < NUM_MOUNT_ENTRIES + 1; meno++) {
    if (meno == NUM_MOUNT_ENTRIES) {
      printf("panic: cannot mount");
      return NULL;
    }
    me = &mount_entry_arr[meno];
    if (!me->mounted)
      break;
  }

  // set fd and names
  me->mounted = true;
  me->fd = dev;
  strcpy(me->dev_path, dev_path);
  strcpy(me->mnt_path, mnt_path);

  // get super block to me
  get_block(me, 1, buf);
  me->super_block = *(super_block *)buf;

  // check magic number
  if (me->super_block.s_magic != EXT2_SUPER_MAGIC) {
    printf("not an EXT2 filesystem please umount\n");
    exit(0);
  }

  // get group descriptor to me
  get_block(me, 2, buf);
  me->group_desc = *(group_desc *)buf;

  // call get_minode(), which inc minode’s ref_count
  me->root = get_minode(me, 2);

  DEBUG_PRINT("mounted %s to with fd %d\n", me->mnt_path, me->fd);
  return me;
}
