#pragma once

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
        std::string currType;

        bool isInFunction;
        bool isInClass;

        void displayQuadruples();
    };
}