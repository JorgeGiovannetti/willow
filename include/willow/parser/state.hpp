#include <iostream>
#include <stack>

#include <willow/willow.hpp>

using willow::codegen::Quadruple;
using willow::symbols::SymbolTable;

namespace willow::parser
{

    struct operand
    {
        std::string id;
        willow::symbols::Type type;
    };

    class State
    {
    public:
        State();

        std::stack<std::string> filepathStack;
        std::stack<operand> operandStack;
        std::stack<std::string> operatorStack;
        std::stack<size_t> jumpStack;
        std::vector<Quadruple> quadruples;
        willow::symbols::ScopeKind currScopeKind;
        std::shared_ptr<SymbolTable> st;
        int tempCounter;

        void displayQuadruples();
    };
}