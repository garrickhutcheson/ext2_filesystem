#ifndef _CPTS360_FS_H
#define _CPTS360_FS_H

#include "../debug/debug.h"
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

//// TYPEDEF ////

// define shorter TYPES for convenience
typedef struct ext2_group_desc group_desc;
typedef struct ext2_super_block super_block;
typedef struct ext2_inode inode;
typedef struct ext2_dir_entry_2 dir_entry;

//// CONST ////

// Block size
#define BLKSIZE_1024 1024

// Inode numbers of EXT2 as defined in ext2fs.h
// EXT2_BAD_INO 1         Bad blocks inode
// EXT2_ROOT_INO 2        Root inode
// EXT4_USR_QUOTA_INO 3   User quota inode
// EXT4_GRP_QUOTA_INO 4   Group quota inode
// EXT2_BOOT_LOADER_INO 5 Boot loader inode
// EXT2_UNDEL_DIR_INO 6   Undelete directory inode
// EXT2_RESIZE_INO 7      Reserved group descriptors inode
// EXT2_JOURNAL_INO 8     Journal inode
// EXT2_EXCLUDE_INO 9     The "exclude" inode, for snapshots
// EXT4_REPLICA_INO 10    Used by non-upstream feature

// File types
// #define __S_IFDIR 0040000  /* Directory.  */
// #define __S_IFCHR 0020000  /* Character device.  */
// #define __S_IFBLK 0060000  /* Block device.  */
// #define __S_IFREG 0100000  /* Regular file.  */
// #define __S_IFIFO 0010000  /* FIFO.  */
// #define __S_IFLNK 0120000  /* Symbolic link.  */
// #define __S_IFSOCK 0140000 /* Socket.  */

// Proc status
#define PROC_FREE 0
#define PROC_BUSY 1

// file system table sizes
#define NUM_MINODES 100
#define NUM_MOUNT_ENTRIES 10
#define NUM_PROCS 2
#define NUM_OFT_PER 10
#define NUM_OFT 40

//// STRUCTS ////

// used to iterate over memory blocks of an inode
typedef struct blk_iter {
  struct minode *mip;
  // buf contains the nth block
  unsigned int lbkno;
  // direct block (buf), indirection block(map1),
  // double indirection(map2), triple indirection(map3);
  int map1[BLKSIZE_1024 / sizeof(int)], map2[BLKSIZE_1024 / sizeof(int)],
      map3[BLKSIZE_1024 / sizeof(int)];
  // block numbers of maps for writing
  int map1_bno, map2_bno, map3_bno;
} blk_iter;

// for parsing paths into
typedef struct path {
  char argv[256][64]; // count of strings
  int argc;           // array of strings
  bool is_absolute;
} path;

// In-memory inodes structure
typedef struct minode {
  // disk inode
  inode inode;
  // inode number
  int ino;
  // use count
  int ref_count;
  // modified flag
  bool dirty;
  // mount point transition
  struct mount_entry *mnt;
  // device containing inode
  struct mount_entry *dev;
  // ignored for simple FS
  // int lock;
} minode;

// Open file Table AKA opened file instance
typedef struct oft {
  // file mode
  int mode;
  // number of PROCs sharing this instance
  int ref_count;
  // pointer to minode of file
  minode *minode;
  // byte offset for R|W
  int offset;
  // for caching
  blk_iter it;
} oft;

// PROC structure
typedef struct proc {
  struct proc *next;
  int pid;
  int uid;
  int gid;
  int ppid;
  int status;
  minode *cwd;
  oft *oft_arr[NUM_OFT_PER];
} proc;

// Mount Entry structure
typedef struct mount_entry {
  // device file descriptor
  int fd;
  // device root inode
  minode *mnt_pnt;
  // device path ex: ~/project/exampledisk
  char dev_path[64];
  // mount path ex: / for root, /A or /B or /C ... for non-root
  char mnt_path[64];
  // superblock
  super_block super_block;
  // group_desc
  group_desc group_desc;
} mount_entry;

// bmap == dev_gd->bg_block_bitmap;
// imap == dev_gd->bg_inode_bitmap;
// iblock == dev_gd->bg_inode_table;

//// VAR ////

// in memory  inodes
minode minode_arr[NUM_MINODES];

// root mounted inode
minode *global_root_inode;

// mount tables
mount_entry mount_entry_arr[NUM_MOUNT_ENTRIES];

mount_entry *global_root_mount;

// Opened file instance
oft oft_arr[NUM_OFT];

// PROC structures
proc proc_arr[NUM_PROCS];

// current executing PROC
proc *running;

//// FUNC ////

// fs_io
oft *alloc_oft();
bool free_oft(oft *);
int *get_lbk(blk_iter *, int);
int open_file(char *, int);
int lseek_file(int, int, int);
int close_file(int);
int read_file(int, void *, unsigned int);
int write_file(int, void *, unsigned int);

// fs_minode
minode *alloc_minode();
bool free_minode(minode *);
minode *get_minode(mount_entry *, int);
bool put_minode(minode *);

// fs_mount
int fs_init();
mount_entry *make_me(char *, char *);

// fs_path
int parse_path(char *, path *);
int search_dir(minode *, char *);
minode *search_path(path);
int list_dir(minode *, dir_entry *);
int count_dir(minode *);

// fs_util
int alloc_inode(mount_entry *);
int free_inode(mount_entry *, int);
int alloc_block(mount_entry *);
int free_block(mount_entry *, int);
int get_block(mount_entry *, int, char *);
int put_block(mount_entry *, int, char *);
int tst_bit(char *, int);
int set_bit(char *, int);
int clr_bit(char *, int);
int add_dir_entry(minode *, dir_entry *);
int rm_dir_entry(minode *, char *);
int free_i_block(minode *);
#endif