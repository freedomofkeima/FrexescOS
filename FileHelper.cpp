/**
  * FileHelper.cpp File
  * @Frexesc
  *
*/

#include "FileHelper.h"

/** Convert int to char* */
unsigned char* convertIntToChar(int input) {
	unsigned char* bytes = new unsigned char[4];
	
	bytes[0] = (input >> 24) & 0xFF;
	bytes[1] = (input >> 16) & 0xFF;
	bytes[2] = (input >> 8) & 0xFF;
	bytes[3] = input & 0xFF;

	return bytes;
}

/** Convert char* to int */
int convertCharToInt(unsigned char input[4]) {
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

	ofstream new_file(filename.c_str()); // create a new file for filesystem
	writeFile(filename); // write to new filesystem
}

void FileHelper::writeFile(string filename) {
	ofstream file(filename.c_str()); // open file
	unsigned char* temp;
	/** Volume Information */
	char header[1024];
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

	/** Root Directory */

	/** Data Pool */

	file.close();
}

void FileHelper::readFile(string filename) {
	ifstream file(filename.c_str(), std::ios::binary);
	vector<char> buffer((
		std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));
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

	/** Root Directory */

	/** Data Pool */

	printInfo();
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
