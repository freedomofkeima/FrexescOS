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
	unsigned int x = 0;
	unsigned char temp[4];
	temp[3] = 0;
	temp[2] = 0;
	temp[1] = input[1];
	temp[0] = input[0];
	x = *(int *) temp;
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
	writeFile(filename, true); // write to new filesystem
	createDummy(); // create a dummy file
}

void FileHelper::writeFile(string filename, bool isNew) {
	FILE *file;
	file = fopen(filename.c_str(), "rb+"); // open file
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
	for (int i = 0; i < 1024; i++) {
		fseek(file, i, SEEK_SET);
		fputc(header[i], file);
	}

	/** Sister Allocation Table (SAT) */
	// 128 KB (SAT_OFFSET - SAT_OFFSET + 128 * 1024 - 1)
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			fseek(file, SAT_OFFSET + i * BLOCK_SIZE + j, SEEK_SET);
			fputc(sat[i].buffer[j], file);
		}
	}

	/** Root Directory */
	// 1024 Byte (ROOT_OFFSET - ROOT_OFFSET + 1023)
	for (int i = 0; i < 32; i++) {
		char temp_info[32];
		for (int j = 0; j < 21; j++) temp_info[j] = root[i].name[j];
		temp_info[21] = root[i].attribute;
		temp_info[22] = root[i].hour[0];
		temp_info[23] = root[i].hour[1];
		temp_info[24] = root[i].date[0];
		temp_info[25] = root[i].date[1];
		
		unsigned char* temp2;
		temp2 = convert2IntToChar(root[i].block_pointer);
		for (int j = 0; j < 2; j++) temp_info[26 + j] = temp2[j];
		temp2 = convertIntToChar(root[i].file_size);
		for (int j = 0; j < 4; j++) temp_info[28 + j] = temp2[j];

		for (int j = 0; j < 32; j++) {
			fseek(file, ROOT_OFFSET + i * 32 + j, SEEK_SET);
			fputc(temp_info[j], file);
		}
	}

	/** Data Pool */
	// 65534 Blocks (POOL_OFFSET - POOL_OFFSET + 65534 * 1024 - 1)
	if (isNew) {
		fseek(file, POOL_OFFSET, SEEK_SET);
		for (int i = 1; i < 65535; i++) {
			for (int j = 0; j < BLOCK_SIZE; j++) {
				fputc('\0', file);
			}
		}
	}

	fclose(file);
}

void FileHelper::readFile(string filename) {
	FILE *file;
	file = fopen(filename.c_str(), "rb");
	char* buffer = (char*) malloc (sizeof(char) * POOL_OFFSET);
	if (buffer == NULL) cout << "Memory Error!" << endl;
	int result = fread(buffer, 1, POOL_OFFSET, file);

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
	for (int i = 0; i < 32; i++) {
		fseek(file, ROOT_OFFSET + i * 32, SEEK_SET);
		char* temp_buffer = (char*) malloc (sizeof(char) * 32);
		if (temp_buffer == NULL) cout << "Memory Error!" << endl;
		result = fread(temp_buffer, 1, 32, file); // read the next 32 bytes
		if (temp_buffer[0] != '\0') {
			/* filename */
			for (int j = 0; j < 21; j++) {
				if (temp_buffer[j] != '\0') {
					root[i].name[j] = temp_buffer[j];
				} else {
					root[i].name[j] = '\0';
					break;
				}
			}
	
			/* attributes */
			/* 0 = readonly, 1 = hidden, 2 = archive, 3 = dir */
			root[i].attribute = temp_buffer[21];
	
			/* time */
			root[i].hour[1] = temp_buffer[22];
			root[i].hour[0] = temp_buffer[23];
	
			/* date */
			root[i].date[1] = temp_buffer[24];
			root[i].date[0] = temp_buffer[25];
	
			/* first block */
			unsigned char dread[2];
			dread[1] = temp_buffer[26];
			dread[0] = temp_buffer[27];
			root[i].block_pointer = convert2CharToInt(dread);
	
			/* file size */
			unsigned char dreads[4];
			dreads[3] = temp_buffer[28];
			dreads[2] = temp_buffer[29];
			dreads[1] = temp_buffer[30];
			dreads[0] = temp_buffer[31];
			root[i].file_size = convertCharToInt(dreads);
		} else {
			for (int j = 0; j < 21; j++) root[i].name[j] = '\0';
			root[i].attribute = '\0';
			root[i].hour[0] = '\0'; root[i].hour[1] = '\0';
			root[i].date[0] = '\0';	root[i].date[1] = '\0';
			root[i].block_pointer = 0;
			root[i].file_size = 0;
		}
	}

	fclose(file);

	printInfo();
	parseFileInfo(getDataPool("sister.fs", 1));
}

/** Read Data Pool (block 1 - 65534) */
char* FileHelper::readDataPool(string filename, int block) {
	char* buffer = (char*) malloc (sizeof(char) * BLOCK_SIZE);
	FILE *file;
	file = fopen(filename.c_str(), "rb");
	fseek(file, POOL_OFFSET + (block - 1) * 1024, SEEK_SET);
	int result = fread(buffer, 1, BLOCK_SIZE, file);
	fclose(file);
	return buffer;
}

/** Update Root Directory (Assumption: filename is not NULL) */
void FileHelper::updateRootDirectory(string filename, char* data) {
	FILE *file;
	file = fopen(filename.c_str(), "rb+");
	for (int i = 0; i < 32; i++) {
		char* temp_buffer = (char*) malloc (sizeof(char) * 1);
		fseek(file, ROOT_OFFSET + i * 32, SEEK_SET);
		int result = fread(temp_buffer, 1, 1, file);
		if (temp_buffer[0] == '\0') {
			for (int j = 0; j < 32; j++) {
				fseek(file, ROOT_OFFSET + i * 32 + j, SEEK_SET);
				fputc(data[j], file);
			}
			break;
		}
	}
	fclose(file);
}

/** Update Data Pool (block 1 - 65534) */
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
	
	unsigned int s = 0, m = 0, h = 0, d = 0, M = 0, y = 0;
	unsigned int tt = convert2CharToInt(infos.hour);
	unsigned int temps = tt;

	temps >>= 5;
	temps <<= 5;
	s = tt-temps;
	temps >>= 11;
	temps <<= 11;
	temps += s;
	m = (tt-temps) >> 5;
	temps = 0 + s + (m << 5);
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
	temps = 0 + d + (M << 5);
	y = 2010 + ((tt-temps) >> 9);
	
	//printf("%u %u %u %u %u %u\n", s, m, h, d, M, y);
	//cout << infos.name << endl;
	//cout << infos.file_size << endl;
}

file_info FileHelper::getDataPool(string filename, int block) {
	file_info infos;
	char* dataread = readDataPool(filename, block);
	
	/* filename */
	for (int i = 0; i < 21; i++) {
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
	infos.date[0] = dataread[25];
	
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

void FileHelper::createDummy() {
	char* datacontent = (char*) malloc (sizeof(char) * BLOCK_SIZE);
	char* data = (char*) malloc (sizeof(char) * BLOCK_SIZE);
	
	memset(datacontent, 0, sizeof(datacontent));
	memset(data, 0, sizeof(data));
	
	int filesize = 1000;
	string filename = "dummy.txt";
	
	/* FILENAME */
	for (int i = 0; i < filename.size(); i++) {
		data[i] = filename[i];
	}
	data[filename.size()] = '\0';
	
	/* ATRIBUT */
	data[21] = 1;
	
	/* GET SYSTEM TIME */
	time_t t;
	t = time(0);
	struct tm *timer;
	timer = localtime(&t);
	
	/* GET JAM */
	int h = timer->tm_hour;
	int m = timer->tm_min;
	int s = timer->tm_sec; s = s/2 + s%2;
	int jam = 0;
	jam += h << 11;
	jam += m << 5;
	jam += s;
	
	/* GET TANGGAL */
	int y = timer->tm_year;	y -= 110;
	int M = timer->tm_mon; M++;
	int d = timer->tm_mday;
	int tanggal = 0;
	tanggal += y << 9;
	tanggal += M << 5;
	tanggal += d;
	
	/* CONVERT JAM, TANGGAL, FIRST BLOCK, FILESIZE MASUKIN KE DATA */
	unsigned char* temp;
	temp = convert2IntToChar(jam);
	data[22] = temp[0];
	data[23] = temp[1];
	temp = convert2IntToChar(tanggal);
	data[24] = temp[0];
	data[25] = temp[1];
	temp = convert2IntToChar(first_pointer+1);
	data[26] = temp[0];
	data[27] = temp[1];
	temp = convertIntToChar(filesize);
	data[28] = temp[0];
	data[29] = temp[1];
	data[30] = temp[2];
	data[31] = temp[3];
	
	/* WRITE TO DATA POOL */
	updateDataPool("sister.fs", first_pointer, data);
	// Update Root Directory
	updateRootDirectory("sister.fs", data);
	
	/* DEC EMPTY_BLOCK BY 1 */
	empty_block--;
	/* MOVE TO NEXT EMPTY */
	first_pointer++;
	
	/* WRITE TO DATA POOL */
	updateDataPool("sister.fs", first_pointer, datacontent);
	/* MOVE TO NEXT EMPTY */
	first_pointer++;
	/* DEC EMPTY_BLOCK BY 1 */
	empty_block--;
	
	/** UPDATE VOLUME INFORMATION */
	readFile("sister.fs");
	writeFile("sister.fs", false);
}
