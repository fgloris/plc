#include "../include/ast.hpp"

namespace plc {

AST::AST(std::string name) : name(name) {}
AST::AST(std::string name, AST child1) : name(name) {
    children.push_back(std::move(child1));
}
AST::AST(std::string name, AST child1, AST child2) : name(name) {
    children.push_back(std::move(child1));
    children.push_back(std::move(child2));
}
AST::AST(std::string name, AST child1, AST child2, AST child3) : name(name) {
    children.push_back(std::move(child1));
    children.push_back(std::move(child2));
    children.push_back(std::move(child3));
}

AST::AST(std::string name, std::vector<AST> children) : name(name), children(std::move(children)) {}

void AST::addChild(AST child) {children.push_back(std::move(child));}

}