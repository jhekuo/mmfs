
#include "include/mmfs_fuse.h"

/**
 *  Compile: g++ -Wall -o fuse `pkg-config fuse3 --cflags --libs` fuse.cpp
 *  之后后面添加 src/dir.cpp src/file.cpp src/super.cpp
 * 
 *  Usage: ./fuse /dev/nvme0n2p1 ~/mmfs_mount 
 *         ./fuse -v
 */

using namespace std;


// int fuse_open (const char* path, struct fuse_file_info *fi) {

//     int fd = mmfs_open(path, fi->flags);
//     fi->fh = fd;
//     return 0;
// }

// int fuse_getattr (const char* path, struct stat* st, struct fuse_file_info *fi) {
//     int res = mmfs_stat(path, st);
//     return res;
// }

// int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
// 			 off_t offset, struct fuse_file_info *fi,
// 			 enum fuse_readdir_flags flags) {
//     (void) offset;
// 	(void) fi;
// 	(void) flags;
//     cout << path << endl;
//     filler(buf, ".", NULL, 0, (enum fuse_fill_dir_flags) 0);

//     return 0;
// }


static void* fuse_init (struct fuse_conn_info *conn,
			struct fuse_config *cfg) {
    const char* device_path = mmfs->device_path;
    const char* mount_path = mmfs->mount_path;

    mmfs_mount(device_path, mount_path);

}

static const struct fuse_operations oper = {
    .init           = fuse_init,
	// .getattr	= fuse_getattr,
	// .readdir	= fuse_readdir,
	// .open		= fuse_open,
	// .read		= fuse_read,
};


int main(int argc, char *argv[]) {
    int ret;
    int i;
    int opt;
    const char *mount_path = nullptr;
	const char *device_path = nullptr;
    while ((opt = getopt(argc, argv, "v")) != -1)
	{
		switch (opt)
		{
			case 'v':
                printf("Version 1.0\n");
				break;
			default:
				break;
		}
	}

    if ((optind + 2) != argc) {
        std::cout << "Usage: ./fuse <device_path> <mount_path>" << std::endl;
        return 0;
    }

    device_path = argv[optind];
	mount_path = argv[optind + 1];

    /* 初始化 */
    // fuse_init();
    mmfs = new MMFS(device_path, mount_path);

    /* 让argc减去1，因为它只接受一个挂载点参数 */

    argv[optind] = argv[optind + 1];
    argv[optind + 1] = NULL;
    argc--;
    printf("starting fuse main...\n");
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    return fuse_main(args.argc, args.argv, &oper, NULL);
    // ret = fuse_main(args.argc, args.argv, &oper, NULL);
    // fuse_opt_free_args(&args);
    // printf("fuse main finished, ret %d\n", ret);
    // return ret;
}
