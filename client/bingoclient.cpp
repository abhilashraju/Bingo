// Client side C/C++ program to demonstrate Socket
// programming
#include "bingo.hpp"
#include <iostream>
int PORT=8089;
using namespace bingo;
int main(int argc, char const* argv[])
{
          
    sock_stream client;
    auto newstream=connect(client,{"127.0.0.1",PORT});
    while (true)
    {
        std::string str;
        std::cin>> str;
        Buffer tosend{str.data(),str.length()};
        send(client,tosend);
        std::array<char,1024> arry{0};
        Buffer buffer{arry.data(),arry.size()};
        int bytes= read(client,buffer);
        auto v=buffer.buffer();
        std::printf("%s",v);
    }
    
   

    
   
    return 0;
}
