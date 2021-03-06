#include <memory>
#include <algorithm>

#include "willow/symbols/symbol_table.hpp"

namespace willow::symbols
{

    SymbolTable::SymbolTable() : globalScope(std::make_shared<Scope>(0, GLOBAL))
    {
        currentScope = globalScope;
        scopeCounter = 1;
    }

    std::shared_ptr<SymbolTable> SymbolTable::instance()
    {
        static std::shared_ptr<SymbolTable> s{new SymbolTable};
        return s;
    }

    Symbol SymbolTable::lookup(std::string id)
    {

        if (currentScope->symbols.count(id))
        {
            return currentScope->symbols[id];
        }

        std::shared_ptr<Scope> curr = currentScope;

        while (curr->parent)
        {
            curr = curr->parent;

            if (curr->symbols.count(id))
            {
                return curr->symbols[id];
            }
        }

        throw std::string("identifier " + id + " is undefined");
    }

    void SymbolTable::insert(std::string id, std::string type, std::string address, std::vector<Dim> dims)
    {
        if (currentScope->symbols.count(id))
        {
            throw std::string("identifier " + id + " is already defined");
        }

        currentScope->symbols[id] = {id, type, address, dims};
    }

    void SymbolTable::add_func_return(std::string id, std::string type, std::string address, std::vector<Dim> dims)
    {
        id = "_" + id;
        if (globalScope->symbols.count(id))
        {
            throw std::string("function return " + id + " is already defined");
        }

        globalScope->symbols[id] = {id, type, address, dims};
    }

    std::shared_ptr<Scope> SymbolTable::createScope(ScopeKind scopeKind)
    {
        if (scopeKind == GLOBAL)
        {
            throw std::string("INTERNAL ERROR: CANNOT DEFINE A GLOBAL SCOPE");
        }

        std::shared_ptr<Scope> scope = std::make_shared<Scope>(scopeCounter, scopeKind);
        std::shared_ptr<Scope> targetParent;

        if (scopeKind == LOCAL || (currentScope->kind == CLASS && scopeKind == FUNCTION))
        {
            targetParent = currentScope;
        }
        else
        {
            targetParent = globalScope;
        }

        scope->parent = targetParent;
        targetParent->children.push_back(scope);
        currentScope = scope;

        scopeCounter++;
        return scope;
    }

    void SymbolTable::setScope(std::shared_ptr<Scope> scope)
    {
        currentScope = scope;
    }

    void SymbolTable::exitScope()
    {
        if (currentScope->kind != GLOBAL)
        {
            currentScope = currentScope->parent;
        }
    }

    void SymbolTable::deleteScope(std::shared_ptr<Scope> scope)
    {
        if (scope->kind == GLOBAL)
        {
            throw std::string("INTERNAL ERROR: CANNOT DELETE GLOBAL SCOPE");
        }

        // Delete children
        for (auto &s : scope->children)
        {
            deleteScope(s);
            s.reset();
        }

        if (scope->uid == currentScope->uid)
        {
            currentScope = currentScope->parent;
        }

        // Delete from parent
        auto &p = scope->parent->children;
        for (auto it = p.begin(); it != p.end(); ++it)
        {
            if ((*it)->uid == scope->uid)
            {
                it->reset();
                it = p.erase(it);
                --it;
                break;
            }
        }
    }

    ScopeKind SymbolTable::getCurrentScopeKind()
    {
        return currentScope->kind;
    }
}