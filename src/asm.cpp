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
    return Error<size_t>(ErrorType::ValueNotFoundError);
}

Result<std::string> Scope::findConst(const std::string& con) const{
    if (auto ptr = std::find(constants.begin(), constants.end(), con); ptr!= constants.end()){
        return Ok(std::to_string(ptr->value));
    }else if (father){
        Result<std::string> res = father->findConst(con);
        if (res.isOk) return Ok(*res);
        else return res;
    }
    return Error<std::string>(ErrorType::ValueNotFoundError);
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

}