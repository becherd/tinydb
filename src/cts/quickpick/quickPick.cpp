#include "quickPick.hpp"


quickPick::quickPick(SQLParser::Result &res, Database &db){
	quickPick::res=res;
	quickPick::db=&db;
	info = new joinInfos(db);
	unionFind = new unionfind(res.relations.size());
}


quickPick::~quickPick(){
}


void quickPick::init(){
	unionFind = new unionfind(res.relations.size());

	//store all joinCondition indices in edges
	edges.erase(edges.begin(), edges.end());
	for(unsigned int i=0; i<res.joinConditions.size(); i++){
		edges.push_back(i);
	}
}

/*
 * returns a random join tree. JoinConditions and selections will be reconstructed later
 */
string quickPick::executeQuickPick(int n){
	string bestJoinTree="";
	string joinTree;
	srand(time(NULL));
	double bestCost=std::numeric_limits<double>::infinity();
	vector<double> costLog;

	for(int t=0; t<n; t++){

	init();

	//store intermediate join trees here
	string joinTrees[res.relations.size()];
	for(unsigned int i=0; i<res.relations.size(); i++){
		joinTrees[i] = res.relations.at(i).binding;
	}

	int index;
	int indexLeftRelation;
	int indexRightRelation;
	while(!unionFind->only_one_set()){
		index=pickNextEdge();

		indexLeftRelation = getRelationIndex(res.joinConditions.at(index).first);
		indexRightRelation = getRelationIndex(res.joinConditions.at(index).second);
		int l = unionFind->find(indexLeftRelation);
		int r = unionFind->find(indexRightRelation);
		if(l != r){
			//The relations which are connected by the picked edge are in distinct sets, thus union the two sets
			unionFind->do_union(indexLeftRelation, indexRightRelation);
			joinTrees[unionFind->find(indexLeftRelation)] = (((string("(")+=joinTrees[l])+=string(" "))+=joinTrees[r])+=string(")");
		}
	}

	joinTree=joinTrees[unionFind->find(0)];
	double cost = getCost(joinTree);

	//store the cost into this vector for distribution analysis
	costLog.push_back(cost);
	if(cost < bestCost){
		//keep best join tree and cost
		bestJoinTree = joinTree;
		bestCost = cost;
	}

	}

	printDistribution(costLog);
	cout << "The best join tree is " << bestJoinTree << endl;
	cout << "Cost of this join tree: " << (int)bestCost <<"\n"<< endl;

	delete(unionFind);

	return bestJoinTree;
}

void quickPick::printDistribution(vector<double> costLog){
	cout << "QuickPick" << endl;
	cout << "Cost distribution: " << endl;
	vector<int> costLogInteger;

	for(unsigned int i=0; i<costLog.size(); i++){
		costLogInteger.push_back((int)costLog.at(i));
	}

	sort(costLogInteger.begin(), costLogInteger.end());

	int count =1;
	int v=costLogInteger.at(0);
	for(unsigned int i=1; i<costLogInteger.size(); i++){
		if(v==costLogInteger.at(i)){
			count++;
		}
		else{
			cout << "Cost " << v << " appears " << count << " times" << endl;
			v = costLogInteger.at(i);
			count=1;
		}
	}
	cout << "Cost " << v << " appears " << count << " times\n" << endl;
}

/*
 * returns the index is the index of a edge in res.joinConditions. This edge has never been chosen before and was picked at uniform random
 */
int quickPick::pickNextEdge(){

	int chosenIndex=rand() % edges.size();
	int chosenEdge=edges.at(chosenIndex);
	edges.erase(edges.begin()+chosenIndex);
	return chosenEdge;
}

/*
 * returns the index of the relation in res.relations
 */
int quickPick::getRelationIndex(SQLParser::RelationAttribute a){
	unsigned int i=0;
	for(; i<res.relations.size(); i++){
		if(res.relations.at(i).binding.compare(a.relation)==0){
			break;
		}
	}
	return i;
}

/*
 * returns the cost of the given join tree
 */
double quickPick::getCost(string joinTree){
	// {binding, {cost, size}}
	vector<pair<string, pair<double, double>>> costVector;

	double relationSize;

	for(unsigned int i=0; i<res.relations.size(); i++){
		relationSize = info->getSizeOfRelation(res.relations.at(i).name);
		for(unsigned int j=0; j<res.selections.size(); j++){
			if(res.selections.at(j).first.relation.compare(res.relations.at(i).binding) == 0){
				relationSize=relationSize*info->computeSelectivity(res.relations.at(i).name, res.selections.at(j).first.name);
			}
		}
		costVector.push_back({res.relations.at(i).binding, {0.0, relationSize}});
	}


	for(unsigned r=0; r<res.relations.size()-1; r++){

	pair<string, string> nextTwoRelationBindings = ExecutionPlan::getNextTwoRelationBindings(joinTree);


	double costLeft=-1.0;
	double costRight=-1.0;
	double sizeLeft=-1.0;
	double sizeRight=-1.0;
	for(unsigned int i=0; i<costVector.size(); i++){
		if(costVector.at(i).first.compare(nextTwoRelationBindings.first) == 0){
			costLeft=costVector.at(i).second.first;
			sizeLeft=costVector.at(i).second.second;
		}
		else if(costVector.at(i).first.compare(nextTwoRelationBindings.second) == 0){
			costRight=costVector.at(i).second.first;
			sizeRight=costVector.at(i).second.second;
		}
	}


	vector<pair<SQLParser::RelationAttribute, SQLParser::RelationAttribute>> joinConditions = joinInfos::getJoinConditions(ExecutionPlan::getBindings(nextTwoRelationBindings.first), ExecutionPlan::getBindings(nextTwoRelationBindings.second), res);

	double sizeOfResult=sizeLeft*sizeRight;
	for(unsigned int i=0; i<joinConditions.size(); i++){
		//apply selectivities of join conditions
		sizeOfResult = sizeOfResult*info->computeSelectivity(joinInfos::getRelationName(joinConditions.at(i).first.relation, res), joinConditions.at(i).first.name, joinInfos::getRelationName(joinConditions.at(i).second.relation, res), joinConditions.at(i).second.name);
	}

	double costOfResult = sizeOfResult + costLeft + costRight;



	string newRelationBinding = ExecutionPlan::generateNewRelationBinding(nextTwoRelationBindings.first, nextTwoRelationBindings.second);
	costVector.push_back({newRelationBinding, {costOfResult, sizeOfResult}});
	joinTree=ExecutionPlan::replaceFirst(joinTree, "("+nextTwoRelationBindings.first+" "+nextTwoRelationBindings.second+")", newRelationBinding)+" ";
	}



	return costVector.at(costVector.size()-1).second.first;
}


