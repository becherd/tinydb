/*
 * Parser.hpp
 *
 *  Created on: Oct 18, 2014
 *      Author: david
 */

#ifndef SRC_PARSER_HPP_
#define SRC_PARSER_HPP_


#include "Database.hpp"

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <unordered_map>


//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------

class  Parser
{
private:
bool found;
string str;
vector<string> v;
int integer;

public:
/// Constructor
explicit Parser();
/// Destructor
~Parser();

vector<string> explode(string inputstring, string delimiter){return v;};
string ltrim(string str ) {return str;}
string rtrim(string str ) {return str;}
string trim(string str ) {return str;}

bool allTablesExist(vector<vector<string>>tables){return found;};

int parseQuery(string query){return integer;};
};

#endif /* SRC_PARSER_HPP_ */
