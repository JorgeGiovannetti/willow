#include <willow/memory/memory_manager.hpp>

namespace willow::memory
{

    MemoryManager::MemoryManager()
    {
        typeSizeMap["none"] = 0;
        typeSizeMap["int"] = 1;
        typeSizeMap["float"] = 1;
        typeSizeMap["bool"] = 1;
        typeSizeMap["char"] = 1;
        typeSizeMap["string"] = 256;
    }

    int MemoryManager::allocMemory(willow::symbols::ScopeKind scopeKind, std::string type, int size)
    {
        int address = memstate.tempCounter;
        memstate.tempCounter += size;
        
        return address;
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
}
