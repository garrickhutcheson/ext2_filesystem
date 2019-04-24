#include "cmd.h"

bool do_mv(cmd *c) {
  if (c->argc != 3) {
    printf("Usage: mv <src> <dest>\n");
    return false;
  }
  path src_path, dest_path;
  parse_path(c->argv[1], &src_path);
  parse_path(c->argv[2], &dest_path);

  char *bname = dest_path.argv[dest_path.argc - 1];

  dest_path.argc--;
  minode *dest_parent = search_path(dest_path);
  minode *mip = search_path(src_path);
  if (!mip || !dest_parent) {
    printf("bad path\n");
    return 0;
  }
  if (dest_parent->dev != mip->dev)
    return _cp(c->argv[1], c->argv[2]);
  else {
    return (_link(c->argv[1], c->argv[2]) && _unlink(c->argv[1]));
  }
}
