#include "Graph.hpp"

Graph::Graph(){
	vertices=new std::vector<Vertex>;
}


Graph::~Graph(){
	delete vertices;
}


void Graph::create_new_vertex(int size, std::string name){
	Vertex* v=new Vertex(size,name);
	vertices->push_back(*v);
}


void Graph::create_new_edge(std::string beginning, std::string end, double weight, std::string edge_name, bool is_directed){
	
	//find beginning and end
	int begin,endd=-1;

	for(unsigned int i=0;i<vertices->size();i++){
		if((*vertices)[i].name.compare(beginning)==0){
			begin=i;
		}
		if((*vertices)[i].name.compare(end)==0){
			endd=i;
		}
	}
	if(begin==-1 || endd==-1){
		throw std::invalid_argument("Recieved the name of an edge, that is not in the graph");
	}
	Edge *e;
	e=new Edge(begin,endd,weight, edge_name);
	vertices->at(begin).add_outgoing_edge(e);
	if(!is_directed && beginning.compare(end)){//if edge is undirected, we have to add also the reverse edge, but not if the edge is a loop
		Edge* f=new Edge(endd,begin,weight, edge_name);
		vertices->at(endd).add_outgoing_edge(f);
	
	}

}


Edge::Edge(int begin,int end, double w, std::string name){
	weight=w;
	begin_id=begin;
	end_id=end;
	name_edge=name;
}


Vertex::Vertex(int sizee, std::string namee){
	size=sizee;
	name=namee;
	outgoings=new std::vector<Edge>;
}


Vertex::~Vertex(){
}


void Vertex::add_outgoing_edge(Edge* e){
	outgoings->push_back(*e);
}


Edge* Graph::from_to(Vertex* from, Vertex* to){
	for(unsigned int i=0;i<from->outgoings->size();i++){
		if(vertices->at(from->outgoings->at(i).end_id).name.compare(to->name)==0){
			return &(from->outgoings->at(i));
		}	
	}
	return NULL;
}


void Graph::DFS_for_component_analysis(){
	std::vector<int> work_stack;//will be "misused" as a stack to store the positions of the vertices in the 'vertices' vector	
	num_components=0;
	for(unsigned int i=0;i<vertices->size();i++){
		vertices->at(i).component=-2; //-2: not yet used, -1: in the working list, >=0: number of the component of the node
	}
	
	for(unsigned int i=0; i<vertices->size();i++){
		if(vertices->at(i).component>=0){
			continue;		
		}	
		num_components++;
		//perform a DFS with start at vertices[i]
		work_stack.push_back(i);
		vertices->at(i).component=-1;
		while(!work_stack.empty()){
			Vertex *v=&(vertices->at(work_stack.back()));
			work_stack.pop_back();
			v->component=num_components-1;
			
			for(unsigned int j=0;j<v->outgoings->size();j++){
				if(vertices->at(v->outgoings->at(j).end_id).component==-2){					
					work_stack.push_back(v->outgoings->at(j).end_id);
					vertices->at(v->outgoings->at(j).end_id).component=-1;
				}			
			}
					
		}

	}
}


void Graph::normalize(){
	//combines two edges(e.g. joinconditions) to one
	
	for(unsigned int i=0;i<vertices->size();i++){
		Vertex* v=&(vertices->at(i));
		for(unsigned int j=0;j<v->outgoings->size();j++){
			Vertex* ending=&(vertices->at(v->outgoings->at(j).end_id));
			for(unsigned int k=j+1;k<v->outgoings->size();k++){
				if(ending->name.compare(vertices->at(v->outgoings->at(k).end_id).name)==0){
					//two edges from v  can be combined to one
					
					create_new_edge(v->name, vertices->at(v->outgoings->at(j).end_id).name,(v->outgoings->at(k).weight)*(v->outgoings->at(j).weight), 
							(v->outgoings->at(k).name_edge)+=std::string(" and ")+=(v->outgoings->at(j).name_edge),true);
					v->outgoings->erase(v->outgoings->begin()+k);
					v->outgoings->erase(v->outgoings->begin()+j);
					return normalize();//this seems not very efficient, but since multiple join conditions are fairly rare
										//and since we usually have not more than 20-30 edges, the advantages regarding readability are higher
										// than the disadvantages regarding runtime(because otw. you had to change k,j in order to still find every 
										//multi-edge,especially triple-edges are than not easy to find)				
				}
			}		
		}
	}	
}


void Graph::print_connectivity_components(){
	void *e;
	DFS_for_component_analysis();
	normalize();
	
	std::cout<<"number of graph components: "<<num_components<<std::endl;
	
	std::vector<Vertex> printers;
	for(int i=0;i<num_components;i++){
		
		while(!printers.empty()){
			printers.pop_back();		
		}
		std::cout<<std::endl<<std::endl;
		std::cout<<"component number: "<<i<<std::endl;
		
		for(unsigned int j=0;j<vertices->size();j++){ //find all vertices in the component, that should be printed
			if(vertices->at(j).component==i){
				printers.push_back(vertices->at(j));
			}
		}
		int max_binding_size=9;
		for(unsigned int j=0;j<printers.size();j++){
			max_binding_size=std::max(max_binding_size,(int) (printers[j].name.size()));
		}
		std::cout<<"bindings"<<std::string(max_binding_size-8,' ')<<"| id | size"<<std::endl;
		for(unsigned int j=0;j<printers.size();j++){
			std::cout<<printers[j].name<<std::string(max_binding_size-printers[j].name.size(),' ')<<"| "<<j<<"  | "<<printers[j].size<<std::endl;
		}
		std::cout<<std::endl;
		
		std::cout<<"adjacency matrix with join_conditions(entries from x to x represent selections), ## means there is no join for the pair of relations"<<std::endl;
				
		int size_column[printers.size()];	//stores the space, that is needed for printing the joinconditions
		int complete_size=printers.size()+7;
		for(unsigned int j=0;j<printers.size();j++){
			size_column[j]=5;
			for(unsigned int k=0;k<printers[j].outgoings->size();k++){
				size_column[j]=std::max(size_column[j],(int)(printers[j].outgoings->at(k).name_edge.size()));
			}		
			complete_size+=size_column[j];
		}
		
		std::cout<<std::string(complete_size,'-')<<std::endl;
		std::cout<<"|id\\id|"<<std::flush;
		for(unsigned int j=0;j<printers.size();j++){//first line of table
			std::cout<<std::string(size_column[j]-(std::to_string((int)j).size()),' ')<<std::to_string((int)j)<<"|"<<std::flush;
		}
		std::cout<<std::endl;
		std::cout<<std::string(complete_size,'-')<<std::endl;
		
		for(unsigned int j=0;j<printers.size();j++){
			std::cout<<"|"<<std::string(5-(std::to_string((int)j).size()),' ')<<std::to_string((int)j)<<"|"<<std::flush;
			for(unsigned int k=0;k<printers.size();k++){
				if((e=(void *)from_to(&(printers[j]),&(printers[k])))==NULL){
					std::cout<<std::string(size_column[k]-2,' ')<<"##"<<"|"<<std::flush;
				} else{
					std::cout<<std::string(size_column[k]-((Edge *) e)->name_edge.size(),' ')<<std::flush;
					std::cout<<(((Edge*)e)->name_edge)<<std::flush;
					std::cout<<"|"<<std::flush;
				}	
			}
			std::cout<<std::endl;		
			std::cout<<std::string(complete_size,'-')<<std::endl;
		}

		std::cout<<std::endl<<std::endl;


		std::cout<<"adjacency matrix with estimated selectivities (1 means there was no join conditions for the relations given."<<std::endl;
		std::cout<<std::string(16*(printers.size()+1)+1,'-')<<std::endl;
		std::cout<<"|          id\\id|"<<std::flush;
		for(unsigned int j=0;j<printers.size();j++){//first line of table
			std::cout<<std::string(15-(std::to_string((int)j).size()),' ')<<std::to_string((int)j)<<"|"<<std::flush;
		}
		std::cout<<std::endl;
		std::cout<<std::string(16*(printers.size()+1),'-')<<std::endl;
		for(unsigned int j=0;j<printers.size();j++){
			//first the id of the printed node:
			std::cout<<"|"<<std::string(15-(std::to_string((int)j).size()),' ')<<std::to_string((int)j)<<"|"<<std::flush;	
			
			for(unsigned int k=0;k<printers.size();k++){
				if((e=(void *)from_to(&(printers[j]),&(printers[k])))==NULL){
					std::cout<<std::string(14,' ')<<"1"<<"|"<<std::flush;
				} else{
					std::cout<<std::string(5,' ');
					printf("%.8f",((Edge*)e)->weight);
					std::cout<<"|"<<std::flush;
				}							
			}		
			std::cout<<std::endl;
			std::cout<<std::string(16*(printers.size()+1)+1,'-')<<std::endl;
		}
		
			
	}
	std::cout<<std::endl<<std::endl<<"Estimated cardinalities after application of the selections (if possible):"<<std::endl;
	for(unsigned int i=0;i<vertices->size();i++){
		if((e=(void*)from_to(&(vertices->at(i)),&(vertices->at(i))))==NULL){
			std::cout<<vertices->at(i).name<<":  "<<vertices->at(i).size<<std::endl;
		}
		else{
			std::cout<<vertices->at(i).name<<":  "<<(int)(vertices->at(i).size* ((Edge *) e)->weight)<<std::endl;
		}
	}

}



