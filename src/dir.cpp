/**
 * 
 * 
 */

#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/mmfs.h"

#include <iostream>

mmfs_inode* mmfs_get_inode(const char *path) {

}


int mmfs_stat(const char* path, struct stat* buf) {
    mmfs_inode * inode = mmfs_get_inode(path);
    
}
