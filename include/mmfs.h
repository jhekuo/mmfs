/**
 *  Created by gmm on Thu Mar 30, 2023
 *  mmfs.h - store memory structure and operations.
 */

#ifndef MMFS_H
#define MMFS_H

#include "mmfs_fs.h"


struct mmfs_inode_info {
    mmfs_inode *raw_inode;
};

/* mmfs superblock memeory structure */
struct mmfs_sb_info {
	struct mmfs_super_block *raw_super;	/* raw super block pointer */
    char* mount_path;
    mmfs_inode_info * root_inode;
};

/* node address table */
struct mmfs_nm_info {
    uint32_t nat_blockaddr;
};

// struct mmfs_nm_info {
	// block_t nat_blkaddr;		/* base disk address of NAT */
	// nid_t max_nid;			/* maximum possible node ids */
	// nid_t init_scan_nid;		/* the first nid to be scanned */
	// nid_t next_scan_nid;		/* the next nid to be scanned */

	// /* NAT cache management */
	// struct radix_tree_root nat_root;/* root of the nat entry cache */
	// rwlock_t nat_tree_lock;		/* protect nat_tree_lock */
	// unsigned int nat_cnt;		/* the # of cached nat entries */
	// struct list_head nat_entries;	/* cached nat entry list (clean) */
	// struct list_head dirty_nat_entries; /* cached nat entry list (dirty) */

	// /* free node ids management */
	// struct list_head free_nid_list;	/* a list for free nids */
	// spinlock_t free_nid_list_lock;	/* protect free nid list */
	// unsigned int fcnt;		/* the number of free node id */
	// struct mutex build_lock;	/* lock for build free nids */

	/* for checkpoint */
	// char *nat_bitmap;		/* NAT bitmap pointer */
	// int bitmap_size;		/* bitmap size */
// };



/* file */
int mmfs_open(const char* path, uint32_t flag);
int mmfs_stat(const char* path, struct stat* buf);

/* dir */

/* super */
int mmfs_mount(const char* device_name, const char* mount_name);

#endif