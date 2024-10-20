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
    std::cout<<(std::string)g.interpretProgram(0)<<std::endl;
    return 0;
}