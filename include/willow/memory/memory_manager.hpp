#pragma once

#include <string>
#include <stack>
#include <unordered_map>

#include "willow/symbols/symbol_table.hpp"

namespace willow::memory
{

    enum MemorySegment {
        GLOBAL,
        LOCAL,
        TEMP
    };

    struct MemoryState
    {
        std::vector<std::vector<int>> segmentPointer;
        MemoryState() : segmentPointer(3, std::vector<int>(5, 0)) {}
    };

    class MemoryManager
    {
    public:
        int allocMemory(int memorySegment, int type_code, int size);
        void deallocMemory();
        void cacheCurrentMemstate();

        int maskAddress(int internal_address, int memorySegment, int type_code);
        int typeFromAddress(int address);
        int segmentFromAddress(int address);
        int internalAddress(int address);
        void addType();

    private:

        int segmentMask(int memorySegment);
        int typeMask(int type_code);

        std::stack<MemoryState> memstateCache;
        MemoryState memstate;
    };
}
