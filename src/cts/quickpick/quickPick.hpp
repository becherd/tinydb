/*
 * dpSize.hpp
 *
 *  Created on: Nov 28, 2014
 *      Author: becher
 */

#ifndef QUICKPICK_HPP_
#define QUICKPICK_HPP_

#include <vector>
#include <string>
#include <iostream>
#include <numeric>
#include <algorithm>
#include "cts/parser/SQLParser.hpp"
#include "cts/joininfos/joinInfos.hpp"
#include "cts/executionplan/ExecutionPlan.hpp"
#include "Database.hpp"
#include "unionfind.hpp"

using namespace std;

class quickPick{


private:
	SQLParser::Result res;
	Database * db;
	joinInfos* info;
	unionfind *unionFind;
	vector<int> edges;
public:
	quickPick(SQLParser::Result &res, Database &db);
	~quickPick();

	string executeQuickPick(int n=1);
private:
	void init();
	int pickNextEdge();
	int getRelationIndex(SQLParser::RelationAttribute a);
	double getCost(string joinTree);
	void printDistribution(vector<double> costLog);
};


#endif /* QUICKPICK_HPP_ */
