#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <string>
#include <stack>
#include <unordered_map>

#include "willow/semantics/semantic_cube.hpp"

namespace willow::memory
{

    enum MemorySegment
    {
        GLOBAL,
        LOCAL,
        TEMP
    };

    struct MemoryState
    {
        std::vector<std::vector<int>> segmentPointer;
        MemoryState() : segmentPointer(3, std::vector<int>(6, 0)) {}
    };

    class MemoryManager
    {
    public:
        int allocMemory(int memorySegment, int type_code, int size, bool isPointer);
        std::string allocMemory(willow::semantics::SemanticCube &sc, int memorySegment, std::string type_str, int dims_size, bool isPointer);

        void deallocMemory();
        void cacheCurrentMemstate(bool shouldClearMemory);

        int maskAddress(int internal_address, int memorySegment, int type_code, bool isPointer);
        int typeFromAddress(int address);
        int segmentFromAddress(int address);
        int internalAddress(int address);
        void addType();
        bool isPointer(int address);
        bool isPointer(std::string address);

    private:
        int segmentMask(int memorySegment);
        int typeMask(int type_code);
        int pointerMask(bool isPointer);

        std::stack<MemoryState> memstateCache;
        MemoryState memstate;
    };
}

#endif
