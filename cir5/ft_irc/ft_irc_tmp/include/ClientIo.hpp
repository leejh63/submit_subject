#ifndef CLIENT_IO_HPP
#define CLIENT_IO_HPP

#include <string>

struct ClientIo
{
    int         fd;
    std::string inBuf;
    std::string outBuf;
    bool        wantWrite;
    bool        closing;
};

#endif