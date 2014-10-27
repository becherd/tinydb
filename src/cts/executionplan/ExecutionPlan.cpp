#include "cts/executionplan/ExecutionPlan.hpp"
//#include <memory>

using namespace std;

bool vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector,
		SQLParser::RelationAttribute attribute);

ExecutionPlan::ExecutionPlan(SQLParser::Result& r, Database& db) {
	ExecutionPlan::r = r;
	ExecutionPlan::db = &db;

}

void ExecutionPlan::scanTables() {

	vector<SQLParser::Relation> relations = r.relations;
	unsigned int i;
	for (i = 0; i < relations.size(); i++) {
		tablePointers.push_back(&db->getTable(relations.at(i).name));
	}

	for (i = 0; i < tablePointers.size(); i++) {

		unique_ptr<Tablescan> thisTable(new Tablescan(*(tablePointers.at(i))));
		tableScans.push_back(move(thisTable));
	}
}

/**
 * fills the attributes in register and stores them in the attributes-vector.
 * this vector will be used for projections, join conditions and selections
 */
void ExecutionPlan::fillRegister() {

	vector<SQLParser::RelationAttribute> allAttributes;


	for (unsigned int i = 0; i < r.projections.size(); i++) {
		SQLParser::RelationAttribute attribute = r.projections.at(i);
		if (!vectorContainsAttribute(allAttributes, attribute)) {
			allAttributes.push_back(attribute);
		}
	}

	for (unsigned int i = 0; i < r.selections.size(); i++) {
		SQLParser::RelationAttribute attribute = r.selections.at(i).first;
		if (!vectorContainsAttribute(allAttributes, attribute)) {
			allAttributes.push_back(attribute);
		}
	}

	for (unsigned int i = 0; i < r.joinConditions.size(); i++) {
		SQLParser::RelationAttribute attribute1 = r.joinConditions.at(i).first;
		SQLParser::RelationAttribute attribute2 = r.joinConditions.at(i).second;
		if (!vectorContainsAttribute(allAttributes, attribute1)) {
			allAttributes.push_back(attribute1);
		}
		if (!vectorContainsAttribute(allAttributes, attribute2)) {
			allAttributes.push_back(attribute2);
		}
	}

	for (unsigned int i = 0; i < allAttributes.size(); i++) {
		//find relation via bining
		//index of this relation = index in the tableScans vector
		// then: getoutput(attributname)

		SQLParser::RelationAttribute attribute = allAttributes.at(i);
		vector<SQLParser::Relation> relations = r.relations;
		unsigned int index = 0;
		for (index = 0; index < relations.size(); index++) {
			if (relations.at(index).binding.compare(attribute.relation)==0) {
				break;
			}
		}
		cout << allAttributes.at(i).getName() << endl;
		attributes.push_back(
				{ allAttributes.at(i), tableScans[index]->getOutput(
						attribute.name) });
	}
}

/**
 * generate execution plan using the tablescans and attributes vector
 */
void ExecutionPlan::generateExecutionPlan() {
	scanTables();
	fillRegister();
	vector<unique_ptr<Operator>> query;

	for(int i=0; i<r.selections.size(); i++){
		//find tablescan
		unsigned int index=0;
		for(; index<r.relations.size(); index++){
			if(r.selections.at(i).first.relation.compare(r.relations.at(index).binding) == 0){
				break;
			}
		}


		unique_ptr<Tablescan> tableS(move(tableScans.at(index)));

		const Register* attributeRegister = getRegister(r.selections.at(i).first);

		Register* constantRegister = new Register();
		constantRegister ->setString(r.selections.at(i).second.value);

		unique_ptr<Selection> select(new Selection(move(tableS),attributeRegister,constantRegister));
		query.push_back(move(select));
		//TODO: replace base relation with relations after selection
	}


	//TODO Joins
}

bool vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector,
		SQLParser::RelationAttribute attribute) {
	bool found = false;
	for (unsigned int i = 0; i < vector.size() && !found; i++) {
		if (vector.at(i).getName() == attribute.getName()) {
			found = true;
		}

	}
	return found;

}

const Register* ExecutionPlan::getRegister (SQLParser::RelationAttribute attr){
	const Register* reg = NULL;
	for(unsigned int i=0; i<attributes.size(); i++){
		if(attributes.at(i).first.getName().compare(attr.getName())){
			reg=attributes.at(i).second;
			break;
		}
	}

	return reg;
}
