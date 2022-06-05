#include <willow/semantics/semantic_cube.hpp>
#include <string>
#include <vector>

using std::string, std::vector;

enum types {
    ERROR = -1,
    INT = 0,
    FLOAT = 1,
    BOOL = 2,
    CHAR = 3,
    STRING = 4,
};

enum operators {
    ERROR = -1,
    SUM = 0,
    MINUS = 1,
    MULTIPLY = 2,
    DIVIDE = 3,
    MOD = 4,
};

namespace willow::semantics
{
    int n = 30;

    SemanticCube::SemanticCube() : semanticMapping(n, vector<vector<int>>(n, vector<int>(n,ERROR))) {
        ///////////////
        //   INT
        ///////////////

        //SUM
        semanticMapping[INT][INT][SUM] = INT;
        semanticMapping[INT][FLOAT][SUM] = FLOAT;
        
        //MINUS
        semanticMapping[INT][INT][MINUS] = INT;
        semanticMapping[INT][FLOAT][MINUS] = FLOAT;
        
        //MULTIPLY
        semanticMapping[INT][INT][MULTIPLY] = INT;
        semanticMapping[INT][FLOAT][MULTIPLY] = FLOAT;

        //DIVIDE
        semanticMapping[INT][INT][DIVIDE] = INT;
        semanticMapping[INT][FLOAT][DIVIDE] = FLOAT;
        
        //MOD
        semanticMapping[INT][INT][MOD] = INT;

        ///////////////
        //   FLOAT
        ///////////////
        
        //SUM
        semanticMapping[FLOAT][INT][SUM] = FLOAT;
        semanticMapping[FLOAT][FLOAT][SUM] = FLOAT;

        //MINUS
        semanticMapping[FLOAT][INT][MINUS] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MINUS] = FLOAT;
        
        //MULTIPLY
        semanticMapping[FLOAT][INT][MULTIPLY] = FLOAT;
        semanticMapping[FLOAT][FLOAT][MULTIPLY] = FLOAT;
        
        //DIVIDE
        semanticMapping[FLOAT][INT][DIVIDE] = FLOAT;
        semanticMapping[FLOAT][FLOAT][DIVIDE] = FLOAT;

        ///////////////
        //   CHAR
        ///////////////

        //SUM
        semanticMapping[CHAR][INT][SUM] = CHAR;
        semanticMapping[CHAR][CHAR][SUM] = INT;
        semanticMapping[CHAR][STRING][SUM] = STRING;

        //MINUS
        semanticMapping[CHAR][INT][MINUS] = CHAR;
        semanticMapping[CHAR][CHAR][MINUS] = INT;
        
        ///////////////
        //   STRING
        ///////////////

        //SUM
        semanticMapping[STRING][CHAR][SUM] = STRING;
        semanticMapping[STRING][STRING][SUM] = STRING;

        ///////////////
        //   UNORDERED_MAPS
        ///////////////

        //TYPES STRING
        typeMapString["int"] = 0;
        typeMapString["float"] = 1;
        typeMapString["bool"] = 2;
        typeMapString["char"] = 3;
        typeMapString["string"] = 4;

        //TYPES INT
        typeMapInt[0] = "int";
        typeMapInt[1] = "float";
        typeMapInt[2] = "bool";
        typeMapInt[3] = "char";
        typeMapInt[4] = "string";

        //OPERATORS
        operatorMap["+"] = 0;
        operatorMap["-"] = 1;
        operatorMap["*"] = 2;
        operatorMap["/"] = 3;
        operatorMap["%"] = 4;
    }

    string SemanticCube::query(string op1, string op2, string oper){
        int result = semanticMapping[typeMapString[op1]][typeMapString[op2]][operatorMap[oper]];
        if(result != -1){
            return typeMapInt[result];
        }
        else{
            throw "ERROR: type mismatch";
        }
    }
}