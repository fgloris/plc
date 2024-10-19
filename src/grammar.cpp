#include "../include/grammar.hpp"

namespace plc{

GrammarInterpreter::GrammarInterpreter(const std::vector<Token>& tokens):token_list(tokens){}

void GrammarInterpreter::debug(const std::string& name, size_t n) const{
    std::cout<<name<<" : "<<static_cast<std::string>(token_list[n])<<"("<<n<<")"<<std::endl;
}

void GrammarInterpreter::error(const std::string& name, size_t n) const{
    std::cout<<name<<" at token "<<static_cast<std::string>(token_list[n])<<"("<<n<<")"<<std::endl;
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
        if (sym == "const")     res = interpretConstDecl(n+1);
        else if (sym == "var")  res = interpretVarDecl(n+1);
        else if (sym == "procedure")  res = interpretProcedure(n+1);
        else if (sym == "begin"){
            res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            else n = res.unwrap();
            debug("end block", n);
            return Ok(n+1);
        }
        
        else{
            error("invalid symbol",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }

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
            error("expecting '='",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].type_ != TokenType::Literal){
            error("expecting literal",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            error("expecting ',' or ';'",n);
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
            error("expecting identifier",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            error("expecting ',' or ';'",n);
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
        error("expecting identifier",n);
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    n++;
    if (token_list[n].value_ !=  ";"){
        error("expecting ';'",n);
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
        //my own addition
        if (token_list[n].value_ == "end"){
            debug("end seq",n);
            return Ok(n);
        }
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
            Result<size_t> res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "end"){
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }else if (token_list[n].value_ == "if"){
            Result<size_t> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "then"){
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            res = interpretStatement(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            return Ok(n);
        }else if (token_list[n].value_ == "while"){
            Result<size_t> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "do"){
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            res = interpretStatement(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            return Ok(n);
        }else if (token_list[n].value_ == ";"){
            //support for empty statement
            return Ok(n+1);
        }else{
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
    }
}

Result<size_t> GrammarInterpreter::interpretExpression(size_t n){
    if (token_list[n].value_ == "+" || token_list[n].value_ == "-"){
        n++;
    }
    debug("start exp",n);
    while (1){
        Result<size_t> res = interpretTerm(n);
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
        Result<size_t> res = interpretFactor(n);
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
        Result<size_t> res = interpretExpression(n+1);
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
    }else {
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
}

Result<size_t> GrammarInterpreter::interpretCondition(size_t n){
    debug("start condition",n);
    if (token_list[n].value_ == "odd"){
        Result<size_t> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        return Ok(n);
    }else{
        Result<size_t> res = interpretExpression(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].type_ != TokenType::Operator){
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        return Ok(n);
    }
}

}