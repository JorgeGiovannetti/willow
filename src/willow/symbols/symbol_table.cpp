#include <willow/symbols/symbol_table.hpp>
#include <algorithm>

namespace willow::symbols
{

    SymbolTable::SymbolTable()
    {
    }

    std::shared_ptr<SymbolTable> SymbolTable::instance()
    {
        static std::shared_ptr<SymbolTable> s{new SymbolTable};
        return s;
    }

    Symbol SymbolTable::lookup(Type type, std::string id)
    {

        if (currentScope.symbols.count(id))
        {
            return currentScope.symbols[id];
        }

        Scope curr = currentScope;

        while (curr.parent)
        {
            curr = *curr.parent;

            if (curr.symbols.count(id))
            {
                return curr.symbols[id];
            }
        }

        throw "ERROR: SYMBOL NOT FOUND"; // TODO: Create willow errors
    }

    void SymbolTable::insert(std::string id, Type type)
    {
        if (currentScope.symbols.count(id))
        {
            throw "ERROR: SYMBOL ALREADY DEFINED"; // TODO: Create willow errors
        }

        currentScope.symbols[id] = {type, id};
    }

    void SymbolTable::setScope(Scope scope)
    {
        currentScope = scope;
    }

    void SymbolTable::deleteScope(Scope &scope)
    {
        if (scope.kind == GLOBAL)
        {
            throw "ERROR: CANNOT DELETE GLOBAL SCOPE"; // TODO: Create willow errors
        }

        // Delete from parent
        Scope p = *scope.parent;

        auto uidMatch = [scope](const std::shared_ptr<Scope> &a)
        { return a->uid == scope.uid; };
        p.children.erase(std::remove_if(p.children.begin(), p.children.end(), uidMatch), p.children.end());

        for (auto &s : scope.children)
        {
            deleteScope(*s);
            s.reset();
        }
    }
}