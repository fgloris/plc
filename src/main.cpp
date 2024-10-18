#include <iostream>
#include "../include/keyword.hpp"

int main() {
    using namespace plc;
    KeyWordInterpreter k;
    Result<std::vector<Token>> res = k.interpretFile("../resource/example.pl");
    for (const Token & word : res.unwrap()){
        std::cout<<((std::string)(word));
        std::cout<<' ';
    }
    std::cout<<std::endl;
    return 0;
}