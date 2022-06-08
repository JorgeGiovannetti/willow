#include <iostream>
#include <string>
#include <functional>

#include <willow/vm/VM.hpp>

#include "internal/arithmetics.cpp"
#include "internal/logic.cpp"

namespace willow::vm::operations
{

    // Utils

    void binaryOperation(VM *vm, std::function<std::string(VariableData, VariableData)> oper)
    {
        Quadruple currQuad = vm->getCurrentQuad();

        if (!vm->isAddress(currQuad.targetAddress))
        {
            throw std::string("Expected address as target of operation " + currQuad.op);
        }

        if (currQuad.address1 == "" || currQuad.address2 == "")
        {
            throw std::string("Binary operation expected two operands");
        }

        VariableData op1 = vm->getVariableData(currQuad.address1);
        VariableData op2 = vm->getVariableData(currQuad.address2);

        std::string result = oper(op1, op2);
        std::cout << "operation result: " << result << std::endl;

        vm->assignToAddress(result, currQuad.targetAddress);
    }

    // VM Operations

    void endExecution(VM *vm)
    {
        vm->hasEnded = true;
        std::cout << "Ending execution through end operation" << std::endl;
    }

    void gotoInstruction(VM *vm)
    {
        std::string target_str = vm->instructions[vm->instruction_pointer].targetAddress;
        try
        {
            int targetInstruction = std::stoi(target_str);
            vm->instruction_pointer = targetInstruction;
            std::cout << "goto going to " << target_str << std::endl;
        }
        catch (const std::invalid_argument &e)
        {
            throw std::string("Expected integer index at goto, received: " + target_str);
        }
    }

    void gotofInstruction(VM *vm)
    {
        Quadruple currQuad = vm->getCurrentQuad();
        std::string data = currQuad.address1;
        if (vm->isAddress(data))
        {
            data = vm->getDataFromMemory(currQuad.address1);
        }

        if (data == "false")
        {
            gotoInstruction(vm);
        }
    }

    // Arithmetics

    void assign(VM *vm)
    {
        Quadruple currQuad = vm->getCurrentQuad();

        vm->assignToAddress(currQuad.address1, currQuad.targetAddress);

        vm->instruction_pointer++;
    }

    void sum(VM *vm)
    {
        std::cout << "calling sum..." << std::endl;

        binaryOperation(vm, &internal::sum_internal);
        vm->instruction_pointer++;
    }

    void subtract(VM *vm)
    {
        std::cout << "calling subtract..." << std::endl;

        binaryOperation(vm, &internal::subtract_internal);
        vm->instruction_pointer++;
    }

    void multiply(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "calling multiply..." << std::endl;

        binaryOperation(vm, &internal::multiply_internal);
        vm->instruction_pointer++;
    }

    void divide(VM *vm)
    {
        std::cout << "calling divide..." << std::endl;

        binaryOperation(vm, &internal::divide_internal);
        vm->instruction_pointer++;
    }

    void modulo(VM *vm)
    {
        std::cout << "calling modulo..." << std::endl;

        binaryOperation(vm, &internal::modulo_internal);
        vm->instruction_pointer++;
    }

    // Logic

    void greater(VM *vm)
    {
        std::cout << "calling greater..." << std::endl;

        binaryOperation(vm, &internal::greater_internal);
        vm->instruction_pointer++;
    }

    void geq(VM *vm)
    {
        std::cout << "calling geq..." << std::endl;

        binaryOperation(vm, &internal::geq_internal);
        vm->instruction_pointer++;
    }

    void lesser(VM *vm)
    {
        std::cout << "calling lesser..." << std::endl;

        binaryOperation(vm, &internal::lesser_internal);
        vm->instruction_pointer++;
    }

    void leq(VM *vm)
    {
        std::cout << "calling leq..." << std::endl;

        binaryOperation(vm, &internal::leq_internal);
        vm->instruction_pointer++;
    }

    void eq(VM *vm)
    {
        std::cout << "calling eq..." << std::endl;

        binaryOperation(vm, &internal::eq_internal);
        vm->instruction_pointer++;
    }

    void neq(VM *vm)
    {
        std::cout << "calling neq..." << std::endl;

        binaryOperation(vm, &internal::neq_internal);
        vm->instruction_pointer++;
    }

    // I/O

    void write(VM *vm)
    {
        std::cout << "calling write..." << std::endl;

        Quadruple currQuad = vm->getCurrentQuad();
        std::string output = vm->getDataFromMemory(currQuad.targetAddress);

        // Willow output
        std::cout << output << std::endl;
    }

    void read(VM *vm)
    {
        std::cout << "calling read..." << std::endl;

        Quadruple currQuad = vm->getCurrentQuad();

        // TODO: Check type for cin
    }

    void readln(VM *vm)
    {
        std::cout << "calling readln..." << std::endl;

        Quadruple currQuad = vm->getCurrentQuad();

        if (!vm->isAddress(currQuad.targetAddress))
        {
            throw std::string("Expected an address in readln");
        }

        AddressInfo addressInfo = vm->addressInfo(currQuad.targetAddress);

        if (addressInfo.type != "string")
        {
            throw std::string("Readln expected a string, got " + addressInfo.type);
        }

        std::string input_str;
        std::getline(std::cin, input_str);

        vm->assignToAddress(input_str, currQuad.targetAddress);
    }
}