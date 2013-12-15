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
		/** Volume Information */
		string volumeName; // volume name
		int empty_block; // out of 65534
		int first_pointer; // first-free block number
		/** Sister Allocation Table (SAT) */
		block sat[128];
		/** Root Directory */
		file_info root[32];

		FileHelper(); // default constructor

		void createNew(string, string); // create a new filesystem
		void writeFile(string, bool); // write to a file
		void readFile(string); // read from file

		char* readDataPool(string, int); // read Data Pool from certain block
		void updateRootDirectory(string, char*); // update Root Directory (seek first)
		void updateDataPool(string, int, char*); // update Data Pool at certain block

		void printInfo(); // print info to screen
		file_info getDataPool(string, int);
		void parseFileInfo(file_info);
		void createDummy();
};

#endif
