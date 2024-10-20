#include "../include/grammar.hpp"

namespace plc{

GrammarInterpreter::GrammarInterpreter(const std::vector<Token>& tokens):token_list(tokens){
    for (int i = 0; i < 5; i++){
        token_list.push_back(Token{TokenType::EndOfFile,"eof"});
    }
}

GrammarInterpreter::GrammarInterpreter(const std::vector<Token>& tokens, std::string log_file_name):token_list(tokens),log_file(std::move(log_file_name)){
    for (int i = 0; i < 5; i++){
        token_list.push_back(Token{TokenType::EndOfFile,"eof"});
    }
    if (!log_file.is_open()){
        std::cerr<<"Error: unable to open log file."<<std::endl;
        exit(1);
    }
}

void GrammarInterpreter::error(const std::string& name, size_t n){
    std::string error_msg(name + " at token " + static_cast<std::string>(token_list[n]) + "(" + std::to_string(n) + ")\n");
    log_file << error_msg;
    std::cerr << error_msg;
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretProgram(size_t n) noexcept{
    Result<std::pair<size_t,AST>> res = interpretBlock(n);
    if (!res.isOk){
        log_file << "Program failed to interpret." << std::endl;
        return res;
    }
    
    n = res.unwrap();
    if (token_list.size() >= n && token_list[n].value_ != "."){
        error("expecting '.'",n);
        log_file << "Program failed to interpret." << std::endl;
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    log_file << "Program successfully interpreted." << std::endl;
    log_file.close();
    return Ok(n);
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretBlock(size_t n){
    while (token_list[n].value_ != "."){
        std::string& sym = token_list[n].value_;
        Result<std::pair<size_t,AST>> res = Ok(std::size_t{0});
        if (sym == "const")     res = interpretConstDecl(n+1);
        else if (sym == "var")  res = interpretVarDecl(n+1);
        else if (sym == "procedure")  res = interpretProcedure(n+1);
        else if (sym == "begin"){
            res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            else n = res.unwrap();
            if (token_list[n].value_ != "end"){
                error("expecting 'end'",n);
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }
        else{
            error("invalid symbol",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }

        if (!res.isOk) return res;
        else n = res.unwrap();
    }
    return Ok(n);
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretConstDecl(size_t n){
    while (1){
        if (token_list[n].type_ != TokenType::Identifier){
            error("expecting identifier",n);
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

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretVarDecl(size_t n){
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

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretProcedure(size_t n){
    if (token_list[n].type_ != TokenType::Identifier){
        error("expecting identifier",n);
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    n++;
    if (token_list[n].value_ !=  ";"){
        error("expecting ';'",n);
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    Result<std::pair<size_t,AST>> res = interpretBlock(n+1);
    if (!res.isOk) return res;
    n = res.unwrap();
    if (token_list[n].value_ !=  ";"){
        error("expecting ';'",n);
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
    return Ok(n+1);
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretStatementSequence(size_t n){
    while (1){
        Result<std::pair<size_t,AST>> res = interpretStatement(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ !=  ";"){
            return Ok(n);
        }
        n++;
        //my own addition
        if (token_list[n].value_ == "end"){
            return Ok(n);
        }
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretStatement(size_t n){
    if (token_list[n].type_ == TokenType::Identifier){
        n++;
        if (token_list[n].value_ != ":="){
            error("expecting ':='",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        return Ok(n);
    }else{
        if (token_list[n].value_ == "call"){
            n++;
            if (token_list[n].type_ != TokenType::Identifier){
                error("expecting identifier",n);
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }else if (token_list[n].value_ == "begin"){
            Result<std::pair<size_t,AST>> res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "end"){
                error("expecting 'end'",n);
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            return Ok(n+1);
        }else if (token_list[n].value_ == "if"){
            Result<std::pair<size_t,AST>> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "then"){
                error("expecting 'then'",n);
                return Error<size_t>(ErrorType::InvalidSyntax);
            }
            res = interpretStatement(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            return Ok(n);
        }else if (token_list[n].value_ == "while"){
            Result<std::pair<size_t,AST>> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            n = res.unwrap();
            if (token_list[n].value_ != "do"){
                error("expecting 'do'",n);
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
            error("expecting statement",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretExpression(size_t n){
    if (token_list[n].value_ == "+" || token_list[n].value_ == "-"){
        n++;
    }
    while (1){
        Result<std::pair<size_t,AST>> res = interpretTerm(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != "+" && token_list[n].value_ != "-"){
            return Ok(n);
        }
        n++;
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretTerm(size_t n){
    while (1){
        Result<std::pair<size_t,AST>> res = interpretFactor(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != "*" && token_list[n].value_ != "/"){
            return Ok(n);
        }
        n++;
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretFactor(size_t n){
    if (token_list[n].value_ == "("){
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].value_ != ")"){
            error("expecting ')'",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        return Ok(n+1);
    }else if (token_list[n].type_ == TokenType::Literal){
        return Ok(n+1);
    }else if (token_list[n].type_ == TokenType::Identifier){
        return Ok(n+1);
    }else {
        error("expecting factor",n);
        return Error<size_t>(ErrorType::InvalidSyntax);
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretCondition(size_t n){
    if (token_list[n].value_ == "odd"){
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        return Ok(n);
    }else{
        Result<std::pair<size_t,AST>> res = interpretExpression(n);
        if (!res.isOk) return res;
        n = res.unwrap();
        if (token_list[n].type_ != TokenType::Operator){
            error("expecting operator",n);
            return Error<size_t>(ErrorType::InvalidSyntax);
        }
        res = interpretExpression(n+1);
        if (!res.isOk) return res;
        n = res.unwrap();
        return Ok(n);
    }
}

}