#ifndef ExecutionPlan_HPP
#define ExecutionPlan_HPP
#include <string>
#include <vector>
#include <iostream>

#include <memory>
#include "cts/parser/SQLParser.hpp"
#include "Database.hpp"
#include "operator/Tablescan.hpp"

using namespace std;


class ExecutionPlan {
private:
	Database * db;
	SQLParser::Result r;
	vector<unique_ptr<Tablescan>> tables;
	vector<Table*> tablePointers;
	vector<unique_ptr<Tablescan>> tableScans;

	vector<pair<SQLParser::RelationAttribute, const Register*>> attributes;

	void scanTables();
	void fillRegister ();
public:
	ExecutionPlan(SQLParser::Result& r, Database& db);
	~ExecutionPlan(){};




	void generateExecutionPlan ();
};


#endif
