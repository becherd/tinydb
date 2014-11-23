#include "Graph.hpp"

Graph::Graph(){
	vertices=new std::vector<Vertex*>;
}


Graph::~Graph(){

	for(auto iter=vertices->begin();iter!=vertices->end();++iter){
		delete(*iter);
	}
	delete vertices;
}


void Graph::create_new_vertex(int size, std::string name){
	Vertex* v=new Vertex(size,name);
	vertices->push_back(v);
}


void Graph::create_new_edge(std::string beginning, std::string end, double weight, std::string edge_name, bool is_directed){
	
	//find beginning and end
	int begin=-1,endd=-1;

	for(unsigned int i=0;i<vertices->size();i++){
		if(vertices->at(i)->name.compare(beginning)==0){
			begin=i;
		}
		if(vertices->at(i)->name.compare(end)==0){
			endd=i;
		}
	}
	if(begin==-1 || endd==-1){
		throw std::invalid_argument("Recieved the name of an edge, that is not in the graph");
	}
	Edge *e;
	e=new Edge(begin,endd,weight, edge_name);
	vertices->at(begin)->add_outgoing_edge(e);
	if(!is_directed && beginning.compare(end)){//if edge is undirected, we have to add also the reverse edge, but not if the edge is a loop
		Edge* f=new Edge(endd,begin,weight, edge_name);
		vertices->at(endd)->add_outgoing_edge(f);
	
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
	outgoings=new std::vector<Edge*>;
}


Vertex::~Vertex(){
	for(auto iter=outgoings->begin();iter!=outgoings->end();++iter){
		delete(*iter);	
	}
	delete (outgoings);
	
}


void Vertex::add_outgoing_edge(Edge* e){
	outgoings->push_back(e);
}


Edge* Graph::from_to(Vertex* from, Vertex* to){
	for(unsigned int i=0;i<from->outgoings->size();i++){
		if(vertices->at(from->outgoings->at(i)->end_id)->name.compare(to->name)==0){
			return from->outgoings->at(i);
		}	
	}
	return NULL;
}


void Graph::DFS_for_component_analysis(){
	std::vector<int> work_stack;//will be "misused" as a stack to store the positions of the vertices in the 'vertices' vector	
	num_components=0;
	for(unsigned int i=0;i<vertices->size();i++){
		vertices->at(i)->component=-2; //-2: not yet used, -1: in the working list, >=0: number of the component of the node
	}
	
	for(unsigned int i=0; i<vertices->size();i++){
		if(vertices->at(i)->component>=0){
			continue;		
		}	
		num_components++;
		//perform a DFS with start at vertices[i]
		work_stack.push_back(i);
		vertices->at(i)->component=-1;
		while(!work_stack.empty()){
			Vertex *v=vertices->at(work_stack.back());
			work_stack.pop_back();
			v->component=num_components-1;
			
			for(unsigned int j=0;j<v->outgoings->size();j++){
				if(vertices->at(v->outgoings->at(j)->end_id)->component==-2){					
					work_stack.push_back(v->outgoings->at(j)->end_id);
					vertices->at(v->outgoings->at(j)->end_id)->component=-1;
				}			
			}
					
		}

	}
}


void Graph::normalize(){
	//combines two edges(e.g. joinconditions) to one
	
	for(unsigned int i=0;i<vertices->size();i++){
		Vertex* v=vertices->at(i);
		for(unsigned int j=0;j<v->outgoings->size();j++){
			Vertex* ending=vertices->at(v->outgoings->at(j)->end_id);
			for(unsigned int k=j+1;k<v->outgoings->size();k++){
				if(ending->name.compare(vertices->at(v->outgoings->at(k)->end_id)->name)==0){
					//two edges from v  can be combined to one
					
					create_new_edge(v->name, vertices->at(v->outgoings->at(j)->end_id)->name,(v->outgoings->at(k)->weight)*(v->outgoings->at(j)->weight), 
							(v->outgoings->at(k)->name_edge)+=std::string(" and ")+=(v->outgoings->at(j)->name_edge),true);
					delete(*(v->outgoings->begin()+k));
					delete(*(v->outgoings->begin()+j));
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
	
	std::vector<Vertex*> printers;
	for(int i=0;i<num_components;i++){
		
		while(!printers.empty()){
			printers.pop_back();		
		}
		std::cout<<std::endl<<std::endl;
		std::cout<<"component number: "<<i<<std::endl;
		
		for(unsigned int j=0;j<vertices->size();j++){ //find all vertices in the component, that should be printed
			if(vertices->at(j)->component==i){
				printers.push_back(vertices->at(j));
			}
		}
		int max_binding_size=9;
		for(unsigned int j=0;j<printers.size();j++){
			max_binding_size=std::max(max_binding_size,(int) (printers[j]->name.size()));
		}
		std::cout<<"bindings"<<std::string(max_binding_size-8,' ')<<"| id | size"<<std::endl;
		for(unsigned int j=0;j<printers.size();j++){
			std::cout<<printers[j]->name<<std::string(max_binding_size-printers[j]->name.size(),' ')<<"| "<<j<<"  | "<<printers[j]->size<<std::endl;
		}
		std::cout<<std::endl;
		
		std::cout<<"adjacency matrix with join_conditions(entries from x to x represent selections), ## means there is no join for the pair of relations"<<std::endl;
				
		int size_column[printers.size()];	//stores the space, that is needed for printing the joinconditions
		int complete_size=printers.size()+7;
		for(unsigned int j=0;j<printers.size();j++){
			size_column[j]=5;
			for(unsigned int k=0;k<printers[j]->outgoings->size();k++){
				size_column[j]=std::max(size_column[j],(int)(printers[j]->outgoings->at(k)->name_edge.size()));
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
				if((e=(void *)from_to(printers[j],printers[k]))==NULL){
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
				if((e=(void *)from_to(printers[j],printers[k]))==NULL){
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
		if((e=(void*)from_to(vertices->at(i),vertices->at(i)))==NULL){
			std::cout<<vertices->at(i)->name<<":  "<<vertices->at(i)->size<<std::endl;
		}
		else{
			std::cout<<vertices->at(i)->name<<":  "<<(int)(vertices->at(i)->size* ((Edge *) e)->weight)<<std::endl;
		}
	}

}

void Graph::push_down_selections(){
	for(unsigned int i=0;i<vertices->size();i++){
		for(unsigned int j=0;j<vertices->at(i)->outgoings->size();j++){
			if(((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)==i){
				vertices->at(i)->size=(vertices->at(i)->size*vertices->at(i)->outgoings->at(j)->weight)+1;

				delete(vertices->at(i)->outgoings->at(j));
				vertices->at(i)->outgoings->erase(vertices->at(i)->outgoings->begin()+j);	
				j--;		
			}
		}
	}
}

void Graph::collapse(unsigned int Vertex1,unsigned int Vertex2){
	
	if(Vertex1>=vertices->size() || Vertex2>=vertices->size()||Vertex1==Vertex2){
		throw std::invalid_argument("bad argument in the collapse method");
	}
	


	
	int size_of_v=(int)((double(vertices->at(Vertex1)->size))*((double)vertices->at(Vertex2)->size)*from_to(vertices->at(Vertex1),vertices->at(Vertex2))->weight)+1;
	std::string	name_of_v=std::string("(")+=vertices->at(Vertex1)->name+=std::string(" ")+=vertices->at(Vertex2)->name+=std::string(")");
	create_new_vertex(size_of_v,name_of_v);
	

	//delete the edge from v1 to v2 and vice versa:
	Edge *e=from_to(vertices->at(Vertex1),vertices->at(Vertex2));
	Edge *f=from_to(vertices->at(Vertex2),vertices->at(Vertex1));
	for(auto iter=vertices->at(Vertex1)->outgoings->begin();iter!=vertices->at(Vertex1)->outgoings->end();++iter){
		if(((unsigned int)(*iter)->end_id)==Vertex2){
			vertices->at(Vertex1)->outgoings->erase(iter);
			break;		
		}	
	}
	for(auto iter=vertices->at(Vertex2)->outgoings->begin();iter!=vertices->at(Vertex2)->outgoings->end();++iter){
		if(((unsigned int)(*iter)->end_id)==Vertex1){
			vertices->at(Vertex2)->outgoings->erase(iter);
			break;		
		}	
	}
	delete(e);
	delete(f);
	
	//adding the edges to the new edge
	for(unsigned int i=0;i<vertices->size()-1;i++){
		for(unsigned int j=0;j<vertices->at(i)->outgoings->size();j++){
			if((i==Vertex1 && ((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)==Vertex2)||
						(i==Vertex2 && ((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)==Vertex1)){
				continue;//Edge between i and j			
			}		
			else if(i==Vertex1 || i==Vertex2){//outgoing Edge from i or j
					create_new_edge(name_of_v,vertices->at(vertices->at(i)->outgoings->at(j)->end_id)->name,
								vertices->at(i)->outgoings->at(j)->weight,vertices->at(i)->outgoings->at(j)->name_edge,true);
			}			
			else if(((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)==Vertex1 || ((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)==Vertex2){
				//Edge to i or j				
				create_new_edge(vertices->at(i)->name, name_of_v,
						vertices->at(i)->outgoings->at(j)->weight,vertices->at(i)->outgoings->at(j)->name_edge,true);	
				delete(vertices->at(i)->outgoings->at(j));	
				vertices->at(i)->outgoings->erase(vertices->at(i)->outgoings->begin()+j);
				j--;		
			}
			
		}
	}

	

	if(Vertex1>Vertex2){
		vertices->erase(vertices->begin()+Vertex1);
		vertices->erase(vertices->begin()+Vertex2);
	}
	else{
		vertices->erase(vertices->begin()+Vertex2);
		vertices->erase(vertices->begin()+Vertex1);
	}
	//adjust the end_id,begin_id of the edges:
	for(unsigned int i=0;i<vertices->size();i++){
		for(unsigned int j=0;j<vertices->at(i)->outgoings->size();j++){
			if(((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)>std::max(Vertex1,Vertex2)){
				vertices->at(i)->outgoings->at(j)->end_id--;			
			}		
			if(((unsigned int)vertices->at(i)->outgoings->at(j)->end_id)>std::min(Vertex1,Vertex2)){
				vertices->at(i)->outgoings->at(j)->end_id--;			
			}
			if(((unsigned int)vertices->at(i)->outgoings->at(j)->begin_id)>std::max(Vertex1,Vertex2)){
				vertices->at(i)->outgoings->at(j)->begin_id--;			
			}		
			if(((unsigned int)vertices->at(i)->outgoings->at(j)->begin_id)>std::min(Vertex1,Vertex2)){
				vertices->at(i)->outgoings->at(j)->begin_id--;			
			}
		}	
	}
	

	
}


Edge* Graph::find_min_edge(){
	
	double min=std::numeric_limits<double>::infinity();
	Edge * minEdge=NULL;
	for(unsigned int i=0;i<vertices->size();i++){
		for(auto edge=vertices->at(i)->outgoings->begin();edge!=vertices->at(i)->outgoings->end();edge++){
			if((*edge)->weight*((double)vertices->at(i)->size)*((double)vertices->at((*edge)->end_id)->size)<min){
				min=(*edge)->weight*((double)vertices->at(i)->size)*((double)vertices->at((*edge)->end_id)->size);
				minEdge=*edge;
			}
		}	
	}
	std::cout << "Select edge from " << vertices->at(minEdge->begin_id)->name << " to " << vertices->at(minEdge->end_id)->name << " with cost " << ((int)min+1) << std::endl;

	return minEdge;
}


std::string Graph::Greedy_operator_ordering(){

	push_down_selections();
	normalize();

	print_connectivity_components();


	std::cout << "\nExecute Greedy Operator Ordering" << std::endl;

	void *e;
	while(vertices->size()>1){
		e=find_min_edge();
		if(e==NULL){
			break;		
		}
		
		collapse(((Edge*) e)->begin_id,((Edge*) e)->end_id);
		normalize();	
	}
	std::cout << std::endl;
	if(vertices->size()==1){
		return vertices->at(0)->name;
	}
	else{
		return std::string("");		
	}
}











