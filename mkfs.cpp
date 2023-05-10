/**
 *  Created by gmm on Mon Mar 27, 2023
 *  mkfs - build a Linux file system
 */

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

#include <linux/fs.h>

#include "include/mmfs_fs.h"
using namespace std;

struct mmfs_super_block super_block;
struct mmfs_global_parameters mmfs_params;


/**
 * @brief   Routine to  check if the device is already mounted
 * @param	None
 * @return	0 if device is not mounted
 * 		   -1 if already mounted
 */

static int8_t is_device_mounted()
{
	printf("Checking the mounting status: ");
	FILE *file;
	struct mntent *mnt; /* mntent structure to retrieve mount info */

	/* 获取文件系统挂载信息*/
	if ((file = setmntent(MOUNTED, "r")) == NULL)
		return 0;

	while ((mnt = getmntent(file)) != NULL) {
		if (!strcmp(mmfs_params.device_name, mnt->mnt_fsname)) {
			printf("Error: %s is already mounted\n",
					mmfs_params.device_name);
			return -1;
		}
	}
	endmntent(file);

	printf("Info: Unmounted!\n");
	return 0;
}

/**
 * @brief   It writes buffer to disk or storage meant to be formatted
 * @param	fd File descriptor for device
 * @param	buf buffer to be written
 * @param	offset where to bw written on the device
 * @param	length length of the device
 * @return	0 if success
 */
static int writetodisk(int32_t fd, void *buf, uint64_t offset, size_t length)
{
	/* lseek和write合起来是原子操作，设置文件偏移量并写入数据 */
	/* 64表示64位，索引长度更长 */
	if (lseek64(fd, offset, SEEK_SET) < 0) {
		printf("\n\tError: While lseek to the derised location!!!\n");
		return -1;
	}

	if (write(fd, buf, length) < 0) {
		printf("\n\tError: While writing data to the disk!!! Error Num : \
				%d\n", errno);
		return -1;
	}

	// cout << "向这里" << offset << "写入数据" << buf << endl;

	return 0;
}

static void parse_options(int argc, char *argv[]) {
	/* 先不考虑参数 */
	static const char *option_string = "v";
	int32_t option = 0;
	while((option = getopt(argc, argv, option_string)) != EOF) {
		switch (option)
		{
		case 'v':
			printf("version 1.0\n");
			break;
		default:
			printf("Unknown parameter\n");
			break;
		}
	}

	if ((optind + 1) != argc) {
		printf("Error: Device not specified\n");
		exit(-1);
	} 

	mmfs_params.device_name = argv[optind];
}


/**
 *
 */
static int8_t get_device_info() {
	int32_t fd = 0;
	struct stat stat_buf;

	/* file descriptor 文件描述符 */
	/* O_RDWR 表示 读写形式打开*/
	fd = open(mmfs_params.device_name, O_RDWR);
	cout << "fd is " << fd << endl;
	if (fd < 0) {
		printf("\n\tError: Failed to open the device!\n");
		return -1;
	}

	mmfs_params.fd = fd;

	/* 获取文件的stat */
	if (fstat(fd, &stat_buf) < 0 ) {
		printf("\n\tError: Failed to get the device stat!!!\n");
		return -1;
	}


	/* 针对块设备/常规文件 */

	if (S_ISREG(stat_buf.st_mode)) {
		mmfs_params.size = stat_buf.st_size;
	} else if  (S_ISBLK(stat_buf.st_mode)) {
		if (ioctl(fd, BLKGETSIZE64, &mmfs_params.size) < 0) {
			printf("\n\tError: Cannot get the device size\n");
			return -1;
		}
	} else {
		printf("\n\n\tError: Volume type is not supported!!!\n");
		return -1;
	}
	/* 21474836480 Bytes
	   21474836480/1024/1024/1024 = 20 GB	
	 */
	/*
		sector 扇区大小是 512KB，不考虑扇区。
	*/
	printf("The Device Size is %ld Bytes.\n", mmfs_params.size);

	mmfs_params.total_blocks = mmfs_params.size / (BLOCK_SIZE);

	printf("The Device has %ld Blocks. (4KB per block)\n", mmfs_params.total_blocks);

	return 0;
}

/**
 * 初始化超级块的内容
 * struct mmfs_super_block {
    uint32_t magic;
    uint32_t version;
    uint32_t checksum_offset;		/* checksum offset inside super block /

    uint32_t block_size;        /* data block size in bytes /
    uint32_t blocks_per_seg; 
    uint64_t block_count;       /* data block count /

    uint32_t segment_count;		/* total # of segments /
    // uint64_t data_table_entries_count;

    uint32_t segment_count_ckpt;	/* # of segments for checkpoint /
	uint32_t segment_count_sit;	    /* # of segments for SIT /
	uint32_t segment_count_nat;	    /* # of segments for NAT /
	uint32_t segment_count_main;	/* # of segments for main area /
	uint32_t segment0_blkaddr;  	/* start block address of segment 0 /

    uint32_t cp_blkaddr;		/* start block address of checkpoint /
	uint32_t sit_blkaddr;		/* start block address of SIT /
	uint32_t nat_blkaddr;		/* start block address of NAT /
    uint32_t main_blkaddr;		/* start block address of main area /

    // uint64_t root_addr; 		    /* root inode address /
    // uint64_t link_inode_addr; 		/* overflow inode(for linear hash) address /
    // uint64_t data_block_table_addr; /* dir block map table address /
    // uint64_t data_blocks_addr; 		/* data blocks address /

    uint32_t root_ino;	/* root inode number(the address of the inode)/
    //uint32_t node_ino;	/* overflow inode number /
	//uint32_t meta_ino;  /* meta inode number /

    // uint32_t inode_count;       
    uint8_t uuid[16];			/* 128-bit uuid for volume /		   
    uint16_t volume_name[512];  /* volume name /
} __attribute__((packed));
*/
static int prepare_super_block() {

	super_block.magic = SUPER_MAGIC; /* magic number */
	super_block.version = 1;
	super_block.checksum_offset = 0;

	super_block.block_size = BLOCK_SIZE;
	super_block.blocks_per_seg = BLOCKS_PER_SEGMENT;

	super_block.block_count = mmfs_params.total_blocks;
	super_block.segment_count = super_block.block_count / BLOCKS_PER_SEGMENT;

	super_block.volume_name[1] = super_block.volume_name[0] = 'M';
	super_block.volume_name[2] = 'F'; 	super_block.volume_name[3] = 'S';
	super_block.volume_name[4] = '\0';

	uuid_generate(super_block.uuid);

	super_block.root_ino = 1; /* 0表示未分配，初始为1 */

	/* 第一个segment存放SuperBlock */
	/* 后面是 cp, sit, nat, main */
	/* SuperBlock*/
	uint32_t offset = SEGMENT_SIZE;
	/* cp */
	super_block.segment0_blkaddr = offset / BLOCK_SIZE;
	super_block.cp_blkaddr = super_block.segment0_blkaddr;
	super_block.segment_count_ckpt = NUMBER_OF_CHECKPOINT_PACK; /* 2个 */

	/* sit */
	super_block.sit_blkaddr = super_block.cp_blkaddr + 
								super_block.segment_count_ckpt * BLOCKS_PER_SEGMENT;
	uint32_t blocks_for_sit = (super_block.segment_count + SIT_ENTRY_PER_BLOCK - 1) / SIT_ENTRY_PER_BLOCK;
	uint32_t sit_segments = (blocks_for_sit + BLOCKS_PER_SEGMENT - 1)
			/ BLOCKS_PER_SEGMENT;
	super_block.segment_count_sit = sit_segments;

	/* nat */
	super_block.nat_blkaddr = super_block.sit_blkaddr + 
								super_block.segment_count_sit * BLOCKS_PER_SEGMENT;
	uint32_t total_valid_blks_available = (super_block.segment_count -
			(super_block.segment_count_ckpt + super_block.segment_count_sit + offset / SEGMENT_SIZE )) * BLOCKS_PER_SEGMENT;
	uint32_t blocks_for_nat = (total_valid_blks_available + NAT_ENTRY_PER_BLOCK - 1)
				/ NAT_ENTRY_PER_BLOCK;
	super_block.segment_count_nat = (blocks_for_nat + BLOCKS_PER_SEGMENT - 1) / BLOCKS_PER_SEGMENT;
	super_block.segment_count_nat = super_block.segment_count_nat;

	/* main */
	super_block.main_blkaddr = super_block.nat_blkaddr +(super_block.segment_count_nat *  BLOCKS_PER_SEGMENT);

	super_block.segment_count_main = super_block.segment_count - (super_block.segment_count_ckpt
			 + super_block.segment_count_sit + super_block.segment_count_nat + offset / SEGMENT_SIZE);

	cout << "block start address and segment counts:" << endl;
	cout << "cp:\t"<< super_block.cp_blkaddr << "\t" << super_block.segment_count_ckpt << endl;
	cout << "sit:\t" <<super_block.sit_blkaddr << "\t" << super_block.segment_count_sit << endl;
	cout << "nat:\t" << super_block.nat_blkaddr << "\t" << super_block.segment_count_nat << endl;
	cout << "main:\t"<< super_block.main_blkaddr << "\t" << super_block.segment_count_main << endl;
	cout << "Segment Count: " << super_block.segment_count << endl;

	return 0;
}

static int8_t write_root_inode() {
	
	struct mmfs_node *raw_node = NULL;
	u_int64_t data_blk_nor;
	u_int64_t main_area_node_seg_blk_offset = 0;

	raw_node = (struct mmfs_node *) calloc(sizeof(struct mmfs_node), 1);
	if (raw_node == NULL) {
		printf("\n\tError: Calloc Failed for raw_node!!!\n");
		return -1;
	}

	raw_node->footer.nid = super_block.root_ino;
	raw_node->footer.ino = super_block.root_ino;
	raw_node->footer.cp_ver = 1;
	raw_node->footer.next_blkaddr = super_block.main_blkaddr + 1;
	/* 不考虑冷热数据分离 */

	raw_node->i.i_mode = 0x41ed;
	raw_node->i.i_links = 2;
	raw_node->i.i_uid = getuid();
	raw_node->i.i_gid = getgid();

	raw_node->i.i_size = 1 * BLOCK_SIZE; /* dentry */
	raw_node->i.i_blocks = 2;

	raw_node->i.i_atime = time(NULL);
	// raw_node->i.i_atime_nsec = 0;
	raw_node->i.i_ctime = time(NULL);
	// raw_node->i.i_ctime_nsec = 0;
	raw_node->i.i_mtime = time(NULL);
	// raw_node->i.i_mtime_nsec = 0;
	// raw_node->i.i_generation = 0;
	// raw_node->i.i_xattr_nid = 0;
	// raw_node->i.i_flags = 0;
	raw_node->i.i_current_depth = 1;

	data_blk_nor = super_block.main_blkaddr;
	raw_node->i.i_addr[0] = data_blk_nor;

	// raw_node->i.i_ext.fofs = 0;
	// raw_node->i.i_ext.blk_addr = data_blk_nor;
	// raw_node->i.i_ext.len = cpu_to_le32(1);

	main_area_node_seg_blk_offset = super_block.main_blkaddr;
	main_area_node_seg_blk_offset *= BLOCK_SIZE;

	if (writetodisk(mmfs_params.fd, raw_node, main_area_node_seg_blk_offset,
				sizeof(struct mmfs_node)) < 0) {
		printf("\n\tError: While writing the raw_node to disk!!!\n");
		return -1;
	}

	memset(raw_node, 0xff, sizeof(struct mmfs_node));

	if (writetodisk(mmfs_params.fd, raw_node,
				main_area_node_seg_blk_offset + BLOCK_SIZE,
				sizeof(struct mmfs_node)) < 0) {
		printf("\n\tError: While writing the raw_node to disk!!!\n");
		return -1;
	}
	free(raw_node);
	return 0;
}

static int8_t update_nat_root() {
	struct mmfs_nat_block *nat_blk = NULL;
	u_int64_t nat_seg_blk_offset = 0;

	nat_blk = (struct mmfs_nat_block *) calloc(sizeof(struct mmfs_nat_block), 1);
	if(nat_blk == NULL) {
		printf("\n\tError: Calloc Failed for nat_blk!!!\n");
		return -1;
	}

	/* update root */
	nat_blk->entries[super_block.root_ino].block_addr = super_block.main_blkaddr;
	nat_blk->entries[super_block.root_ino].ino = super_block.root_ino;

	// /* update node nat */
	// nat_blk->entries[super_block.node_ino].block_addr = 1;
	// nat_blk->entries[super_block.node_ino].ino = super_block.node_ino;

	// /* update meta nat */
	// nat_blk->entries[super_block.meta_ino].block_addr = 1;
	// nat_blk->entries[super_block.meta_ino].ino = super_block.meta_ino;

	nat_seg_blk_offset = super_block.nat_blkaddr;
	nat_seg_blk_offset *= BLOCK_SIZE;

	if (writetodisk(mmfs_params.fd, nat_blk, nat_seg_blk_offset ,
				sizeof(struct mmfs_nat_block)) < 0) {
		printf("\n\tError: While writing the nat_blk set0 to disk!!!\n");
		return -1;
	}

	free(nat_blk);
	return 0;
}

/**
 * update default dentries in Root Inode
 */
static int8_t add_default_dentry_root() {
	struct mmfs_dentry_block *dent_blk = NULL;
	u_int64_t data_blk_offset = 0;

	dent_blk = (struct mmfs_dentry_block *) calloc(sizeof(struct mmfs_dentry_block), 1);
	if(dent_blk == NULL) {
		printf("\n\tError: Calloc Failed for dent_blk!!!\n");
		return -1;
	}

	/* 默认包含..和. */
	dent_blk->dentry[0].hash_code = 0;
	dent_blk->dentry[0].ino = super_block.root_ino;
	dent_blk->dentry[0].name_len = 1;
	dent_blk->dentry[0].file_type = FT_DIR;
	memcpy(dent_blk->filename[0], ".", 1);

	dent_blk->dentry[1].hash_code = 0;
	dent_blk->dentry[1].ino = super_block.root_ino;
	dent_blk->dentry[1].name_len = 2;
	dent_blk->dentry[1].file_type = FT_DIR;
	memcpy(dent_blk->filename[1], "..", 2);

	/* bitmap for . and .. */
	dent_blk->dentry_bitmap[0] = (1 << 1) | (1 << 0);
	data_blk_offset = super_block.main_blkaddr + 2;
	data_blk_offset *= BLOCK_SIZE;

	if (writetodisk(mmfs_params.fd, dent_blk, data_blk_offset,
				sizeof(struct mmfs_dentry_block)) < 0) {
		printf("\n\tError: While writing the dentry_blk to disk!!!\n");
		return -1;
	}

	free(dent_blk);
	return 0;
}


static int8_t create_root_dir() {
	int8_t err = 0;

	err = write_root_inode();
	if (err < 0) {
		printf("\n\tError: Failed to write root inode!!!\n");
		goto exit;
	}


	err = update_nat_root();
	if (err < 0) {
		printf("\n\tError: Failed to update NAT for root!!!\n");
		goto exit;
	}


	err = add_default_dentry_root();
	if (err < 0) {
		printf("\n\tError: Failed to add default dentries for root!!!\n");
		goto exit;
	}
exit:
	if (err)
		printf("\n\tError: Could not create the root directory!!!\n");

	return err;
}

static int8_t write_super_block() {
	uint32_t index = 0;
	uint8_t *zero_buff;

	zero_buff = (uint8_t *) calloc(BLOCK_SIZE, 1);

	memcpy(zero_buff, &super_block,
						sizeof(super_block));

	// memcpy(zero_buff + SUPER_OFFSET, &super_block,
						// sizeof(super_block));

	for (index = 0; index < 2; index++) {
		if (writetodisk(mmfs_params.fd, zero_buff,
				index * BLOCK_SIZE, BLOCK_SIZE) < 0) {
			printf("\n\tError: While while writing supe_blk \
					on disk!!! index : %d\n", index);
			return -1;
		}
	}

	free(zero_buff);
	return 0;
}


/**
 * trim 命令是用于清除闪存设备上的无效数据的命令
 * 帮助SSD了解被标记为无效的文件位置，提前擦写
 * lsblk --discard查看是否支持Trim
 * loop 设备介绍：在类 UNIX 系统里，loop 设备是一种伪设备(pseudo-device)，或者也可以说是仿真设备。它能使我们像块设备一样访问一个文件
 * https://wjw465150.github.io/blog/Linux/my_data/%E7%A3%81%E7%9B%98/loop%20%E8%AE%BE%E5%A4%87%E4%BB%8B%E7%BB%8D/noname.htm
 */
int trim_device() {
	unsigned long long range[2];
	struct stat stat_buf;

	range[0] = 0;
	range[1] = mmfs_params.size;

	if (fstat(mmfs_params.fd, &stat_buf) < 0 ) {
		printf("\n\tError: Failed to get the device stat!!!\n");
		return -1;
	}

	if (S_ISREG(stat_buf.st_mode))
		return 0;
	else if (S_ISBLK(stat_buf.st_mode)) {
		if (ioctl(mmfs_params.fd, BLKDISCARD, &range) < 0)
			printf("Info: This device doesn't support TRIM\n");
	} else
		return -1;
	return 0;
}

/**
 * @brief   initialize SIT Data structure
 * @param	None
 * @return	0 if success
 */
static int8_t init_sit_area(void) {
	uint32_t index = 0;
	uint64_t sit_seg_blk_offset = 0;
	uint8_t *zero_buf = NULL;

	zero_buf = (uint8_t *) calloc(sizeof(uint8_t), SEGMENT_SIZE);
	if(zero_buf == NULL) {
		printf("\n\tError: Calloc Failed for sit_zero_buf!!!\n");
		return -1;
	}

	/* 偏移量 */
	sit_seg_blk_offset = super_block.sit_blkaddr * BLOCK_SIZE;

	for (index = 0;
		index < (super_block.segment_count_sit);
								index++) {
		if (writetodisk(mmfs_params.fd, zero_buf, sit_seg_blk_offset,
					SEGMENT_SIZE) < 0) {
			printf("\n\tError: While zeroing out the sit area \
					on disk!!!\n");
			return -1;
		}
		sit_seg_blk_offset = sit_seg_blk_offset + SEGMENT_SIZE;
	}

	free(zero_buf);
	return 0 ;
}

/**
 * @brief   It initialize NAT Area
 * @param	None
 * @return	0 if success
 */
static int8_t init_nat_area(void)
{
	uint32_t index = 0;
	uint64_t nat_seg_blk_offset = 0;
	uint8_t *nat_buf = NULL;

	nat_buf = (uint8_t *) calloc(sizeof(uint8_t), SEGMENT_SIZE);
	if (nat_buf == NULL) {
		printf("\n\tError: Calloc Failed for nat_zero_blk!!!\n");
		return -1;
	}

	nat_seg_blk_offset = super_block.nat_blkaddr;
	nat_seg_blk_offset *= BLOCK_SIZE;

	for (index = 0;
		index < (super_block.segment_count_nat );
								index++) {
		if (writetodisk(mmfs_params.fd, nat_buf, nat_seg_blk_offset,
					SEGMENT_SIZE) < 0) {
			printf("\n\tError: While zeroing out the nat area \
					on disk!!!\n");
			return -1;
		}
		nat_seg_blk_offset = nat_seg_blk_offset + ( SEGMENT_SIZE);
	}

	free(nat_buf);
	return 0 ;
}

static int8_t format_device() {
	
	int8_t err = 0;

	err = trim_device();
	if (err < 0) {
		printf("\n\tError: Failed to trim whole device!!!\n");
		goto exit;
	}

	err = prepare_super_block();
	if (err < 0)
		goto exit;

	err = init_sit_area();
	if (err < 0) {
		printf("\n\tError: Failed to Initialise the SIT AREA!!!\n");
		goto exit;
	}

	err = init_nat_area();
	if (err < 0) {
		printf("\n\tError: Failed to Initialise the NAT AREA!!!\n");
		goto exit;
	}

	err = create_root_dir();
	if (err < 0) {
		printf("\n\tError: Failed to create the root directory!!!\n");
		goto exit;
	}

	// err = write_check_point_pack();
	// if (err < 0) {
	// 	printf("\n\tError: Failed to write the check point pack!!!\n");
	// 	goto exit;
	// }

	err = write_super_block();
	if (err < 0) {
		printf("\n\tError: Failed to write the Super Block!!!\n");
		goto exit;
	}
exit:
	if (err)
		printf("\n\tError: Could not format the device!!!\n");

	/*
	 * We should call fsync() to flush out all the dirty pages
	 * in the block device page cache.
	 * 同步内存中所有已修改的文件数据到储存设备。
	 */
	if (fsync(mmfs_params.fd) < 0)
		printf("\n\tError: Could not conduct fsync!!!\n");

	if (close(mmfs_params.fd) < 0)
		printf("\n\tError: Failed to close device file!!!\n");

	return err;
}


int main(int argc, char *argv[]) {
    printf("\n	(MMFS) Start formatting:\n\n");

	parse_options(argc, argv);

    if (is_device_mounted() < 0)
        return -1;
    
	if (get_device_info() < 0) 
		return -1;

	if (format_device() < 0)
		return -1;

	printf("Info: format successfully!\n");

    return 0;
}