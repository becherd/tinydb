/*
 * Parser.cpp
 *
 *  Created on: Oct 17, 2014
 *      Author: david
 */

#include "Parser.hpp"

//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

//function Prototypes
vector<string> explode(string inputstring, string delimiter);
string ltrim(string str );
string rtrim(string str );
string trim(string str );
bool allTablesExist(vector<vector<string>>tables);
int parseQuery(string query);


int parseQuery(string query)
{
    query = "select * from Studenten s, hoeren h where s.Name='Fichte' and s.MatrNr=h.MatrNr";


string from;
string where;
string select;

int posSelect = query.find("select");
int posFrom = query.find("from");
int posWhere = query.find("where");

select = trim(query.substr(posSelect+6, posFrom-(posSelect+6)));
from = trim(query.substr(posFrom+4, posWhere-(posFrom+4)));
where = trim(query.substr(posWhere+5, query.length()-(posWhere+5)));


vector<string> selectVector = explode(select, ",");
vector<string> fromVector = explode(from, ",");
vector<string> whereVector = explode(where, " and ");


//deal with select
vector<vector<string>> selectVector2;
for(int i=0; (unsigned)i<selectVector.size(); i++){
	string strAtPos = trim(selectVector.at(i));


vector<string>v;
if (strAtPos.find(" ",0)==std::string::npos){
	//no space characters
	v.push_back(strAtPos);
	v.push_back("");
}
else{
	v=explode(strAtPos, " ");
}
selectVector2.push_back(v);

}

for(int j=0; (unsigned)j<selectVector2.size(); j++){
	cout << selectVector2.at(j).at(0) << " - " << selectVector2.at(j).at(1) << "\n";
}





//deal with from
vector<vector<string>> fromVector2;
for(int i=0; (unsigned)i<fromVector.size(); i++){
	string strAtPos = trim(fromVector.at(i));

	fromVector2.push_back(explode(strAtPos, " "));
}

for(int j=0; (unsigned)j<fromVector2.size(); j++){
	cout << fromVector2.at(j).at(0) << " - " << fromVector2.at(j).at(1) << "\n";
}


//deal with where
vector<vector<string>> whereVector2;
for(int i=0; (unsigned)i<whereVector.size(); i++){
	string strAtPos = trim(whereVector.at(i));

	whereVector2.push_back(explode(strAtPos, "="));
}

for(int j=0; (unsigned)j<whereVector2.size(); j++){
	cout << whereVector2.at(j).at(0) << " - " << whereVector2.at(j).at(1) << "\n";
}


bool allTablesFound=allTablesExist(selectVector2);
if(!allTablesFound){
	cout << "Syntax Error!";
}

cout<<select<<"\n"<< from << "\n"<< where << "\n";



return 1;
}

vector<string> explode(string inputstring, string delimiter){
    vector<string> explodes;
    inputstring.append(delimiter);
    while(inputstring.find(delimiter)!=string::npos){
        explodes.push_back(inputstring.substr(0, inputstring.find(delimiter)));
        inputstring.erase(inputstring.begin(), inputstring.begin()+inputstring.find(delimiter)+delimiter.size());
    }
    return explodes;
}


string ltrim(string str ) {
	str.erase(0,str.find_first_not_of(" ")); //trim left
	return str;
}

string rtrim(string str ) {
	str.erase(str.find_last_not_of(" ")+1);  //trim right
	return str;
}

string trim(string str ) {
    return ltrim( rtrim( str ) );
}

bool allTablesExist(vector<vector<string>>tables){
	   Database db;
	   db.open("data/uni");
	   bool found=true;
	   for(int i=0; (unsigned)i<tables.size(); i++){
	   try{
		   db.getTable(tables.at(i).at(0));
	   }catch(int e){
		   found=false;
	   }
	   }
	   return found;
}




//---------------------------------------------------------------------------


