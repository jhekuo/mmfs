/**
 *  Created by gmm on Mon Mar 27, 2023
 *  mkfs - build a Linux file system
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mntent.h>
#include <unistd.h>
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
	FILE *file;
	struct mntent *mnt; /* mntent structure to retrieve mount info */

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
	return 0;
}


static void parse_options(int argc, char *argv[]) {
	/* 先不考虑参数 */

	if ((optind + 1) != argc) {
		printf("Error: Device not specified\n");
	} 

	mmfs_params.device_name = argv[optind];
}


/**
 *
 */
static int8_t get_device_info() {



}


int main(int argc, char *argv[]) {
    printf("Start formatting\n");

	parse_options(argc, argv);

    if (is_device_mounted() < 0)
        return -1;
    
	if (get_device_info() < 0) 
		return -1;

    return 0;
}