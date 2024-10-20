#include <iostream>
#include "../include/grammar.hpp"

int main() {
    using namespace plc;
    KeyWordInterpreter k;
    Result<std::vector<Token>> res = k.interpretFile("../resource/example.pl");

    std::cout<<(std::string)res<<std::endl;
    std::vector<Token> token_list = res.unwrap();
    for (const Token & word : token_list){
        std::cout<<((std::string)(word));
        std::cout<<' ';
    }
    std::cout<<std::endl;

    GrammarInterpreter g(token_list, "../output/example-log.txt");
    Result<std::pair<size_t,AST>> res2 = g.interpretProgram(0);
    std::cout<<std::endl<<(std::string)res2<<std::endl;

    std::pair<size_t,AST> pair = res2.unwrap();
    AST ast = pair.second;
    ast.compile();
    std::cout<<(std::string)ast.output("../output/example-code.txt")<<std::endl;


    return 0;
}