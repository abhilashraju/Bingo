#pragma once
#include "file_stdio.hpp"

namespace bingo{
    struct file_body
    {
       using value_type = file_stdio;
       value_type file;
       file_body(value_type f):file(std::move(f)){}
       friend std::ostream& operator<<(std::ostream& stream,const file_body& fbody){
        std::string str;
        str.reserve(fbody.file.size());
        fbody.file.read(str.data(),fbody.file.size());
        stream << str.data();
        return stream;
       }
       
    };
    
}