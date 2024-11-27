#include <cstdlib>
#include "../include/asm.hpp"

namespace plc{

Label::Label(const std::string& name):name(name){}

Label::operator std::string() const{
    std::string res = name + ":\n";
    for (const auto& line : lines){
        res += "\t" + line + "\n";
    }
    return res;
}

bool Label::operator==(const Label& other) const{
    return name == other.name;
}

template<>
MacroConstant<int>::MacroConstant(const std::string& name, const int& value):name(name),value(value){}

template<typename T>
bool MacroConstant<T>::operator==(const std::string& other) const{
    return name == other;
}

Section::Section(const std::string& name):name(name){}

Section::operator std::string() const{
    std::string res = "section " + name + "\n";
    for (const std::string& l:lines){
        res += l + "\n";
    }
    for (const auto& label : labels){
        res += (std::string)label;
    }
    return res;
}

void Section::addLine(size_t label_ptr, const std::string& line){
    if (label_ptr >= labels.size()) return;
    labels[label_ptr].lines.emplace_back(line);
}

Result<std::string> NASMLinuxELF64::getVarPos(const std::string& var, bool left){
    std::vector<std::string>::iterator ptr = std::find(vars.begin(), vars.end(), var);
    if (ptr == vars.end()){
        return Error<std::string>(ErrorType::SymbolLookupError);
    }
    int pos = std::distance(vars.begin(), ptr)+1;
    std::string res;
    if (left){
        res = "qword [rsp+" + std::to_string(pos*8)+"]";
    }else res = "[rsp+" + std::to_string(pos*8)+"]";
    return Ok(res);
}

NASMLinuxELF64::NASMLinuxELF64():text(".text"),bss(".bss"),data(".data"){
    text.labels.emplace_back("_start");
    text.lines.emplace_back("global _start");
}

Result<int> NASMLinuxELF64::generate(const AST& input, size_t label_ptr){
    const std::string& name = input.name;
    if (name == "Var"){
        text.addLine(label_ptr, "sub rsp,"+std::to_string(8*input.children.size()));
        for (size_t i=0;i<input.children.size();i++){
            vars.emplace_back(input.children[i].name);
        }
    }else if (name == "Const"){
        for (size_t i=0;i<input.children.size();i+=2){
            constants.emplace_back(input.children[i].name,std::stoi(input.children[i+1].name));
        }
    }else if (name == "Assign"){
        Result<std::string> var_pos = getVarPos(input.children[0].name, true);
        if (!var_pos.isOk) return Error<int>(var_pos);
        std::string value = input.children[1].name;
        if (auto value_ptr = std::find(constants.begin(), constants.end(), value); value_ptr!= constants.end()){
            text.addLine(label_ptr, "mov "+*var_pos+","+ std::to_string(value_ptr->value));
        }
        else if (Result<std::string> res = getVarPos(value, false); res.isOk){
            text.addLine(label_ptr, "mov "+*var_pos+","+*res);
        }
    }else if (name == "Program" || name == "Block" || name == "Sequence"){
        for (const AST& child : input.children){
            Result<int> res = generate(child, label_ptr);
            if (!res.isOk) return res;
        }
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
    return Ok(0);
    return Error<int>(ErrorType::InvalidSyntax);
}

Result<std::string> NASMLinuxELF64::generate(const AST& input){
    Result<int> res = generate(input,0);
    if (!res.isOk) return Error<std::string>(res);
    std::string res_str = "";
    res_str += (std::string)text;
    res_str += (std::string)bss;
    res_str += (std::string)data;
    return Ok(res_str);
}

Result<int> NASMLinuxELF64::generate(const AST& input, const std::string &asmfile, const std::string &objfile, const std::string &exefile){
    std::ofstream f(asmfile);
    if (!f) return Result<int>(ErrorType::IOError);
    Result<std::string> result = generate(input);
    if (!result.isOk){
        return Error<int>(result);
    }
    f << *result;
    f.close();
    std::string cmd = std::string("nasm -f elf64 ")+asmfile+" -o "+objfile+" && ld "+objfile+" -o "+exefile;
    system(cmd.c_str());
    return Ok(0);
}


}