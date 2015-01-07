#include "unionfind.hpp"


unionfind::unionfind(int number){

	length=number;

	if(number<0){
		throw std::invalid_argument("received negative number in constructor");
	}

	array= (int*) malloc(number*sizeof(int));
	size = (int*) malloc(number*sizeof(int));
	
	if(array==NULL||size==NULL){
		
		free(array);	//just in case one of them is not NULL
		free(size);
		throw std::bad_alloc();
	}

	for(int i=0;i<number;i++){
		size[i]=1;
		array[i]=i;	//producing sets of size one initially
	}

}


void unionfind::do_union(int x, int y){
	
	if(x<0|| y<0 ||x>=length || y>=length){
		throw std::invalid_argument("bad argument in do_union function");
	}
	
	int xx=find(x);
	int yy=find(y);
	
	if(size[xx]<size[yy]){
		array[xx]=yy;
		size[yy]+=size[xx];
	}
	else{
		array[yy]=xx;
		size[xx]+=size[yy];
	}
}

bool unionfind::only_one_set(){
	if(!length)
		return true;
	return size[find(0)]==length;
}

int unionfind::find(int x){
	
	if(x<0 || x>=length){
		throw std::invalid_argument("bad argument in find function");
	}

	if(array[x]==x){
		return x;
	}
	
	else{
		array[x]=find(array[x]);	
		return array[x];
	}
}

unionfind::~unionfind(){
	free(size);
	free(array);
}


