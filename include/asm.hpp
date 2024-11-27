#pragma once

#include "ast.hpp"

namespace plc {

struct Label{
    std::string name;
    std::vector<std::string> lines;
    Label(const std::string& name);
    explicit operator std::string() const;
    bool operator==(const Label& other) const;
};

struct Section{
    std::string name;
    std::vector<Label> labels;
    std::vector<std::string> lines;
    Section(const std::string& name);
    explicit operator std::string() const;
    void addLine(size_t label_ptr, const std::string& line);
};

template<typename T>
struct MacroConstant{
    std::string name;
    T value;
    MacroConstant(const std::string& name, const T& value);
    bool operator==(const std::string& other) const;
};

class ASMGenerator {
    public:
    virtual Result<std::string> generate(const AST& input) = 0;
    virtual Result<int> generate(const AST& input, const std::string &asmfile, const std::string &objfile = "a.o", const std::string &exefile = "a.out") = 0;
};

class NASMLinuxELF64 : public ASMGenerator{
    public:
    NASMLinuxELF64();
    Result<int> generate(const AST& input, size_t label_ptr);
    Result<std::string> generate(const AST& input) override;
    Result<int> generate(const AST& input, const std::string &asmfile, const std::string &objfile, const std::string &exefile) override;
    private:
    Result<std::string> getVarPos(const std::string& var, bool left = false);
    std::vector<MacroConstant<int>> constants;
    std::vector<std::string> vars;
    Section text,bss,data;
};

class WASMWin32 : public ASMGenerator{
    public:
};

}