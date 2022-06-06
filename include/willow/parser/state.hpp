#include <iostream>
#include <stack>

#include "willow/symbols/symbol_table.hpp"
#include "willow/symbols/function_directory.hpp"
#include "willow/symbols/class_directory.hpp"
#include "willow/codegen/quadruple.hpp"

namespace willow::parser
{

    class State
    {
    public:
        State();

        std::stack<std::string> filepathStack;
        std::stack<willow::symbols::Symbol> operandStack;
        std::stack<std::string> operatorStack;
        std::stack<size_t> jumpStack;
        std::vector<willow::codegen::Quadruple> quadruples;
        willow::symbols::ScopeKind currScopeKind;
        willow::semantics::SemanticCube sc;
        std::shared_ptr<willow::symbols::SymbolTable> st;
        willow::symbols::FunctionDirectory fd;
        willow::symbols::ClassDirectory cd;

        bool isInFunction = false;
        int tempCounter;

        void displayQuadruples();
    };
}