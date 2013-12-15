/**
  * Accelerator.cpp File
  * Copyright (C) 2013
  * @Frexesc
  *
*/

#define FUSE_USE_VERSION 26

#include "Accelerator.h"

FileHelper fs;

/**
  * Implementation of getattr
  */
static int sister_getattr(const char *path, struct stat *stbuf)
{
	int res = -ENOENT;

	time_t test_time = time(0); // current time
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) { // ROOT Directory
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		res = 0;
	}

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			if (strcmp(bname, path) == 0) {
				if (fs.root[i].attribute & (1 << 3)) { // If directory
					stbuf->st_mode = S_IFDIR | 0444;
				} else { // If regular file
					if (fs.root[i].attribute & (1 << 0)) stbuf->st_mode = S_IFREG | 0444;
					else if (fs.root[i].attribute & (1 << 2)) stbuf->st_mode = S_IFREG | 0555;
					else stbuf->st_mode = S_IFREG | 0666;
				}
				stbuf->st_nlink = 1;
				stbuf->st_mtime = test_time; // TODO : change to file time
				stbuf->st_size = fs.root[i].file_size; // file size
				res = 0;
			}
		}
	}

	return res;
}

/**
  * Implementation of readdir
  */
static int sister_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{

	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			filler(buf, bname + 1, NULL, 0);
		}
	}

	return 0;
}

/**
  * Implementation of mkdir

  */
static int sister_mkdir(const char *path, mode_t mode)
{
	int res = -ENOENT;
	string name(path);
	name = name.substr(1);
	fs.createDir(name);
	res = 0;
	/*res = mkdir(path, mode);

	if (res == -1)
		return -errno;*/

	return res;
}

/**
  * Implementation of mknod (TODO)
  */
static int sister_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	// res = sister_mkdir(path, mode);

	return 0;
}


/**
  * Implementation of rmdir
  */
static int sister_rmdir(const char *path)
{
	int res = -ENOENT;	
	for (int i = 0; i < 32; i++) {
		string c = ""; string fr(fs.root[i].name); c.push_back('/'); c += fr;
		string n(path);
		if (c == n) {
			fs.rmDir(i);
			res = 0;
			break;
		}
	}

	return res;
}

/**
  * Implementation of rename
  */
static int sister_rename(const char *from, const char *to)
{
	int res = -ENOENT;
	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			string c = ""; string fr(fs.root[i].name); c.push_back('/'); c += fr;
			string n(from);
			if (c == n) {
				for (int j = 0; j < 21; j++) {
					if (to[j+1] != '\0') {
						fs.root[i].name[j] = to[j+1];
					} else {
						fs.root[i].name[j] = '\0';
						break;
					}
				}
				res = 0;
			}
		}
	}

	return res;
}

// TODO : Implement newFile at FileHelper.cpp
/**
  * Implementation of truncate
  * USAGE EXAMPLE: truncate -s 1k dummy.txt (truncate file to 1024 byte)
  */
static int sister_truncate(const char *path, off_t size)
{
	int res= -ENOENT;

	for (int i = 0; i < 32; i++) {
		if (fs.root[i].attribute & (1 << 3)) {} else { // if not a directory
			string c = ""; string fr(fs.root[i].name); c.push_back('/'); c += fr;
			string n(path);
			if (c == n) {
				// Truncate here
				fs.truncateFile(i, size);
				res = 0;
				break;
			}
		}
	}

	return res;
}

/**
  * Implementation of open
  */
static int sister_open(const char *path, struct fuse_file_info *fi)
{
	int res = 0;

	/*res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res); */
	return res;
}

/**
  * Implementation of read
  */
static int sister_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res = 0;
	/*int fd;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd); */ 
	return res;
}

/**
  * Implementation of write
  */
static int sister_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int res = 0;
	/*int fd;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd); */
	return res;
}

/** C++ only */
static struct sister_fuse_operations: fuse_operations {
	sister_fuse_operations ()
	{
		getattr		= sister_getattr;
		readdir		= sister_readdir;
		mknod		= sister_mknod;
		mkdir		= sister_mkdir;
		rmdir		= sister_rmdir;
		rename		= sister_rename;
		truncate	= sister_truncate;
		open		= sister_open;
		read		= sister_read;
		write		= sister_write;
	}
} sister_oper;

int main(int argc, char *argv[]) {
	/** Parsing parameter */
	string filename = "";
	for (int i = 0; i < argc; i++) { // Depends on new and .fs filename
		if (string(argv[i]) == "new") fs.createNew("newPartition");
		if (string(argv[i]).find(".fs")!=std::string::npos) {
			filename = string(argv[i]);
			fs.filename = filename;
		}
		if (string(argv[i]).find(".fs")!=std::string::npos || string(argv[i]) == "new") {
			for (int j = i; j < argc - 1; j++) argv[j] = argv[j+1];
			argc--; i--;
		}
	}
	if (filename == "") return -1; // exception
	//fs.readFile();

	return fuse_main(argc, argv, &sister_oper, NULL);

	//return 0;
}
