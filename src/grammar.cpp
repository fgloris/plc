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
    AST ast("Program");
    Result<std::pair<size_t,AST>> res = interpretBlock(n);
    if (!res.isOk){
        log_file << "Program failed to interpret." << std::endl;
        return res;
    }
    std::pair<size_t,AST> pair = res.unwrap();
    n = pair.first;
    ast.addChild(pair.second);
    if (token_list.size() >= n && token_list[n].value_ != "."){
        error("expecting '.'",n);
        log_file << "Program failed to interpret." << std::endl;
        return ErrorPair(ErrorType::InvalidSyntax);
    }
    log_file << "Program successfully interpreted." << std::endl;
    std::cout<<std::endl;
    ast.print(log_file);
    log_file.close();
    return Ok(std::make_pair(n,ast));
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretBlock(size_t n){
    AST ast("Block");
    while (token_list[n].value_ != "."){
        std::string& sym = token_list[n].value_;
        Result<std::pair<size_t,AST>> res = Ok(std::make_pair(std::size_t{0},ast));
        if (sym == "const")     res = interpretConstDecl(n+1);
        else if (sym == "var")  res = interpretVarDecl(n+1);
        else if (sym == "procedure")  res = interpretProcedure(n+1);
        else if (sym == "begin"){
            res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair = res.unwrap();
            n = pair.first;
            ast.addChild(pair.second);

            if (token_list[n].value_ != "end"){
                error("expecting 'end'",n);
                return ErrorPair(ErrorType::InvalidSyntax);
            }
            return Ok(std::make_pair(n+1,ast));
        }
        else{
            error("invalid symbol",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }

        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);
    }
    return Ok(std::make_pair(n,ast));
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretConstDecl(size_t n){
    AST ast("Const");
    while (1){
        if (token_list[n].type_ != TokenType::Identifier){
            error("expecting identifier",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        ast.addChild(token_list[n].value_);
        n++;
        if (token_list[n].value_ !=  "="){
            error("expecting '='",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        n++;
        if (token_list[n].type_ != TokenType::Literal){
            error("expecting literal",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        ast.addChild(token_list[n].value_);
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            error("expecting ',' or ';'",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }else if (token_list[n].value_ ==  ";"){
            break;
        }
        n++;
    }
    return Ok(std::make_pair(n+1,ast));
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretVarDecl(size_t n){
    AST ast("Var");
    while (1){
        if (token_list[n].type_ != TokenType::Identifier){
            error("expecting identifier",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        ast.addChild(token_list[n].value_);
        n++;
        if (token_list[n].value_ !=  "," && token_list[n].value_ !=  ";"){
            error("expecting ',' or ';'",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }else if (token_list[n].value_ ==  ";"){
            break;
        }
        n++;
    }
    return Ok(std::make_pair(n+1,ast));
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretProcedure(size_t n){
    AST ast("Procedure");
    if (token_list[n].type_ != TokenType::Identifier){
        error("expecting identifier",n);
        return ErrorPair(ErrorType::InvalidSyntax);
    }
    ast.addChild(token_list[n].value_);
    n++;
    if (token_list[n].value_ !=  ";"){
        error("expecting ';'",n);
        return ErrorPair(ErrorType::InvalidSyntax);
    }
    Result<std::pair<size_t,AST>> res = interpretBlock(n+1);
    if (!res.isOk) return res;
    std::pair<size_t,AST> pair = res.unwrap();
    n = pair.first;
    ast.addChild(pair.second);

    if (token_list[n].value_ !=  ";"){
        error("expecting ';'",n);
        return ErrorPair(ErrorType::InvalidSyntax);
    }
    return Ok(std::make_pair(n+1,ast));
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretStatementSequence(size_t n){
    AST ast("Sequence");
    while (1){
        Result<std::pair<size_t,AST>> res = interpretStatement(n);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        if (token_list[n].value_ !=  ";"){
            return Ok(std::make_pair(n,ast));
        }
        n++;
        //my own addition
        if (token_list[n].value_ == "end"){
            return Ok(std::make_pair(n,ast));
        }
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretStatement(size_t n){
    if (token_list[n].type_ == TokenType::Identifier){
        AST ast("Define", token_list[n].value_);
        n++;
        if (token_list[n].value_ != ":="){
            error("expecting ':='",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        return Ok(std::make_pair(n,ast));
    }else{
        if (token_list[n].value_ == "call"){
            n++;
            if (token_list[n].type_ != TokenType::Identifier){
                error("expecting identifier",n);
                return ErrorPair(ErrorType::InvalidSyntax);
            }
            AST ast("Call", token_list[n].value_);
            return Ok(std::make_pair(n+1,ast));
        }else if (token_list[n].value_ == "begin"){
            Result<std::pair<size_t,AST>> res = interpretStatementSequence(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair = res.unwrap();
            n = pair.first;
            AST ast = pair.second;

            if (token_list[n].value_ != "end"){
                error("expecting 'end'",n);
                return ErrorPair(ErrorType::InvalidSyntax);
            }
            return Ok(std::make_pair(n+1,ast));
        }else if (token_list[n].value_ == "if"){
            Result<std::pair<size_t,AST>> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair = res.unwrap();
            n = pair.first;
            AST ast("If", pair.second);

            if (token_list[n].value_ != "then"){
                error("expecting 'then'",n);
                return ErrorPair(ErrorType::InvalidSyntax);
            }
            res = interpretStatement(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair1 = res.unwrap();
            n = pair1.first;
            ast.addChild(pair1.second);

            return Ok(std::make_pair(n,ast));
        }else if (token_list[n].value_ == "while"){
            Result<std::pair<size_t,AST>> res = interpretCondition(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair = res.unwrap();
            n = pair.first;
            AST ast("While", pair.second);

            if (token_list[n].value_ != "do"){
                error("expecting 'do'",n);
                return ErrorPair(ErrorType::InvalidSyntax);
            }
            res = interpretStatement(n+1);
            if (!res.isOk) return res;
            std::pair<size_t,AST> pair1 = res.unwrap();
            n = pair1.first;
            ast.addChild(pair1.second);

            return Ok(std::make_pair(n,ast));
        }else if (token_list[n].value_ == ";"){
            //support for empty statement
            return Ok(std::make_pair(n+1,AST("EmptyStatement")));
        }else{
            error("expecting statement",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretExpression(size_t n){
    AST ast("Calc");
    if (token_list[n].value_ == "+" || token_list[n].value_ == "-"){
        ast.addChild(token_list[n].value_);
        n++;
    }
    while (1){
        Result<std::pair<size_t,AST>> res = interpretTerm(n);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        if (token_list[n].value_ != "+" && token_list[n].value_ != "-"){
            if (ast.children.size() == 1){
                return Ok(std::make_pair(n,ast.children[0]));
            }
            else return Ok(std::make_pair(n,ast));
        }
        ast.addChild(token_list[n].value_);
        n++;
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretTerm(size_t n){
    AST ast("Calc");
    while (1){
        Result<std::pair<size_t,AST>> res = interpretFactor(n);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        if (token_list[n].value_ != "*" && token_list[n].value_ != "/"){
            if (ast.children.size() == 1){
                return Ok(std::make_pair(n,ast.children[0]));
            }
            else return Ok(std::make_pair(n,ast));
        }
        ast.addChild(token_list[n].value_);
        n++;
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretFactor(size_t n){
    if (token_list[n].value_ == "("){
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        if (token_list[n].value_ != ")"){
            error("expecting ')'",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        return Ok(std::make_pair(n+1,pair.second));
    }else if (token_list[n].type_ == TokenType::Literal){
        return Ok(std::make_pair(n+1,AST(token_list[n].value_)));
    }else if (token_list[n].type_ == TokenType::Identifier){
        return Ok(std::make_pair(n+1,AST(token_list[n].value_)));
    }else {
        error("expecting factor",n);
        return ErrorPair(ErrorType::InvalidSyntax);
    }
}

Result<std::pair<size_t,AST>> GrammarInterpreter::interpretCondition(size_t n){
    AST ast("Condition");
    if (token_list[n].value_ == "odd"){
        ast.addChild("odd");
        Result<std::pair<size_t,AST>> res = interpretExpression(n+1);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        return Ok(std::make_pair(n,ast));
    }else{
        Result<std::pair<size_t,AST>> res = interpretExpression(n);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair = res.unwrap();
        n = pair.first;
        ast.addChild(pair.second);

        if (token_list[n].type_ != TokenType::Operator){
            error("expecting operator",n);
            return ErrorPair(ErrorType::InvalidSyntax);
        }
        ast.addChild(token_list[n].value_);

        res = interpretExpression(n+1);
        if (!res.isOk) return res;
        std::pair<size_t,AST> pair1 = res.unwrap();
        n = pair1.first;
        ast.addChild(pair1.second);

        return Ok(std::make_pair(n,ast));
    }
}

}