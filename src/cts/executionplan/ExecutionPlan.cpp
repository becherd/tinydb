#include "cts/executionplan/ExecutionPlan.hpp"

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
		
		unsigned int i;
		for (i = 0; i < r.relations.size(); i++) {
			tablePointers.push_back(&db->getTable(r.relations.at(i).name));
			if(tablePointers[tablePointers.size()-1]==NULL){
				throw "";			
			}
		}

		for (i = 0; i < tablePointers.size(); i++) {
			Table& mytable=db->getTable(r.relations.at(i).name);

			cout<<"Scan table : "<<r.relations.at(i).name<<endl;

			Tablescan* tab=new Tablescan(mytable);

			unique_ptr<Tablescan> thisTable(tab);

			tableScans.push_back(move(thisTable));
		}
}


/**
 * find the index of a relation via its binding
 * the index of the relation in r.relations eaquals the index of the TableScan in the vector tableScans
 */
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

		unsigned int index = findIndexOfRelation(allAttributes.at(i).relation);

		attributes.push_back(
				{ allAttributes.at(i), tableScans[index]->getOutput(allAttributes.at(i).name) });
	}


}


/**
 * Apply all Selections of the form <attribute>='<constant>' to the joinPlan
 */
void ExecutionPlan::applySelections(){
	
	//replace base relations by resulting tables after selection if there is a selection <attribute>='<constant>'
		cout<<"---Selections:---"<<endl;
		for (unsigned int i = 0; i < r.selections.size(); i++) {
			//find tablescan using the binding
			unsigned int index = 0;
			for (; index < r.relations.size(); index++) {
				if (r.selections.at(i).first.relation.compare(
						r.relations.at(index).binding) == 0) {
					break;
				}
			}

			//apply selections
			const Register* attributeRegister = getRegister(
					r.selections.at(i).first);

			Register* constantRegister = new Register();

			constantRegister->setString(r.selections.at(i).second.value);
			cout<<"Register:  "<<r.selections.at(i).first.getName()<<"="<<r.selections.at(i).second.value<<endl;

			unique_ptr<Selection> select(
					new Selection(move(joinPlan.at(index).second), attributeRegister,
							constantRegister));


			//replace base relation with relations after selection
			joinPlan.at(index).second = move(select);

			if(joinPlan.at(index).second==NULL){
				throw "";
			}
		}
}

void ExecutionPlan::printJoinTree(){
	std::string jt = joinTree;

	std::string joinSymbol=" |><|";
	bool closedBracket =false;
	for(unsigned int i=0; i<jt.size(); i++){


		if((jt[i] == ' ') || (closedBracket && jt[i] != ')')){

			jt.insert(i, joinSymbol);
			i=i+joinSymbol.size();
		}

		if(jt[i] == ')'){
					closedBracket=true;
				}
				else{
					closedBracket=false;
				}

	}

	cout << "---"<<endl;
	cout << "Join tree: " << jt << endl;
}

void ExecutionPlan::applyJoins(){
	vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions;


		printJoinTree();

		while(joinTree.find("(")!=std::string::npos){


		pair<string,string> nextRelationBindings = getNextTwoRelationBindings();


		unique_ptr<Operator> table_left;
		unique_ptr<Operator> table_right;

		for(unsigned int i=0; i<joinPlan.size(); i++){

			if(joinPlan.at(i).first.compare(nextRelationBindings.first)==0){
				if(joinPlan.at(i).second==NULL){
					throw "";
				}
				table_left=move(joinPlan.at(i).second);
				joinPlan.erase(joinPlan.begin()+i);

				if(table_left==NULL){
					throw"";
				}

				i--;
			}
			else if(joinPlan.at(i).first.compare(nextRelationBindings.second)==0){
						table_right=move(joinPlan.at(i).second);
						joinPlan.erase(joinPlan.begin()+i);
						i--;
						if(table_right==NULL){
										throw"";
									}
			}


		}
		
		if(table_left==NULL || table_right==NULL){
			throw "";
		}


		vector<string> bindingsLeftTable = getBindings(nextRelationBindings.first);
		vector<string> bindingsRightTable = getBindings(nextRelationBindings.second);

		cout<<"Bindings Left Table: ";
		for(unsigned int f=0; f<bindingsLeftTable.size(); f++){
		cout << bindingsLeftTable[f]<<" ";
		}
		cout << endl;
		cout<<"Bindings Right Table: ";
			for(unsigned int f=0; f<bindingsRightTable.size(); f++){
			cout << bindingsRightTable[f]<<" ";
			}
		cout << endl;


		//delete the joinConditions vector, we need a new one now
		joinConditions.erase(joinConditions.begin(), joinConditions.end());
		for(unsigned int i=0; i<r.joinConditions.size(); i++){
			string leftBinding=r.joinConditions.at(i).first.relation;
			string rightBinding=r.joinConditions.at(i).second.relation;



			for(unsigned int j=0; j<bindingsLeftTable.size(); j++){
				for(unsigned int k=0; k<bindingsRightTable.size(); k++){
					if(leftBinding.compare(bindingsLeftTable.at(j))==0 && rightBinding.compare(bindingsRightTable.at(k))==0){
									joinConditions.push_back({r.joinConditions.at(i).first, r.joinConditions.at(i).second});
									}
					else if(rightBinding.compare(bindingsLeftTable.at(j))==0 && leftBinding.compare(bindingsRightTable.at(k))==0){
									joinConditions.push_back({r.joinConditions.at(i).second, r.joinConditions.at(i).first});
					}
				}
			}
		}


		if(table_left==NULL || table_right==NULL){
			throw "";
		}


		//apply join conditions on the table
		const Register* joinAttribute1 = getRegister(
							joinConditions.at(0).first);

		const Register* joinAttribute2 = getRegister(
							joinConditions.at(0).second);

		if(table_left==NULL||table_right==NULL)
		{
			throw "";
		}

		cout << "Now join " <<nextRelationBindings.first<< " with " << nextRelationBindings.second << " on ("<< joinConditions.at(0).first.getName()<<"="<<joinConditions.at(0).second.getName();



		unique_ptr<HashJoin> hj(new HashJoin(move(table_left), move(table_right),joinAttribute1,joinAttribute2));


		vector<unique_ptr<Operator>> vec_tmp;
		vec_tmp.push_back(move(hj));

		for(unsigned int i=1; i<joinConditions.size(); i++){//used if more than one joincondition is necessary

			cout << " and "<< joinConditions.at(i).first.getName()<<"="<<joinConditions.at(i).second.getName();

			const Register* joinAttribute1 = getRegister(
							joinConditions.at(i).first);
					const Register* joinAttribute2 = getRegister(
							joinConditions.at(i).second);

			unique_ptr<Selection> s(new Selection(move(vec_tmp[i-1]),joinAttribute1,joinAttribute2));
			vec_tmp.push_back(move(s));
		}
		cout << ")"<<endl;

		string newRelationBinding=generateNewRelationBinding(nextRelationBindings.first, nextRelationBindings.second);

		joinPlan.push_back({newRelationBinding, move(vec_tmp[vec_tmp.size()-1])});

		joinTree=replaceFirst(joinTree, "("+nextRelationBindings.first+" "+nextRelationBindings.second+")", newRelationBinding)+" ";
		joinTree.erase(joinTree.find_last_not_of(" ")+1);



		printJoinTree();

	}
}

/**
 * apply the projection to the JoinPlan
 */
unique_ptr<Projection> ExecutionPlan::applyProjection(){
	vector<const Register*> projectionRegister;
	for (unsigned int i = 0; i < r.projections.size(); i++) {
		projectionRegister.push_back(getRegister(r.projections.at(i)));
	}


	unique_ptr<Projection> projection = unique_ptr<Projection>(
			new Projection(move(joinPlan.at(joinPlan.size() - 1).second), projectionRegister));
	return projection;
}


/**
 * generate execution plan using the tablescans and attributes vector
 */
void ExecutionPlan::generateExecutionPlan() {
	scanTables();

	fillRegister();

	//fill JoinPlan with all relations. Store binding with the corresponding TableScan in this vector.
	for(unsigned int i=0; i<r.relations.size(); i++){
		joinPlan.push_back({r.relations.at(i).binding, move(tableScans[i])});
	}

	applySelections();


	//---------------------------------------------------------
	/*
	 *Now we have all base relations as a pair of binding
	 *and unique_ptr<Operator> with selections applied in the joinPlan vector.
	 *
	 *Now apply the joins depending on the joinTree string
	 */
	applyJoins();

	unique_ptr<Projection> projection = applyProjection();


	//Print result
	cout << "\nResult of the query: "<<endl;
	Printer out(move(projection));

	out.open();
	unsigned int numberOfRows = 0;
	while (out.next()){
		numberOfRows ++;
	}
	 cout << "Returned " << numberOfRows << " rows"<<endl;

	out.close();

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
