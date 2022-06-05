#include <willow/semantics/semantic_cube.hpp>
#include <string>
#include <vector>

namespace willow::semantics
{

    enum types
    {
        ERROR = -1,
        INT = 0,
        FLOAT = 1,
        BOOL = 2,
        CHAR = 3,
        STRING = 4,
    };

    enum operators
    {
        SUM = 0,
        MINUS = 1,
        MULTIPLY = 2,
        DIVIDE = 3,
        MOD = 4,
    };

    int types_count = 5;
    const int operator_count = 5;

    SemanticCube::SemanticCube() : semanticMapping(types_count, std::vector<std::vector<int>>(types_count, std::vector<int>(operator_count, ERROR)))
    {
        ///////////////
        //   INT
        ///////////////

        // SUM
        semanticMapping[INT][INT][SUM] = INT;
        semanticMapping[INT][FLOAT][SUM] = FLOAT;

        // MINUS
        semanticMapping[INT][INT][MINUS] = INT;
        semanticMapping[INT][FLOAT][MINUS] = FLOAT;

        // MULTIPLY
        semanticMapping[INT][INT][MULTIPLY] = INT;
        semanticMapping[INT][FLOAT][MULTIPLY] = FLOAT;

        // DIVIDE
        semanticMapping[INT][INT][DIVIDE] = INT;
        semanticMapping[INT][FLOAT][DIVIDE] = FLOAT;

        // MOD
        semanticMapping[INT][INT][MOD] = INT;

        ///////////////
        //   FLOAT
        ///////////////

        // SUM
        semanticMapping[FLOAT][INT][SUM] = FLOAT;
        semanticMapping[FLOAT][FLOAT][SUM] = FLOAT;

        // MINUS
        semanticMapping[FLOAT][INT][MINUS] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MINUS] = FLOAT;

        // MULTIPLY
        semanticMapping[FLOAT][INT][MULTIPLY] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MULTIPLY] = FLOAT;

        // DIVIDE
        semanticMapping[FLOAT][INT][DIVIDE] = FLOAT;
        semanticMapping[FLOAT][FLOAT][DIVIDE] = FLOAT;

        ///////////////
        //   CHAR
        ///////////////

        // SUM
        semanticMapping[CHAR][INT][SUM] = CHAR;
        semanticMapping[CHAR][CHAR][SUM] = INT;
        semanticMapping[CHAR][STRING][SUM] = STRING;

        // MINUS
        semanticMapping[CHAR][INT][MINUS] = CHAR;
        semanticMapping[CHAR][CHAR][MINUS] = INT;

        ///////////////
        //   STRING
        ///////////////

        // SUM
        semanticMapping[STRING][CHAR][SUM] = STRING;
        semanticMapping[STRING][STRING][SUM] = STRING;

        ///////////////
        //   UNORDERED_MAPS
        ///////////////

        // TYPES STRING
        typeStringToInt["int"] = 0;
        typeStringToInt["float"] = 1;
        typeStringToInt["bool"] = 2;
        typeStringToInt["char"] = 3;
        typeStringToInt["string"] = 4;

        // TYPES INT
        typeIntToString[0] = "int";
        typeIntToString[1] = "float";
        typeIntToString[2] = "bool";
        typeIntToString[3] = "char";
        typeIntToString[4] = "string";

        // OPERATORS
        operatorMap["+"] = 0;
        operatorMap["-"] = 1;
        operatorMap["*"] = 2;
        operatorMap["/"] = 3;
        operatorMap["%"] = 4;
    }

    std::string SemanticCube::query(std::string op1, std::string op2, std::string oper)
    {
        int return_type = semanticMapping[typeStringToInt[op1]][typeStringToInt[op2]][operatorMap[oper]];
        if (return_type == ERROR)
        {
            throw "operator " + oper + " not supported with types " + op1 + " and " + op2;
        }
        return typeIntToString[return_type];
    }

    void SemanticCube::newType(std::string type_name)
    {
        typeIntToString[types_count] = type_name;
        typeStringToInt[type_name] = types_count;
        types_count++;
        semanticMapping.push_back(std::vector<std::vector<int>>(types_count, std::vector<int>(operator_count, ERROR)));
    }
}