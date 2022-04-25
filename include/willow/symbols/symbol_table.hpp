#pragma once

#include <memory>
#include <unordered_map>
#include <string>

using std::shared_ptr, std::unordered_map;

namespace willow::symbols 
{

    enum ScopeKind
    {
        GLOBAL,
        LOCAL,
        CLASS,
        FUNCTION
    };

    struct Scope
    {
        uint16_t id;
        ScopeKind kind;
    };

    struct Type
    {

    };

    struct STData
    {
        Type type;
        std::string name;
    };

    struct STNode
    {
        std::unordered_map<std::string, STData> data;
        std::shared_ptr<STNode> left, right, parent;
    };

    class SymbolTable
    {
    public:
        SymbolTable(SymbolTable const&) = delete;
        SymbolTable& operator=(SymbolTable const&) = delete;

        static std::shared_ptr<SymbolTable> instance();
        STData lookup(Type, Scope, std::string);
        void insert(std::string, Type);
        void setScope(Scope);
        void deleteScope(Scope);
    private:
        Scope currentScope;

        SymbolTable();
        static SymbolTable* mInstance;
    };
}