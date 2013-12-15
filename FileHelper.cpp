/**
  * FileHelper.cpp File
  * Copyright (C) 2013
  * @Frexesc
  *
*/

#include "FileHelper.h"

/**
  * TODO:
  * 1. Create file attribute parser
  * 2. Create date parser
  * 3. Create hour parser
  *
  */

/** Convert int to char* */
unsigned char* convertIntToChar(int input) {
	unsigned char* bytes = new unsigned char[4];
	
	bytes[0] = (input >> 24) & 0xFF;
	bytes[1] = (input >> 16) & 0xFF;
	bytes[2] = (input >> 8) & 0xFF;
	bytes[3] = input & 0xFF;

	return bytes;
}

unsigned char* convert2IntToChar(int input) {
	unsigned char* bytes = new unsigned char[2];

	bytes[0] = (input >> 8) & 0xFF;
	bytes[1] = input & 0xFF;

	return bytes;
}

/** Convert char* to int */
int convertCharToInt(unsigned char input[4]) {
	unsigned int x;
	
	x = *(int *) input;

	return x;
}

int convert2CharToInt(unsigned char input[2]) {
	unsigned int x;

	x = *(int *) input;

	return x;
}

FileHelper::FileHelper() {}

void FileHelper::createNew(string filename, string name) {
	if (name == "") volumeName = "sisterFS";
	else volumeName = name;

	empty_block = 65534; // initialize
	first_pointer = 1;

	for (int i = 0; i < 128; i++) // SAT initialize
		for (int j = 0; j < BLOCK_SIZE; j++)
			sat[i].buffer[j] = '0';

	for (int i = 0; i < 32; i++) { // ROOT initialize
		for (int j = 0; j < 21; j++) root[i].name[j] = '\0';
		root[i].attribute = '\0';
		root[i].hour[0] = '\0'; root[i].hour[1] = '\0';
		root[i].date[0] = '\0';	root[i].date[1] = '\0';
		root[i].block_pointer = 0;
		root[i].file_size = 0;
	}

	ofstream new_file(filename.c_str()); // create a new file for filesystem
	writeFile(filename); // write to new filesystem
}

void FileHelper::writeFile(string filename) {
	ofstream file(filename.c_str()); // open file
	unsigned char* temp;
	/** Volume Information */
	char header[BLOCK_SIZE];
	// 8 byte (0 - 7)
	header[0] = 's'; header[1] = 'i'; header[2] = 's'; header[3] = 't';
	header[4] = 'e'; header[5] = 'r'; header[6] = 'F'; header[7] = 'S';
	// 256 byte (8 - 263)
	for (int i = 8; i < 8 + volumeName.length(); i++) header[i] = volumeName[i - 8];
	for (int i = 8 + volumeName.length(); i < 264; i++) header[i] = '\0';
	// 4 byte (264 - 267)
	header[264] = '1'; header[265] = '0'; header[266] = '2'; header[267] = '4';
	// 4 byte (268 - 271)
	temp = convertIntToChar(empty_block * BLOCK_SIZE);
	for (int i = 0; i < 4; i++) header[268 + i] = temp[i];
	// 4 byte (272 - 275)
	temp = convertIntToChar(empty_block);
	for (int i = 0; i < 4; i++) header[272 + i] = temp[i];
	// 4 byte (276 - 279)
	temp = convertIntToChar(first_pointer);
	for (int i = 0; i < 4; i++) header[276 + i] = temp[i];
	// 740 byte (280 - 1019)
	for (int i = 280; i < 1020; i++) header[i] = '0';
	// 4 byte (1020 - 1023)
	header[1020] = 'S'; header[1021] = 'I'; header[1022] = 'S'; header[1023] = 'T';
	for (int i = 0; i < 1024; i++) file << header[i];

	/** Sister Allocation Table (SAT) */
	// 128 KB (SAT_OFFSET - SAT_OFFSET + 128 * 1024 - 1)
	for (int i = 0; i < 128; i++)
		for (int j = 0; j < BLOCK_SIZE; j++) file << sat[i].buffer[j];

	/** Root Directory */
	// 1024 Byte (ROOT_OFFSET - ROOT_OFFSET + 1023)
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 21; j++) file << root[i].name[j];
		file << root[i].attribute;
		file << root[i].hour[0]; file << root[i].hour[1];
		file << root[i].date[0]; file << root[i].date[1];
		file << root[i].block_pointer;
		file << root[i].file_size;
	}

	/** Data Pool */
	// 65534 Blocks (POOL_OFFSET - POOL_OFFSET + 65534 * 1024 - 1)
	for (int i = 1; i < 65535; i++)
		for (int j = 0; j < BLOCK_SIZE; j++)
			file << '\0';

	file.close();
}

void FileHelper::readFile(string filename) {
	FILE *file;
	file = fopen(filename.c_str(), "rb");
	char* buffer = (char*) malloc (sizeof(char) * POOL_OFFSET);
	if (buffer == NULL) cout << "Memory Error!" << endl;
	int result = fread(buffer, 1, POOL_OFFSET, file);
	fclose(file);

	unsigned char* temp = new unsigned char[4];
	/** Volume Information */
	// 256 byte (8 - 263)
	volumeName = "";
	for (int i = 8; i < 264; i++) {
		if (buffer[i] != '\0') volumeName = volumeName + buffer[i];
		else break;
	}
	// 4 byte (272 - 275) [Warning : Little Endian]
	for (int i = 0; i < 4; i++) temp[3 - i] = buffer[272 + i];
	empty_block = convertCharToInt(temp);
	// 4 byte (276 - 279)
	for (int i = 0; i < 4; i++) temp[3 - i] = buffer[276 + i];
	first_pointer = convertCharToInt(temp);

	/** Sister Allocation Table (SAT) */
	// 128 KB (SAT_OFFSET - SAT_OFFSET + 128 * 1024 - 1)

	/** Root Directory */
	// 1024 Byte (ROOT_OFFSET - ROOT_OFFSET + 1023)

	printInfo();
}

/** Read Data Pool (block 1 - 65534) */
// NOTE : UNTESTED
char* FileHelper::readDataPool(string filename, int block) {
	char* buffer = (char*) malloc (sizeof(char) * BLOCK_SIZE);
	FILE *file;
	file = fopen(filename.c_str(), "rb");
	fseek(file, POOL_OFFSET + (block - 1) * 1024, SEEK_SET);
	int result = fread(buffer, 1, BLOCK_SIZE, file);
	fclose(file);
	return buffer;
}

/** Update Data Pool (block 1 - 65534) */
// NOTE : UNTESTED
void FileHelper::updateDataPool(string filename, int block, char* data) {
	FILE *file;
	file = fopen(filename.c_str(), "rb+");
	for (int i = 0; i < BLOCK_SIZE; i++) {
		fseek(file, POOL_OFFSET + (block - 1) * 1024 + i, SEEK_SET);
		fputc(data[i], file);
	}
	fclose(file);
}

void FileHelper::parseFileInfo(file_info infos) {
	bool attrs[4];
	for (int i = 0; i < 4; i++) {
		attrs[i] = infos.attribute & (1 << i);
	}
	
	int s, m, h, d, M, y;
	unsigned int tt = convert2CharToInt(infos.hour);
	unsigned int temps = tt;
	temps >>= 5;
	temps <<= 5;
	s = tt-temps;
	temps >>= 11;
	temps <<= 11;
	temps += s;
	m = (tt-temps) >> 5;
	temps = 0 + s + m << 5;
	h = (tt-temps) >> 11;
	
	tt = convert2CharToInt(infos.date);
	temps = tt;
	temps >>= 5;
	temps <<= 5;
	d = tt-temps;
	temps >>= 9;
	temps <<= 9;
	temps += d;
	M = (tt-temps) >> 5;
	temps = 0 + d + M << 5;
	y = 2010 + (tt-temps) >> 9;
}

file_info FileHelper::getDataPool(string filename, int block) {
	file_info infos;
	char* dataread = readDataPool(filename, block);
	
	/* filename */
	for (int i = 0; i < 20; i++) {
		if (dataread[i] != '\0') {
			infos.name[i] = dataread[i];
		} else {
			infos.name[i] = '\0';
			break;
		}
	}
	
	/* attributes */
	/* 0 = readonly, 1 = hidden, 2 = archive, 3 = dir */
	infos.attribute = dataread[21];
	
	/* time */
	infos.hour[1] = dataread[22];
	infos.hour[0] = dataread[23];
	
	/* date */
	infos.date[1] = dataread[24];
	infos.hour[0] = dataread[25];
	
	/* first block */
	unsigned char dread[2];
	dread[1] = dataread[26];
	dread[0] = dataread[27];
	infos.block_pointer = convert2CharToInt(dread);
	
	/* file size */
	unsigned char dreads[4];
	dreads[3] = dataread[28];
	dreads[2] = dataread[29];
	dreads[1] = dataread[30];
	dreads[0] = dataread[31];
	infos.file_size = convertCharToInt(dreads);
	
	return infos;
}

// for debugging purposes
void FileHelper::printInfo() {
	/** Volume Information */
	cout << "Volume Information:" << endl;
	cout << "Volume Name: " << volumeName << endl;
	cout << "Free Block: " << empty_block << " / 65534" << endl;
	cout << "First Empty Index: " << first_pointer << endl;
	/** Sister Allocation Table (SAT) */

	/** Root Directory */

	/** Data Pool */

}
