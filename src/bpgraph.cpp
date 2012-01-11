#include "bpgraph.h"
#include <utility>
#include <iostream>
//#include <boost/pending/disjoint_sets.hpp>

using namespace std;

static int simiClusterNum = 1;

multimap<int,string> dupFiles;
//typedef boost::disjoint_sets_with_storage<boost::identity_property_map, boost::identity_property_map, 
//							boost::find_with_full_path_compression> ds_type;
//ds_type ds(count);

BipartiteGraph::BipartiteGraph(int n):ds(n),edges(ncompare),
				      candidate(ncompare),formal(ncompare)
{
//	edges.reserve(AVERAGE_CHUNK_NUM);
	vertexNum = 0;
//	count = n;
}


int BipartiteGraph::addEdge(file_name_t fn, file_len_t fl, 
				chunk_md5_t m, chunk_len_t cl)
{
	FileVertex_t fPtr( new FileVertex());		//shared_ptr
	fPtr->fileNameKey = fn;
	fPtr->fileLen = fl;
	ChunkVertex_t cPtr(  new ChunkVertex() );	//shared_ptr
	cPtr->md5 = m;
	cPtr->chunkLen = cl;
	edges.insert(pair<FileVertex_t, ChunkVertex_t>(fPtr, cPtr));
	candidate.insert(fPtr);
	//++edgeNum;
	return 0;
}

BipartiteGraph::~BipartiteGraph()
{
}

struct msetCompare{
	bool operator() (const ChunkVertex_t &c1, const ChunkVertex_t &c2) const{
		return (c1->md5) < (c2->md5);
	}
}myCompare;

inline
file_len_t BipartiteGraph::testSimilarity(multimap<FileVertex_t, ChunkVertex_t>::iterator m_itr, 
			       set<FileVertex_t>::iterator s_itr_others,			//the candidate file(has at least 1 same chunk)
			       multimap<FileVertex_t, ChunkVertex_t>::size_type num1,
			       multimap<FileVertex_t, ChunkVertex_t>::size_type num2,
			       multimap<FileVertex_t, ChunkVertex_t>::size_type m_index)	//position where m_itr already in
{
	double simiRatio;				//similarity ratio
	file_len_t commonBytes=0;
	multimap<FileVertex_t, ChunkVertex_t>::size_type  i=0, j=0;
	multimap<FileVertex_t, ChunkVertex_t>::iterator m_itr_others, m_itr_others_back;
	multiset<ChunkVertex_t, msetCompare> mset1(myCompare), mset2(myCompare);
	multiset<ChunkVertex_t, msetCompare>::iterator itr1, itr2;
	multimap<FileVertex_t, ChunkVertex_t>::iterator m_itr_back = m_itr;

	m_itr_others_back = m_itr_others = edges.find(*s_itr_others);				//iterate the chunks of the candidate file

/*	
	for(int m=m_index; m<num1; ++m){
		mset1.insert(m_itr->second);
		++m_itr;
	}
	--m_itr;
	for(int n=0; n<num2; ++n){
		mset2.insert(m_itr_others->second);
		++m_itr_others;
	}
	--m_itr_others;

	itr1 = mset1.begin();
	itr2 = mset2.begin();
	 while(num1>0 && num2>0){
	 	if((*itr1)->md5 < (*itr2)->md5){
			++itr1;
			--num1;
		}
		else if	((*itr1)->md5 > (*itr2)->md5){
			++itr2;
			--num2;
		}
		else{
			commonBytes = commonBytes + 2*((*itr1)->chunkLen);
			--num1;
			--num2;
		}
	 }

*/

	while(m_index < num1){
		for(i=0; i<num2; ++i){
			if(m_itr->second->md5 == m_itr_others->second->md5){		
				//assert(m_itr->second->chunkLen == m_itr_others->second->chunkLen);
				commonBytes = commonBytes + 2*(m_itr->second->chunkLen);
//				cout << "chunkLen*2:" << (2*(m_itr->second->chunkLen)) << "  commonBytes:" << commonBytes << endl;
			}
			++m_itr_others;
		}
		++m_itr;
		++m_index;
		m_itr_others = m_itr_others_back;
	}
	--m_itr;
	m_itr_others = m_itr_others_back;
	simiRatio = (double)commonBytes/(double)(m_itr->first->fileLen+m_itr_others->first->fileLen);
	if(DEBUG_LEVEL==LEVEL_SIMILARITY || DEBUG_LEVEL==LEVEL_ALL){
		cout << "commonBytes:" << commonBytes << "	fileLen:" << m_itr->first->fileLen << 
			"	fileLen2:" << m_itr_others->first->fileLen << "     simiRatio:" << simiRatio << endl;
	}
	if(simiRatio > THRESHOLD ){
		return commonBytes;
	}
	return (file_len_t)-1;

}

int BipartiteGraph::buildFFGraph()
{
	typedef multimap<FileVertex_t, ChunkVertex_t>::iterator m_itr_t;
	multimap<FileVertex_t, ChunkVertex_t>::size_type num1, num2, i=0, j=0;
	set<FileVertex_t>::iterator s_itr, 		//iterator of distinct file 
			           s_itr_others;	//it iterates files other than those having been tested by s_itr
	m_itr_t m_itr, m_itr_others;  			//similar to s_itr , but it's for the edges multiset
	file_len_t ret;
	pair<set<FileVertex_t>::iterator, bool> result;
	int vn1, vn2;
	bool matchFlag = false;

	
	for(s_itr=candidate.begin(); s_itr!=candidate.end(); ++s_itr){		//iterate different files
			num1 = edges.count(*s_itr);
			m_itr = edges.find(*s_itr);
			s_itr_others = s_itr;
			++s_itr_others;				//point to the file next to s_itr, we needn't consider files before s_itr
			for(i=0; i<num1; ++i){			//iterate chunks in some file
				while(s_itr_others!=candidate.end()){
					num2 = edges.count(*s_itr_others);
					m_itr_others = edges.find(*s_itr_others);
					for(j=0; j<num2; ++j){			//iterate chunks in some other file, compare with s_itr's file
						if(m_itr->second->md5 == m_itr_others->second->md5){		
						//	assert(m_itr->second->chunkLen == m_itr_others->second->chunkLen);
							if((ret=testSimilarity(m_itr,s_itr_others,num1,num2,i))>0){//from first place where they'er the same
								result = formal.insert(*s_itr);
								if(result.second){
									ds.make_set(vertexNum);
									fileToUnifindVertex.insert(pair<file_name_t,int>(
												(*s_itr)->fileNameKey, vertexNum));
									vn1 = vertexNum;
									++vertexNum;
								}else{	//ds structure already include it
									vn1 = fileToUnifindVertex.find((*s_itr)->fileNameKey)->second;
								}
								result = formal.insert(*s_itr_others);
								if(result.second){
									ds.make_set(vertexNum);
									fileToUnifindVertex.insert(pair<file_name_t,int>(
												(*s_itr_others)->fileNameKey, vertexNum));
									vn2 = vertexNum;
									++vertexNum;
								}else{
									vn2 = fileToUnifindVertex.find((*s_itr_others)->fileNameKey)->second;
								}
								ds.union_set(vn1, vn2);
							}
							matchFlag = true;
							break;
						}
						++m_itr_others;
					}
					++s_itr_others;
				}
				if(matchFlag)
					break;
				s_itr_others = s_itr;		//move s_itr_others to the start place(next to s_itr)
				++s_itr_others;
				++m_itr;
			}
		}
}

int BipartiteGraph::buildFileClusters()
{
	int i=-1, clusterNum;
	file_name_t name;
	if((i=buildFFGraph()) < 0){
		cerr << "build file-file similarity graph failed." << endl;
		return -1;
	}

	set<FileVertex_t>::iterator s_itr;
	for(s_itr=formal.begin(); s_itr!=formal.end(); ++s_itr){
		clusterNum = ds.find_set(
				fileToUnifindVertex.find((*s_itr)->fileNameKey)->second);
		similarFileClusters.insert(pair<int, file_name_t>(clusterNum, (*s_itr)->fileNameKey));
		if(DEBUG_LEVEL==LEVEL_OUTPUT_SIMI_FILE_CLUSTER || DEBUG_LEVEL==LEVEL_ALL)
			cout << "cluster#" << clusterNum << "	" << (*s_itr)->fileNameKey << endl;

	}
	return 0;
}

int BipartiteGraph::outputClusters()
{
	multimap<int, file_name_t>::iterator itr;
	string cmd="echo ";
	char clusterNumStr[10];
	char *realFilePath ;
	int count = 0;		//number of elements in one cluster
	int keyInt = 0;
	multimap<int,string>::iterator dup_itr_beg, dup_itr_end;
	TCHDB *hdb = tchdbnew();

	if(!tchdbopen(hdb, "metadata.hdb", HDBOREADER)){
		cerr << "failed to open metadata.hdb in outputClusters" << endl;
		return ERR;
	}
	cmd = "touch " + resultFileName;
	system(cmd.c_str());
	//echo FILEPATH >> resultFileName
	for(itr=similarFileClusters.begin(); itr!=similarFileClusters.end(); ++itr){
		if(count == 0){
			count = similarFileClusters.count(itr->first);
			sprintf(clusterNumStr, "%d", simiClusterNum);
			cmd = "echo cluster#" + string(clusterNumStr) + " >> " + resultFileName;
			system(cmd.c_str());
			++simiClusterNum;
		}
		keyInt = atoi((itr->second).c_str());
		dup_itr_beg = dupFiles.lower_bound(keyInt);
		dup_itr_end = dupFiles.upper_bound(keyInt);
		while(dup_itr_beg != dup_itr_end){
			cmd = "echo '" + dup_itr_beg->second + "' >> " + resultFileName;
			system(cmd.c_str());
			++dup_itr_beg;
		}
		dupFiles.erase(dup_itr_beg, dup_itr_end);
		realFilePath = tchdbget2(hdb, (itr->second).c_str());
		cmd = "echo '" + string(realFilePath) + "' >> " + resultFileName;
		system(cmd.c_str());
		free(realFilePath);			//release value memory
		--count;
	}
	tchdbdel(hdb);
	return 1;
}

