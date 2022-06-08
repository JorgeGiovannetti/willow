#pragma once

#include <unordered_map>
#include <functional>

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

    struct AddressInfo
    {
        std::string type;
        int scopeKind;
        int internal_address;
    };

    struct VariableData
    {
        std::string type;
        std::string data;
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
        AddressInfo addressInfo(const std::string &address);
        VariableData getLiteralData(const std::string &data);
        VariableData getVariableData(const std::string &address);
        std::string getDataFromMemory(const std::string &address);
        void setDataInMemory(const std::string &address, std::string data);

        std::unordered_map<std::string, std::function<void(VM*)>> ops;
        willow::memory::MemoryManager mm;
        std::vector<std::string> globalMemory;
        std::stack<CallMemory> memoryStack;
        size_t instruction_pointer;
        std::stack<size_t> callStack;
        std::vector<Quadruple> instructions;
        willow::semantics::TypeManager typeManager;

        bool hasEnded;

    private:
        void loadOperations();
        std::string getDataFromMemory(AddressInfo addressData);
        AddressInfo addressInfo(const int &address);
    };
}