#include <willow/symbols/symbol_table.hpp>

namespace willow::symbols
{

    SymbolTable::SymbolTable()
    {
    }

    std::shared_ptr<SymbolTable> SymbolTable::instance()
    {
        static std::shared_ptr<SymbolTable> s{ new SymbolTable };
        return s;
    }

    STData SymbolTable::lookup(Type type, Scope scope, std::string id)
    {

    }
    void SymbolTable::insert(std::string id, Type type)
    {

    }
    void SymbolTable::setScope(Scope scope)
    {

    }
    void SymbolTable::deleteScope(Scope scope)
    {

    }

}