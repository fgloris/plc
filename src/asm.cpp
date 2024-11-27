#include <cstdlib>
#include "../include/asm.hpp"

namespace plc{

Label::Label(const std::string& name):name(name){}

Label::operator std::string() const{

}

bool Label::operator==(const Label& other) const{
    return name == other.name;
}

template<>
MacroConstant<int>::MacroConstant(const std::string& name, const int& value):name(name),value(value){}

Section::Section(const std::string& name):name(name){}

Section::operator std::string() const{
    std::string res = "section" + name;
}

void Section::addLine(size_t label_ptr, const std::string& line){
    if (label_ptr >= labels.size()) return;
    labels[label_ptr].lines.emplace_back(line);
}

NASMLinuxELF64::NASMLinuxELF64():text(".text"),bss(".bss"),data(".data"){
    text.labels.emplace_back("_start");
    text.labels[0].lines.emplace_back("global _start");
}

Result<int> NASMLinuxELF64::generate(const AST& input, size_t label_ptr){
    const std::string& name = input.name;
    if (name == "Var"){
        
    }else if (name == "Const"){
        constants.emplace_back(input.name, std::stoi(input.children[0].name));
    }else if (name == "Assign"){
        
    }else if (name == "Program" || name == "Block" || name == "Sequence"){
        
    }else if (name == "Procedure"){
        text.labels.emplace_back(input.children[0].name);
        size_t child_label_ptr = text.labels.size();
        for (size_t i = 1; i < input.children.size(); i++){
            const AST& child = input.children[i];
            Result<int> res = generate(child, child_label_ptr);
            if (!res.isOk) return res;
        }
    }else if (name == "Call"){
        if (std::find(text.labels.begin(), text.labels.end(), input.children[0].name) == text.labels.end()){
            return Error<int>(ErrorType::SymbolLookupError);
        }
        text.addLine(label_ptr, "call "+input.children[0].name);
    }else if (name == "If" || name == "While"){
        
    }else if (name == "Calc"){
        
    }
}

Result<std::string> NASMLinuxELF64::generate(const AST& input){
    Result<int> res = generate(input,0);
    if (!res.isOk) return Error<std::string>(res);
    //else return "";
}

Result<int> NASMLinuxELF64::generate(const AST& input, const std::string &asmfile, const std::string &objfile, const std::string &exefile){
    std::ofstream f(asmfile);
    if (!f) return Result<int>(ErrorType::IOError);
    Result<std::string> result = generate(input);
    if (!result.isOk){
        return Error<int>(result);
    }
    f << result.unwrap();
    f.close();
    std::string cmd = std::string("nasm -f elf64 ")+asmfile+" -o "+objfile+" && ld "+objfile+" -o "+exefile;
    system(cmd.c_str());
    return Ok(0);
}


}