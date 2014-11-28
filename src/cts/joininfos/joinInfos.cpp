/*
 * joinInfos.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: becher
 */

#include "joinInfos.hpp"


joinInfos::joinInfos(Database &db){
	joinInfos::db = &db;
}

joinInfos::~joinInfos() {
}


unsigned int joinInfos::getSizeOfRelation(std::string relationName) {
	Table& t = joinInfos::db->getTable(relationName);
	return t.getCardinality();
}

unsigned int joinInfos::getNumberOfDistinctValues(std::string relationName,
		std::string column) {
	Table& t = joinInfos::db->getTable(relationName);

	unsigned int numberOfDistinctValues = t.getAttribute(
			t.findAttribute(column)).getUniqueValues();

	return numberOfDistinctValues;
}


string joinInfos::getRelationName(string binding, SQLParser::Result res) {
	string name = "";
	for (unsigned int i = 0; i < res.relations.size(); i++) {
		if (res.relations.at(i).binding.compare(binding) == 0) {
			name = res.relations.at(i).name;
		}
	}
	return name;
}


//compute the expected selectivity of selection on the given column
double joinInfos::computeSelectivity(std::string relationName,
		std::string column) {

	double numberOfDistinctValues = (double) getNumberOfDistinctValues(
			relationName, column);
	double selectivity = 1.0 / numberOfDistinctValues;

	return selectivity;
}
//compute the expected selectivity of the join between the given relations on the given columns
double joinInfos::computeSelectivity(std::string relationName1,
		std::string column1, std::string relationName2, std::string column2) {
	double numberOfDistinctValues1 = (double) getNumberOfDistinctValues(
			relationName1, column1);
	double numberOfDistinctValues2 = (double) getNumberOfDistinctValues(
			relationName2, column2);
	double selectivity = 1.0
			/ ((double) max(numberOfDistinctValues1, numberOfDistinctValues2));
	return selectivity;
}


//returns a list of join conditions between the given relation lists
//if bindingsLeft and bindingsRight are not disjunct, return empty vector
vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinInfos::getJoinConditions(vector<string> bindingsLeft, vector<string> bindingsRight, SQLParser::Result res){

	vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions;

	for(unsigned int i=0; i<res.joinConditions.size(); i++){
				string leftBinding=res.joinConditions.at(i).first.relation;
				string rightBinding=res.joinConditions.at(i).second.relation;

				for(unsigned int j=0; j<bindingsLeft.size(); j++){
					for(unsigned int k=0; k<bindingsRight.size(); k++){
						if(bindingsLeft.at(j).compare(bindingsRight.at(k))==0){
							vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> *empty;
							empty= new vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>>();
							return *empty;
						}
						if(leftBinding.compare(bindingsLeft.at(j))==0 && rightBinding.compare(bindingsRight.at(k))==0){
										joinConditions.push_back({res.joinConditions.at(i).first, res.joinConditions.at(i).second});
										}
						else if(rightBinding.compare(bindingsLeft.at(j))==0 && leftBinding.compare(bindingsRight.at(k))==0){
										joinConditions.push_back({res.joinConditions.at(i).second, res.joinConditions.at(i).first});
						}
					}
				}
			}


	return joinConditions;
}


