#include <iostream>
#include <grammar.hpp>
#include <asm.hpp>

int main() {
    using namespace plc;
    KeyWordInterpreter k;
    Result<std::vector<Token>> res = k.interpretFile("../resource/example.pl0");

    std::cout<<(std::string)res<<std::endl;
    for (const Token & word : *res){
        std::cout<<((std::string)(word));
        std::cout<<' ';
    }
    std::cout<<std::endl;

    GrammarInterpreter g(*res, "../output/example-log.txt");
    Result<std::pair<size_t,AST>> res2 = g.interpretProgram(0);
    std::cout<<std::endl<<(std::string)res2<<std::endl;

    std::pair<size_t,AST> pair = res2.unwrap();
    AST ast = pair.second;
    auto res3 = ast.getQuaternary();
    std::cout<<(std::string)ast.output("../output/example-code.txt")<<std::endl;

    std::cout<<std::endl;

    NASMLinuxELF64 compiler;
    Result<std::string> res4 = compiler.generate(ast);
    std::cout<<*res4<<std::endl;

    return 0;
}