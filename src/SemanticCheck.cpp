#include "SemanticCheck.hpp"
#include "Database.hpp"

using namespace std;

void SemanticCheck::init() {
  for (Parser::Relation& rel : result.relations) {
    string key = rel.getName();

    if (relations.find(key) != relations.end()) {
      throw SemanticError("Duplicate binding: '" + key + "'.");
    }

    try {
      database.getTable(rel.relation);
    }
    catch (...) {
      throw SemanticError("The Table '" + rel.relation + "' does not exist!");
    }

    relations[key] = rel;
  }
}

void SemanticCheck::checkAttribute(Parser::Attribute& attr, ::Attribute::Type& type, string context) {
  if (attr.name == Parser::Attribute::STAR) return;

  if (attr.relation == "") {
    // Search for attribute
    bool found = false;

    for (auto relIt : relations) {
      Table& table = database.getTable(relIt.second.relation);
      for (unsigned i = 0; i < table.getAttributeCount(); i++) {
        ::Attribute tableAttr = table.getAttribute(i);
		
        if (tableAttr.getName() == attr.name) {
          if (found) {
            throw SemanticError("Attribute '" + attr.name + "' is not clear in " + context + ".");
          }
		  
          type = tableAttr.getType();
          found = true;
		  
          break;
        }
      }
    }

    if (!found) {
      throw SemanticError("Attribute '" + attr.name + "' used in " + context + " cannot be found in the selected tables.");
    }
  }
  else {
    auto rIt = relations.find(attr.relation);
    if (rIt == relations.end()) {
      throw SemanticError("Relation or binding '" + attr.relation + "' used in " + context + " cannot be found in the selected tables.");
    }

    Table& table = database.getTable(rIt->second.relation);
    int attrIndex;
    if ((attrIndex = table.findAttribute(attr.name)) == -1) {
      throw SemanticError("There ist no attribute '" + attr.name + "' in table '" + rIt->second.relation + "'!");
    }

    type = table.getAttribute(attrIndex).getType();
  }
}

void SemanticCheck::execute() {
  init();

  ::Attribute::Type projectionType;
  for (Parser::Attribute& attr : result.projections) {
    checkAttribute(attr, projectionType, "SELECT");
  }

  // Check of "SELECT"
  for (auto selection : result.selections) {
    Parser::Attribute& attr = selection.first;
    Parser::Constant& c = selection.second;

    ::Attribute::Type type;
    checkAttribute(attr, type, "WHERE");

    if (type != c.type) {
      throw SemanticError("Types do not match ('" + attr.name + "' and '" + c.value + "')!");
    }
  }

  // Check of "WHERE"
  for (auto joinCondition : result.joinConditions) {
    Parser::Attribute& attr1 = joinCondition.first;
    Parser::Attribute& attr2 = joinCondition.second;

    ::Attribute::Type type1;
    ::Attribute::Type type2;
    checkAttribute(attr1, type1, "WHERE");
    checkAttribute(attr2, type2, "WHERE");

    if (type1 != type2) {
      throw SemanticError("Types do not match ('" + attr1.name + "' and '" + attr2.name + "')!");
    }
  }
}

