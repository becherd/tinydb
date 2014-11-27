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
#include "cts/joininfos/joinInfos.hpp"
#include "Database.hpp"

using namespace std;

class queryGraph{

private:
	Graph* g;
	Database * db;
	SQLParser::Result res;
	joinInfos* info;
	public:
		queryGraph(SQLParser::Result &res, Database &db);
		~queryGraph();


		string generateQueryGraph();
	private:
		void addVertices(std::vector<SQLParser::Relation> relations);
		void addEdges(vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions);
		void addSelections(vector<pair<SQLParser::RelationAttribute, SQLParser::Constant>> selections);
};



#endif /* QUERYGRAPH_HPP_ */
