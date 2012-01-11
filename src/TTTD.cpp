#include"TTTD.h"

using namespace std;

SourceFile::SourceFile(const char *key, const char *p)
{
	db_key = new char[KEY_LEN];
	strcpy(db_key, key);
	path = new char[FILE_NAME_LEN];
	strcpy(path, p);
}

SourceFile::~SourceFile()
{
	delete[] db_key;
	delete[] path;
}
/*
const char* SourceFile::getFileName()
{
	return (const char*)name;
}
*/
int SourceFile::createChunk()
{
	int i=0,
	    fd=0;			//file descriptor of opened file
	FILE* pFile;			
	unsigned char nextByte[1];	//byte read from file
	list<unsigned char> slidingWindow;
	unsigned int hash;	//hash value of sliding Window
	unsigned long  p=1,		//current position
	      	       l=0,		//last 
		       backupBreak=0,	//possible backup break
		       len=0
	     	       ; 
	if((fd=open(path, O_RDONLY)) < 0){
		cerr << "open file " << path << " failed! Exit!" << endl;
		return -1;
	}

	if((DEBUG_LEVEL==LEVEL_CREATE_CHUNK) || (DEBUG_LEVEL==LEVEL_ALL))
		if(!breakPoints.empty())							//test!
			cout << "OMG!Not Empty!" << endl;
	for(;(i=read(fd,nextByte,1))>0;++p){
		slidingWindow.push_back(nextByte[0]);
		if(len >= SLIDING_WINDOW_SIZE)
			slidingWindow.pop_front();
		else{
			++len;
		}
		if(p-l < TMIN){
			continue;
		}
		if((len%2==1) && (len<SLIDING_WINDOW_SIZE)){
			continue;	//because params of fingerprint algorithm need to be even, so we just ignore first several odd positon
		}
		hash = getFingerprint(slidingWindow);
		if((hash%DDASH) == DDASH-1){
			//possible backup break point
			backupBreak = p;
		}
		if((hash%D) == D-1){
			//find a breakpoint
			backupBreak = 0;
			breakPoints.push_back(p);
			if(DEBUG_LEVEL==LEVEL_CREATE_CHUNK || DEBUG_LEVEL==LEVEL_ALL)
				if(p>10000)							//test!
					cout << "OMG!p:" << p << endl;
			l = p;
			continue;
		}
		if(p-l < TMAX){
			//fail to find a bp(breakpoint)
			//but not exceed max window length
			continue;
		}
		//exceed max window length
		if(backupBreak != 0){
			breakPoints.push_back(backupBreak);
			if(DEBUG_LEVEL==LEVEL_CREATE_CHUNK || DEBUG_LEVEL==LEVEL_ALL)
				if(backupBreak>10000)						//test!
					cout << "OMG!back:" << backupBreak << endl;
			l = backupBreak;
			backupBreak = 0;
		}
		else{	//there's no backup break point
			breakPoints.push_back(p);
			if(DEBUG_LEVEL==LEVEL_CREATE_CHUNK || DEBUG_LEVEL==LEVEL_ALL)
				if(p>100000)							//test!
					cout << "OMG!elsep:" << p << endl;
			l = p;
			backupBreak = 0;
		}
	}
	if(DEBUG_LEVEL==LEVEL_CREATE_CHUNK || DEBUG_LEVEL==LEVEL_ALL){
		vector<unsigned long>::iterator itr = breakPoints.begin();			//test!
		for(i=0;i<breakPoints.size();++i){
			if(*itr > 10000)
				cout << "OMG!Too large!" << endl;
			++itr;
		}
	}
	pFile = fopen(path,"r");
	fseek(pFile, 0, SEEK_END);
	fileSize = (unsigned long)ftell(pFile);
	if(fileSize != p)	//add the end point of file 
		breakPoints.push_back(p);
	if(DEBUG_LEVEL==LEVEL_CREATE_CHUNK || DEBUG_LEVEL==LEVEL_ALL)
		cout << "## filename:" << path << ";  chunk number:" << breakPoints.size() << " ##\n";
	fclose(pFile);
	close(fd);
	return breakPoints.size();

}

unsigned int SourceFile::getFingerprint(list<unsigned char>& window)
{
	//compute 13-bit Robin's Fingerprinting, condition: windex.size() should >=4, and be even
	unsigned int DIVISOR = 0x3fff;
	unsigned int dividend;
	list<unsigned char>::iterator itr = window.begin();

	dividend = ((*itr)<<24) | ((*(++itr))<<16) | ((*(++itr))<<8) | (*(++itr));	//first 4 bytes
	dividend = dividend % DIVISOR;
//	cout << "window.size: " << window.size() << endl;
	++itr;
	while(itr != window.end()){							//next 44 bytes
		dividend = (dividend<<16) | ((*itr)<<8) | (*(++itr));
		++itr;
		dividend = dividend % DIVISOR;
	}
	return dividend;
}

int SourceFile::createMD5()
{
	MD5_CTX c;
	unsigned char md[MD5_DIGEST_LENGTH];
	unsigned char buf[TMAX] = {'\0'};
	char temp[10];
//	FILE* pFile;
	int fd = -1,
	    j = 0,
	    i = 0;
	unsigned long last=0;			//last break point position
	bool lackFlag = false;
	vector<unsigned long>::iterator itr = breakPoints.begin();
	ofstream out("metadata",ios::out|ios::binary|ios::app);

	if((fd=open(path, O_RDONLY)) < 0){
		cerr << "open file:" << path << " error in creating MD5" << endl;
		cerr << "fd:" << fd << endl;
		return -1;
	}
	if(DEBUG_LEVEL==LEVEL_BREAKPOINTS || DEBUG_LEVEL==LEVEL_ALL){
		cout << "All breakpoints: " ;
	}
	while(itr != breakPoints.end()){
		MD5_Init(&c);
		if(DEBUG_LEVEL==LEVEL_BREAKPOINTS || DEBUG_LEVEL==LEVEL_ALL){
			cout << *itr << " ";
		}
		if(lackFlag){
			for(j=0; j<TMAX; ++j)
				buf[j] = '\0';
		}
		i = read(fd, buf, *itr-last);
		if(DEBUG_LEVEL==LEVEL_BREAKPOINTS || DEBUG_LEVEL==LEVEL_ALL){
			cout << "(i=" << i << ") ";
		}
		if(i < 0){									//test!
			cout << "can't read metadata" << endl;
			return -1;
		}else if(i < (*itr-last))
			lackFlag = true;
		MD5_Update(&c, buf, (unsigned long)i);
		MD5_Final(md, &c);
		for(j=0; j<MD5_DIGEST_LENGTH; ++j){
			sprintf(temp, "%02x", md[j]);
			if(DEBUG_LEVEL==LEVEL_CREATE_MD5 || DEBUG_LEVEL==LEVEL_ALL)
				printf("%02x", md[j]);
			out.write(temp, 2);
		}
		if(DEBUG_LEVEL==LEVEL_CREATE_MD5 || DEBUG_LEVEL==LEVEL_ALL)
			cout << endl;
		out.write("\t", 1);
		out.write(db_key, strlen(db_key));		//store pathname's key in db
		out.write("\t", 1);	
		sprintf(temp, "%d", i);				//store chunk length
		out.write(temp,strlen(temp));
		out.write("\n", 1);
		out.flush();
		last = *itr;
		++itr;
	}
	if(DEBUG_LEVEL==LEVEL_BREAKPOINTS || DEBUG_LEVEL==LEVEL_ALL){
		cout << endl;
	}
	out.close();
	close(fd);
	return 0;

}




