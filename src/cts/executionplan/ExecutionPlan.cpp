#include "cts/executionplan/ExecutionPlan.hpp"

using namespace std;

bool vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector, SQLParser::RelationAttribute attribute);

ExecutionPlan::ExecutionPlan(SQLParser::Result& r, Database& db){
	ExecutionPlan::r = r;
	ExecutionPlan::db = &db;
	
}

void ExecutionPlan::scanTables (){

	vector<SQLParser::Relation> relations = r.relations;
	unsigned int i;
	for(i=0; i<relations.size(); i++){
		tablePointers.push_back(&db->getTable(relations.at(i).name));
	}

	for(i=0; i<tablePointers.size(); i++){
			unique_ptr<Tablescan> thisTable = unique_ptr<Tablescan>(new Tablescan(*(tablePointers.at(i))));
			tableScans.push_back(thisTable);
		}
}

void ExecutionPlan::fillRegister(){
	/*
	vector<SQLParser::RelationAttribute> projectionAttributes = ExecutionPlan::r.projections;
	vector<SQLParser::RelationAttribute> selectionAttributes;
	vector<SQLParser::RelationAttribute> joinConditionAttributes;

	for(int i=0; i<ExecutionPlan::r.selections.size(); i++){
		selectionAttributes.push_back(ExecutionPlan::r.selections.at(i).first);
	}

	for(int i=0; i<ExecutionPlan::r.joinConditions.size(); i++){
		joinConditionAttributes.push_back(ExecutionPlan::r.joinConditions.at(i).first);
		joinConditionAttributes.push_back(ExecutionPlan::r.joinConditions.at(i).second);
	}
	*/

	vector<SQLParser::RelationAttribute> allAttributes = r.projections;
		vector<SQLParser::RelationAttribute> selectionAttributes;
		vector<SQLParser::RelationAttribute> joinConditionAttributes;

		for(unsigned int i=0; i<r.projections.size(); i++){
			SQLParser::RelationAttribute attribute = r.projections.at(i);
			if(!vectorContainsAttribute(allAttributes, attribute)){
			allAttributes.push_back(attribute);
				}
		}



		for(unsigned int i=0; i<r.selections.size(); i++){
					SQLParser::RelationAttribute attribute = r.selections.at(i).first;
					if(!vectorContainsAttribute(allAttributes, attribute)){
					allAttributes.push_back(attribute);
						}
				}

		for(unsigned int i=0; i<r.joinConditions.size(); i++){
							SQLParser::RelationAttribute attribute1 = r.joinConditions.at(i).first;
							SQLParser::RelationAttribute attribute2 = r.joinConditions.at(i).second;
							if(!vectorContainsAttribute(allAttributes, attribute1)){
							allAttributes.push_back(attribute1);
								}
							if(!vectorContainsAttribute(allAttributes, attribute2)){
														allAttributes.push_back(attribute2);
								}
						}





	for (unsigned int i=0; i<allAttributes.size(); i++){
		//relation über binding finden
		//index dieser relation = index im tableScans vector
		// dann darauf getoutput(attributname)
		SQLParser::RelationAttribute attribute = allAttributes.at(i);
		vector<SQLParser::Relation> relations = r.relations;
		unsigned int index=0;
		for(index=0; index<relations.size(); index++){
			if(relations.at(index).binding == attribute.relation){
				break;
			}
		}
		cout << allAttributes.at(i).getName() << endl;
		attributes.push_back({allAttributes.at(i), tableScans[index]->getOutput(attribute.name)});
	}


}

bool vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector, SQLParser::RelationAttribute attribute){
	bool found=false;
for(unsigned int i=0; i<vector.size() && !found; i++){
	if(vector.at(i).getName()==attribute.getName()){
		found=true;
	}

}
	return found;


}
