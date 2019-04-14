#include "cmd.h"

bool do_read() {

  //   int read_file()
  // {
  //   Preparations:
  //     ASSUME: file is opened for RD or RW;
  //     ask for a fd  and  nbytes to read;
  //     verify that fd is indeed opened for RD or RW;
  //     return(myread(fd, buf, nbytes));
  // }

  // int myread(int fd, char buf[ ], nbytes) behaves EXACTLY the same as the
  // read() system call in Unix/Linux.
  // The algorithm of myread() can be best explained in terms of the following
  // diagram.

  // (1).  PROC              (2).                          |
  //      =======   |--> OFT oft[ ]                        |
  //      | pid |   |   ============                       |
  //      | cwd |   |   |mode=flags|                       |
  //      | . ..|   |   |minodePtr ------->  minode[ ]     |      BlockDevice
  //      | fd[]|   |   |refCount=1|       =============   | ==================
  //  fd: | .------>|   |offset    |       |  INODE    |   |   | INODE ->
  //  blocks|
  //      |     |       |===|======|       |-----------|   | ==================
  //      =======           |              |  dev,ino  |   |
  //                        |              =============   |
  //                        |
  //                        |<------- avil ------->|
  //     -------------------|-----------------------
  //     |    |    | ...  |lbk  |   |  ...| .......|
  //     -------------------|---|------------------|-
  // lbk   0    1 .....     |rem|                   |
  //                      start                   fsize

  // ------------------------------------------------------------------------------
  //                  Data structures for reading file

  // (1). Assume that fd is opened for READ.
  // (2). The offset in the OFT points to the current byte position in the file
  // from
  //      where we wish to read nbytes.
  // (3). To the kernel, a file is just a sequence of contiguous bytes, numbered
  // from
  //      0 to file_size - 1. As the figure shows, the current byte position,
  //      offset falls in a LOGICAL block (lbk), which is

  //              lbk = offset / BLKSIZE

  //      the byte to start read in that logical block is

  //              start = offset % BLKSIZE

  //      and the number of bytes remaining in the logical block is

  //              remain = BLKSIZE - start.

  //      At this moment, the file has

  //              avil = file_size - offset

  //      bytes available for read.

  //      These numbers are used in the read algorithm.

  // (4). myread() behaves exactly the same as the read(fd, buf, nbytes) syscall
  // of
  //      Unix/Linux. It tries to read nbytes from fd to buf[ ], and returns the
  //      actual number of bytes read.

  // (5). ============ Algorithm and pseudo-code of myread()
  // =======================

  // int myread(int fd, char *buf, int nbytes)
  // {

  //  1. int count = 0;
  //     avil = fileSize - OFT's offset // number of bytes still available in
  //     file. char *cq = buf;                // cq points at buf[ ]

  //  2. while (nbytes && avil){

  //        Compute LOGICAL BLOCK number lbk and startByte in that block from
  //        offset;

  //              lbk       = oftp->offset / BLKSIZE;
  //              startByte = oftp->offset % BLKSIZE;

  //        // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and
  //        D_INDIRECT

  //        if (lbk < 12){                     // lbk is a direct block
  //            blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL
  //            blk
  //        }
  //        else if (lbk >= 12 && lbk < 256 + 12) {
  //             //  indirect blocks
  //        }
  //        else{
  //             //  double indirect blocks
  //        }

  //        /* get the data block into readbuf[BLKSIZE] */
  //        get_block(mip->dev, blk, readbuf);

  //        /* copy from startByte to buf[ ], at most remain bytes in this block
  //        */ char *cp = readbuf + startByte; remain = BLKSIZE - startByte; //
  //        number of bytes remain in readbuf[]

  //        while (remain > 0){
  //             *cq++ = *cp++;             // copy byte from readbuf[] into
  //             buf[]
  //              oftp->offset++;           // advance offset
  //              count++;                  // inc count as number of bytes read
  //              avil--; nbytes--;  remain--;
  //              if (nbytes <= 0 || avil <= 0)
  //                  break;
  //        }

  //        // if one data block is not enough, loop back to OUTER while for
  //        more ...

  //    }
  //    printf("myread: read %d char from file descriptor %d\n", count, fd);
  //    return count;   // count is the actual number of bytes read
  // }

  //                   OPTMIAZATION OF THE READ CODE:

  // Instead of reading one byte at a time and updating the counters on each
  // byte, TRY to calculate the maximum number of bytes available in a data
  // block and the number of bytes still needed to read. Take the minimum of the
  // two, and read that many bytes in one operation. Then adjust the counters
  // accordingly. This would make the read loops more efficient.

  // REQUIRED: optimize the read algorithm in your project.
}