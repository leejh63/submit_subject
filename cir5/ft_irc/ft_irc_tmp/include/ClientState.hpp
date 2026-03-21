#ifndef CLIENT_STATE_HPP
#define CLIENT_STATE_HPP

#include <string>

struct ClientState
{
    bool        passOk;
    bool        hasNick;
    bool        hasUser;
    bool        registered;

    std::string nick;
    std::string user;
    std::string realName;
};

#endif