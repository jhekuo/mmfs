/**
 *  Created by gmm on Thu Mar 30, 2023
 *  mmfs.h - store memory structure and operations.
 */

#ifndef MMFS_H
#define MMFS_H

#include <cstdint>

/* NAT memeory structure */
struct mmfs_nm_info {

};


/* mmfs superblock memeory structure */
struct mmfs_sb_info {
    struct super_block *sb;			/* pointer to VFS super block */
	struct mmfs_super_block *raw_super;	/* raw super block pointer */
};

#endif