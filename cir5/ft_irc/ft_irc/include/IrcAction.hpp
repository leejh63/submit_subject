#ifndef IRC_ACTION_HPP
#define IRC_ACTION_HPP

#include <string>

enum IrcActionType
{
    ACT_NONE = 0,
    ACT_SEND,
    ACT_CLOSE
};

struct IrcAction
{
    IrcActionType type;
    int           fd;
    std::string   message;
};

#endif