#include "../include/ast.hpp"

namespace plc {
size_t AST::temp_name = 0;
std::vector<Quaternary> AST::code;
std::map<std::string,size_t> AST::procedure_line;

Quaternary::Quaternary(std::string cmd, std::string value1, std::string value2, std::string result):
    cmd(std::move(cmd)),value1(std::move(value1)),value2(std::move(value2)),result(std::move(result)){}

Quaternary::operator std::string() const{
    return std::string("(") + cmd + ", " + value1 + ", " + value2 + ", " + result + ")";
}

std::string AST::getTempName(){
    return "T" + std::to_string(temp_name++);
}

AST::AST(std::string name) : name(name) {}
AST::AST(std::string name, AST child1) : name(name) {
    children.push_back(std::move(child1));
}
AST::AST(std::string name, AST child1, AST child2) : name(name) {
    children.push_back(std::move(child1));
    children.push_back(std::move(child2));
}
AST::AST(std::string name, AST child1, AST child2, AST child3) : name(name) {
    children.push_back(std::move(child1));
    children.push_back(std::move(child2));
    children.push_back(std::move(child3));
}

AST::AST(std::string name, std::vector<AST> children) : name(name), children(std::move(children)) {}

void AST::addChild(AST child) {children.push_back(std::move(child));}
void AST::addChild(std::string childname) {children.push_back(std::move(childname));}

void AST::print(std::ofstream& log_file) const {
    if (children.empty()) {
        std::cout << name;
        log_file << name;
        return;
    }
    std::cout << name << "(";
    log_file << name << "(";
    for (auto& child : children) {
        child.print(log_file);
        if (&child!= &children.back()){
            std::cout << ", ";
            log_file << ", ";
        }
    }
    std::cout << ")";
    log_file << ")";
}

Result<size_t> AST::output(std::string log_file_name) const{
    size_t n = 0;
    std::ofstream log_file(std::move(log_file_name));
    for (const Quaternary& q: code){
        log_file<<(std::string)q<<std::endl;
        n++;
    }
    return Ok(n);
}

Result<std::string> AST::compile(){
    if (name == "Var"){
        for (auto& child : children) code.emplace_back(":=","_","_",child.name);
    }else if (name == "Const"){
        for (size_t i=0; i<children.size(); i+=2) {
            code.emplace_back(":=",children[i+1].name,"_",children[i].name);
        }
    }else if (name == "Define"){
        for (size_t i=0; i<children.size(); i+=2) {
            Result<std::string> res = children[i+1].compile();
            if (!res.isOk) return res;
            std::string temp_name = res.unwrap();
            code.emplace_back(":=",temp_name,"_",children[i].name);
        }
    }else if (name == "Program" || name == "Block" || name == "Sequence"){
        for (AST& child: children){
            Result<std::string> res = child.compile();
            if (!res.isOk) return res;
        }
    }else if (name == "Procedure"){
        size_t current_size = code.size();
        code.emplace_back("j","_","_",std::to_string(-1));
        procedure_line[children[0].name] = code.size();
        for (AST& child: children){
            Result<std::string> res = child.compile();
            if (!res.isOk) return res;
        }
        code[current_size].result = std::to_string(code.size());
    }else if (name == "Call"){
        size_t dest = procedure_line[children[0].name];
        code.emplace_back("j","_","_",std::to_string(dest));

    }else if (name == "If" || name == "While"){
        if (children.size() != 2 || children[0].name != "Condition") return Error<std::string>(ErrorType::CompileError);
        size_t current_size = code.size();
        if (children[0].children.size() == 3){
            Result<std::string> res1 = children[0].children[0].compile();
            Result<std::string> res2 = children[0].children[2].compile();
            if (!res1.isOk) return res1;
            if (!res2.isOk) return res2;
            std::string tmp1 = res1.unwrap();
            std::string tmp2 = res2.unwrap();
            code.emplace_back("j"+children[0].children[1].name,tmp1,tmp2,std::to_string(current_size+2));
            code.emplace_back("j","_","_",std::to_string(current_size+3));
            children[1].compile();
        }else if (children[0].children.size() == 2){
            Result<std::string> res = children[0].children[1].compile();
            if (!res.isOk) return res;
            std::string tmp = res.unwrap();
            code.emplace_back("j"+children[0].children[0].name,tmp,"_",std::to_string(current_size+2));
            code.emplace_back("j","_","_",std::to_string(current_size+3));
            children[1].compile();
        }
        code[current_size+1].result = std::to_string(code.size());
        if (name == "while"){
            code.emplace_back("j","_","_",std::to_string(current_size));
        }
    }else if (name == "Calc"){
        if (children.size() <3) return Error<std::string>(ErrorType::CompileError);
        std::string tmp = getTempName();
        code.emplace_back(":=",children[0].name,"_",tmp);
        for (size_t i=1; i<children.size(); i+=2){
            Result<std::string> res = children[i+1].compile();
            if (!res.isOk) return res;
            std::string name = res.unwrap();
            code.emplace_back(children[i].name,tmp,name,tmp);
        }
        return Ok(tmp);
    }
    return Ok(name);
}

Result<std::pair<size_t, AST>> ErrorPair(ErrorType err){
    return Result(std::make_pair(size_t{0}, AST("Error")), err);
}

}