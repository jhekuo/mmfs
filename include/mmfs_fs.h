/**
 * Define meta data
 * 
 */

#ifndef MMFS_FS_H
#define MMFS_FS_H
#define FUSE_USE_VERSION 31

#include <cstdint>
#include <fuse3/fuse.h>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <cstddef>
#include <cassert>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <uuid/uuid.h>

using namespace std;

#define BLOCK_SIZE 4096     /* Size of Block is 4KB */
#define SB_SIZE 4096        /* Super Block Size */
#define SUPER_MAGIC	0x19062023	/* Magic Number */
#define SUPER_OFFSET	1024		/* byte-size offset */

#define	BLOCKS_PER_SEGMENT	512 /* 每个Segment有512个BLOCK */
#define SEGMENT_SIZE (BLOCK_SIZE * BLOCKS_PER_SEGMENT) /* 2MB */

#define	NUMBER_OF_CHECKPOINT_PACK	2

/* NAT */
struct mmfs_nat_entry {
	uint8_t version;		/* latest version of cached nat entry */
	uint32_t ino;		    /* inode number inode号 */
	uint32_t block_addr;	/* block address 块地址 */
} __attribute__((packed));  /* __attribute__((packed)) 用于紧凑对齐 */

#define NAT_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(struct mmfs_nat_entry))
struct mmfs_nat_block {
	struct mmfs_nat_entry entries[NAT_ENTRY_PER_BLOCK];
} __attribute__((packed));


/* SUPER BLOCK */
struct mmfs_super_block {
    uint32_t magic;
    uint32_t version;
    uint32_t checksum_offset;		/* checksum offset inside super block */

    uint32_t block_size;        /* data block size in bytes */
    uint32_t blocks_per_seg; 
    uint64_t block_count;       /* data block count */

    uint32_t segment_count;		/* total # of segments */
    // uint64_t data_table_entries_count;

    uint32_t segment_count_ckpt;	/* # of segments for checkpoint */
	uint32_t segment_count_sit;	    /* # of segments for SIT */
	uint32_t segment_count_nat;	    /* # of segments for NAT */
	uint32_t segment_count_main;	/* # of segments for main area */
	uint32_t segment0_blkaddr;  	/* start block address of segment 0 */

    uint32_t cp_blkaddr;		/* start block address of checkpoint */
	uint32_t sit_blkaddr;		/* start block address of SIT */
	uint32_t nat_blkaddr;		/* start block address of NAT */
    uint32_t main_blkaddr;		/* start block address of main area */

    // uint64_t root_addr; 		    /* root inode address */
    // uint64_t link_inode_addr; 		/* overflow inode(for linear hash) address */
    // uint64_t data_block_table_addr; /* dir block map table address */
    // uint64_t data_blocks_addr; 		/* data blocks address */

    uint32_t root_ino;	/* root inode number(the address of the inode)*/
    // uint32_t node_ino;	/* overflow inode number */
	// uint32_t meta_ino;  /* meta inode number */

    // uint32_t inode_count;       
    uint8_t uuid[16];			/* 128-bit uuid for volume */		   
    uint16_t volume_name[512];  /* volume name */
} __attribute__((packed));

/*
Meta inode（元数据inode）：Meta inode是文件系统中的元数据管理节点。它存储了文件系统的整体结构和元数据信息，包括超级块（superblock）、区域管理表（segment management table）、节点表（node table）等。Meta inode对于维护文件系统的完整性和一致性非常重要。

Node inode（节点inode）：Node inode用于管理中的节点（node）。使用节点作为文件系统的主要数据结构，用于存储文件和目录的数据和元数据。Node inode存储了节点的地址和其他相关信息，包括节点所属的区域和位置等。

Root inode（根inode）：Root inode是文件系统的根节点。它代表文件系统中最高层的目录，是整个文件系统的起点。Root inode存储了根目录的元数据，包括文件名、权限等信息，通过它可以访问整个文件系统中的其他文件和目录。
*/

/* NODE: inode, direct node, indirect node */
#define ADDRS_PER_INODE 923	    /* 每个Inode的Address Pointers */
#define MAX_NAME_LEN 256        /* 文件名最大长度 */
struct mmfs_inode { 
    uint16_t i_mode;    /* 文件类型 */
    // uint8_t i_advise;			/* file hints */
	// uint8_t i_reserved;		/* reserved */
    uint32_t i_uid;     /* user ID */
    uint32_t i_gid;     /* group ID */
    uint32_t i_links;			/* links count */
    uint64_t i_size;    /* file size (bytes) */
    uint64_t i_blocks;  /* file size (blocks) */
    uint64_t i_atime;	/* access time */
	uint64_t i_ctime;	/* change time */
	uint64_t i_mtime;	/* modification time */
    uint32_t i_current_depth;	/* directory depth */
    // uint32_t i_flags;			/* file attributes */
    uint32_t i_pino;		/* parent inode number */
	uint32_t i_namelen;		/* file name length */
    uint8_t i_name[MAX_NAME_LEN];    /* file name, 最大是256 */

    uint32_t i_addr[ADDRS_PER_INODE]; /* 直接索引 */

    uint32_t  i_nid[5]; /* direct node 0-1, indirect node 2-3, double_indirect node 4 */
} __attribute__((packed));


#define ADDRS_PER_BLOCK 1018    /* 在每个Direct Node Block存放的指针数 */
struct direct_node {
    uint32_t addr[ADDRS_PER_BLOCK];
} __attribute__((packed));


#define NIDS_PER_BLOCK 1018     /* 在每个Indirect Node Block存放的Node ID数量 */
struct indirect_node {
    uint32_t nid[NIDS_PER_BLOCK];
} __attribute__((packed));


struct node_footer {
    uint32_t nid;           /* node id */
    uint32_t ino;           /* inode number */
    // uint32_t flag;          
    uint64_t cp_ver;        /* checkpoint version, 暂不考虑 */
    uint32_t next_blkaddr;  /* next block address */
} __attribute__((packed));


/* 三种类型：inode, direct node, indirect node */
struct mmfs_node {
    union 
    {
        struct mmfs_inode i;
        struct direct_node dn;
		struct indirect_node in;
    };
    struct node_footer footer;  /* 判断node的类型 */
} __attribute__((packed));

/* dentry(directory entry) */
/* 11 bytes */
struct mmfs_dir_entry {
    uint32_t hash_code;  /* file name hash code */
    uint32_t ino;        /* inode number */
    uint16_t name_len;   /* file name lengh */
    uint8_t file_type;   /* file type */
} __attribute__((packed));


#define BITS_PER_BYTE 8
#define NR_DENTRY_IN_BLOCK 214
#define SIZE_OF_DENTRY_BITMAP ((NR_DENTRY_IN_BLOCK + BITS_PER_BYTE - 1) / \
					BITS_PER_BYTE)
#define MMFS_NAME_LEN   8

struct mmfs_dentry_block {
    uint8_t dentry_bitmap[SIZE_OF_DENTRY_BITMAP]; /* 指示每个dentry是否有效 */
    // uint8_t 
    struct mmfs_dir_entry dentry[NR_DENTRY_IN_BLOCK]; /* 214个dentry */
    uint8_t filename[NR_DENTRY_IN_BLOCK][MMFS_NAME_LEN]; /* dentry对应的文件名 */
} __attribute__((packed));



/*
 * For SIT entries
 *
 * Each segment is 2MB in size by default so that a bitmap for validity of
 * there-in blocks should occupy 64 bytes, 512 bits.
 * Not allow to change this.
 * 
 * 64个字节，即512个比特，这512个比特指示了512个4KB大小的block的占用状态
 */
#define SIT_VBLOCK_MAP_SIZE 64
#define SIT_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(struct mmfs_sit_entry))

/*
 * Note that sit_entry->vblocks has the following bit-field information.
 * [15:10] : allocation type such as CURSEG_XXXX_TYPE
 * [9:0] : valid block count
 */
#define SIT_VBLOCKS_SHIFT	10
#define SIT_VBLOCKS_MASK	((1 << SIT_VBLOCKS_SHIFT) - 1)
#define GET_SIT_VBLOCKS(raw_sit)				\
	(le16_to_cpu((raw_sit)->vblocks) & SIT_VBLOCKS_MASK)
#define GET_SIT_TYPE(raw_sit)					\
	((le16_to_cpu((raw_sit)->vblocks) & ~SIT_VBLOCKS_MASK)	\
	 >> SIT_VBLOCKS_SHIFT)

struct mmfs_sit_entry {
	int16_t vblocks;				/* reference above */
	int8_t valid_map[SIT_VBLOCK_MAP_SIZE];	/* bitmap for valid blocks */
	// __le64 mtime;				/* segment age for cleaning */
} __attribute__((packed));

struct mmfs_sit_block {
	struct mmfs_sit_entry entries[SIT_ENTRY_PER_BLOCK];
} __attribute__((packed));


/* file types used in inode_info->flags */
enum {
	FT_UNKNOWN,
	FT_REG_FILE,
	FT_DIR,
	FT_CHRDEV,
	FT_BLKDEV,
	FT_FIFO,
	FT_SOCK,
	FT_SYMLINK,
	FT_MAX
};

/*------------------------------------------------------------*/
/* format */

struct mmfs_global_parameters {
	// uint32_t       sector_size;
	// uint32_t       reserved_segments;
	// uint32_t       overprovision;
	// uint32_t	    cur_seg[6];
	// uint32_t       segs_per_sec;
	// uint32_t       secs_per_zone;
	// uint32_t       start_sector;
	// uint64_t	    total_sectors;
	// uint32_t       sectors_per_blk;
	// uint32_t       blks_per_seg;
	// uint8_t        vol_label[16];
	// int heap;
    uint64_t total_blocks;
    uint64_t size;  /* 2 ^ 64 = 16EB = 16GGB */
	int32_t   fd; /* file descriptor 文件描述符 */
	char   *device_name;
	// char   *extension_list;
} __attribute__((packed));


#endif
