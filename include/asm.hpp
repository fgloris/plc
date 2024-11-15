#pragma once

#include "ast.hpp"

namespace plc {

struct Label{
    std::string name;
    std::vector<std::string> lines;
    Label(const std::string& name);
    explicit operator std::string() const;
};

struct Section{
    std::string name;
    std::vector<Label> labels;
    std::vector<std::string> lines;
    explicit operator std::string() const;
};

class ASMGenerator {
    public:
    virtual Result<std::string> generate(const AST& input) = 0;
    virtual Result<int> generate(const AST& input, const std::string &asmfile, const std::string &objfile = "a.o", const std::string &exefile = "a.out") = 0;
};

class NASMLinuxELF64 : public ASMGenerator{
    public:
    Result<int> generate(const AST& input, size_t label_ptr);
    Result<std::string> generate(const AST& input) override;
    Result<int> generate(const AST& input, const std::string &asmfile, const std::string &objfile, const std::string &exefile) override;
    private:
    Section text,bss,data;
};

}