#pragma once

namespace plc {

enum class ErrorType{
    Empty,
    Ambiguity,
    InvalidSyntax,
    RegexError,
    IOError,
    CompileError,
    SymbolLookupError,
};

template <class T>
class Result{
private:
    T value;
    ErrorType err;
public:
    bool isOk;
    Result() = delete;
    explicit Result(T res, ErrorType err) : value(res), isOk(false), err(err) {};
    explicit Result(T res) : value(res), isOk(true) {};
    explicit Result(ErrorType err) : err(err), isOk(false) {};
    T operator*() const{
        if(isOk) return value;
        else throw std::runtime_error(std::string("attempting to dereference an error result ")+static_cast<std::string>(*this));
    }
    const T &unwrap() const{
        if(isOk) return value;
        else throw std::runtime_error(std::string("attempting to unwrap an error result ")+static_cast<std::string>(*this));
    }
    const T* operator->() const{
        if(isOk) return &value;
        else throw std::runtime_error(std::string("attempting to dereference an error result ")+static_cast<std::string>(*this));
    }
    ErrorType unwrapErr(){if (!isOk) return err; else throw std::runtime_error("attempting to get error in an Ok result");}
    explicit operator std::string() const{
        if (isOk){
            return "Ok()";
        }else{
            std::string errstring;
            switch(err){
                case ErrorType::Ambiguity:
                    errstring = "Ambiguity";
                    break;
                case ErrorType::InvalidSyntax:
                    errstring = "InvalidSyntax";
                    break;
                case ErrorType::Empty:
                    errstring = "Empty";
                    break;
                case ErrorType::RegexError:
                    errstring = "RegexError";
                    break;
                case ErrorType::IOError:
                    errstring = "IOError";
                    break;
                case ErrorType::CompileError:
                    errstring = "CompileError";
                    break;
                case ErrorType::SymbolLookupError:
                    errstring = "SymbolLookupError";
            }
            return "Error(" + errstring + ")";
        }
    }
};

template<class T>
Result<T> Ok(const T &res) {
    return Result<T>(res);
}

template<class T>
Result<T> Error(const ErrorType &err) {
    return Result<T>(err);
}

template<class T, class E>
Result<T> Error(Result<E> other) {
    return Result<T>(other.unwrapErr());
}

}