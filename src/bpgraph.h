/*
 * This head file defines the structure of bipartite graph
 * then build similar file clusters of the graph
 */
#ifndef BPGRAPH_H
#define BPGRAPH_H
#include <string>
#include <vector>
#include <set>
#include <map>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/shared_ptr.hpp>
#include <tcutil.h>
#include <tchdb.h>
#include <stdlib.h>		//for atoi
#include "glo.h"


typedef std::string file_name_t;
typedef int file_len_t;
typedef std::string chunk_md5_t;
typedef int chunk_len_t;


typedef struct Vertex1{
	file_name_t fileNameKey;
	file_len_t fileLen;	 
}FileVertex;

typedef struct Vertex2{
	chunk_md5_t md5;
	chunk_len_t chunkLen;
}ChunkVertex;

//smart pointer
typedef boost::shared_ptr<FileVertex> FileVertex_t;
typedef boost::shared_ptr<ChunkVertex> ChunkVertex_t;

typedef bool (*CompType)(const FileVertex_t &, const FileVertex_t &) ;

inline
bool ncompare(const FileVertex_t &vPtr1, const FileVertex_t &vPtr2)
{
	return ((vPtr1->fileNameKey) < (vPtr2->fileNameKey));
}

//key(int) is its representing file's key in db, value(string) is this dup file's path name
//we use it find one file's dup files
extern std::multimap<int, std::string> dupFiles;		

class BipartiteGraph
{
	public:
		BipartiteGraph(int);
		int addEdge(file_name_t, file_len_t, chunk_md5_t, chunk_len_t);
		int buildFileClusters();				//build similar file clusters
		int outputClusters();
		~BipartiteGraph();					//remove unused edges in memory
	private:
		int buildFFGraph();
//		int getFileLen(file_name_t);			
		file_len_t testSimilarity(std::multimap<FileVertex_t, ChunkVertex_t>::iterator,
						std::set<FileVertex_t>::iterator ,
						std::multimap<FileVertex_t, ChunkVertex_t>::size_type,
						std::multimap<FileVertex_t, ChunkVertex_t>::size_type,
						std::multimap<FileVertex_t, ChunkVertex_t>::size_type);
		std::multimap<FileVertex_t, ChunkVertex_t, CompType> edges;	
		std::set<FileVertex_t, CompType> candidate;			//all the distinct files	
		std::set<FileVertex_t, CompType> formal;			//used to test if some file is already added;  if not, make_set it	
		std::map<file_name_t, int> fileToUnifindVertex;
		std::multimap<int, file_name_t> similarFileClusters;	//"int" is cluster in unifind, "file_name_t" is files in same cluster
		boost::disjoint_sets_with_storage<boost::identity_property_map, boost::identity_property_map,
						boost::find_with_full_path_compression> ds;
		int vertexNum;	//count the number of vertex in unifind structure
//		int count;
};

#endif
