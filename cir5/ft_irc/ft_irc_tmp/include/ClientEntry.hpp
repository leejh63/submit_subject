#ifndef CLIENTENTRY_HPP
#define CLIENTENTRY_HPP

#include "ClientIo.hpp"
#include "ClientState.hpp"

struct ClientEntry
{
    ClientIo    io;
    ClientState state;
};

#endif