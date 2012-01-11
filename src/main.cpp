#include "TTTD.h"
#include "bpgraph.h"
#include <boost/pending/disjoint_sets.hpp>
#include <map>
#include <Python.h>
#include "glo.h"
#include "subProblem.h"
#include <tcutil.h>
#include <tchdb.h>
#include <sys/time.h>		//for gettimeofday

namespace fs = boost::filesystem;
using namespace std;

inline
void clearRubbish()
{
	system("rm linenum.tmp metadata sub-metafile* metadata.hdb result.txt 2>/dev/null");
}

typedef bool (*StrCompType)(const string &, const string &);

inline
bool strCompare(const string &s1, const string &s2)
{
	return (s1 < s2);
}

string getFileMD5(const char *path);
int bin_packing(map<int,int> &n_to_s, map<int,int> &n_to_n);		//bin-packing, using first fit
void outputDupFiles();

int main()
{
	struct timeval t_start, t_end;				//for getting running time
	gettimeofday(&t_start, NULL);

	int i = 0,
	    vertexNum = 0,
	    intKey=0;
	vector<SourceFile*> sourceFiles;
	SourceFile* file;
	char *db_key = new char[KEY_LEN];
	char *value = new char[FILE_NAME_LEN];
	map<string, int, StrCompType> fileMd5Map(strCompare);			//key is MD5 of the entire file, value is its key in database
	pair<map<string, int>::iterator, bool> md5MapRet;
	string md5ValueStr;
	clearRubbish();
	TCHDB *hdb = tchdbnew();

	/*
	 *parse directory
	 */
	fs::path myPath(sourceDir);
	fs::recursive_directory_iterator end;
	cout << "parsing directory and creating chunk..." << endl;

	if(!tchdbopen(hdb, "metadata.hdb", HDBOWRITER|HDBOCREAT)){
		cerr << "open database failed!" << endl;
		return ERR;
	}

	//add distinct path to database
	struct timeval t_addPath_s, t_addPath_e;
	gettimeofday(&t_addPath_s, NULL);
	try{
		for(fs::recursive_directory_iterator dir_itr(myPath); dir_itr!=end; ++dir_itr){
			if(fs::is_regular_file(dir_itr->status())){
				if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_PARSING_DIR)
					cout << dir_itr->path().filename() << " @ " << dir_itr->path().parent_path().string()<< "\n" ;
				md5ValueStr = getFileMD5(dir_itr->path().string().c_str()) ;
				cout << "file:" << dir_itr->path().string() << "   md5:" << md5ValueStr << endl;
				md5MapRet = fileMd5Map.insert(pair<string, int>(md5ValueStr, intKey));
				if(md5MapRet.second == true){
					sprintf(db_key, "%d", intKey);
					tchdbput2(hdb, db_key, dir_itr->path().string().c_str());
					++vertexNum;
					++intKey;	
				}else{
					dupFiles.insert( pair<int,string>((md5MapRet.first)->second, (dir_itr->path()).string()) );
				}
			}

		}
	}
	catch(const std::exception& ex){
		cout << ex.what() << std::endl;
		cout << "exit!" << endl;
		return 0;
	}
	gettimeofday(&t_addPath_e, NULL);
	cout << "add path use " << (double)(t_addPath_e.tv_sec-t_addPath_s.tv_sec)+(double)(t_addPath_e.tv_usec-t_addPath_s.tv_usec)/1000000 << endl;

	struct timeval t_crtChunk_s, t_crtChunk_e;
	gettimeofday(&t_crtChunk_s, NULL);
	tchdbiterinit(hdb);
	while((db_key=tchdbiternext2(hdb)) != NULL){
		value = tchdbget2(hdb, db_key);
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_PARSING_DIR)
			cout << "##get from DB## key:" << db_key << "   value:" << value << endl;
		if(value){
			file = new SourceFile(db_key, value);
			if((i=file->createChunk()) < 0){
				cerr << "failed when creating chunk" << endl;
				return ERR;
			}
			if((i=file->createMD5()) < 0){
				cerr << "failed when creating MD5" << endl;
				return ERR;
			}
			free(value);
		}else{
			cerr << "failed to get key:" << db_key << "'s value from database" << endl;
			return ERR;
		}

	}
	free(db_key);
	cout << "parse directory and create chunk  end!\n    -------" << endl;
	if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_DUPFILES){
		cout << "dupFiles'size: " << dupFiles.size() << endl;
		multimap<int,string>::iterator itr = dupFiles.begin();
		for(; itr!=dupFiles.end(); ++itr){
			cout << "key: " << itr->first << "   path: " << itr->second << endl;
		}
	}
	gettimeofday(&t_crtChunk_e, NULL);
	cout << "create chunk use " << (double)(t_crtChunk_e.tv_sec-t_crtChunk_s.tv_sec)+(double)(t_crtChunk_e.tv_usec-t_crtChunk_s.tv_usec)/1000000 << endl;

	//sort the metadata file
	cout << "sorting metadata file ..." << endl;
	system("sort -k 1 -o sort-metadata -t '\t' metadata");
	cout << "sort metadata file end!\n    -------" << endl;

	// remove "single md5" line
	// we can just use " system(python remove_single.py); "
	Py_Initialize();
	char pyFile[] = "remove_single.py";
	FILE *fp = fopen(pyFile, "r");
	if(PyRun_SimpleFile(fp, pyFile) != 0){
		cerr << "run python failed!" << endl;
		return -1;
	}
	fclose(fp);
	Py_Finalize();
//	system("rm sort-metadata");
/*	cout << "removing single md5 line ..." << endl;
	system("uniq -w 32 -u sort-metadata > .del.tmp");
	system("diff sort-metadata .del.tmp | sed -n '/^</s/^< //p' > metadata");
	system("rm .del.tmp sort-metadata");
	cout << "remove single md5 line end!\n    ------" << endl;
*/

	if(ENABLE_FITTING){
		FILE *metafp = fopen("metadata", "r");
		fseek(metafp, 0, SEEK_END);
		int metaf_size = ftell(metafp);
		if(metaf_size < MAX_FILE_SIZE){
			system("cp metadata sub-metafile0");
			tchdbdel(hdb);
			dealWithSubproblem(1);
			gettimeofday(&t_end, NULL);
			cout << "Totally use " << (double)(t_end.tv_sec-t_start.tv_sec)+(double)(t_end.tv_usec-t_start.tv_usec)/1000000 << endl;
			return OK;
		}
		fclose(metafp);
	}


	/*
	 * read metadata file, create disjoint set structure
	 */
	char* record = new char[RECORD_LEN];
	char* md5Str = new char[2*MD5_DIGEST_LENGTH+1];
	strcpy(md5Str, "new");
	char* lastMd5Str = new char[2*MD5_DIGEST_LENGTH+1];			
	strcpy(lastMd5Str, "old");
	char* filePath = new char[KEY_LEN];
	char* lastFilePath = new char[KEY_LEN];
	map<string, int> path_to_vertex;	
	typedef
	  boost::disjoint_sets_with_storage<boost::identity_property_map, boost::identity_property_map,
		       boost::find_with_full_path_compression> ds_type;
	ds_type ds(vertexNum);

	//initialize ds and vertex map
	//TODO: initialize it with metadata file later, or we can rm "single MD5" line and initialize ds simutaneously
	//	also we can make unifind a separate module
	cout << "initializing disjoint set structure and vertex map..." << endl;
	vertexNum = 0;
	tchdbiterinit(hdb);
	while((db_key=tchdbiternext2(hdb)) != NULL){
//		value = tchdbget2(hdb, db_key);
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_PARSING_DIR)
			cout << "##get from DB in DS initializing## key:" << db_key << endl;
//		if(value){

			path_to_vertex.insert(pair<string, int>(string(db_key), vertexNum));
			ds.make_set(vertexNum);
			++vertexNum;
//			free(value);
//		}else{
//			cerr << "failed to get key:" << db_key << "'s value from database" << endl;
//			return ERR;
//		}

	}
	free(db_key);
	cout << "initialize disjoint set structure and vertex map end!\n   ------" << endl;

	struct timeval t_unionSame_s, t_unionSame_e;
	gettimeofday(&t_unionSame_s, NULL);

	//read metadata file, union same line 
	//and build the clusterNum-clusterSize map, prepared for later bin-packing algorithm
	set<int> unionedVertexNum;
	pair<set<int>::iterator, bool> ret;
	map<int, int> clusterNum_to_clusterSize;				//store the cluster size, that is number of records for each cluster
	map<int, int>::iterator n_to_s;
	int eachClusterNum=0, clusterSize=0, vertexNum1=0, vertexNum2=0;
	cout << "unioning same line of metafile ..." << endl;
	int pos = 0;
	char *tabPtr1,*tabPtr2;
	ifstream in("metadata", ios::in);
	if(!in){
		cerr << "open file: metadata failed" << endl;
		return -1;
	}
	in.getline(record, RECORD_LEN);		//first line
	if(*record == '\0'){
		cout << "no files similar" << endl;
		cout << "output identical files... " << endl; 
		outputDupFiles();	
		cout << "end!" << endl;
		return 0;
	}
	tabPtr1 = strchr(record, '\t');		//point to the first tab
	*tabPtr1 = '\0';			//set first '\t' to '\0'
	++tabPtr1;				//point to filePath string
	strcpy(lastMd5Str, record);
	tabPtr2 = strchr(tabPtr1, '\t');	//point to the second tab
	*tabPtr2 = '\0';			//set second '\t' to '\0'
	strcpy(lastFilePath, tabPtr1);
	in.getline(record, RECORD_LEN);		//second line
	tabPtr1 = strchr(record, '\t');		//point to the first tab
	*tabPtr1 = '\0';			//set first '\t' to '\0'
	++tabPtr1;				//point to filePath string
	strcpy(md5Str, record);
	tabPtr2 = strchr(tabPtr1, '\t');	//point to the second tab
	*tabPtr2 = '\0';			//set second '\t' to '\0'
	strcpy(filePath, tabPtr1);
	if(strcmp(md5Str, lastMd5Str) == 0){
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_UNION_SAME){
			cout << "filePath:" << filePath << endl; 
			cout << (path_to_vertex.find(string(filePath)))->second << " " << (path_to_vertex.find(string(lastFilePath)))->second << endl;
		}
		vertexNum1 =  ( path_to_vertex.find(string(filePath)) )->second;
		vertexNum2 =  ( path_to_vertex.find(string(lastFilePath)) )->second;
		ds.union_set(vertexNum1, vertexNum2);
		eachClusterNum = ds.find_set(vertexNum1);
		if( (n_to_s=clusterNum_to_clusterSize.find(eachClusterNum)) ==  clusterNum_to_clusterSize.end())
			clusterNum_to_clusterSize.insert(pair<int,int>(eachClusterNum, 2));
		else {
			++(n_to_s->second);
		}

	}
	in.getline(record, RECORD_LEN);
	while(*record != '\0'){
//		cout << "record:" << record << endl;
		tabPtr1 = strchr(record, '\t');		//point to the first tab
		*tabPtr1 = '\0';			//set first '\t' to '\0'
		++tabPtr1;				//point to filePath string
		strcpy(md5Str, record);
		tabPtr2 = strchr(tabPtr1, '\t');	//point to the second tab
		*tabPtr2 = '\0';			//set second '\t' to '\0'
		strcpy(filePath, tabPtr1);
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_UNION_SAME){
			cout << "md5Str:" << md5Str << endl;
			cout << "filePath:" << filePath << endl;
		}
		if(strcmp(md5Str, lastMd5Str) == 0){
			if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_UNION_SAME){
				cout << "filePath:" << filePath << endl; 
				cout << (path_to_vertex.find(string(filePath)))->second << " " <<(path_to_vertex.find(string(lastFilePath)))->second << endl;
			}
			vertexNum1 =  ( path_to_vertex.find(string(filePath)) )->second;
			vertexNum2 =  ( path_to_vertex.find(string(lastFilePath)) )->second;
			ds.union_set(vertexNum1, vertexNum2);
			eachClusterNum = ds.find_set(vertexNum1);
			if( (n_to_s=clusterNum_to_clusterSize.find(eachClusterNum)) ==  clusterNum_to_clusterSize.end())
				clusterNum_to_clusterSize.insert(pair<int,int>(eachClusterNum, 2));
			else {
				++(n_to_s->second);
			}
		}
		strcpy(lastFilePath, filePath);
		strcpy(lastMd5Str, md5Str);
		in.getline(record, RECORD_LEN);
	}
	cout << "union same line of metafile end!\n   ------" << endl;
	in.close();

	gettimeofday(&t_unionSame_e, NULL);
	cout << "union same line of metadata use " << (double)(t_unionSame_e.tv_sec-t_unionSame_s.tv_sec)+(double)(t_unionSame_e.tv_usec-t_unionSame_s.tv_usec)/1000000 << endl;

	//bin-packing algorithm
	map<int,int> clusterNum_to_subNumber;		//store the plan, each cluster will be output to a subproblem file. subNumber is 0,1,2...
	int total_subproblem_num ;
	total_subproblem_num = bin_packing(clusterNum_to_clusterSize, clusterNum_to_subNumber);
	map<int,int>::iterator number_itr;
/*	for(number_itr=clusterNum_to_subNumber.begin(); number_itr!=clusterNum_to_subNumber.end(); ++number_itr){
		cout << "cluster number:" << number_itr->first << "    custerSize:" << (clusterNum_to_clusterSize.find(number_itr->first))->second  << "    subnumber:" << number_itr->second << endl;
	}
*/

	//output cluster size
	if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_GET_EACH_CLUSTER_SIZE){
		map<int, int>::iterator ntc_itr;
		for(ntc_itr=clusterNum_to_clusterSize.begin(); ntc_itr!=clusterNum_to_clusterSize.end(); ++ntc_itr){
			cout << "clusterNum:" << ntc_itr->first << "    clusterSize:" << ntc_itr->second << endl;
		}

		size_t test[vertexNum], l=0;
		for(; l<vertexNum; ++l){
			test[l] = (size_t)l;
		}
	
		int clusterNumAll = ds.count_sets(test, test+vertexNum);
		cout << "cluster number:" << clusterNumAll << endl;
		cout << "vertex number" << vertexNum << endl;
	}

	/*
	 * create sub-metadata file
	 */
	struct timeval t_crtSub_s, t_crtSub_e;
	gettimeofday(&t_crtSub_s, NULL);

	cout << "creating sub-metadata file ..." << endl;
	map<int, string> subproblemNum_to_path;
	map<int, string>::iterator problem_map_itr;
	map<int, int>::iterator nton_itr;
	int clusterNum = 0;
	char *str_num = new char[20];		//string of sub-problem number
	string subFileName("sub-metafile");
	string cmd("echo '");
	ifstream in1("metadata", ios::in);

	in1.getline(record, RECORD_LEN);
	while(*record != '\0'){
		tabPtr1 = strchr(record, '\t');
		*tabPtr1 = '\0';
		strcpy(md5Str, record);
		tabPtr2 = strchr(tabPtr1+1, '\t');
		*tabPtr2 = '\0';
		*tabPtr1 = '\t';			//restore "record", because later we'll use it
		++tabPtr1;
		strcpy(filePath, tabPtr1);
		*tabPtr2 = '\t';			//restore "record", because later we'll use it
		clusterNum = ds.find_set(path_to_vertex.find(string(filePath))->second);	//find cluster N.O. of some filepath
//		if((problem_map_itr=subproblemNum_to_path.find(clusterNum)) == subproblemNum_to_path.end()){	
		if((nton_itr=clusterNum_to_subNumber.find(clusterNum)) == clusterNum_to_subNumber.end()){
			sprintf( str_num, "%d", 0 );
			cout << "not get it!" << endl;
		}else
			sprintf( str_num, "%d", nton_itr->second );
		subFileName += str_num;
//			subproblemNum_to_path.insert(			//every cluster has a unique sub-metadata file name
//					pair<int, string>(clusterNum, subFileName));
		//using shell cmd to write : "echo ($record) >> ($subFileName)"
		//"echo" add '\n' automatically to the end of record
		cmd += record;
		cmd += "' >> ";
		cmd += subFileName;
		system(cmd.c_str());
/*		}else{
			subFileName = problem_map_itr->second;
			//using shell cmd to write : "echo ($record) >> ($subFileName)"
			//"echo" add '\n' automatically to the end of record
			cmd += record;
			cmd += "' >> ";
			cmd += subFileName;
			system(cmd.c_str());
		}
*/		subFileName = "sub-metafile";
		cmd = "echo '";
		in1.getline(record, RECORD_LEN);
	}
	cout << "create sub-metadata file end!\n   ------" << endl;
	in1.close();
	gettimeofday(&t_crtSub_e, NULL);
	cout << "create sub-metadata file use " << (double)(t_crtSub_e.tv_sec-t_crtSub_s.tv_sec)+(double)(t_crtSub_e.tv_usec-t_crtSub_s.tv_usec)/1000000 << endl;
	
	tchdbdel(hdb);
	if(!dealWithSubproblem(total_subproblem_num)){
		cout << "deal with subproblem failed!\n";
		return ERR;
	}
	outputDupFiles();

	gettimeofday(&t_end, NULL);
	cout << "Totally use " << (double)(t_end.tv_sec-t_start.tv_sec)+(double)(t_end.tv_usec-t_start.tv_usec)/1000000 << endl;
	return OK;
}


string getFileMD5(const char *path)
{
	const int BUF_LEN = 50;
	int fd=0,j=0;
	MD5_CTX c;
	unsigned char *md5Str = new unsigned char[MD5_DIGEST_LENGTH];
	unsigned char *buf = new unsigned char[BUF_LEN];
	string ret = "";
	char *temp = new char[3];
	for(int i=0; i<BUF_LEN; ++i)
		buf[i] = '\0';

	if((fd=open(path, O_RDONLY)) < 0){
		cerr << "failed to open file: " << path << "when getting file MD5" << endl;
		return ERR;
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
	for(int i=0; i<MD5_DIGEST_LENGTH; ++i){
		sprintf(temp, "%02x", md5Str[i]);
		ret += string(temp);
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_FILE_MD5)
			cout << "ret:" << ret << endl;
	}
	delete[] md5Str;
	delete[] buf;
	if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_FILE_MD5)
		cout << "#getMD5#   path:" << path << "  MD5:" << ret << endl;
	close(fd);
	return ret;
}

//bin-packing, using first fit
int bin_packing(map<int,int> &n_to_s, map<int,int> &n_to_n)
{
	map<int,int>::iterator size_itr;
	int count = n_to_s.size();
	int subNumber=0,i=0;
	vector<int> bins( 1, MAX_FILE_SIZE );
/*	
	cout << "n_to_s size:" << n_to_s.size();
	for(size_itr=n_to_s.begin(); size_itr!=n_to_s.end(); ++size_itr){
		cout << size_itr->first << " : " << size_itr->second << endl;
	}
*/
	if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_BIN_PACKING)
		cout << "initial: " << bins.size() << endl;
	for(size_itr=n_to_s.begin(); size_itr!=n_to_s.end(); ++size_itr){
		while(bins[i] < (size_itr->second)*RECORD_SIZE){
			++i;
			if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_BIN_PACKING)
				cout << "in while: " << bins.size() << endl;
			if(i > bins.size()-1){
				bins.push_back(MAX_FILE_SIZE);
				break;
			}
		}
		n_to_n.insert( pair<int,int>(size_itr->first,i) );
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_BIN_PACKING){
			cout << "in for: " << bins.size() << endl;
			if(bins[i] < (size_itr->second)*RECORD_SIZE)
				cout << "bin" << i << ":" << bins[i] << "     clusterSize:" << (size_itr->second)*RECORD_SIZE << endl;
		}
		bins[i] -= (size_itr->second)*RECORD_SIZE;
		i = 0;
	}
	if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_BIN_PACKING){
		for(int i=0; i<bins.size(); ++i)
			cout << "bins" << i << ": " << bins[i] << endl;
		cout << "return: " << bins.size() << endl;
	}
	return bins.size();
	
}

void outputDupFiles()
{
	multimap<int,string>::iterator mmap_itr = dupFiles.begin();
	int count = dupFiles.count(mmap_itr->first);
	int i=0, num=0;
	string cmd;
	char *strNum = new char[10];
/*	for(;mmap_itr!=dupFiles.end(); ++mmap_itr){
		cout << "key:" << mmap_itr->first << "   value:" << mmap_itr->second << endl;
	}
*/	mmap_itr = dupFiles.begin();
	while(mmap_itr != dupFiles.end()){
		sprintf(strNum, "%d", num);
		cmd = "echo dupfiles Cluster#" + string(strNum) + " >>" + resultFileName;
		system(cmd.c_str());
		for(i=0; i<count; ++i){
			cmd = "echo '"	+ mmap_itr->second + "' >>" + resultFileName;
			system(cmd.c_str());
			++mmap_itr;
		}
		if(mmap_itr == dupFiles.end())
			break;
		count = dupFiles.count(mmap_itr->first);
		++num;
	}
	delete[] strNum;
}

