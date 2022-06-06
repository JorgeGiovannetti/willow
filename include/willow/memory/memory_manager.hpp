#pragma once

#include <string>
#include <stack>
#include <unordered_map>

#include "willow/symbols/symbol_table.hpp"

namespace willow::memory
{

    struct MemoryState
    {
        std::vector<int> scopePointer;
    };

    class MemoryManager
    {
    public:
        MemoryManager();
        int allocMemory(willow::symbols::ScopeKind scopeKind, int type_code, int size);
        void deallocMemory();
        void cacheCurrentMemstate();


    private:
        int maskAddress(int internal_address, willow::symbols::ScopeKind scopeKind, int type_code);
        int typeFromAddress(int address);
        int scopeFromAddress(int address);

        int scopeMask(willow::symbols::ScopeKind scopeKind);
        int typeMask(int type_code);

        std::stack<MemoryState> memstateCache;
        MemoryState memstate;
    };
}
