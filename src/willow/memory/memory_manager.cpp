#include <iostream>

#include <willow/memory/memory_manager.hpp>

namespace willow::memory
{

    void MemoryManager::addType()
    {
        for (auto &typePointer : memstate.segmentPointer)
        {
            typePointer.push_back(0);
        }
    }

    int MemoryManager::allocMemory(int memorySegment, int type_code, int size, bool isPointer)
    {
        std::cout << "alloccing " << memorySegment << " " << type_code << " " << size << " " << isPointer << std::endl;
        int internal_address = memstate.segmentPointer[memorySegment][type_code - 1];
        memstate.segmentPointer[memorySegment][type_code - 1] += size;

        return maskAddress(internal_address, memorySegment, type_code - 1, isPointer);
    }

    void MemoryManager::deallocMemory()
    {
        if (memstateCache.empty())
        {
            memstate = MemoryState();
        }
        else
        {
            memstate = memstateCache.top();
            memstateCache.pop();
        }
    }

    void MemoryManager::cacheCurrentMemstate(bool shouldClearMemory)
    {
        memstateCache.push(memstate);

        // Clears all segment pointers except global (for use in function calls)
        if (shouldClearMemory)
        {
            int segmentPointerSize = memstate.segmentPointer[GLOBAL].size();
            for (int i = GLOBAL + 1; i < memstate.segmentPointer.size(); i++)
            {
                memstate.segmentPointer[i] = std::vector<int>(segmentPointerSize, 0);
            }
        }
    }

    int MemoryManager::maskAddress(int internal_address, int memorySegment, int type_code, bool isPointer)
    {
        return internal_address | segmentMask(memorySegment) | typeMask(type_code) | pointerMask(isPointer);
    }

    int MemoryManager::typeFromAddress(int address)
    {
        return ((address & 0xff << 21) >> 21);
    }

    int MemoryManager::segmentFromAddress(int address)
    {
        return ((address & 0x3 << 29) >> 29);
    }

    int MemoryManager::internalAddress(int address)
    {
        return (address & ~(0x7ff << 21));
    }

    int MemoryManager::segmentMask(int memorySegment)
    {
        return memorySegment << 29;
    }

    int MemoryManager::typeMask(int type_code)
    {
        return type_code << 21;
    }

    int MemoryManager::pointerMask(bool isPointer)
    {
        return isPointer << 31;
    }

    bool MemoryManager::isPointer(int address)
    {
        return ((address >> 31) & 1);
    }

    bool MemoryManager::isPointer(std::string address)
    {
        if (address.empty() || address[0] != '&')
        {
            return false;
        }
        return isPointer(std::stoi(address.substr(1)));
    }
}