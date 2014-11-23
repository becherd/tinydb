#ifndef ExecutionPlan_HPP
#define ExecutionPlan_HPP
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <iterator>

#include <memory>
#include "cts/parser/SQLParser.hpp"
#include "Database.hpp"

#include "operator/Tablescan.hpp"
#include "operator/CrossProduct.hpp"
#include "operator/Selection.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Projection.hpp"
#include "operator/Printer.hpp"
#include "operator/Chi.hpp"

using namespace std;


class ExecutionPlan {
private:
	Database * db;
	SQLParser::Result r;
	string joinTree;
	vector<Table*> tablePointers;
	vector<unique_ptr<Tablescan>> tableScans;
	vector<pair<string, unique_ptr<Operator>>> joinPlan;
	vector<pair<SQLParser::RelationAttribute, const Register*>> attributes;

	unsigned int findIndexOfRelation(string binding);
	void scanTables();
	void fillRegister ();
	void applySelections();
	void applyJoins();
	void printJoinTree();
	unique_ptr<Projection> applyProjection();
	const Register* getRegister (SQLParser::RelationAttribute attr);
	pair<string,string> getNextTwoRelationBindings();
	pair<string, string> splitStringAtFirstDelimiter(string s, string delimiter);
	vector<string> getBindings(string bindings);
	string generateNewRelationBinding(string bindingLeft, string bindingRight);
	string replaceFirst(string s, string pattern, string replacement);
	bool vectorContainsAttribute(vector<SQLParser::RelationAttribute> vector,
			SQLParser::RelationAttribute attribute);

public:
	ExecutionPlan(string joinTree, SQLParser::Result& r,Database& db);
	~ExecutionPlan(){};




	void generateExecutionPlan ();
};


#endif
