/**
 * 
 * 
 */


#include "../include/mmfs_fuse.h"



mmfs_inode* mmfs_get_inode(const char *path) {

}


int mmfs_stat(const char* path, struct stat* buf) {
    mmfs_inode * inode = mmfs_get_inode(path);
    
}
