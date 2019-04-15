#include "cmd.h"

bool do_write(cmd *c) {
  //   int write_file()
  // {
  //   1. Preprations:
  //      ask for a fd   and   a text string to write;

  //   2. verify fd is indeed opened for WR or RW or APPEND mode

  //   3. copy the text string into a buf[] and get its length as nbytes.

  //      return(mywrite(fd, buf, nbytes));
  // }

  // The algorithm of write_file() can also be explained in terms of the
  // following figure.

  // (1).  PROC              (2).                          |
  //      =======   |--> OFT oft[ ]                        |
  //      | pid |   |   ============                       |
  //      | cwd |   |   |mode=flags|                       |
  //      | . ..|   |   |minodePtr ------->  minode[ ]     |      BlockDevice
  //      | fd[]|   |   |refCount=1|       =============   | ==================
  //  fd: | .------>|   |offset    |       |  INODE    |   |   | INODE ->
  //  blocks|
  //      |     |       |===|======|       |-----------|   | ==================
  //      =======           |              | dev,inode |   |
  //                        |              |  dirty    |   |
  //                        |              =============   |
  //                        |
  //     -------------------|-----------------
  //     |    |    | ...  |lbk  | ............
  //     -------------------|-----------------
  // lbk   0    1 .....     |rem|            |
  //                      start           fileSize (in INODE)

  // ------------------------------------------------------------------------------
  //                Data Structures for write()

  // mywrite behaves exactly the same as Unix's write(fd, buf, nbytes) syscall.
  // It writes nbytes from buf[ ] to the file descriptor fd, extending the file
  // size as needed.

  // int mywrite(int fd, char buf[ ], int nbytes)
  // {
  //   while (nbytes > 0 ){

  //      compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

  //           lbk       = oftp->offset / BLKSIZE;
  //           startByte = oftp->offset % BLKSIZE;

  //     // I only show how to write DIRECT data blocks, you figure out how to
  //     // write indirect and double-indirect blocks.

  //      if (lbk < 12){                         // direct block
  //         if (ip->INODE.i_block[lbk] == 0){   // if no data block yet

  //            mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a
  //            block
  //         }
  //         blk = mip->INODE.i_block[lbk];      // blk should be a disk block
  //         now
  //      }
  //      else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks
  //               // HELP INFO:
  //               if (i_block[12] == 0){
  //                   allocate a block for it;
  //                   zero out the block on disk !!!!
  //               }
  //               get i_block[12] into an int ibuf[256];
  //               blk = ibuf[lbk - 12];
  //               if (blk==0){
  //                  allocate a disk block;
  //                  record it in i_block[12];
  //               }
  //               .......
  //      }
  //      else{
  //             // double indirect blocks */
  //      }

  //      /* all cases come to here : write to the data block */
  //      get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
  //      char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
  //      remain = BLKSIZE - startByte;     // number of BYTEs remain in this
  //      block

  //      while (remain > 0){               // write as much as remain allows
  //            *cp++ = *cq++;              // cq points at buf[ ]
  //            nbytes--; remain--;         // dec counts
  //            oftp->offset++;             // advance offset
  //            if (offset > INODE.i_size)  // especially for RW|APPEND mode
  //                mip->INODE.i_size++;    // inc file size (if offset >
  //                fileSize)
  //            if (nbytes <= 0) break;     // if already nbytes, break
  //      }
  //      put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk

  //      // loop back to outer while to write more .... until nbytes are
  //      written
  //   }

  //   mip->dirty = 1;       // mark mip dirty for iput()
  //   printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);
  //   return nbytes;
  // }

  //                 OPTIMIZATION OF write Code

  // As in read(), the above inner while(remain > 0) loop can be optimized:
  // Instead of copying one byte at a time and update the control variables on
  // each byte, TRY to copy only ONCE and adjust the control variables
  // accordingly.

  // REQUIRED: Optimize the write() code in your project.
  return true;
}