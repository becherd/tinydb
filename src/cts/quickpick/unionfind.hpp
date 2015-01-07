#ifndef UNIONFIND_HEADER_GUARD
#define UNIONFIND_HEADER_GUARD

#include <stdexcept>


class unionfind{
	private:
		int length;
		int* array;
		int*  size;	//for union by size
	
	public:
		unionfind(int number);			//creates a new UnionFind-datastructure for elements 0,1,...,number-1
										//this takes approximately number*4 bytes
		

		void do_union(int x, int y);	//unites the set which contains x to the set containing y
										//it uses implicitely the find-Operation!
		
		
		int find(int x);				//returns the unique representative	of the set containing x,
										//while also performing path compression			

		bool only_one_set();

		~unionfind();



};




#endif
