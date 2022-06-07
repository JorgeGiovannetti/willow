#include <willow/memory/memory_manager.hpp>

namespace willow::memory
{

    MemoryManager::MemoryManager()
    {
        for (int i = 0; i <= symbols::CONSTANT; i++)
        {
            memstate.scopePointer.push_back(0);
        }
    }

    int MemoryManager::allocMemory(willow::symbols::ScopeKind scopeKind, int type_code, int size)
    {
        int internal_address = memstate.scopePointer[scopeKind];
        memstate.scopePointer[scopeKind] += size;

        return maskAddress(internal_address, scopeKind, type_code);
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

    int MemoryManager::maskAddress(int internal_address, willow::symbols::ScopeKind scopeKind, int type_code)
    {
        return internal_address | scopeMask(scopeKind) | typeMask(type_code);
    }

    int MemoryManager::typeFromAddress(int address)
    {
        return ((address & 0xff << 20) >> 20);
    }

    int MemoryManager::scopeFromAddress(int address)
    {
        return ((address & 0xf << 28) >> 28);
    }

    int MemoryManager::internalAddress(int address)
    {
        return (address & ~(0xfff << 20));
    }

    int MemoryManager::scopeMask(willow::symbols::ScopeKind scopeKind)
    {
        return scopeKind << 28;
    }

    int MemoryManager::typeMask(int type_code)
    {
        return type_code << 20;
    }
}