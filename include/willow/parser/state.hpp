#include <iostream>
#include <stack>

#include <willow/willow.hpp>

using willow::codegen::Quadruple;
using willow::symbols::SymbolTable;

namespace willow::parser
{

    struct operand
    {
        std::string address;
        willow::symbols::Type type;
    };

    class State
    {
    public:
        State();

        std::stack<operand> operandStack;
        std::stack<std::string> operatorStack;
        std::vector<Quadruple> quadruples;
        willow::symbols::ScopeKind currScopeKind;
        std::shared_ptr<SymbolTable> st;
        int tempCounter;

        void displayQuadruples();
    };
}