#include <iostream>
#include <stack>

#include <willow/willow.hpp>

namespace willow::parser
{

    struct operand
    {
        std::string id;
        willow::symbols::Type type;
        std::string address;
    };

    class State
    {
    public:
        State();

        std::stack<std::string> filepathStack;
        std::stack<operand> operandStack;
        std::stack<std::string> operatorStack;
        std::stack<size_t> jumpStack;
        std::vector<willow::codegen::Quadruple> quadruples;
        willow::symbols::ScopeKind currScopeKind;
        std::shared_ptr<willow::symbols::SymbolTable> st;
        willow::semantics::SemanticCube sc;
        int tempCounter;

        void displayQuadruples();
    };
}