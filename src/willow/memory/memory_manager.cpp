#include <iostream>

#include <willow/memory/memory_manager.hpp>

namespace willow::memory
{
    
    void MemoryManager::addType()
    {
        for (auto& typePointer : memstate.segmentPointer)
        {
            typePointer.push_back(0);
        }
    }

    int MemoryManager::allocMemory(int memorySegment, int type_code, int size)
    {
        int internal_address = memstate.segmentPointer[memorySegment][type_code - 1];
        memstate.segmentPointer[memorySegment][type_code - 1] += size;
        
        return maskAddress(internal_address, memorySegment, type_code - 1);
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

    void MemoryManager::cacheCurrentMemstate()
    {
        memstateCache.push(memstate);
    }

    int MemoryManager::maskAddress(int internal_address, int memorySegment, int type_code)
    {
        return internal_address | segmentMask(memorySegment) | typeMask(type_code);
    }

    int MemoryManager::typeFromAddress(int address)
    {
        return ((address & 0xff << 20) >> 20);
    }

    int MemoryManager::segmentFromAddress(int address)
    {
        return ((address & 0xf << 28) >> 28);
    }

    int MemoryManager::internalAddress(int address)
    {
        return (address & ~(0xfff << 20));
    }

    int MemoryManager::segmentMask(int memorySegment)
    {
        return memorySegment << 28;
    }

    int MemoryManager::typeMask(int type_code)
    {
        return type_code << 20;
    }
}