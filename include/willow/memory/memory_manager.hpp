#pragma once

#include <string>
#include <stack>
#include <unordered_map>

#include "willow/symbols/symbol_table.hpp"

namespace willow::memory
{

    struct MemoryState
    {
        int tempCounter = 0;
    };

    class MemoryManager
    {
    public:
        MemoryManager();
        int allocMemory(willow::symbols::ScopeKind scopeKind, std::string type, int size);
        void deallocMemory();
        void cacheCurrentMemstate();
        std::unordered_map<std::string, int> typeSizeMap;

    private:
        std::stack<MemoryState> memstateCache;
        MemoryState memstate;
    };
}
