#include <willow/symbols/symbol_table.hpp>

#include <memory>
#include <algorithm>

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

        throw std::string("identifier " + id +" is undefined"); // TODO: Create willow errors
    }

    void SymbolTable::insert(std::string id, std::string type, std::string address, std::vector<int> dims)
    {
        if (currentScope->symbols.count(id))
        {
            throw std::string("identifier " + id + " is already defined"); // TODO: Create willow errors
        }

        currentScope->symbols[id] = {id, type, address, dims};
    }

    std::shared_ptr<Scope> SymbolTable::createScope(ScopeKind scopeKind)
    {
        if (scopeKind == GLOBAL)
        {
            throw std::string("INTERNAL ERROR: CANNOT DEFINE A GLOBAL SCOPE"); // TODO: Create willow errors
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
            throw std::string("INTERNAL ERROR: CANNOT DELETE GLOBAL SCOPE"); // TODO: Create willow errors
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
}