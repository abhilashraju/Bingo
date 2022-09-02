#include "errors.hpp"
namespace bingo{
    struct file_not_found:std::runtime_error
    {
        file_not_found(const std::string& error):std::runtime_error(error){}
    };
}