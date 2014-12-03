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
#include <iomanip>
#include <algorithm>
#include "cts/parser/SQLParser.hpp"
#include "cts/joininfos/joinInfos.hpp"
#include "Database.hpp"

using namespace std;

class dpSize{


private:
	SQLParser::Result res;
	Database * db;
	joinInfos* info;


public:
	dpSize(SQLParser::Result &res, Database &db);
	~dpSize();

	string executeDpSize();

	struct dpEntry{
		vector<string> relationSet;

		string bestTree;

		double cost;

		double size;

		dpEntry(vector<string> relationSet, string bestTree, double cost, double size);
		string relationSetToString();

	};


private:

	vector<list<dpSize::dpEntry*>* > dpTable;

	bool equalRelationSetsFound(const dpSize::dpEntry &e1, const dpSize::dpEntry &e2);
	double getSizeOfRelationAfterSelection(string binding, SQLParser::Result res);
	void initDpTable();
	void printDpTable();
	vector<unsigned int> getWidthOfColumns(string rs, string bt, string c, string s);
	void printDpTableRow(string rs, string bt, string c, string s, vector<unsigned int> columnWidths);
};


#endif /* DPSIZE_HPP_ */
