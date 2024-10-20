#pragma once
#include <filesystem>
#include "ast.hpp"

namespace plc{

class GrammarInterpreter{
    public:
    GrammarInterpreter() = default;
    GrammarInterpreter(const std::vector<Token>& tokens);
    GrammarInterpreter(const std::vector<Token>& tokens, std::string log_file_name);

    [[nodiscard]] Result<size_t> interpretProgram(size_t n) noexcept;

    private:
    [[nodiscard]] Result<size_t> interpretBlock(size_t n);
    [[nodiscard]] Result<size_t> interpretConstDecl(size_t n);
    [[nodiscard]] Result<size_t> interpretVarDecl(size_t n);
    [[nodiscard]] Result<size_t> interpretStatement(size_t n);
    [[nodiscard]] Result<size_t> interpretStatementSequence(size_t n);
    [[nodiscard]] Result<size_t> interpretCondition(size_t n);
    [[nodiscard]] Result<size_t> interpretExpression(size_t n);
    [[nodiscard]] Result<size_t> interpretTerm(size_t n);
    [[nodiscard]] Result<size_t> interpretFactor(size_t n);
    [[nodiscard]] Result<size_t> interpretProcedure(size_t n);
    void error(const std::string& name, size_t n);

    std::vector<std::string> symbol_table;
    std::vector<Token> token_list;
    std::ofstream log_file;
};

}