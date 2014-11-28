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


void dpSize::dpEntry::toString(){
	cout << "| {";
	for(unsigned int i=0; i<this->relationSet.size(); i++){
		cout << this->relationSet[i];
		if(i<this->relationSet.size()-1){
			cout<<",";
		}
	}
	cout <<"}" << " | " << this->bestTree << " | " << this->cost << " | " << this->size << " |" << endl;
}

/*
template <string*>
inline bool operator<(const string& s1, const string& s2){
	return s1.compare(s2)<0;
}

template <string*>
inline bool operator==(const string& s1, const string& s2){
	return s1.compare(s2)==0;
}
*/

bool compare(string s1, string s2){
	return (s1.compare(s2)<0);
}

template <struct dpSize::dpEntry*>
inline bool operator<(const struct dpSize::dpEntry &e1, const struct dpSize::dpEntry &e2){
	auto iter1 = e1.relationSet.begin();
	auto iter2 = e2.relationSet.begin();

	while(iter1 != e1.relationSet.end() && iter2 != e2.relationSet.end()){
		if(iter1->compare(*iter2)<0){
			return true;
		}
		else if(iter1->compare(*iter2)>0){
			return false;
		}
		++iter1;
		++iter2;
	}
	if(iter2!=e2.relationSet.end()){
		return true;
	}
    return false;
}


template <struct dpSize::dpEntry*>
inline bool operator==(const struct dpSize::dpEntry &e1, const struct dpSize::dpEntry &e2){
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
	//****
	//todo selections
	//****


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
		size = (double) info->getSizeOfRelation(joinInfos::getRelationName(relationBinding, res));

		relationSet = new vector<string>();
		relationSet->push_back(relationBinding);

		e = new dpEntry(*relationSet, relationBinding, 0.0, size);

		dpTable[0]->push_back(e);
	}
}



void dpSize::executeDpSize(){
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
					double cost = (*leftRelation)->cost + (*leftRelation)->cost + size;

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
					cout << "bestTree " << bestTree << endl;
					//fill new dpEntry
					e = new dpEntry(*relationSet, bestTree, cost, size);

					auto cmp = find(dpTable[i]->begin(), dpTable[i]->end(), e);

					if(cmp==dpTable[i]->end()){
						//nothing found with same relations
						dpTable[i]->push_front(e);
					}
					else if((*cmp)-> cost > e->cost){
						//we found a cheaper solution
						dpTable[i]->erase(cmp);
						dpTable[i]->push_front(e);
					}
				}
			}
		}
	}

	printDpTable();
}



//prints the current dpTable
void dpSize::printDpTable(){
	cout << "Relation Set | Best Join Tree | Cost" << endl;

	for(auto &d : dpTable){

		 for (list<dpEntry*>::iterator iter = d->begin(); iter != d->end(); iter++){
		    (**iter).dpSize::dpEntry::toString();
		 }
	}
}



