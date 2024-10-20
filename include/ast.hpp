#pragma once

#include "keyword.hpp"

namespace plc {

class AST{
    public:
    std::string name;
    std::vector<AST> children;
    AST(std::string name);
    AST(std::string name, AST child1);
    AST(std::string name, AST child1, AST child2);
    AST(std::string name, AST child1, AST child2, AST child3);
    AST(std::string name, std::vector<AST> children);
    void addChild(AST child);
};

}