#include <willow/vm/VM.hpp>
#include <willow/codegen/quadruple.hpp>
#include "operations/operations.cpp"

namespace willow::vm
{

    using willow::codegen::Quadruple;

    VM::VM()
    {
        instruction_pointer = 0;
        hasEnded = false;
        memoryStack.push(CallMemory());
        loadOperations();
    }

    void VM::loadOperations()
    {
        ops["="] = &operations::assign;
        ops["+"] = &operations::sum;
        ops["-"] = &operations::subtract;
        ops["*"] = &operations::multiply;
        ops["/"] = &operations::divide;
        ops["%"] = &operations::modulo;
        ops[">"] = &operations::greater;
        ops[">="] = &operations::geq;
        ops["<"] = &operations::lesser;
        ops["<="] = &operations::leq;
        ops["=="] = &operations::eq;
        ops["write"] = &operations::write;
        ops["read"] = &operations::read;
        ops["readln"] = &operations::readln;
        ops["end"] = &operations::endExecution;
        ops["goto"] = &operations::gotoInstruction;
        ops["gotof"] = &operations::gotofInstruction;
    }

    void VM::loadQuadruples(std::string filepath)
    {
        // TODO
    }

    void VM::loadQuadruples(std::vector<Quadruple> quadruples)
    {
        instructions = quadruples;
    }

    Quadruple VM::getCurrentQuad()
    {
        return instructions[instruction_pointer];
    }

    bool VM::isAddress(const std::string &address)
    {
        return address.length() > 0 && address[0] == '&';
    }

    int VM::getAddress(std::string address)
    {
        try
        {
            return std::stoi(address.substr(1));
        }
        catch (const std::invalid_argument &e)
        {
            throw std::string("Expected integer literal at getAddress, received: " + address);
        }
    }

    AddressInfo VM::addressInfo(const int &address)
    {
        std::string address_type = typeManager.getType(mm.typeFromAddress(address));
        int address_scope = mm.scopeFromAddress(address);
        int internal_address = mm.internalAddress(address);
        return {address_type, address_scope, internal_address};
    }

    AddressInfo VM::addressInfo(const std::string &address)
    {
        return addressInfo(getAddress(address));
    }

    VariableData VM::getLiteralData(const std::string &data)
    {
        std::cout << "getliteraldata with data: " << data << std::endl;
        try
        {
            if (data[0] == '\"')
            {
                return {"string", data.substr(1, data.length() - 2)};
            }
            else if (data[0] == '\'')
            {
                return {"char", data.substr(1, data.length() - 2)};
            }
            else if (data == "true" || data == "false")
            {
                return {"bool", data};
            }
            else
            {
                return {(std::stof(data) == std::stoi(data) ? "int" : "float"), data};
            }
        }
        catch (std::invalid_argument e)
        {
            throw std::string("Failed to retrieve literal");
        }
    }

    VariableData VM::getVariableData(const std::string &address)
    {
        if (isAddress(address))
        {
            AddressInfo addressData = addressInfo(address);
            return {addressData.type, getDataFromMemory(addressData)};
        }
        else
        {
            return getLiteralData(address);
        }
    }

    std::string VM::getDataFromMemory(AddressInfo addressData)
    {
        try
        {
            if (addressData.scopeKind == willow::symbols::GLOBAL)
            {
                return globalMemory[addressData.internal_address];
            }
            else if (addressData.scopeKind == willow::symbols::LOCAL)
            {
                return memoryStack.top().localMemory[addressData.internal_address];
            }
            else
            {
                return memoryStack.top().tempMemory[addressData.internal_address];
            }
        }
        catch (std::invalid_argument e)
        {
            throw std::string("Failed to retrieve memory at address");
        }
    }

    std::string VM::getDataFromMemory(const std::string &address)
    {
        AddressInfo addressData = addressInfo(address);
        return getDataFromMemory(addressData);
    }

    void VM::setDataInMemory(const std::string &address, std::string data)
    {
        std::cout << "setting data " << data << " in memory address " << address << " aka memory slot " << mm.internalAddress(getAddress(address)) << std::endl;
        if (!isAddress(address))
        {
            throw std::string("Tried to set data in a non-address");
        }
        if (isAddress(data))
        {
            throw std::string("Tried to set non-data in an address");
        }

        AddressInfo addressData = addressInfo(address);

        if (addressData.scopeKind == willow::symbols::GLOBAL)
        {
            if (globalMemory.size() <= addressData.internal_address)
            {
                globalMemory.reserve(addressData.internal_address + 1);
            }

            globalMemory[addressData.internal_address] = data;
        }
        else if (addressData.scopeKind == willow::symbols::LOCAL)
        {
            if (memoryStack.top().localMemory.size() <= addressData.internal_address)
            {
                memoryStack.top().localMemory.reserve(addressData.internal_address + 1);
            }
            memoryStack.top().localMemory[addressData.internal_address] = data;
        }
        else
        {
            if (memoryStack.top().tempMemory.size() <= addressData.internal_address)
            {
                memoryStack.top().tempMemory.reserve(addressData.internal_address + 1);
            }
            memoryStack.top().tempMemory[addressData.internal_address] = data;
        }

        std::cout << "finished assigning data " << data << " in address " << address << std::endl;
    }

    void VM::assignToAddress(std::string src, std::string target)
    {
        std::cout << "Assign to address " << target << std::endl;
        std::string src_data = src;

        if (isAddress(src_data))
        {
            src_data = getDataFromMemory(src);
        }
        setDataInMemory(target, src_data);
    }

    void VM::run()
    {
        std::cout << "Started running willow VM!" << std::endl;
        try
        {
            while (!hasEnded && instruction_pointer < instructions.size())
            {
                Quadruple currQuad = instructions[instruction_pointer];

                std::cout << "Executing quadruple " << instruction_pointer << " with op " << currQuad.op << std::endl;

                if (!ops.count(currQuad.op))
                {
                    throw std::string("Invalid operation " + currQuad.op);
                }

                ops[currQuad.op](this);
            }
            std::cout << "Stopped running willow VM" << std::endl;
        }
        catch (std::string msg)
        {
            std::cerr << "Error: " << msg << std::endl;
        }
    }
}