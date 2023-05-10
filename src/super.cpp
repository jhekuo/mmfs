/**
 *
 *
 */

#include "../include/mmfs_fuse.h"

mmfs_sb_info sbi;

int readdisk(int32_t fd, void *buf, uint64_t offset, uint32_t length) {
	/* lseek和read合起来 */
	/* 64表示64位，索引长度更长 */
	if (lseek64(fd, offset, SEEK_SET) < 0) {
		printf("\n\tError: While lseek to the derised location!!!\n");
		return -1;
	}

	if (read(fd, buf, length) < 0) {
		printf("\n\tError: While writing data to the disk!!! Error Num : \
				%d\n", errno);
		return -1;
    }

	return 0;
}


int fill_super() {
    
}

int mmfs_mount(const char* device_name, const char* mount_name) {
    /* open device */
    int32_t fd = 0;
    fd = open(device_name, O_RDWR);

    if (fd < 0) {
        printf("open device error");exit(-1);
    }

    cout << "fd is " << fd << endl;

    uint8_t *buff = (uint8_t *) calloc(1, BLOCK_SIZE);
    readdisk(fd, buff, 0, BLOCK_SIZE);
    
    /* get superblock */
    sbi.raw_super = (mmfs_super_block *) malloc (sizeof(mmfs_super_block));
    memcpy(sbi.raw_super, buff, sizeof(mmfs_super_block));
    cout << "block count is "<< sbi.raw_super->block_count << endl;


    

}

int mmfs_unmount() {

}