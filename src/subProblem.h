#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include "TTTD.h"
#include "bpgraph.h"
#include "glo.h"
#include <tcutil.h>
#include <tchdb.h>
#include <sys/time.h>
using namespace std;

int dealWithSubproblem(int total_subproblem_num)
{
	struct timeval t_sub_s, t_sub_e;
	gettimeofday(&t_sub_s, NULL);
		//deal with subproblem
	cout << "dealing with  each sub-metafile(subproblem) ..." << endl;
	int taskNum=0, lineNum=0, flag=0;
	FILE *pFile;
	int fileSize=0, chunkLen=0;
	char *lineNumStr = new char[10];
	char *chunkLenStr = new char[10];
	char *str_num = new char[20];		//string of sub-problem number
	char *record = new char[RECORD_LEN];
	char* md5Str = new char[2*MD5_DIGEST_LENGTH+1];
	char* filePathKey = new char[KEY_LEN];
	char* filePath  ;
	char *tabPtr1,*tabPtr2;
	string subFileName, cmd;
	ifstream in2;
	BipartiteGraph *bg ;	

	TCHDB *hdb = tchdbnew();

	for(taskNum=0; taskNum<total_subproblem_num; ++taskNum){
		cout << "task:" << taskNum << " ";
		hdb = tchdbnew();
		if(!tchdbopen(hdb, "metadata.hdb", HDBOREADER)){
			cerr << "can't open metadata.hdb in dealing with subproblem" << endl;
			return ERR;
		}
		subFileName = "sub-metafile";
		sprintf(str_num,"%d", taskNum);
		subFileName += str_num;
		//get line number of file
		cmd = "cat "+ subFileName + " |wc -l >linenum.tmp";	//output line number to file "linenum.tmp"
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_SUBPROBLEM)
			cout << "cmd: " << cmd << endl;
		system(cmd.c_str());
		if((pFile=fopen("linenum.tmp", "r")) == NULL) {cerr<<"err opening linenum.tmp\n";return ERR;}
		fgets(lineNumStr, 10, pFile);			//get it!
		lineNum = atoi(lineNumStr);
		bg = new BipartiteGraph(lineNum);	
		fclose(pFile);
		//deal with content
		if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_SUBPROBLEM)
			cout << "lineNum:" << lineNum << "   subFileName:" << subFileName << endl;
		in2.open(subFileName.c_str(), ios::in);
		if(!in2){
			cerr << "open file:" << subFileName << "  failed" << endl;
			return ERR;
		}
		in2.getline(record, 200);		//first line
		if(*record == '\0'){
			cout << "no files similar for this cluster!" << endl;
			continue;
		}
		while(*record != '\0'){
			tabPtr1 = strchr(record, '\t');		//point to the first tab
			*tabPtr1 = '\0';			//set first '\t' to '\0'
			++tabPtr1;				//point to filePathKey string
			strcpy(md5Str, record);
			tabPtr2 = strchr(tabPtr1, '\t');	//point to the second tab
			*tabPtr2 = '\0';			//set second '\t' to '\0'
			++tabPtr2;
			strcpy(filePathKey, tabPtr1);
			strcpy(chunkLenStr, tabPtr2);
			chunkLen = atoi(chunkLenStr);
			//get file path from db
			filePath = tchdbget2(hdb, filePathKey);
			//get file size
			if((pFile=fopen(filePath, "r")) == NULL) {cerr<<"err opening file "<<subFileName<<endl;return ERR;}
			fseek(pFile, 0, SEEK_END);
			fileSize = ftell(pFile);
			fclose(pFile);
			if(DEBUG_LEVEL==LEVEL_ALL || DEBUG_LEVEL==LEVEL_SUBPROBLEM)
				cout << "md5:" << md5Str << "\nfilePath:" << filePath << "\nchunkLen:" << chunkLen << endl;
			bg->addEdge(string(filePathKey), fileSize, string(md5Str), chunkLen);		//this step we "new" vertex and point
			in2.getline(record, 200);
			free(filePath);			//release the value memory
		}
		tchdbdel(hdb);
		if((flag=bg->buildFileClusters()) < 0){
			cerr << "build file clusters failed! file is " << subFileName  << endl;
			return ERR;
		}
		if((flag=bg->outputClusters()) < 0){
			cerr << "output file clusters failed! file is " << subFileName  << endl;
			return ERR;
		}
		in2.close();
		delete bg;
	}
	cout << "deal with  each sub-metafile(subproblem) end!" << endl;

	gettimeofday(&t_sub_e, NULL);
	cout << "sub problem use " << (double)(t_sub_e.tv_sec-t_sub_s.tv_sec) + (double)(t_sub_e.tv_usec-t_sub_s.tv_usec)/1000000 << endl;

	delete[] lineNumStr;
	delete[] chunkLenStr;
	delete[] str_num;
	delete[] record;
	delete[] md5Str;
	delete[] filePathKey;
	return OK;
}

#endif
