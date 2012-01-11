#include <string>
#include <iostream.h>
#include <openssl/md5.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

void getFileMD5(const char *path)
{
	const int BUF_LEN = 50;
	int fd=0,j=0;
	MD5_CTX c;
	unsigned char *md5Str = new unsigned char[MD5_DIGEST_LENGTH];
	unsigned char *buf = new unsigned char[BUF_LEN];
	string ret;
	for(int i=0; i<BUF_LEN; ++i)
		buf[i] = '\0';

	if((fd=open(path, O_RDONLY)) < 0){
		cerr << "failed to open file: " << path << " when getting file MD5" << endl;
	}
	MD5_Init(&c);
	while((j=read(fd,buf,BUF_LEN)) > 0){
		MD5_Update(&c, buf, (unsigned long)j);
		if(j < BUF_LEN){
			for(int i=0; i<BUF_LEN; ++i)
				buf[i] = '\0';
		}
	}
	MD5_Final(md5Str, &c);
	for(int i=0; i< MD5_DIGEST_LENGTH; ++i)
		printf("%02x", md5Str[i]);
	cout << endl;
	delete[] md5Str;
	delete[] buf;
}

int main()
{
	char path[200] = {'\0'};
	strcpy(path, "/home/wy/work/专题新闻/HINI流感病毒报道/陈水扁相相关报道(有道)/enws6.txt");
	getFileMD5(path);
	return 0;
}
