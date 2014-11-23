/*
 * queryGraph.cpp
 *
 *  Created on: Nov 14, 2014
 *      Author: becher
 */

#include "queryGraph.hpp"

queryGraph::queryGraph(SQLParser::Result &result, Database &database) {
	g = new Graph();
	queryGraph::db = &database;
	queryGraph::res = result;
}

queryGraph::~queryGraph() {

	delete (g);

}

string queryGraph::generateQueryGraph() {
	cout << "\n\n---------- Query Graph ----------" << endl;

	vector<SQLParser::Relation> relations = res.relations;
	vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions =
			res.joinConditions;
	vector<pair<SQLParser::RelationAttribute, SQLParser::Constant>> selections =
			res.selections;

	addVertices(relations);
	addEdges(joinConditions);
	addSelections(selections);

	g->print_connectivity_components();

	return g->Greedy_operator_ordering();
}

unsigned int queryGraph::getSizeOfRelation(std::string relationName) {
	Table& t = queryGraph::db->getTable(relationName);
	return t.getCardinality();

}

unsigned int queryGraph::getNumberOfDistinctValues(std::string relationName,
		std::string column) {
	Table& t = queryGraph::db->getTable(relationName);

	unsigned int numberOfDistinctValues = t.getAttribute(
			t.findAttribute(column)).getUniqueValues();

	return numberOfDistinctValues;

}

//add vertices to the graph. The names of the vertices are the bindings of the given relations
void queryGraph::addVertices(std::vector<SQLParser::Relation> relations) {
	string name;
	string binding;
	unsigned int cardinality;
	for (unsigned int i = 0; i < relations.size(); i++) {
		name = relations.at(i).name;
		binding = relations.at(i).binding;
		cardinality = getSizeOfRelation(name);
		queryGraph::g->create_new_vertex(cardinality, binding);
	}
}

//add edges to the graph. An edge represents a join condition.
void queryGraph::addEdges(
		vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions) {
	string vertex1;
	string vertex2;
	string name;
	string column1;
	string column2;
	double selectivity;
	for (unsigned int i = 0; i < joinConditions.size(); i++) {
		vertex1 = joinConditions.at(i).first.relation;
		vertex2 = joinConditions.at(i).second.relation;
		column1 = joinConditions.at(i).first.name;
		column2 = joinConditions.at(i).second.name;
		name = joinConditions.at(i).first.getName() + "="
				+ joinConditions.at(i).second.getName();
		selectivity = computeSelectivity(getRelationName(vertex1), column1,
				getRelationName(vertex2), column2);
		queryGraph::g->create_new_edge(vertex1, vertex2, selectivity, name);
	}
}

//add selections to the graph. A selection on relation R is represented by an edge from the node that represents the
//relation to the same node again
void queryGraph::addSelections(
		vector<pair<SQLParser::RelationAttribute, SQLParser::Constant>> selections) {
	string vertex;
	string name;
	string column;
	string relationName;
	double selectivity;
	for (unsigned int i = 0; i < selections.size(); i++) {
		vertex = selections.at(i).first.relation;

		column = selections.at(i).first.name;
		name = selections.at(i).first.getName() + "="
				+ selections.at(i).second.value;
		selectivity = computeSelectivity(getRelationName(vertex), column);
		queryGraph::g->create_new_edge(vertex, vertex, selectivity, name);
	}
}

string queryGraph::getRelationName(string binding) {
	string name = "";
	for (unsigned int i = 0; i < queryGraph::res.relations.size(); i++) {
		if (res.relations.at(i).binding.compare(binding) == 0) {
			name = res.relations.at(i).name;
		}
	}
	return name;
}

//compute the expected selectivity of selection on the given column
double queryGraph::computeSelectivity(std::string relationName,
		std::string column) {

	double numberOfDistinctValues = (double) getNumberOfDistinctValues(
			relationName, column);
	double selectivity = 1.0 / numberOfDistinctValues;

	return selectivity;
}
//compute the expected selectivity of the join between the given relations on the given columns
double queryGraph::computeSelectivity(std::string relationName1,
		std::string column1, std::string relationName2, std::string column2) {
	double numberOfDistinctValues1 = (double) getNumberOfDistinctValues(
			relationName1, column1);
	double numberOfDistinctValues2 = (double) getNumberOfDistinctValues(
			relationName2, column2);
	double selectivity = 1.0
			/ ((double) max(numberOfDistinctValues1, numberOfDistinctValues2));
	return selectivity;
}

