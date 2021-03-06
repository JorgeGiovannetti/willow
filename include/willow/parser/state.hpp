#ifndef STATE_HPP
#define STATE_HPP

#include <iostream>
#include <stack>

#include "willow/symbols/symbol_table.hpp"
#include "willow/symbols/function_directory.hpp"
#include "willow/symbols/class_directory.hpp"
#include "willow/codegen/quadruple.hpp"
#include "willow/memory/memory_manager.hpp"

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
        willow::symbols::FunctionDirectory funcdir;
        willow::symbols::ClassDirectory classdir;
        willow::memory::MemoryManager memory;
        std::vector<willow::symbols::Dim> currDims;
        std::string currId;
        std::string currType;
        std::string currMemberAccess;
        
        std::stack<willow::symbols::FunctionSignature> currFuncCall;
        std::stack<int> currParam;
        
        std::string isInClass;
        std::string isInFunction;
        int functionReturns = 0; // 0 = no return, 1 = return may not happen, 2 = returns nicely
        int mainStart = -1;
        int mainEnd = -1;

        void displayQuadruples();
    };
}

#endif