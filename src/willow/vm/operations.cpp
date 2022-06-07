#include <iostream>
#include <string>
#include <any>

#include <willow/vm/VM.hpp>

namespace willow::vm
{
    void endExecution(VM *vm)
    {
        vm->hasEnded = true;
    }

    void gotoInstruction(VM *vm)
    {
        std::string target_str = vm->instructions[vm->instruction_pointer].targetAddress;
        try
        {
            int targetInstruction = std::stoi(target_str);
            vm->instruction_pointer = targetInstruction;
            std::cout << "GOTO going to " << target_str << std::endl;
        }
        catch (const std::invalid_argument &e)
        {
            throw std::string("Expected integer index at GOTO, received: " + target_str);
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

    void assign(VM *vm)
    {

        Quadruple currQuad = vm->getCurrentQuad();

        vm->assignToAddress(currQuad.address1, currQuad.targetAddress);
    }

    void sum(VM *vm)
    {
        std::cout << "TODO: right now calling sum uwu" << std::endl;


        // Quadruple currQuad = vm->getCurrentQuad();
        // AddressData addressData1 = vm->valueFromAddress(currQuad.address1);
        // AddressData addressData2 = vm->valueFromAddress(currQuad.address2);
    }

    void subtract(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling subtract uwu" << std::endl;
    }

    void multiply(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling multiply uwu" << std::endl;
    }

    void divide(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling divide uwu" << std::endl;
    }

    void modulo(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling modulo uwu" << std::endl;
    }

    void greater(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling greater uwu" << std::endl;
    }

    void geq(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling geq uwu" << std::endl;
    }

    void lesser(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling lesser uwu" << std::endl;
    }

    void leq(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling leq uwu" << std::endl;
    }

    void eq(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling eq uwu" << std::endl;
    }

    void write(VM *vm)
    {
        // TODO: Handle VM stuff
        std::cout << "TODO: right now calling write uwu" << std::endl;
    }
}