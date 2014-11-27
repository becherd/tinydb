/*
 * joinInfos.hpp
 *
 *  Created on: Nov 27, 2014
 *      Author: becher
 */

#ifndef JOININFOS_HPP_
#define JOININFOS_HPP_

#include <string>
#include "Database.hpp"
#include "cts/parser/SQLParser.hpp"

using namespace std;

/*
 * contains some methods needed for the join order algorithms
 * (compute selectivity of selections/joins, cardinality of relation...)
 */

class joinInfos{
private:
		Database* db;

public:
		joinInfos(Database &db);
		~joinInfos();


		unsigned int getSizeOfRelation(std::string relationName);
		unsigned int getNumberOfDistinctValues(std::string relationName, std::string column);
		double computeSelectivity(std::string relationName, std::string column);
		double computeSelectivity(std::string relationName1, std::string column1, std::string relationName2, std::string column2);
		static string getRelationName(string binding, SQLParser::Result res);

private:


};





#endif /* JOININFOS_HPP_ */
