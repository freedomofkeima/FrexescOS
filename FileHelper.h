/**
  * FileHelper.h File
  * Copyright (C) 2013
  * @Frexesc
  *
*/

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <bitset>
#include <stdlib.h>

using namespace std;

#define BLOCK_SIZE 1024
#define SAT_OFFSET 1024
#define ROOT_OFFSET 132096
#define POOL_OFFSET 133120

typedef struct {
	char buffer[BLOCK_SIZE];
} block;

typedef struct {
	char name[21];
	char attribute;
	unsigned char hour[2];
	unsigned char date[2];
	int block_pointer; // 2 byte
	int file_size; // 4 byte
} file_info;

unsigned char* convertIntToChar(int input);
unsigned char* convert2IntToChar(int input);
int convertCharToInt(unsigned char input[4]);
int convert2CharToInt(unsigned char input[2]);

class FileHelper {
	public:
		string filename; // filename
		/** Volume Information */
		string volumeName; // volume name
		int empty_block; // out of 65534
		int first_pointer; // first-free block number
		/** Sister Allocation Table (SAT) */
		block sat[128];
		/** Root Directory */
		file_info root[32];

		FileHelper(); // default constructor

		void createNew(string); // create a new filesystem
		void writeFile(bool); // write to a file
		void readFile(); // read from file
		
		char* readDataPool(int); // read Data Pool from certain block
		void updateRootDirectory(file_info); // update Root Directory using file_info
		void updateRootDirectory(char*); // update Root Directory (seek first)
		void updateDataPool(int, file_info); // update Data Pool using file_info
		void updateDataPool(int, char*); // update Data Pool at certain block

		void printInfo(); // print info to screen
		file_info getDataPool(int);
		time_t getTimeInfo(file_info);
		void createDummy();
		void createDir(string);
		bitset<4> getAttr(char);
		void deleteRootDirectory(int);
		void rmDir(int);
		
		void truncateFile(int, int); // Truncate file
		void newFile(file_info, char*); // Create new file (for truncate)
		void newFile(string, char*); // Create new file

		int getSAT(int); // Get SAT at certain index
		void setSAT(int, int); // Set SAT at certain index with certain value
		int newSAT(); // Creating new file / directory and returning SAT address block
		void nextSAT(int, char*, int); // Reserving SAT based on block_pointer and file size
		void removeSAT(int); // Remove SAT based on block_pointer till zero indices
};

#endif
