#ifndef NETLIBCC_CORE_STRINGARG_H_
#define NETLIBCC_CORE_STRINGARG_H_

#include <string>

namespace netlibcc {

class StringArg {
public:
    // ctors
    StringArg(const char* str) : str_(str) {}
    StringArg(const std::string& str) : str_(str.c_str()) {}
    StringArg(const std::string&& str) : str_(str.c_str()) {}

    const char* c_str() const {
        return str_;
    }

private:
    const char* str_;
};

} // namespace netlibcc

#endif // NETLIBCC_CORE_STRINGARG_H_