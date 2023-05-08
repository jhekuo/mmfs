#define FUSE_USE_VERSION 31
#include "include/mmfs_fuse.h"

/**
 *  Compile: g++ -Wall fuse.cpp `pkg-config fuse3 --cflags --libs` -o fuse
 *  Usage:   ./fuse -o modules=subdir,subdir=/dev/nvme0n2 ~/mmfs_mount
 *                                                  设备   挂载点
 */

void fuse_init () {
    
}

int fuse_open (const char* path, struct fuse_file_info *fi) {

    int fd = mmfs_open(path, fi->flags);
    fi->fh = fd;

    // #ifdef FUSE_DEBUG
    //     FILE* fp = fopen("/home/test/fuselog", "a+");
    //     fprintf(fp, "fuse_open path: %s, res: %d\n", path, fd);
    //     fclose(fp);
    // #endif
    return 0;
}

int fuse_getattr (const char* path, struct stat* st, struct fuse_file_info *fi) {
    int res = mmfs_stat(path, st);
    return res;
}

static const struct fuse_operations oper = {
    // .init           = fuse_init,
	.getattr	= fuse_getattr,
	// .readdir	= fuse_readdir,
	// .open		= fuse_open,
	// .read		= fuse_read,
};


int main(int argc, char *argv[]) {
    int ret;
    int i;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    // for (i = 0; i < argc; i++) {
    //     if (strcmp(argv[i], "--help") == 0) {
    //         show_help(argv[0]);
    //         return 0;
    //     }
    // }

    printf("starting fuse main...\n");
    fuse_init();
    ret = fuse_main(args.argc, args.argv, &oper, NULL);
    fuse_opt_free_args(&args);
    printf("fuse main finished, ret %d\n", ret);
    return ret;
}
