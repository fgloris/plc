#include <cstdlib>
#include "../include/asm.hpp"

namespace plc{

Label::Label(const std::string& name):name(name){}

Label::operator std::string() const{

}

Section::operator std::string() const{
    std::string res = "section" + name;
}

NASMLinuxELF64::NASMLinuxELF64():text.name(".text"),bss.name(".bss"),data.name(".data"){
    text.labels.emplace_back("_start");
    text.labels[0].lines.emplace_back("global _start");
}

Result<int> NASMLinuxELF64::generate(const AST& input, size_t label_ptr){
    const std::string& name = input.name;
    if (name == "Var"){
    }else if (name == "Const"){
        
    }else if (name == "Define"){
        
    }else if (name == "Program" || name == "Block" || name == "Sequence"){
        
    }else if (name == "Procedure"){
        size_t label_ptr = text.labels.size();
        text.labels.emplace_back(input.children[0].name);
        for (const AST& child: input.children){
            Result<int> res = generate(child, label_ptr);
            if (!res.isOk) return res;
        }
    }else if (name == "Call"){
        
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