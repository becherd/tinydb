#include <iostream>
#include <fstream>
#include <sstream>
#include "Database.hpp"
#include "cts/parser/SQLLexer.hpp"
#include "cts/parser/SQLParser.hpp"
#include "cts/semana/SemanticAnalysis.hpp"
#include "cts/executionplan/ExecutionPlan.hpp"
#include "cts/graph/queryGraph.hpp"
#include "cts/dp/dpSize.hpp"
#include "cts/quickpick/quickPick.hpp"


using namespace std;

typedef SQLLexer::Token Token;

string getQuery(ifstream& in){
	ostringstream buf;
	string line;
	while (true){
		getline(in, line);
		buf << line;
		if (!in.good()){
			break;
		}
		buf << endl;
	}
	return buf.str();
}


void displayParserResult(SQLParser::Result& result){
	cout << "projections: " << endl;
	for (auto& p: result.projections){
		cout << "  " << p.getName() << " " << endl;
	}

	cout << "relations: " << endl;
	for (auto& r: result.relations){
		cout << "  " << r.getName() << endl;
	}

	cout << "join conditions: ";
	for (auto& join: result.joinConditions){
		cout << "  " << join.first.getName() << "=" << join.second.getName();
	}
	cout << endl;

	cout << "selections: ";
	for (auto& sel: result.selections){
		cout << "  " << sel.first.getName() << "=" << sel.second.value << endl;
	}
	cout << endl;

}

int main(int argc, char* argv[]){
	if (argc < 3){
		cerr << "usage: "<<argv[0]<<" <db> <query file> "<< endl;
		return 1;
	}

	Database db;
	db.open(argv[1]);
	ifstream input(argv[2]);
	string query = getQuery(input);
	input.close();

	SQLLexer lexer(query);
	SQLParser parser(lexer);

	try{
		parser.parse(db);
	}
	catch (runtime_error& e){
		cout << "exception: "<<e.what() << endl;
		return 1;
	}

	SQLParser::Result res = parser.getParserResult();





	displayParserResult(res);
	try {
		SemanticAnalysis semana(db);
		semana.analyze(res);
	}
	catch (SemanticAnalysis::SemanticError& ex){
		cerr << "exception: "<<ex.what() << endl;
		return 1;
	}
	cout << "Semantic Analysis successful" << endl;





	string joinTree;
	/*
	 * uncomment this if you want to use GOO to produce the query graph
	 */
	//queryGraph q(res, db);
	//joinTree= q.generateQueryGraph();

	dpSize dpS(res, db);
	joinTree=dpS.executeDpSize();

	quickPick qP(res,db);
	joinTree=qP.executeQuickPick(100);

	//ExecutionPlan ep(joinTree, res, db);
	//ep.generateExecutionPlan();



	return 0;

}
