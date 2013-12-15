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
				stbuf->st_mtime = fs.getTimeInfo(fs.root[i]);
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
  * Implementation of mknod (TODO : Implement newFile)
  */
static int sister_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res = -ENOENT, idx = 0;

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			if (strcmp(bname, path) == 0) {
				idx = i;
				res = 0;
			}
		}
	}

	if (res == -ENOENT) {
		/** Create new file */
		string filename = path, temp_name = "";
		char data[1] = {'\0'};
		for (int j = 1; j < (int) filename.length(); j++) temp_name += filename[j];
		fs.newFile(temp_name, data);
	}

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
	int res = -ENOENT;

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			if (strcmp(bname, path) == 0) {
				if ((fi->flags & 3) != O_RDONLY) res = -EACCES;
				res = 0;
			}
		}
	}

	return res;
}

/**
  * Implementation of read
  */
static int sister_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res = -ENOENT, idx = 0;

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			if (strcmp(bname, path) == 0) {
				idx = i;
				res = 0;
			}
		}
	}

	if (res == 0) {
		if (offset < fs.root[idx].file_size) {
			if (offset + size > fs.root[idx].file_size)
				size = fs.root[idx].file_size - offset;

			if (fs.root[idx].file_size == 0) fs.root[idx].file_size = 1; // prevent error
			char* oldContent = (char*) malloc (sizeof(char) * (fs.root[idx].file_size - 1));

			/** Read oldContent (Call READ) [Note: Data is started from 2nd Pointer] */
			int current_pointer;
			for (int i = 0; i <= ((fs.root[idx].file_size - 1) / 1024); i++) { // file_size > 0
				char* tempContent = (char*) malloc (sizeof(char) * BLOCK_SIZE);
				// indexing
				int lower_bound = i * BLOCK_SIZE;
				int upper_bound = (i + 1) * BLOCK_SIZE - 1;
				if (i == ((fs.root[idx].file_size - 1) / 1024)) upper_bound = fs.root[idx].file_size - 1;
				current_pointer = fs.getSAT(fs.root[idx].block_pointer);
				tempContent = fs.readDataPool(current_pointer);
				int idx2 = 0;
				for (int j = lower_bound; j <= upper_bound; j++) {
					oldContent[j] = tempContent[idx2];
					idx2++; // next element
				}
			}
			/** End of reading */
			for (int i = offset; i < offset + size; i++) buf[i - offset] = oldContent[i];
			buf[size] = '\0';
			cout << buf << " " << size << endl;
		} else size = 0;
	} else size = 0;
 
	return size;
}

/**
  * Implementation of write
  */
static int sister_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int res = -ENOENT, idx = 0;

	for (int i = 0; i < 32; i++) { // if exact match
		if (fs.root[i].name[0] != '\0') { // not NULL
			char bname[22]; strcpy(bname, "/"); strcat(bname, fs.root[i].name);
			if (strcmp(bname, path) == 0) {
				idx = i;
				if (fs.root[i].attribute & (1 << 0)) res = -EACCES;
				else res = 0;
			}
		}
	}
	
	if (res == -ENOENT) {
		/** Create new file */
		string filename = path, temp_name = "";
		char data[1] = {'\0'};
		for (int j = 1; j < (int) filename.length(); j++) temp_name += filename[j];
		fs.newFile(temp_name, data);
	}

	if (res == 0) {

	}
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
