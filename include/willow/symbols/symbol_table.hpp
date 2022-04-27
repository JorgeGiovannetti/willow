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
    };

    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable const &) = delete;
        SymbolTable &operator=(SymbolTable const &) = delete;
        static std::shared_ptr<SymbolTable> instance();

        Symbol lookup(Type, std::string);
        void insert(std::string, Type);
        void createScope(ScopeKind);
        void setScope(Scope);
        void deleteScope(Scope &);

    private:
        Scope currentScope;
        uint16_t scopeCounter;
        std::shared_ptr<Scope> globalScope;

        SymbolTable();
    };
}