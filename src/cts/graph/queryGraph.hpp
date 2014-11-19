/*
 * queryGraph.hpp
 *
 *  Created on: Nov 14, 2014
 *      Author: becher
 */
#ifndef QUERYGRAPH_HPP_
#define QUERYGRAPH_HPP_

#include <iostream>
#include <string>
#include <vector>

#include "Graph.hpp"
#include "cts/parser/SQLParser.hpp"
#include "operator/Tablescan.hpp"
#include "Database.hpp"

using namespace std;

class queryGraph{

private:
	Graph* g;
	Database * db;
	SQLParser::Result res;
	public:
		queryGraph(SQLParser::Result &res, Database &db);
		~queryGraph();


		string generateQueryGraph();
	private:
		void addVertices(std::vector<SQLParser::Relation> relations);
		void addEdges(vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions);
		void addSelections(vector<pair<SQLParser::RelationAttribute, SQLParser::Constant>> selections);
		unsigned int getSizeOfRelation(std::string relationName);
		unsigned int getNumberOfDistinctValues(std::string relationName, std::string column);
		double computeSelectivity(std::string relationName, std::string column);
		double computeSelectivity(std::string relationName1, std::string column1, std::string relationName2, std::string column2);
		string getRelationName(string binding);
};



#endif /* QUERYGRAPH_HPP_ */
