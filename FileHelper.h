/**
  * FileHelper.h File
  * @Frexesc
  *
*/

#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

using namespace std;

#define BLOCK_SIZE 1024

class FileHelper {
	public:
		/** Volume Information */
		string volumeName; // volume name
		int empty_block; // out of 65534
		int first_pointer; // first-free block number
		/** Sister Allocation Table (SAT) */

		/** Root Directory */

		/** Data Pool */

		FileHelper(); // default constructor

		void createNew(string, string); // create a new filesystem
		void writeFile(string); // write to a file
		void readFile(string); // read from file

		void printInfo(); // print info to screen

};

#endif
