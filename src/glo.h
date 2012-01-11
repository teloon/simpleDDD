/*
 *Define some global information
 */
#include "debug.h"
#include <string>

#ifndef GLO_H
#define GLO_H

#define DEBUG_ENABLE			//debug switch
#define DEBUG_LEVEL	-1 		//debug level 

#define MAX_FILE_SIZE 512*1024		//that is bin size, byte in unit
//average size of one record, used in bin-packing
//that is, 32byte(MD5) + 2(tab) + 1(\n) + 15(two intString)
#define RECORD_SIZE 50			
#define ERR 0
#define OK 1

using namespace std;

const int DDASH = 40;  
const int D = 80;
const int TMIN = 175;	//minum length of sliding window
const int TMAX = 500;
const int SLIDING_WINDOW_SIZE = 28;	//it is related to getFingerprint(), it should be >=4 and even
const double THRESHOLD = 0.8;
const bool ENABLE_FITTING = false;
const string resultFileName = "result.txt";
const string sourceDir = "/home/wy/work/C000022";

const int FILE_NAME_LEN = 200;
const int KEY_LEN = 20;			//totally we can store 1000,000,000,000,000,000 paths, that's enough :)
const int RECORD_LEN = 300;		//max length of a single line in metadata file

#endif
