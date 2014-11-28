/*
 * dpSize.hpp
 *
 *  Created on: Nov 28, 2014
 *      Author: becher
 */

#ifndef DPSIZE_HPP_
#define DPSIZE_HPP_

#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include "cts/parser/SQLParser.hpp"
#include "cts/joininfos/joinInfos.hpp"
#include "Database.hpp"

using namespace std;
using namespace std::rel_ops;

class dpSize{


private:
	SQLParser::Result res;
	Database * db;
	joinInfos* info;


public:
	dpSize(SQLParser::Result &res, Database &db);
	~dpSize();

	void executeDpSize();

	struct dpEntry{
		vector<string> relationSet;

		string bestTree;

		double cost;

		double size;

		dpEntry(vector<string> relationSet, string bestTree, double cost, double size);
		void toString();
	};


private:

	vector<list<dpSize::dpEntry*>* > dpTable;

	void initDpTable();
	void printDpTable();

};


#endif /* DPSIZE_HPP_ */
