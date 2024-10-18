#include "../include/grammar.hpp"

namespace plc{

GrammarInterpreter::GrammarInterpreter(const std::vector<Token>& tokens):token_list(tokens){}

void GrammarInterpreter::debug(const std::string& name, size_t n) const{
    std::cout<<name<<" : "<<(std::string)token_list[n]<<"("<<n<<")"<<std::endl;
}

Result<size_t> GrammarInterpreter::interpretProgram(size_t n){
    Result<size_t> res = interpretBlock(n);
    if (!res.isOk) return res;
    n = res.unwrap();
    if (token_list[n] == Token(TokenType::Delimiter, ".")) return Ok(n);
    else return Error<size_t>(ErrorType::InvalidSyntax);
}

Result<size_t> GrammarInterpreter::interpretBlock(size_t n){
    debug("start block",n);
    while (token_list[n].value_ != "."){
        std::string& sym = token_list[n].value_;
        Result<size_t> res = Ok(std::size_t{0});
        if (sym == "const")     res = GrammarInterpreter::interpretConstDecl(n+1);
        else if (sym == "var")  res = GrammarInterpreter::interpretVarDecl(n+1);
        else if (sym == "procedure")  res = GrammarInterpreter::interpretProcedure(n+1);
        else if (sym == "begin"){
            res = GrammarInterpreter::interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            else n = res.unwrap();
            debug("end block", n);
            return Ok(n+1);
        }
        
        else return Error<size_t>(ErrorType::InvalidSyntax);

        if (!res.isOk) return res;
        else n = res.unwrap();
    }
    debug("end block", n);
    return Ok(n);
}

Result<size_t> GrammarInterpreter::interpretConstDecl(size_t n){
    while (1){
        if (token_list[n].type_ != TokenType::Identifier){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].value_ !=  "="){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].type_ != TokenType::Literal){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }else if (token_list[n].value_ ==  ";"){
            break;
        }
        n++;
    }
    return Ok(n+1);
}

Result<size_t> GrammarInterpreter::interpretVarDecl(size_t n){
    while (1){
        if (token_list[n].type_ != TokenType::Identifier){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }else if (token_list[n].value_ ==  ";"){
            break;
        }
        n++;
    }
    return Ok(n+1);
}

Result<size_t> GrammarInterpreter::interpretProcedure(size_t n){
    if (token_list[n].type_ != TokenType::Identifier){
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    n++;
    if (token_list[n].value_ !=  ";"){
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    Result<size_t> res = interpretBlock(n+1);
    if (!res.isOk) return res;
    n = res.unwrap();
    if (token_list[n].value_ !=  ";"){
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    return Ok(n+1);
}

Result<size_t> GrammarInterpreter::interpretStatementSequence(size_t n){
    debug("start seq",n);
    while (1){
        Result<size_t> res = interpretStatement(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ !=  ";"){
            debug("end seq",n);
            return Ok(n);
        }
        n++;
    }
}

Result<size_t> GrammarInterpreter::interpretStatement(size_t n){
    debug("start statement", n);
    if (token_list[n].type_ == TokenType::Identifier){
        n++;
        if (token_list[n].value_ != ":="){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        Result<size_t> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        debug("end state", n);
        return Ok(n);
    }else{
        if (token_list[n].value_ == "call"){
            n++;
            if (token_list[n].type_ != TokenType::Identifier){
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }else if (token_list[n].value_ == "begin"){
            Result<size_t> res = GrammarInterpreter::interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "end"){
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }
    }
}

Result<size_t> GrammarInterpreter::interpretExpression(size_t n){
    if (token_list[n].value_ == "+" || token_list[n].value_ == "-"){
        n++;
    }
    debug("start exp",n);
    while (1){
        Result<size_t> res = GrammarInterpreter::interpretTerm(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != "+" && token_list[n].value_ != "-"){
            debug("end exp",n);
            return Ok(n);
        }
        n++;
    }
}

Result<size_t> GrammarInterpreter::interpretTerm(size_t n){
    debug("start term",n);
    while (1){
        Result<size_t> res = GrammarInterpreter::interpretFactor(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != "*" && token_list[n].value_ != "/"){
            debug("end term",n);
            return Ok(n);
        }
        n++;
    }
}

Result<size_t> GrammarInterpreter::interpretFactor(size_t n){
    debug("start factor",n);
    if (token_list[n].value_ == "("){
        Result<size_t> res = GrammarInterpreter::interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != ")"){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        return Ok(n+1);
    }else if (token_list[n].type_ == TokenType::Literal){
        return Ok(n+1);
    }else if (token_list[n].type_ == TokenType::Identifier){
        debug("end factor",n);
        return Ok(n+1);
    }
}

}