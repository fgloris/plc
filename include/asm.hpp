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

template<typename T>
struct MacroConstant{
    std::string name;
    T value;
    MacroConstant(const std::string& name, const T& value);
    bool operator==(const std::string& other) const;
};

struct Scope {
    int label_ptr;
    std::vector<MacroConstant<int>> constants;
    std::vector<std::string> vars;
    Scope *father;
    Scope();
    Scope(Scope *father);
    Scope(Scope* father, size_t label_ptr);
    Result<size_t> findVarPos(const std::string& name) const;
    Result<std::string> findVar(const std::string& var) const;
    Result<std::string> findConst(const std::string& con) const;
    Result<std::string> findRValue(const std::string& val) const;
    void addVar(const std::string& var);
    void addConst(const std::string& name, int value);
};

struct Section{
    std::string name;
    std::vector<Label> labels;
    std::vector<std::string> lines;
    explicit Section(const std::string& name);
    explicit operator std::string() const;
    void addLine(size_t label_ptr, const std::string& line);
    void addFreeScopeLine(const Scope&s);
};

class ASMGenerator {
    public:
    virtual ~ASMGenerator() = default;
    [[nodiscard]] virtual Result<std::string> generate(const AST& input) = 0;
    [[nodiscard]] virtual Result<int> compile(const AST& input, const std::string &asmfile, const std::string &objfile, const std::string &exefile) = 0;
};

class NASMLinuxELF64 : public ASMGenerator{
    public:
    NASMLinuxELF64();
    [[nodiscard]] Result<int> generate(const AST& input, Scope& s);
    [[nodiscard]] Result<std::string> generate(const AST& input) override;
    [[nodiscard]] Result<int> compile(const AST& input, const std::string &asmfile = "a.asm", const std::string &objfile = "a.o", const std::string &exefile = "a.out") override;
    std::string addTempLabelName();
    std::string getCurrentTempLabelName();
    private:
    Section text,bss,data;
    int temp_label_ptr;
};

enum class JWASMInstructionSet{
    Set8086,
    Set386,
    Set586,
};

class JWASMWin32 : public ASMGenerator{
    public:
    JWASMWin32();
    private:

};

}