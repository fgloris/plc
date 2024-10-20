#pragma once
#include <filesystem>
#include "ast.hpp"

namespace plc{

class GrammarInterpreter{
    public:
    GrammarInterpreter() = default;
    GrammarInterpreter(const std::vector<Token>& tokens);
    GrammarInterpreter(const std::vector<Token>& tokens, std::string log_file_name);

    [[nodiscard]] Result<std::pair<size_t,AST>> interpretProgram(size_t n) noexcept;

    private:
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretBlock(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretConstDecl(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretVarDecl(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretStatement(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretStatementSequence(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretCondition(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretExpression(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretTerm(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretFactor(size_t n);
    [[nodiscard]] Result<std::pair<size_t,AST>> interpretProcedure(size_t n);
    void error(const std::string& name, size_t n);

    std::vector<std::string> symbol_table;
    std::vector<Token> token_list;
    std::ofstream log_file;
};

}