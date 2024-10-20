#pragma once

namespace plc {

enum class ErrorType{
    Empty,
    Ambiguity,
    InvalidSyntax,
    RegexError,
    IOError,
    CompileError,
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
    const T &unwrap() const{
        if(isOk) return value;
        else throw std::runtime_error(std::string("attempting to unwrap an error result ")+static_cast<std::string>(*this));
    }
    ErrorType unwrapErr(){if (!isOk) return err; else throw std::runtime_error("attempting to get error in an Ok result");}
    explicit operator std::string() const{
        if (isOk){
            return std::string("Ok()");
        }else{
            std::string errstring;
            switch(err){
                case ErrorType::Ambiguity:
                    errstring = "Ambiguity";
                    goto end;
                case ErrorType::InvalidSyntax:
                    errstring = "InvalidSyntax";
                    goto end;
                case ErrorType::Empty:
                    errstring = "Empty";
                    goto end;
                case ErrorType::RegexError:
                    errstring = "RegexError";
                    goto end;
                case ErrorType::IOError:
                    errstring = "IOError";
                case ErrorType::CompileError:
                    errstring = "CompileError";
            }
            end: return "Error(" + errstring + ")";
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