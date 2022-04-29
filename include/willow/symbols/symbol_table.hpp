#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace willow::symbols
{

    enum ScopeKind
    {
        GLOBAL,
        LOCAL,
        CLASS,
        FUNCTION
    };

    struct Type
    {
        std::string name;
    };

    struct Symbol
    {
        Type type;
        std::string id;
    };

    struct Scope
    {
        uint16_t uid;
        ScopeKind kind;
        std::unordered_map<std::string, Symbol> symbols;
        std::shared_ptr<Scope> parent;
        std::vector<std::shared_ptr<Scope>> children;
        Scope(uint16_t id, ScopeKind k) : uid{id}, kind{k}
        {
        }
    };

    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable const &) = delete;
        SymbolTable &operator=(SymbolTable const &) = delete;
        static std::shared_ptr<SymbolTable> instance();

        Symbol lookup(Type, std::string);
        void insert(std::string, Type);
        std::shared_ptr<Scope> createScope(ScopeKind);
        void setScope(std::shared_ptr<Scope>);
        void exitScope();
        void deleteScope(std::shared_ptr<Scope>);

    private:
        uint16_t scopeCounter;
        std::shared_ptr<Scope> currentScope;
        const std::shared_ptr<Scope> globalScope;

        SymbolTable();
    };
}