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
};

class Token{
    public:
    TokenType type_;
    std::string value_;
    Token() = default;
    Token(TokenType type, const std::string &value);
    operator std::string() const;
};

class KeyWordInterpreter{
    public:
    KeyWordInterpreter();
    explicit KeyWordInterpreter(const std::vector<std::pair<TokenType, std::string>> &keyword_regrex_pair_);

    Result<Token> interpret(const std::string &input) const;
    Result<Token> interpretCheckAmbiguity(const std::string &input) const;
    Result<std::vector<Token>> interpretStream(const std::string &input) const;
    Result<std::vector<Token>> interpretFile(const std::string &filename) const;

    Result<std::string> splitStream(const std::string &input) const;

    public:
    std::vector<std::pair<TokenType, std::string>> keyword_regrex_pair_;

    private:
    bool debug_ = true;
};

}
