#pragma once

#include "keyword.hpp"

namespace plc {

struct Quaternary{
    std::string cmd,value1,value2,result;
    Quaternary() = delete;
    Quaternary(std::string cmd, std::string value1, std::string value2, std::string result);
    explicit operator std::string() const;
};

class AST{
public:
    AST(std::string name);
    AST(std::string name, AST child1);
    AST(std::string name, AST child1, AST child2);
    AST(std::string name, AST child1, AST child2, AST child3);
    AST(std::string name, std::vector<AST> children);
    void addChild(AST child);
    void addChild(std::string childname);
    void print(std::ofstream& log_file) const;
    Result<std::string> compile();
    Result<size_t> output(std::string log_file_name) const;
    static std::string getTempName();

public:
    std::string name;
    std::vector<AST> children;
    static size_t temp_name;
    static std::vector<Quaternary> code;
    static std::map<std::string,size_t> procedure_line;
};

Result<std::pair<size_t, AST>> ErrorPair(ErrorType err);

}