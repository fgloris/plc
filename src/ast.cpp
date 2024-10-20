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
void AST::addChild(std::string childname) {children.push_back(std::move(childname));}

void AST::print(std::ofstream& log_file) const {
    if (children.empty()) {
        std::cout << name;
        log_file << name;
        return;
    }
    std::cout << name << "(";
    log_file << name << "(";
    for (auto& child : children) {
        child.print(log_file);
        if (&child!= &children.back()){
            std::cout << ", ";
            log_file << ", ";
        }
    }
    std::cout << ")";
    log_file << ")";
}

Result<std::pair<size_t, AST>> ErrorPair(ErrorType err){
    return Result(std::make_pair(size_t{0}, AST("Error")), err);
}

}