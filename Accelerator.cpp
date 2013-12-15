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

	//res = lstat(path, stbuf);
	//if (res == -1)
	//	return -errno;

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
				if (fs.root[i].attribute & (1 << 3)) {
					stbuf->st_mode = S_IFDIR | 0444;
				} else {
					stbuf->st_mode = S_IFREG | 0444;
				}
				stbuf->st_nlink = 1;
				stbuf->st_mtime = test_time;
				stbuf->st_size = fs.root[i].file_size;
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
	//DIR *dp;
	//struct dirent *de;

	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	
	/*
	char *hello_path = "/dummy.txt";
	filler(buf, hello_path + 1, NULL, 0);
	*/
	
	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			filler(buf, bname + 1, NULL, 0);
		}
	}

	/*while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}*/

	//closedir(dp);
	return 0;
}

/**
  * Implementation of mknod
  */
static int sister_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	/*if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno; */

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
			cout << "LUCU\n";
			res = 0;
			break;
		}
	}
	/*res = rmdir(path);
	if (res == -1)
		return -errno;*/

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
	
	/*res = rename(from, to);
	if (res == -1)
		return -errno;*/

	return res;
}

/**
  * Implementation of truncate
  */
static int sister_truncate(const char *path, off_t size)
{
	int res;

	/*res = truncate(path, size);
	if (res == -1)
		return -errno; */

	return 0;
}

/**
  * Implementation of open
  */
static int sister_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	/*res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res); */
	return 0;
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
	for (int i = 0; i < argc; i++) {
		if (string(argv[i]) == "new") fs.createNew(filename, "newPartition");
		if (string(argv[i]).find(".fs")!=std::string::npos) filename = string(argv[i]);
		if (string(argv[i]).find(".fs")!=std::string::npos || string(argv[i]) == "new") {
			for (int j = i; j < argc - 1; j++) argv[j] = argv[j+1];
			argc--; i--;
		}
	}
	//fs.readFile(filename);

	//umask(0);
	return fuse_main(argc, argv, &sister_oper, NULL);

	//return 0;
}
