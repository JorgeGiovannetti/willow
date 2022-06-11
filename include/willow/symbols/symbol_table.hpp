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
        TEMP,
        CONSTANT,
        FUNCTION,
        CLASS
    };

    static const std::string NONE_TYPE = "none";

   struct Dim {
      int size;
      int displacement_size;
   };

    struct Symbol
    {
        std::string id;
        std::string type;
        std::string address;
        std::vector<Dim> dims;
        int currDimPosition = 0;
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

        Symbol lookup(std::string id);
        void insert(std::string id, std::string type, std::string address, std::vector<Dim> dims);
        std::shared_ptr<Scope> createScope(ScopeKind);
        void setScope(std::shared_ptr<Scope>);
        void exitScope();
        void deleteScope(std::shared_ptr<Scope>);
        ScopeKind getCurrentScopeKind();

    private:
        size_t scopeCounter;
        std::shared_ptr<Scope> currentScope;
        const std::shared_ptr<Scope> globalScope;

        SymbolTable();
    };
}