#pragma once

#include <unordered_map>

#include <willow/memory/memory_manager.hpp>
#include <willow/codegen/quadruple.hpp>
#include <willow/semantics/type_manager.hpp>

namespace willow::vm
{
    using willow::codegen::Quadruple;

    struct CallMemory
    {
        std::vector<std::string> localMemory;
        std::vector<std::string> tempMemory;
        std::vector<std::string> constantsMemory;
    };

    struct AddressData
    {
        std::string type;
        int scopeKind;
        int internal_address;
    };

    class VM
    {
    public:
        VM();
        void run();
        void loadQuadruples(std::vector<Quadruple> quadruples);
        void loadQuadruples(std::string filepath);

        bool isAddress(const std::string &address);
        int getAddress(std::string address);
        Quadruple getCurrentQuad();
        void assignToAddress(std::string value, std::string target);
        AddressData addressInfo(const std::string &address);
        std::string getDataFromMemory(const std::string &address);
        void setDataInMemory(const std::string &address, std::string data);

        std::unordered_map<std::string, void (*)(VM *)> ops;
        willow::memory::MemoryManager mm;
        std::vector<std::string> globalMemory;
        std::stack<CallMemory> memoryStack;
        int instruction_pointer;
        std::stack<int> callStack;
        std::vector<Quadruple> instructions;
        willow::semantics::TypeManager typeManager;

        bool hasEnded;

    private:
        void loadOperations();
        std::string getDataFromMemory(AddressData addressData);
        AddressData addressInfo(const int &address);
    };
}