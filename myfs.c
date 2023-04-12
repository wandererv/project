#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";

static int myfs_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else {
        res = -ENOENT;
    }

    return res;
}

static int myfs_open(const char *path, struct fuse_file_info *fi)
{
    if (strcmp(path, hello_path) != 0)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
		     struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else {
        size = 0;
    }

    return size;
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) 
{
    int fd;

    fd = creat(path, mode);
    if (fd == -1)
        return -errno;

    fi->fh = fd;

    return 0;
}

static int myfs_utimens(const char *path, const struct timespec tv[2])
{
    int res;

    res = utimensat(0, path, tv, AT_SYMLINK_NOFOLLOW);
    if (res == -1)
        return -errno;

    return 0;
}

static struct fuse_operations myfs_ops = {
    .getattr    = myfs_getattr,
    .open       = myfs_open,
    .read       = myfs_read,
    .create     = myfs_create,
    .utimens    = myfs_utimens,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &myfs_ops, NULL);
}
