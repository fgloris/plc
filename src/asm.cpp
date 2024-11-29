#include <cstdlib>
#include <utility>
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
    if (lines.empty() && labels.empty()) return "";
    std::string res = "section " + name + "\n";
    for (const std::string& l:lines){
        res += l + "\n";
    }
    for (const auto& label : labels){
        res += static_cast<std::string>(label);
    }
    return res;
}

void Section::addLine(size_t label_ptr, const std::string& line){
    if (label_ptr >= labels.size()) return;
    labels[label_ptr].lines.emplace_back(line);
}

void Section::addFreeScopeLine(const Scope& scope){
    if (!scope.vars.empty()) {
        size_t free_size = scope.vars.size();
        if (scope.vars[0]=="ret") free_size-=1;
        if (free_size) addLine(scope.label_ptr, "add rsp,"+std::to_string(8*free_size));
    }
}

Scope::Scope():father(nullptr),label_ptr(0){}

Scope::Scope(Scope* father):father(father),label_ptr(father->label_ptr){}

Scope::Scope(Scope* father, size_t label_ptr):father(father),label_ptr(label_ptr){}

void Scope::addVar(const std::string& var){
    vars.emplace_back(var);
}

void Scope::addConst(const std::string& name, int value){
    constants.emplace_back(name, value);
}

Result<size_t> Scope::findVarPos(const std::string& var) const{
    if (auto ptr = std::find(vars.begin(), vars.end(), var); ptr!= vars.end()){
        size_t distance = std::distance(ptr, vars.end()) - 1;
        return Ok(distance);
    }else if (father){
        Result<size_t> res = father->findVarPos(var);
        if (res.isOk) return Ok(*res+vars.size());
        else return res;
    }
    return Error<size_t>(ErrorType::SymbolLookupError);
}

Result<std::string> Scope::findConst(const std::string& con) const{
    if (auto ptr = std::find(constants.begin(), constants.end(), con); ptr!= constants.end()){
        return Ok(std::to_string(ptr->value));
    }else if (father){
        Result<std::string> res = father->findConst(con);
        if (res.isOk) return Ok(*res);
        else return res;
    }
    return Error<std::string>(ErrorType::SymbolLookupError);
}

Result<std::string> Scope::findVar(const std::string& var) const{
    Result<size_t> pos = findVarPos(var);
    if (!pos.isOk) return Error<std::string>(pos);
    if (*pos == 0) return Ok(std::string("[rsp]"));
    return Ok("[rsp+" + std::to_string(*pos*8)+"]");
}

Result<std::string> Scope::findRValue(const std::string& val) const{
    Result<std::string> res = findConst(val);
    if (!res.isOk){
        res = findVar(val);
        if (!res.isOk){
            char* p;
            strtol(val.c_str(), &p, 10);
            if (*p) return Error<std::string>(res);
            return Ok(val);
        }
        return Ok(*res);
    }return Ok(*res);
}

NASMLinuxELF64::NASMLinuxELF64():text(".text"),bss(".bss"),data(".data"),temp_label_ptr(0){
    text.labels.emplace_back("_start");
    text.lines.emplace_back("global _start");
}

std::string NASMLinuxELF64::addTempLabelName(){
    return "_temp_label"+std::to_string(temp_label_ptr++);
}

std::string NASMLinuxELF64::getCurrentTempLabelName(){
    return "_temp_label"+std::to_string(temp_label_ptr-1);
}

Result<int> NASMLinuxELF64::generate(const AST& input, Scope& s){
    const std::string& name = input.name;
    if (name == "Var"){
        text.addLine(s.label_ptr, "sub rsp,"+std::to_string(8*input.children.size()));
        for (const auto & i : input.children){
            s.addVar(i.name);
        }
    }else if (name == "Const"){
        for (size_t i=0;i<input.children.size();i+=2){
            s.addConst(input.children[i].name,std::stoi(input.children[i+1].name));
        }
    }else if (name == "Assign"){
        Result<std::string> lvalue_str = s.findVar(input.children[0].name);
        if (!lvalue_str.isOk) return Error<int>(lvalue_str);
        std::string rvalue = input.children[1].name;
        if (rvalue == "Calc"){
            Result<int> res = generate(input.children[1], s);
            if (!res.isOk) return res;
            text.addLine(s.label_ptr, "mov qword"+*lvalue_str+",rax");
            return Ok(0);
        }
        Result<std::string> rvalue_str = s.findRValue(rvalue);
        if (!rvalue_str.isOk) return Error<int>(rvalue_str);

        if ((*rvalue_str)[0]=='['){
            text.addLine(s.label_ptr, "mov rax,"+*rvalue_str);
            text.addLine(s.label_ptr, "mov qword"+*lvalue_str+",rax");
        }else{
            text.addLine(s.label_ptr, "mov qword"+*lvalue_str+","+*rvalue_str);
        }
    }else if (name == "Program"){
        for (const AST& child : input.children){
            Result<int> res = generate(child, s);
            if (!res.isOk) return res;
        }
        text.addFreeScopeLine(s);
        text.addLine(s.label_ptr, "mov rax,60");
        text.addLine(s.label_ptr, "xor rdi,rdi");
        text.addLine(s.label_ptr, "syscall");
    }else if (name == "Block"){
        Scope scope(&s);
        for (const AST& child : input.children){
            Result<int> res = generate(child, scope);
            if (!res.isOk) return res;
        }
        text.addFreeScopeLine(scope);
    }else if (name == "Sequence"){
        for (const AST& child : input.children){
            Result<int> res = generate(child, s);
            if (!res.isOk) return res;
        }
    }else if (name == "Procedure"){
        Scope scope(&s, text.labels.size());
        scope.addVar("ret");
        text.labels.emplace_back(input.children[0].name);
        for (size_t i = 1; i < input.children.size(); i++){
            const AST& child = input.children[i];
            Result<int> res = generate(child, scope);
            if (!res.isOk) return res;
        }
        text.addFreeScopeLine(scope);
        text.addLine(scope.label_ptr, "ret");
    }else if (name == "Call"){
        if (std::find(text.labels.begin(), text.labels.end(), input.children[0].name) == text.labels.end()){
            return Error<int>(ErrorType::SymbolLookupError);
        }
        text.addLine(s.label_ptr, "call "+input.children[0].name);
    }else if (name == "Condition"){
        std::string label_name = addTempLabelName();
        if (input.children[0].name == "odd"){
            if (input.children[1].name=="Calc"){
                Result<int> res = generate(input.children[1], s);
                if (!res.isOk) return res;
                text.addLine(s.label_ptr, "test rax,1");
                text.addLine(s.label_ptr, "jz "+label_name);
            }else{
                Result<std::string> value = s.findRValue(input.children[1].name);
                if (!value.isOk) return Error<int>(value);
                text.addLine(s.label_ptr, "mov rax,"+*value);
                text.addLine(s.label_ptr, "test rax,1");
                text.addLine(s.label_ptr, "jz "+label_name);
            }
        }else{
            std::string jump;
            const std::string& cmp = input.children[1].name;
            if (cmp == "<>") jump = "je";
            else if (cmp == ">=") jump = "jl";
            else if (cmp == "<=") jump = "jg";
            else if (cmp == ">") jump = "jle";
            else if (cmp == "<") jump = "jge";
            else if (cmp == "=") jump = "jne";

            std::string lvalue,rvalue;
            if (input.children[0].name=="Calc"){
                Result<int> res = generate(input.children[0], s);
                if (!res.isOk) return res;
                lvalue = "rax";
            }else{
                Result<std::string> lvalue_res = s.findRValue(input.children[0].name);
                if (!lvalue_res.isOk) return Error<int>(lvalue_res);
                lvalue = *lvalue_res;
            }
            if (input.children[2].name=="Calc"){
                if (lvalue == "rax"){
                    text.addLine(s.label_ptr, "mov rbx,"+lvalue);
                    lvalue = "rbx";
                }
                Result<int> res = generate(input.children[0], s);
                if (!res.isOk) return res;
                rvalue = "rax";
            }else{
                Result<std::string> rvalue_res= s.findRValue(input.children[2].name);
                if (!rvalue_res.isOk) return Error<int>(rvalue_res);
                rvalue = *rvalue_res;
            }

            if (lvalue[0]=='[' && rvalue[0]=='['){
                text.addLine(s.label_ptr, "mov rax,"+lvalue);
                text.addLine(s.label_ptr, "cmp rax,"+rvalue);
            }else if (lvalue[0]=='['){
                text.addLine(s.label_ptr, "cmp qword"+lvalue+","+rvalue);
            }else text.addLine(s.label_ptr, "cmp "+lvalue+","+rvalue);
            text.addLine(s.label_ptr, jump+" "+label_name);
        }
    }else if (name == "If"){
        if (input.children[0].name != "Condition") return Error<int>(ErrorType::CompileError);
        Result<int> res = generate(input.children[0], s);
        if (!res.isOk) return res;
        std::string exit_label_name = getCurrentTempLabelName();

        Scope scope(&s);
        for (size_t i=1; i<input.children.size(); i++){
            const AST& child = input.children[i];
            Result<int> res = generate(child, scope);
            if (!res.isOk) return res;
        }
        text.addFreeScopeLine(scope);
        text.addLine(s.label_ptr, exit_label_name + ":");
    }else if (name == "While"){
        std::string loop_label_name = addTempLabelName();
        text.addLine(s.label_ptr, loop_label_name + ":");
        
        if (input.children[0].name != "Condition") return Error<int>(ErrorType::CompileError);
        Result<int> res = generate(input.children[0], s);
        if (!res.isOk) return res;
        std::string exit_label_name = getCurrentTempLabelName();

        Scope scope(&s);
        for (size_t i=1; i<input.children.size(); i++){
            const AST& child = input.children[i];
            Result<int> res = generate(child, scope);
            if (!res.isOk) return res;
        }
        text.addFreeScopeLine(scope);
        
        text.addLine(s.label_ptr, "jmp "+loop_label_name);
        text.addLine(s.label_ptr, exit_label_name + ":");
    }else if (name == "Calc"){
        if (input.children.size() <3) return Error<int>(ErrorType::CompileError);
        Result<std::string> first_value = s.findRValue(input.children[0].name);
        if (!first_value.isOk) return Error<int>(first_value);
        text.addLine(s.label_ptr, "mov rax,"+*first_value);

        for (size_t i=1; i<input.children.size(); i+=2){
            const std::string& operand = input.children[i].name;
            Result<std::string> value = s.findRValue(input.children[i+1].name);
            if (!value.isOk) return Error<int>(value);

            if (operand=="+"){
                text.addLine(s.label_ptr, "add rax,"+*value);
            }else if (operand=="-"){
                text.addLine(s.label_ptr, "sub rax,"+*value);
            }else if (operand=="*"){
                text.addLine(s.label_ptr, "mov rbx,"+*value);
                text.addLine(s.label_ptr, "imul rbx");
            }else if (operand=="/"){
                text.addLine(s.label_ptr, "mov rbx,"+*value);
                text.addLine(s.label_ptr, "idiv rbx");
            }
        }
    }else return Error<int>(ErrorType::InvalidSyntax);
    return Ok(0);
}

Result<std::string> NASMLinuxELF64::generate(const AST& input){
    Scope global_scope;
    Result<int> res = generate(input,global_scope);
    if (!res.isOk) return Error<std::string>(res);
    std::string res_str;
    res_str += static_cast<std::string>(text);
    res_str += static_cast<std::string>(bss);
    res_str += static_cast<std::string>(data);
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