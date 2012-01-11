#ifndef TTTD_H
#define TTTD_H

//#include <dirent.h>
#include <sys/types.h>				//O_RDONLY..
#include <errno.h>
#include <unistd.h>				//write() read()
#include <iostream>
#include <vector>
#include <list>
#include <fcntl.h>				//open() 
#include <cstdio>
#include <string>
#include <cstdlib>				//malloc()
#include <string.h>
#include <openssl/md5.h>
#include <fstream>
#include "boost/filesystem.hpp"
#include "glo.h"

class SourceFile
{
public:
	SourceFile();
	SourceFile(const char *key, const char *path);
	int createChunk();
	int createMD5();
//	const char* getFileName();
	~SourceFile();
private:
	char* db_key;		//key value of the path record in database
	char* path;
	int fileSize;
	int chunkNum;
	std::vector<unsigned long> breakPoints;
	unsigned int getFingerprint(std::list<unsigned char>& window);
//	void addMetainfo(unsigned char (&md)[MD5_DIGEST_LENGTH]);
};

#endif

