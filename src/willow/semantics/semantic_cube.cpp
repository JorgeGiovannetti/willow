#include <iostream>
#include <string>
#include <vector>

#include "willow/semantics/semantic_cube.hpp"

namespace willow::semantics
{

    enum types
    {
        ERROR = -1,
        NONE = 0,
        INT = 1,
        FLOAT = 2,
        BOOL = 3,
        CHAR = 4,
        STRING = 5,
    };

    enum operators
    {
        SUM = 0,
        MINUS = 1,
        MULTIPLY = 2,
        DIVIDE = 3,
        MOD = 4,
        NOT = 5,
        LESSER_THAN = 6,
        GREATER_THAN = 7,
        LESSER_EQ_THAN = 8,
        GREATER_EQ_THAN = 9,
        EQUAL = 10,
        NOT_EQUAL = 11,
        AND = 12,
        OR = 13,
        ASSIGN = 14,
    };

    int types_count = 6;
    const int operator_count = 15;

    SemanticCube::SemanticCube() : semanticMapping(types_count, std::vector<std::vector<int>>(types_count, std::vector<int>(operator_count, ERROR)))
    {

        ///////////////
        //   SUM
        ///////////////

        // INT
        semanticMapping[INT][INT][SUM] = INT;
        semanticMapping[INT][FLOAT][SUM] = FLOAT;
        semanticMapping[INT][STRING][SUM] = STRING;

        // FLOAT
        semanticMapping[FLOAT][INT][SUM] = FLOAT;
        semanticMapping[FLOAT][FLOAT][SUM] = FLOAT;

        // CHAR
        semanticMapping[CHAR][INT][SUM] = CHAR;
        semanticMapping[CHAR][CHAR][SUM] = INT;
        semanticMapping[CHAR][STRING][SUM] = STRING;

        // STRING
        semanticMapping[STRING][INT][SUM] = STRING;
        semanticMapping[STRING][CHAR][SUM] = STRING;
        semanticMapping[STRING][STRING][SUM] = STRING;

        ///////////////
        //   MINUS
        ///////////////

        // INT
        semanticMapping[INT][NONE][MINUS] = INT;
        semanticMapping[INT][INT][MINUS] = INT;
        semanticMapping[INT][FLOAT][MINUS] = FLOAT;

        // FLOAT
        semanticMapping[FLOAT][NONE][MINUS] = FLOAT;
        semanticMapping[FLOAT][INT][MINUS] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MINUS] = FLOAT;

        // CHAR
        semanticMapping[CHAR][INT][MINUS] = CHAR;
        semanticMapping[CHAR][CHAR][MINUS] = INT;

        ///////////////
        //   MULTIPLY
        ///////////////

        // INT
        semanticMapping[INT][INT][MULTIPLY] = INT;
        semanticMapping[INT][FLOAT][MULTIPLY] = FLOAT;
        semanticMapping[INT][CHAR][MULTIPLY] = INT;
        semanticMapping[INT][STRING][MULTIPLY] = STRING;

        // FLOAT
        semanticMapping[FLOAT][INT][MULTIPLY] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MULTIPLY] = FLOAT;

        // CHAR
        semanticMapping[CHAR][INT][MULTIPLY] = INT;

        // STRING
        semanticMapping[STRING][INT][MULTIPLY] = STRING;

        ///////////////
        //   DIVIDE
        ///////////////

        // INT
        semanticMapping[INT][INT][DIVIDE] = INT;
        semanticMapping[INT][FLOAT][DIVIDE] = FLOAT;

        // FLOAT
        semanticMapping[FLOAT][INT][DIVIDE] = FLOAT;
        semanticMapping[FLOAT][FLOAT][DIVIDE] = FLOAT;

        ///////////////
        //   MOD
        ///////////////

        // INT
        semanticMapping[INT][INT][MOD] = INT;

        ///////////////
        //   LESSER_THAN
        ///////////////

        // INT
        semanticMapping[INT][INT][LESSER_THAN] = BOOL;
        semanticMapping[INT][FLOAT][LESSER_THAN] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][LESSER_THAN] = BOOL;
        semanticMapping[FLOAT][FLOAT][LESSER_THAN] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][LESSER_THAN] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][LESSER_THAN] = BOOL;

        ///////////////
        //   GREATER_THAN
        ///////////////

        // INT
        semanticMapping[INT][INT][GREATER_THAN] = BOOL;
        semanticMapping[INT][FLOAT][GREATER_THAN] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][GREATER_THAN] = BOOL;
        semanticMapping[FLOAT][FLOAT][GREATER_THAN] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][GREATER_THAN] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][GREATER_THAN] = BOOL;

        ///////////////
        //   LESSER_EQ_THAN
        ///////////////

        // INT
        semanticMapping[INT][INT][LESSER_EQ_THAN] = BOOL;
        semanticMapping[INT][FLOAT][LESSER_EQ_THAN] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][LESSER_EQ_THAN] = BOOL;
        semanticMapping[FLOAT][FLOAT][LESSER_EQ_THAN] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][LESSER_EQ_THAN] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][LESSER_EQ_THAN] = BOOL;

        ///////////////
        //   GREATER_EQ_THAN
        ///////////////

        // INT
        semanticMapping[INT][INT][GREATER_EQ_THAN] = BOOL;
        semanticMapping[INT][FLOAT][GREATER_EQ_THAN] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][GREATER_EQ_THAN] = BOOL;
        semanticMapping[FLOAT][FLOAT][GREATER_EQ_THAN] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][GREATER_EQ_THAN] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][GREATER_EQ_THAN] = BOOL;

        ///////////////
        //   EQUAL
        ///////////////

        // INT
        semanticMapping[INT][INT][EQUAL] = BOOL;
        semanticMapping[INT][FLOAT][EQUAL] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][EQUAL] = BOOL;
        semanticMapping[FLOAT][FLOAT][EQUAL] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][EQUAL] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][EQUAL] = BOOL;

        // BOOL
        semanticMapping[BOOL][BOOL][EQUAL] = BOOL;

        ///////////////
        //   NOT_EQUAL
        ///////////////

        // INT
        semanticMapping[INT][INT][NOT_EQUAL] = BOOL;
        semanticMapping[INT][FLOAT][NOT_EQUAL] = BOOL;

        // FLOAT
        semanticMapping[FLOAT][INT][NOT_EQUAL] = BOOL;
        semanticMapping[FLOAT][FLOAT][NOT_EQUAL] = BOOL;

        // CHAR
        semanticMapping[CHAR][CHAR][NOT_EQUAL] = BOOL;

        // STRING
        semanticMapping[STRING][STRING][NOT_EQUAL] = BOOL;

        // BOOL
        semanticMapping[BOOL][BOOL][NOT_EQUAL] = BOOL;

        ///////////////
        //   ASSIGN
        ///////////////

        // INT
        semanticMapping[INT][INT][ASSIGN] = INT;

        // FLOAT
        semanticMapping[FLOAT][INT][ASSIGN] = FLOAT;
        semanticMapping[FLOAT][FLOAT][ASSIGN] = FLOAT;

        // CHAR
        semanticMapping[CHAR][CHAR][ASSIGN] = CHAR;

        // STRING
        semanticMapping[STRING][STRING][ASSIGN] = STRING;

        // BOOL
        semanticMapping[BOOL][BOOL][ASSIGN] = BOOL;

        ///////////////
        //   NOT
        ///////////////

        // BOOL
        semanticMapping[BOOL][NONE][NOT] = BOOL;

        ///////////////
        //   AND
        ///////////////

        // BOOL
        semanticMapping[BOOL][BOOL][AND] = BOOL;

        ///////////////
        //   OR
        ///////////////

        // BOOL
        semanticMapping[BOOL][BOOL][OR] = BOOL;

        ///////////////
        //   UNORDERED_MAPS
        ///////////////

        // OPERATORS
        operatorMap["+"] = 0;
        operatorMap["-"] = 1;
        operatorMap["*"] = 2;
        operatorMap["/"] = 3;
        operatorMap["%"] = 4;
        operatorMap["!"] = 5;
        operatorMap["<"] = 6;
        operatorMap[">"] = 7;
        operatorMap["<="] = 8;
        operatorMap[">="] = 9;
        operatorMap["=="] = 10;
        operatorMap["!="] = 11;
        operatorMap["and"] = 12;
        operatorMap["or"] = 13;
        operatorMap["="] = 14;
    }

    std::string SemanticCube::query(const std::string &op1, const std::string &op2, const std::string &oper)
    {
        int return_type = semanticMapping[typeManager.getType(op1)][typeManager.getType(op2)][operatorMap[oper]];

        if (return_type == ERROR)
        {
            throw std::string("operator " + oper + " not supported with types " + op1 + " and " + op2);
        }

        return typeManager.getType(return_type);
    }

    void SemanticCube::newType(const std::string &type_name, const int &size)
    {
        typeManager.newType(type_name, size);
        semanticMapping.push_back(std::vector<std::vector<int>>(types_count, std::vector<int>(operator_count, ERROR)));
    }

    std::string SemanticCube::getType(int type_code)
    {
        return typeManager.getType(type_code);
    }

    int SemanticCube::getType(std::string type_name)
    {
        return typeManager.getType(type_name);
    }

    int SemanticCube::getTypeSize(int type_code)
    {
        return typeManager.getTypeSize(type_code);
    }

    int SemanticCube::getTypeSize(std::string type_name)
    {
        return typeManager.getTypeSize(type_name);
    }

}