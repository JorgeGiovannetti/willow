#include <iostream>
#include <text_table/text_table.hpp>

#include <willow/willow.hpp>

using willow::codegen::Quadruple;
using willow::symbols::SymbolTable;

namespace willow::parser
{

    State::State()
    {
        currScopeKind = willow::symbols::LOCAL;
        st = SymbolTable::instance();
        currDimPosition = 0;
        isInClass = false;
        isInFunction = false;
    }

    void State::displayQuadruples()
    {
        TextTable table('-', '|', '+');

        table.add("Index");
        table.add("Operator");
        table.add("Address 1");
        table.add("Address 2");
        table.add("Target Address");
        table.endOfRow();

        size_t i = 0;
        for (Quadruple &quad : quadruples)
        {
            table.add(std::to_string(i));
            table.add(quad.op);
            table.add(quad.address1);
            table.add(quad.address2);
            table.add(quad.targetAddress);
            table.endOfRow();
            table.setAlignment(i++, TextTable::Alignment::RIGHT);
        }

        std::cout << table << std::endl;
    }
}