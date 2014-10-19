#ifndef H_SemanticCheck
#define H_SemanticCheck

#include "Database.hpp"
#include "Exception.hpp"
#include "Parser.hpp"
#include <unordered_map>
#include <string>

class SemanticCheck {
  private:
    Database& database;
    Parser::Result& result;

    std::unordered_map<std::string, Parser::Relation> relations;

    void init();
    void checkAttribute(Parser::Attribute& attributes, ::Attribute::Type& type, std::string context);

  public:
    SemanticCheck(Database& database, Parser::Result& result) : database(database), result(result) {}

    void execute();

    class SemanticError : public Exception {
      public:
        SemanticError(const std::string message) : Exception(message) {}
    };
};

#endif

