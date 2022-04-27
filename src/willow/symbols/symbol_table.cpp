#include <willow/symbols/symbol_table.hpp>

#include <memory>
#include <algorithm>

namespace willow::symbols
{

    SymbolTable::SymbolTable()
    {
        Scope global = {0, GLOBAL};
        globalScope = std::make_shared<Scope>(global);
        scopeCounter = 1;
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

    void SymbolTable::createScope(ScopeKind scopeKind)
    {
        if (scopeKind == GLOBAL)
        {
            throw "ERROR: CANNOT DEFINE A GLOBAL SCOPE"; // TODO: Create willow errors
        }

        Scope scope = {scopeCounter, scopeKind};
        std::shared_ptr<Scope> targetParent;

        if (scopeKind == LOCAL)
        {
            targetParent = std::make_shared<Scope>(currentScope);
        }
        else
        {
            targetParent = globalScope;
        }

        scope.parent = targetParent;
        targetParent->children.push_back(std::make_shared<Scope>(scope));

        scopeCounter++;
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