#include "cmd.h"

int do_entries(cmd *c) { return count_dir(running->cwd); }