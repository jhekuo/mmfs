
#ifndef MMFS_FUSE_H
#define MMFS_FUSE_H

#include "mmfs.h"

class MMFS {
public:
    const char * device_path;
    const char * mount_path;
    MMFS(const char * device_path, const char* mount_path) {
        // 初始化，打开设备，读取信息
        this->device_path = device_path;
        this->mount_path = mount_path;
    }

};

MMFS* mmfs;

int fuse_getattr (const char *, struct stat *, struct fuse_file_info *fi);

int fuse_readlink (const char *, char *, size_t);

int fuse_mknod (const char *, mode_t, dev_t);

int fuse_mkdir (const char *, mode_t);

int fuse_unlink (const char *);

int fuse_rmdir (const char *);

int fuse_symlink (const char *, const char *);

int fuse_rename (const char *, const char *, unsigned int flags);

int fuse_link (const char *, const char *);

int fuse_chmod (const char *, mode_t, struct fuse_file_info *fi);

int fuse_chown (const char *, uid_t, gid_t, struct fuse_file_info *fi);

int fuse_truncate (const char *, off_t, struct fuse_file_info *fi);

int fuse_open (const char *, struct fuse_file_info *);

int fuse_read (const char *, char *, size_t, off_t,
             struct fuse_file_info *);

int fuse_write (const char *, const char *, size_t, off_t,
              struct fuse_file_info *);

int fuse_statfs (const char *, struct statvfs *);

int fuse_flush (const char *, struct fuse_file_info *);

int fuse_release (const char *, struct fuse_file_info *);

int fuse_fsync (const char *, int, struct fuse_file_info *);


int fuse_setxattr (const char *, const char *, const char *, size_t, int);
int fuse_getxattr (const char *, const char *, char *, size_t);
int fuse_listxattr (const char *, char *, size_t);
int fuse_removexattr (const char *, const char *);

int fuse_opendir (const char *, struct fuse_file_info *);

int fuse_readdir (const char *, void *, fuse_fill_dir_t, off_t,
                struct fuse_file_info *, enum fuse_readdir_flags);

int fuse_releasedir (const char *, struct fuse_file_info *);

int fuse_fsyncdir (const char *, int, struct fuse_file_info *);

#endif