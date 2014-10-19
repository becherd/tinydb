#include "SemanticCheck.hpp"
#include "Database.hpp"

using namespace std;

void SemanticCheck::init() {
  for(Parser::Relation& rel : result.relations) {
    string key = rel.getName();

    if(relations.find(key) != relations.end()) {
      throw SemanticError("Error: There is a duplicate binding: '" + key + "'.");
    }

    try {
      database.getTable(rel.relation);
    } catch (...) {
      throw SemanticError("Error: The Table '" + rel.relation + "' does not exist!");
    }

    relations[key] = rel;
  }
}

void SemanticCheck::checkAttribute(Parser::Attribute& attr, ::Attribute::Type& type, string string) {
  if(attr.name == Parser::Attribute::STAR) {
    return;
  }

  if(attr.relation == "") {
    // Search for attribute
    bool found = false;

    for(auto relIt : relations) {
      Table& table = database.getTable(relIt.second.relation);
      for (unsigned i = 0; i < table.getAttributeCount(); i++) {
        ::Attribute tableAttr = table.getAttribute(i);
		
        if(tableAttr.getName() == attr.name) {
          if(found) {
            throw SemanticError("Error: There are several Attributes called '" + attr.name + "' in " + string + ".");
          }
		  
          type = tableAttr.getType();
          found = true;

          break;
        }
      }
    }

    if (!found) {
      throw SemanticError("Error: Attribute '" + attr.name + "'which is used in " + string + " cannot be found in the selected tables.");
    }
  }
  else {
    auto rIt = relations.find(attr.relation);
    if (rIt == relations.end()) {
      throw SemanticError("Error: Relation or binding '" + attr.relation + "'which is used in " + string + " cannot be found in the selected tables.");
    }

    Table& table = database.getTable(rIt->second.relation);
    int attrIndex;
    if((attrIndex = table.findAttribute(attr.name)) == -1) {
      throw SemanticError("Error: There is no attribute '" + attr.name + "' in table '" + rIt->second.relation + "'!");
    }

    type = table.getAttribute(attrIndex).getType();
  }
}

void SemanticCheck::execute() {
  init();

  ::Attribute::Type projection;
  for(Parser::Attribute& attr : result.projections) {
    checkAttribute(attr, projection, "SELECT");
  }

  // Check of "SELECT"
  for(auto selection : result.selections) {
    Parser::Attribute& attr = selection.first;
    Parser::Constant& c = selection.second;

    ::Attribute::Type type;
    checkAttribute(attr, type, "WHERE");
  }

  // Check of "WHERE"
  for(auto joinCondition : result.joinConditions) {
    Parser::Attribute& attr1 = joinCondition.first;
    Parser::Attribute& attr2 = joinCondition.second;

    ::Attribute::Type type1;
    ::Attribute::Type type2;
    checkAttribute(attr1, type1, "WHERE");
    checkAttribute(attr2, type2, "WHERE");
  }
}

