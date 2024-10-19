#pragma once

#include <iostream>
#include <regex>
#include <utility>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include "error.hpp"

namespace plc{

enum class TokenType{
    Keyword     = 0,
    Identifier  = 1,
    Literal     = 3,
    Delimiter   = 4,
    Operator    = 5,
    EndOfFile   = 6,
};

class Token{
    public:
    TokenType type_;
    std::string value_;
    Token() = default;
    Token(TokenType type, std::string value);
    explicit operator std::string() const;
    bool operator==(const Token &other) const;
};

class KeyWordInterpreter{
    public:
    KeyWordInterpreter();
    explicit KeyWordInterpreter(const std::vector<std::pair<TokenType, std::string>> &keyword_regex_pair);

    [[nodiscard]] Result<Token> interpret(const std::string &input) const noexcept;
    [[nodiscard]] Result<Token> interpretCheckAmbiguity(const std::string &input) const noexcept;
    [[nodiscard]] Result<std::vector<Token>> interpretStream(const std::string &input) const noexcept;
    [[nodiscard]] Result<std::vector<Token>> interpretFile(const std::string &filename) const noexcept;

    static Result<std::string> splitStream(const std::string &input) noexcept;

    public:
    std::vector<std::pair<TokenType, std::string>> keyword_regex_pair_;

    private:
    bool debug_ = true;
};

}
