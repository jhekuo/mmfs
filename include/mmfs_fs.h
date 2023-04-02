/**
 * Define meta data
 * 
 */

#ifndef MMFS_FS_H
#define MMFS_FS_H

#include <cstdint>

#define BLOCK_SIZE 4096     /* Size of Block is 4KB */
#define SB_SIZE 4096        /* Super Block Size */


/* NAT */
struct mmfs_nat_entry {
	uint8_t version;		/* latest version of cached nat entry */
	uint32_t ino;		    /* inode number */
	uint32_t block_addr;	/* block address */
} __attribute__((packed));  /* __attribute__((packed)) 用于紧凑对齐 */

#define NAT_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(struct mmfs_nat_entry))
struct mmfs_nat_block {
	struct mmfs_nat_entry entries[NAT_ENTRY_PER_BLOCK];
} __attribute__((packed));

/* SUPER BLOCK */
struct mmfs_super_block {
    uint32_t magic;
    uint32_t version;
    uint32_t data_block_size;	/* data block size in bytes */
    uint64_t data_block_count;  /* data block count */
    uint64_t data_table_entries_count;

    uint64_t root_addr; 		/* root inode address */
    uint64_t link_inode_addr; 		/* overflow inode(for linear hash) address */
    uint64_t data_block_table_addr; 		/* dir block map table address */
    uint64_t data_blocks_addr; 		/* data blocks address */

    uint32_t root_ino;	/* root inode number(the address of the inode)*/
    uint32_t link_ino;	/* overflow inode number */

    uint32_t inode_count;		   
} __attribute__((packed));


/* NODE: inode, direct node, indirect node */
#define ADDRS_PER_INODE 923	    /* 每个Inode的Address Pointers */
#define MAX_NAME_LEN 256        /* 文件名最大长度 */
struct mmfs_inode { 
    uint16_t i_mode;    /* 文件类型 */
    uint32_t i_uid;     /* user ID */
    uint32_t i_gid;     /* group ID */
    uint64_t i_size;    /* file size (bytes) */
    uint64_t i_blocks;  /* file size (blocks) */
    uint64_t i_atime;	/* access time */
	uint64_t i_ctime;	/* change time */
	uint64_t i_mtime;	/* modification time */
    uint32_t i_current_depth;	/* directory depth */

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
    uint8_t file_type;   /* file type*/
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

/*------------------------------------------------------------*/
/* format */

struct mmfs_global_parameters {
	// u_int32_t       sector_size;
	// u_int32_t       reserved_segments;
	// u_int32_t       overprovision;
	// u_int32_t	    cur_seg[6];
	// u_int32_t       segs_per_sec;
	// u_int32_t       secs_per_zone;
	// u_int32_t       start_sector;
	// u_int64_t	    total_sectors;
	// u_int32_t       sectors_per_blk;
	// u_int32_t       blks_per_seg;
	// u_int8_t        vol_label[16];
	int heap;
	int32_t     fd;
	char   *device_name;
	char   *extension_list;
} __attribute__((packed));


#endif
