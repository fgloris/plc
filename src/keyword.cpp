#include <utility>

#include "../include/keyword.hpp"

namespace plc {

Token::Token(TokenType type, std::string value): type_(type), value_(std::move(value)){}

KeyWordInterpreter::KeyWordInterpreter(const std::vector<std::pair<TokenType, std::string>> &keyword_regex_pair): keyword_regex_pair_(keyword_regex_pair){}

KeyWordInterpreter::KeyWordInterpreter(){
    keyword_regex_pair_ = std::vector<std::pair<TokenType, std::string>>();
    keyword_regex_pair_.emplace_back(TokenType::Keyword, "(begin)|(end)|(if)|(then)|(while)|(do)|(procedure)|(call)|(const)|(var)|(odd)");
    keyword_regex_pair_.emplace_back(TokenType::Delimiter, ":=|\\.|;|,|\\(|\\)");
    keyword_regex_pair_.emplace_back(TokenType::Operator, ">=||<=|<>|>|=|<|\\+|-|/|\\*");
    keyword_regex_pair_.emplace_back(TokenType::Literal, "([1-9]\\d*|0)");
    keyword_regex_pair_.emplace_back(TokenType::Identifier, "([[:alpha:]])(\\w)*");
}

Result<Token> KeyWordInterpreter::interpret(const std::string &input) const noexcept{
    if (input.empty()) return Error<Token>(ErrorType::Empty);
    using pair = std::pair<TokenType, std::string>;
    for (const pair &pair : keyword_regex_pair_){
        const auto &regex_str = std::get<std::string>(pair);
        try{
            std::regex regex(regex_str);
            if (std::regex_match(input, regex)){
                return Ok(Token(std::get<TokenType>(pair), input));
            }
        }catch (std::regex_error& e){
            return Error<Token>(ErrorType::RegexError);
        }
    }
    return Error<Token>(ErrorType::InvalidSyntax);
}

Result<Token> KeyWordInterpreter::interpretCheckAmbiguity(const std::string &input) const noexcept{
    if (input.empty()) return Error<Token>(ErrorType::Empty);
    using Pair = std::pair<TokenType, std::string>;
    Result<Token> res(ErrorType::InvalidSyntax);
    for (const Pair &pair : keyword_regex_pair_){
        const auto &regex_str = std::get<std::string>(pair);
        try{
            std::regex regex(regex_str);
            if (std::regex_match(input, regex)){
                if (res.isOk) return Error<Token>(ErrorType::Ambiguity);
                res = Ok(Token(std::get<TokenType>(pair), input));
            }
        }catch (std::regex_error& e){
            return Error<Token>(ErrorType::RegexError);
        }
    }
    return res;
}

Result<std::string> KeyWordInterpreter::splitStream(const std::string &input) noexcept{
    try{
        std::regex patten = std::regex("([^a-zA-Z0-9_ ])([^a-zA-Z0-9_ ])");
        std::string res = std::regex_replace(input,patten,"$1 $2");
        while (std::regex_replace(res,patten,"$1 $2") != res){
            res = std::regex_replace(res,patten,"$1 $2");
        }
        patten = std::regex("([\\w])([^a-zA-Z0-9_ ])");
        while (res != std::regex_replace(res,patten,"$1 $2")){
            res = std::regex_replace(res,patten,"$1 $2");
        }
        patten = std::regex("([^a-zA-Z0-9_ ])([\\w])");
        while (res != std::regex_replace(res,patten,"$1 $2")){
            res = std::regex_replace(res,patten,"$1 $2");
        }

        patten = std::regex("(:|>|<) =");
        res = std::regex_replace(res,patten,"$1=");
        patten = std::regex("< >");
        res = std::regex_replace(res,patten,"<>");
        std::cout<<res<<std::endl;

        return Ok(res);
    }catch (std::regex_error& e){
        return Error<std::string>(ErrorType::RegexError);
    }
}

Result<std::vector<Token>> KeyWordInterpreter::interpretStream(const std::string &input) const noexcept{
    Result<std::string> spl = splitStream(input);
    if (!spl.isOk) {return Error<std::vector<Token>>(spl);}
    std::stringstream stream(spl.unwrap());
    std::string w;
    std::vector<Token> res;
    while (stream >> w){
        Result<Token> token_res = interpret(w);
        if (!token_res.isOk) return Error<std::vector<Token>>(token_res);
        res.emplace_back(token_res.unwrap());
    }
    return Ok(res);
}

Result<std::vector<Token>> KeyWordInterpreter::interpretFile(const std::string &filename) const noexcept{
    std::ifstream f(filename);
    if (!f) return Result<std::vector<Token>>(ErrorType::IOError);
    std::stringstream stream;
    stream << f.rdbuf();
    return interpretStream(stream.str());
}

bool Token::operator==(const Token &other) const{
    return (type_==other.type_ && value_==other.value_);
}

Token::operator std::string() const {
        std::string type_str;
        switch(type_){
            case TokenType::Keyword:
                type_str = "Keyword";
                goto end;
            case TokenType::Identifier:
                type_str = "Identifier";
                goto end;
            case TokenType::Literal:
                type_str = "Literal";
                goto end;
            case TokenType::Operator:
                type_str = "Operator";
                goto end;
            case TokenType::Delimiter:
                type_str = "Delimiter";
                goto end;
            case TokenType::EndOfFile:
                type_str = "EndOfFile";
                goto end;
            default:
                type_str = "Unknown";
                goto end;
        }
        end:return type_str+"(" + value_ + ")";
    }
}