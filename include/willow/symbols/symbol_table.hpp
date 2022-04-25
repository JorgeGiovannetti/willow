#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "scope.hpp"

using std::shared_ptr, std::unordered_map;

namespace willow::symbols {
    struct type {

    };
    
    struct STData {
        type type;
        std::string name;
    };

    struct STNode {
        std::unordered_map<std::string, STData> data;
        std::shared_ptr<STNode> left, right, parent;
    };

    class SymbolTable {
        public:
            STData lookup(type, Scope, std::string);
            void insert(std::string, type);
            void setScope(Scope);
            void deleteScope(Scope);
        private:
            Scope currentScope;
    };
}