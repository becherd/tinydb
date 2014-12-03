/*
 * dpSize.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: becher
 */
#include "dpSize.hpp"


dpSize::dpSize(SQLParser::Result &res, Database &db){
	dpSize::res=res;
	dpSize::db=&db;
	info = new joinInfos(db);
}


dpSize::~dpSize(){
	for(auto &iter : dpTable){
		iter->erase(iter->begin(), iter->end());
	}
	dpTable.erase(dpTable.begin(), dpTable.end());
	//delete (&dpTable);
}


dpSize::dpEntry::dpEntry(vector<string> relationSet, string bestTree, double cost, double size){
	this->relationSet=relationSet;
	this->bestTree=bestTree;
	this->cost = cost;
	this->size=size;
}

//returns a string representation of the relation set of this dpEntry
string dpSize::dpEntry::relationSetToString(){
	string thisRelationSet="{";

	for(unsigned int i=0; i<this->relationSet.size(); i++){
		thisRelationSet = thisRelationSet + this->relationSet[i];
		if(i<this->relationSet.size()-1){
			thisRelationSet = thisRelationSet+",";
		}
	}
	thisRelationSet = thisRelationSet+"}";
	return thisRelationSet;
}







bool compare(string s1, string s2){
	return (s1.compare(s2)<0);
}


bool dpSize::equalRelationSetsFound(const dpSize::dpEntry &e1, const dpSize::dpEntry &e2){

	auto iter1 = e1.relationSet.begin();
	auto iter2 = e2.relationSet.begin();

	while(iter1 != e1.relationSet.end() && iter2 != e2.relationSet.end()){
		if(iter1->compare(*iter2)!=0){
			return false;
		}
		++iter1;
		++iter2;
	}
	if(iter2==e2.relationSet.end() && iter1 == e1.relationSet.end()){
		return true;
	}
    return false;
   }


void dpSize::initDpTable(){

	//initialize empty lists
	for(unsigned int i=0;i<res.relations.size();i++){
		dpTable.push_back(new list<dpEntry*>());
	}


	struct dpEntry *e;
	double size;
	vector<string> *relationSet;
	string relationBinding;

	for(unsigned int i=0; i<res.relations.size(); i++){
		relationBinding = res.relations.at(i).binding;

		size=getSizeOfRelationAfterSelection(relationBinding, res);

		relationSet = new vector<string>();
		relationSet->push_back(relationBinding);

		e = new dpEntry(*relationSet, relationBinding, 0.0, size);

		dpTable[0]->push_back(e);
	}
}

//returns the size after selection
double dpSize::getSizeOfRelationAfterSelection(string binding, SQLParser::Result res){
	string relationName = joinInfos::getRelationName(binding, res);
	double size = (double) info->getSizeOfRelation(relationName);
	for(auto &s : res.selections){
		if(s.first.relation.compare(binding)==0){
			//selection found
			size=size*info->computeSelectivity(relationName, s.first.name);
		}
	}
	return size;
}

string dpSize::executeDpSize(){
	initDpTable();

	list<dpEntry*> leftRelationSet;
	list<dpEntry*> rightRelationSet;

	vector<string>* relationSet;
	struct dpEntry* e;

	for(unsigned int i=1; i<dpTable.size(); i++){
		for(unsigned int j=0; j<i; j++){
			leftRelationSet=*dpTable[j];
			rightRelationSet=*dpTable[(i-j)-1];

			//iterate over both relation sets
			for(auto leftRelation=leftRelationSet.begin();leftRelation!=leftRelationSet.end();leftRelation++){
				for(auto rightRelation=rightRelationSet.begin();rightRelation!=rightRelationSet.end();rightRelation++){

					vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions;

					joinConditions= joinInfos::getJoinConditions((*leftRelation)->relationSet, (*rightRelation)->relationSet, res);

					if(joinConditions.size()==0){
						continue;
					}

					double selectivity = 1.0;
					for(auto &condition : joinConditions){
						string leftRelationName = joinInfos::getRelationName(condition.first.relation, res);
						string rightRelationName = joinInfos::getRelationName(condition.second.relation, res);

						selectivity=selectivity*info->computeSelectivity(leftRelationName, condition.first.name, rightRelationName, condition.second.name);
					}

					double size = selectivity * ((*leftRelation)->size) * ((*rightRelation)->size);
					double cost = (*leftRelation)->cost + (*rightRelation)->cost + size;

					relationSet = new vector<string>();
					for(auto &r : (*leftRelation)->relationSet){
						relationSet->push_back(r);
					}
					for(auto &r : (*rightRelation)->relationSet){
						relationSet->push_back(r);
					}

					std::sort(relationSet->begin(), relationSet->end(), compare);

					//compute new join tree
					string bestTree= (((string("(")+=(*leftRelation)->bestTree)+=string(" "))+=(*rightRelation)->bestTree)+=string(")");

					//fill new dpEntry
					e = new dpEntry(*relationSet, bestTree, cost, size);


					bool found=false;
					list<dpEntry*>::iterator iter;

					for(iter=dpTable[i]->begin(); iter!=dpTable[i]->end(); iter++)
					{
					     found=equalRelationSetsFound(**iter, *e);
					     if(found){
					    	 break;
					     }
					}

					if(!found){
						//nothing found with same relations
						dpTable[i]->push_back(e);
					}
					else if((*iter)->cost > e->cost){
						//we found a cheaper solution
						dpTable[i]->erase(iter);
						dpTable[i]->push_back(e);
					}
				}
			}
		}
	}

	printDpTable();


	list<dpEntry*>::iterator iter = dpTable[dpTable.size()-1]->begin();
	return (**iter).bestTree;
}



//prints the current dpTable
void dpSize::printDpTable(){
	cout << "\nDPTable:"<<endl;

	string hline;

	string rs = "Relation Set";
	string bt = "Best Join Tree";
	string c = "Cost";
	string s = "Size";

	vector<unsigned int> columnWidths = getWidthOfColumns(rs, bt, c, s);
	hline=" "+string(columnWidths[0]+columnWidths[1]+columnWidths[2]+columnWidths[3]+13, '-');

	cout << hline << endl;
	printDpTableRow(rs, bt, c, s, columnWidths);
	cout << hline << endl;


	for(auto &d : dpTable){

		 for (list<dpEntry*>::iterator iter = d->begin(); iter != d->end(); iter++){
			 rs=(**iter).relationSetToString();
			 bt=(**iter).bestTree;
			 c=to_string((**iter).cost);
			 s=to_string((**iter).size);

		     printDpTableRow(rs, bt, c, s, columnWidths);
		 }
	}
	cout << hline << endl;
	cout << endl;
}

//print one row in the dpTable. The requested strings specify the four columns.
void dpSize::printDpTableRow(string rs, string bt, string c, string s,vector<unsigned int> columnWidths){
    const char separator    = ' ';
	cout << " | " << right << setw(columnWidths[0]) << setfill(separator) << rs;
	cout << " | " << right << setw(columnWidths[1]) << setfill(separator) << bt;
	cout << " | " << right << setw(columnWidths[2]) << setfill(separator) << c;
	cout << " | " << right << setw(columnWidths[3]) << setfill(separator) << s;
	cout << " |" << endl;
}


//returns the maximum entry width for each column. Needed to print the columns beautifully aligned.
vector<unsigned int> dpSize::getWidthOfColumns(string rs, string bt, string c, string s){
	vector<unsigned int> columnWidths;

	columnWidths.push_back(rs.size());
	columnWidths.push_back(bt.size());
	columnWidths.push_back(c.size());
	columnWidths.push_back(s.size());

	for(auto &d : dpTable){

			 for (list<dpEntry*>::iterator iter = d->begin(); iter != d->end(); iter++){
				 rs=(**iter).relationSetToString();
				 bt=(**iter).bestTree;
				 c=to_string((**iter).cost);
				 s=to_string((**iter).size);
			    if(rs.size()>columnWidths[0]){
			    	columnWidths[0] = rs.size();
			    }
			    if(bt.size()>columnWidths[1]){
			    	columnWidths[1] = bt.size();
			    }
			    if(c.size()>columnWidths[2]){
			    	columnWidths[2] = c.size();
			    }
			    if(s.size()>columnWidths[3]){
			    	columnWidths[3] = s.size();
			    }
			 }
		}

	return columnWidths;
}


