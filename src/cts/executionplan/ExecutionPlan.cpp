#include "cts/executionplan/ExecutionPlan.hpp"
//#include <memory>

using namespace std;



ExecutionPlan::ExecutionPlan(string joinTree, SQLParser::Result& r,Database& db) {
	ExecutionPlan::r = r;
	ExecutionPlan::joinTree=joinTree;
	ExecutionPlan::db = &db;
}




pair<string,string> ExecutionPlan::getNextTwoRelationBindings(){
	unsigned int indexOfLastOpeningBracket=0;
	unsigned int indexOfClosingBracket=0;
	for(unsigned int i= 0; i<joinTree.size(); i++){
		if(joinTree[i]=='('){
			indexOfLastOpeningBracket=i;
		}
		else if(joinTree[i]==')'){
			indexOfClosingBracket=i;
			break;
		}
	}

	string join = joinTree.substr(indexOfLastOpeningBracket+1, indexOfClosingBracket-indexOfLastOpeningBracket-1);

	pair<string, string> relationBindings = splitStringAtFirstDelimiter(join, " ");


	return relationBindings;
}

pair<string, string> ExecutionPlan::splitStringAtFirstDelimiter(string s, string delimiter){
	pair<string, string> p = {s,""};
	if(s.find(delimiter)!=std::string::npos){
	p.first=s.substr(0, s.find(delimiter));
	p.second=s.substr(s.find(' ')+delimiter.size(), s.size()-1);
	}
	return p;
}


void ExecutionPlan::scanTables() {

	vector<SQLParser::Relation> relations = r.relations;
		unsigned int i;
		for (i = 0; i < relations.size(); i++) {
			tablePointers.push_back(&db->getTable(relations.at(i).name));
		}
		cout<<"blubb10"<<endl;

		for (i = 0; i < tablePointers.size(); i++) {
			unique_ptr<Tablescan> thisTable(new Tablescan(*(tablePointers.at(i))));
			tableScans.push_back(move(thisTable));

			joinPlan.push_back({relations.at(i).binding, move(thisTable)});
		}

		cout<<"blubb11"<<endl;

}



unsigned int ExecutionPlan::findIndexOfRelation(string binding){
	vector<SQLParser::Relation> relations = r.relations;
			unsigned int index = 0;
			for (index = 0; index < relations.size(); index++) {
				if (relations.at(index).binding.compare(binding) == 0) {
					break;
				}
			}
			return index;
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
		//find relation via binding
		//index of this relation = index in the tableScans vector
		// then: getoutput(attributname)

		SQLParser::RelationAttribute attribute = allAttributes.at(i);
		vector<SQLParser::Relation> relations = r.relations;
		unsigned int index = findIndexOfRelation(attribute.relation);

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

/*
 * TODO
 * this does not work as joinPlan contains unique_ptr<Operator>
 * maybe use tableScans vector and remember binding-index-relationship somewhere else?
 */

	Printer out(move(joinPlan.at(0).second));

			out.open();
			while (out.next());
			out.close();



	fillRegister();
	//vector<unique_ptr<Operator>> query;

	//copy base relations into query
	//for (unsigned int i = 0; i < tableScans.size(); i++) {
	//	query.push_back(move(tableScans.at(i)));
	//}

	//replace base relations by resulting tables after selection if there is a selection <attribute>='<constant>'
	for (unsigned int i = 0; i < r.selections.size(); i++) {
		//find tablescan using the binding
		unsigned int index = 0;
		for (; index < r.relations.size(); index++) {
			if (r.selections.at(i).first.relation.compare(
					r.relations.at(index).binding) == 0) {
				break;
			}
		}




		unique_ptr<Operator> tableS(move(joinPlan.at(index).second));






		//apply selections
		const Register* attributeRegister = getRegister(
				r.selections.at(i).first);

		Register* constantRegister = new Register();
		constantRegister->setString(r.selections.at(i).second.value);

		unique_ptr<Selection> select(
				new Selection(move(tableS), attributeRegister,
						constantRegister));
		//replace base relation with relations after selection
		joinPlan.at(index).second = move(select);

	}




	vector<unique_ptr<Operator>> query;

	//---------------------------------------------------------
	/*
	 *Now we have all base relations as a pair of binding
	 *and unique_ptr<Operator> with selections applied in the joinPlan vector.
	 *
	 *Now apply the joins depending on the joinTree string
	 */

while(joinTree.find("(")!=std::string::npos){

	pair<string,string> nextRelationBindings = getNextTwoRelationBindings();



	unique_ptr<Operator> table_left;
	unique_ptr<Operator> table_right;



	for(unsigned int i=0; i<joinPlan.size(); i++){
		if(joinPlan.at(i).first.compare(nextRelationBindings.first)==0){
			cout << "binding_left: "<<nextRelationBindings.first<<endl;
			table_left=move(joinPlan.at(i).second);
		}
		else if(joinPlan.at(i).first.compare(nextRelationBindings.second)==0){
					table_right=move(joinPlan.at(i).second);
					cout << "binding_right: "<<nextRelationBindings.second<<endl;

				}
	}






	cout << joinTree << endl;

/*
	unique_ptr<CrossProduct> cp(new CrossProduct(move(table_left),move(table_right)));

	query.push_back(move(cp));
*/

	vector<string> bindingsLeftTable = getBindings(nextRelationBindings.first);
	vector<string> bindingsRightTable = getBindings(nextRelationBindings.second);



	//store all pairs of RelationAttributes which are join conditions for the tables contained in the left and right relation
	//in this vector
	vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions;
	for(unsigned int i=0; i<r.joinConditions.size(); i++){
		string leftBinding=r.joinConditions.at(i).first.relation;
		string rightBinding=r.joinConditions.at(i).second.relation;



		for(unsigned int j=0; j<bindingsLeftTable.size(); j++){
			for(unsigned int k=0; k<bindingsRightTable.size(); k++){
				if((leftBinding.compare(bindingsLeftTable.at(j))==0 && rightBinding.compare(bindingsRightTable.at(j))==0)
						|| (rightBinding.compare(bindingsLeftTable.at(j))==0 && leftBinding.compare(bindingsRightTable.at(j))==0))
				joinConditions.push_back(r.joinConditions.at(i));

			}
		}
	}

cout<<"joinConditions size: "<<joinConditions.size()<<endl;

	//apply join conditions on the table
	for(unsigned int i=0; i<joinConditions.size(); i++){
		const Register* joinAttribute1 = getRegister(
						joinConditions.at(i).first);
				const Register* joinAttribute2 = getRegister(
						joinConditions.at(i).second);

		unique_ptr<Operator> cp(new HashJoin(move(table_left), move(table_right),joinAttribute1,joinAttribute2));
		query.push_back(move(cp));

	}

	cout << "first: "<<nextRelationBindings.first<<", second: "<<nextRelationBindings.second<<endl;
	string newRelationBinding=generateNewRelationBinding(nextRelationBindings.first, nextRelationBindings.second);

	joinPlan.push_back({newRelationBinding, move(query.at(query.size()-1))});

	cout << "newrb: "<<newRelationBinding << endl;

	joinTree=replaceFirst(joinTree, "("+nextRelationBindings.first+" "+nextRelationBindings.second+")", newRelationBinding)+" ";

	cout <<"nachher: "<< joinTree << endl;

}

cout << "Join tree: " << joinTree << endl;


	//do projection
	vector<const Register*> projectionRegister;
	for (unsigned int i = 0; i < r.projections.size(); i++) {
		projectionRegister.push_back(getRegister(r.projections.at(i)));
	}
	unique_ptr<Projection> projection = unique_ptr<Projection>(
			new Projection(move(joinPlan.at(joinPlan.size() - 1).second), projectionRegister));



	//Print result
	/**
	cout << "\nResult of the query: "<<endl;
	Printer out(move(projection));

	out.open();
	while (out.next());
	out.close();
*/
}



//build new relation binding name
//format: _R1_R2, where R1 is the binding of the left Relation, R2 is the binding of the right Relation
string ExecutionPlan::generateNewRelationBinding(string bindingLeft, string bindingRight){
	string newRelationBinding="_"+bindingLeft+"_"+bindingRight;
		//delete double underscores (__->_)
		while (newRelationBinding.find("__")!=std::string::npos){
			newRelationBinding.replace(newRelationBinding.find("__"),2, "_");
		}
		return newRelationBinding;
}



string ExecutionPlan::replaceFirst(string s, string pattern, string replacement){
	if(s.find(pattern)!=std::string::npos){
	int start_pos=s.find(pattern);
	s.replace(start_pos, pattern.size(), replacement);
	}
	return s;
}

//transform bindings string (for example _s1_h1) to a vector (s1,h1)
vector<string> ExecutionPlan::getBindings(string bindings){
	vector<string>bindingsTable;

	if(bindings.find("_",0,1)!=std::string::npos){
		bindings=bindings.substr(1);
	}
	while(bindings.find("_")!=std::string::npos){
			string binding = splitStringAtFirstDelimiter(bindings, "_").first;
			bindingsTable.push_back(binding);
			bindings=bindings.erase(0,binding.size()+1);

	}
	bindingsTable.push_back(bindings);
	return bindingsTable;
}



bool ExecutionPlan::vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector,
		SQLParser::RelationAttribute attribute) {
	bool found = false;
	for (unsigned int i = 0; i < vector.size() && !found; i++) {
		if (vector.at(i).getName() == attribute.getName()) {
			found = true;
		}

	}
	return found;

}

const Register* ExecutionPlan::getRegister(SQLParser::RelationAttribute attr) {
	const Register* reg = NULL;
	for (unsigned int i = 0; i < attributes.size(); i++) {
		if (attributes.at(i).first.getName().compare(attr.getName()) == 0) {
			reg = attributes.at(i).second;
			break;
		}
	}

	if (reg == NULL) {
		throw "Error! Register not found";
	}
	return reg;
}
