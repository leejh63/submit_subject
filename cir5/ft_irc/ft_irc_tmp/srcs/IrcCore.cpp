#include "IrcCore.hpp"
#include "IrcParser.hpp"

#include <iostream>
#include <sstream>

static std::string build_cmd_struct_string(
    const ClientEntry& entry,
    const IrcCommand& cmd )
{
    std::ostringstream oss;

    oss << "=========[CMD fd=" << entry.io.fd << "]=========\n";

    // -------- IrcCommand --------
    oss << "[COMMAND]\n";
    oss << "raw_line=     '" << cmd.raw_Line << "'\n";
    oss << "prefix=       '" << cmd.prefix << "'\n";
    oss << "verb=         '" << cmd.verb << "'\n";

    oss << "params(" << cmd.params.size() << ")=    ";
    for (size_t i = 0; i < cmd.params.size(); ++i)
        oss << "[" << cmd.params[i] << "]";
    oss << "\n";

    oss << "trailing=     '" << cmd.trailing << "'\n";

    // -------- ClientIo --------
    oss << "[IO]\n";
    oss << "fd=           " << entry.io.fd << "\n";
    oss << "inBuf=       =" << entry.io.inBuf << "=\n";
    oss << "outBuf=      =" << entry.io.outBuf << "=\n";
    oss << "wantWrite=    " << (entry.io.wantWrite ? "true" : "false") << "\n";
    oss << "closing=      " << (entry.io.closing ? "true" : "false") << "\n";

    // -------- ClientState --------
    oss << "[STATE]\n";
    oss << "passOk=       " << (entry.state.passOk ? "true" : "false") << "\n";
    oss << "hasNick=      " << (entry.state.hasNick ? "true" : "false") << "\n";
    oss << "hasUser=      " << (entry.state.hasUser ? "true" : "false") << "\n";
    oss << "registered=   " << (entry.state.registered ? "true" : "false") << "\n";

    oss << "nick=         '" << entry.state.nick << "'\n";
    oss << "user=         '" << entry.state.user << "'\n";
    oss << "realName=     '" << entry.state.realName << "'\n";

    oss << "===============================\n";

    return oss.str();
}

IrcCore::IrcCore( const std::string& password, ClientRegistry& clients, ChannelRegistry&  channels )
: _server_password(password)
, _clients(clients)
, _channels(channels)
{

}

IrcCore::~IrcCore( void )
{
}

void IrcCore::handle_Line( ClientEntry& entry,
                           const std::string& raw_Line,
                           std::vector<IrcAction>& out )
{
    IrcCommand cmd;

    if (!IrcParser::parse_Line(raw_Line, cmd))
    {
        cmd.raw_Line = raw_Line;
        return handle_Error(entry, cmd, out);
    }

    handle_Command(entry, cmd, out);
}

// void IrcCore::debug_Short( const char* msg ) const
// {
//     if (!msg)
//         return;
//     std::cout << msg;
// }

void IrcCore::debug_Full( const ClientEntry& entry,
                          const IrcCommand& cmd,
                          const char* msg ) const
{
    std::cout << "\n========================================\n";
    _clients.debug_Print_All();
    _channels.debug_Print_All();

    std::cout << build_cmd_struct_string(entry, cmd);
    std::cout << msg;


    std::cout << "========================================\n";
}

const char* IrcCore::debug_Message( handleResult result ) const
{
    switch (result)
    {
        // password
        case PASS_ALREADY_REGISTERED:
            return "[PASS] already registered\n";
        case PASS_PARAM_MISSING:
            return "[PASS] parameter missing\n";
        case PASS_PASSWORD_OK:
            return "[PASS] password same\n";
        case PASS_PASSWORD_BAD:
            return "[PASS] password not same\n";

        // nickname
        case NICK_ERRONEUS:
            return "[NICK] erroneous nickname\n";
        case NICK_PARAM_MISSING:
            return "[NICK] nickname missing\n";
        case NICK_IN_USE:
            return "[NICK] nickname already in use\n";
        case NICK_OK:
            return "[NICK] nickname good\n";

        // user
        case USER_PARAM_MISSING:
            return "[USER] parameter missing\n";
        case USER_REALNAME_MISSING:
            return "[USER] realname missing\n";
        case USER_ALREADY_REGISTERED:
            return "[USER] already registered\n";
        case USER_OK:
            return "[USER] user good\n";

        // etc
        case HANDLE_ERROR:
            return "[ERROR] unknown result\n";
        case HANDLE_UNKNOWN:
            return "[UNKNOWN] unknown command\n";
        default:
            return "[default] unknown result\n";
    }
}

void IrcCore::push_Send( std::vector<IrcAction>& out,
                         int fd,
                         const std::string& message ) const
{
    IrcAction act;
    act.type = ACT_SEND;
    act.fd = fd;
    act.message = message;
    out.push_back(act);
}

void IrcCore::push_Close( std::vector<IrcAction>& out,
                          int fd ) const
{
    IrcAction act;
    act.type = ACT_CLOSE;
    act.fd = fd;
    out.push_back(act);
}

IrcCore::handleResult IrcCore::check_Pass( int fd,
                                           const IrcCommand& cmd ) const
{
    if (_clients.is_Registered(fd) ||
        _clients.has_Nick(fd) ||
        _clients.has_User(fd))
        return PASS_ALREADY_REGISTERED;

    if (cmd.params.size() < 1)
        return PASS_PARAM_MISSING;

    if (cmd.params[0] == _server_password)
        return PASS_PASSWORD_OK;

    return PASS_PASSWORD_BAD;
}

// 임시 클래스나 아니면 따로 뺄지 고민중
static bool is_nick_first_char(char c)
{
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;

    if (c == '[' || c == ']' || c == '\\' || c == '`' ||
        c == '_' || c == '^' || c == '{' || c == '|' || c == '}')
        return true;

    return false;
}

static bool is_nick_char(char c)
{
    if (is_nick_first_char(c)) return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '-') return true;
    return false;
}

IrcCore::handleResult IrcCore::check_Nick( const IrcCommand& cmd ) const
{
    if (cmd.params.size() < 1)
        return NICK_PARAM_MISSING;

    const std::string& nick = cmd.params[0];

    if (nick.empty())
        return NICK_ERRONEUS;

    if (!is_nick_first_char(nick[0]))
        return NICK_ERRONEUS;

    for (size_t i = 1; i < nick.size(); ++i)
    {
        if (!is_nick_char(nick[i]))
            return NICK_ERRONEUS;
    }

    return NICK_OK;
}

IrcCore::handleResult IrcCore::check_User( int fd,
                                           const IrcCommand& cmd ) const
{
    if (_clients.has_User(fd))
        return USER_ALREADY_REGISTERED;

    if (cmd.params.size() < 3)
        return USER_PARAM_MISSING;

    if (!cmd.hasTrailing)
        return USER_REALNAME_MISSING;

    return USER_OK;
}

IrcCore::handleResult IrcCore::check_Join( const std::string& channelName,
                                           int fd,
                                           const std::string& providedKey ) const
{
    if (_channels.has_Member(channelName, fd))
        return JOIN_ALREADY_MEMBER;

    if (_channels.is_Invite_Only(channelName) &&
        !_channels.is_Invited(channelName, fd))
        return JOIN_INVITE_ONLY;

    if (_channels.has_Key(channelName))
    {
        std::string actualKey;
        if (_channels.get_Key(channelName, actualKey) && actualKey != providedKey)
            return JOIN_BAD_KEY;
    }

    if (_channels.is_Channel_Full(channelName))
        return JOIN_CHANNEL_FULL;

    return JOIN_OK;
}

IrcCore::handleResult IrcCore::apply_Mode_Toggle( const std::string& channelName,
                                                  char sign,
                                                  char modeChar,
                                                  std::string& appliedMode )
{
    const bool enable = (sign == '+');

    if (modeChar == 'i')
    {
        _channels.set_Invite_Only(channelName, enable);
        appliedMode = std::string(1, sign) + "i";
        return MODE_APPLY_OK;
    }

    if (modeChar == 't')
    {
        _channels.set_Topic_Op_Only(channelName, enable);
        appliedMode = std::string(1, sign) + "t";
        return MODE_APPLY_OK;
    }

    return MODE_UNKNOWN_CHAR;
}

IrcCore::handleResult IrcCore::apply_Mode_Param( const std::string& channelName,
                                                 char sign,
                                                 char modeChar,
                                                 const std::string& modeParam,
                                                 std::string& appliedMode,
                                                 std::string& appliedParam )
{
    if (modeChar == 'k')
    {
        if (sign == '+')
        {
            if (modeParam.empty())
                return MODE_PARAM_MISSING;

            _channels.set_Key(channelName, modeParam);

            appliedMode = "+k";
            appliedParam = "*";

            return MODE_APPLY_OK;
        }

        _channels.clear_Key(channelName);

        appliedMode = "-k";
        appliedParam.clear();

        return MODE_APPLY_OK;
    }

    if (modeChar == 'l')
    {
        if (sign == '+')
        {
            if (modeParam.empty())
                return MODE_PARAM_MISSING;

            if (!is_Number_String(modeParam))
                return MODE_BAD_VALUE;

            std::istringstream iss(modeParam);
            int limit = 0;
            iss >> limit;

            if (limit <= 0)
                return MODE_BAD_VALUE;

            _channels.set_User_Limit(channelName, limit);

            appliedMode = "+l";
            appliedParam = modeParam;

            return MODE_APPLY_OK;
        }

        _channels.clear_User_Limit(channelName);

        appliedMode = "-l";
        appliedParam.clear();

        return MODE_APPLY_OK;
    }

    return MODE_UNKNOWN_CHAR;
}

IrcCore::handleResult IrcCore::apply_Mode_Member( const std::string& channelName,
                                                  char sign,
                                                  char modeChar,
                                                  const std::string& modeParam,
                                                  std::string& appliedMode,
                                                  std::string& appliedParam )
{
    if (modeChar != 'o')
        return MODE_UNKNOWN_CHAR;

    if (modeParam.empty())
        return MODE_PARAM_MISSING;

    const std::string& targetNick = modeParam;

    ClientEntry* target = _clients.find_By_Nick(targetNick);
    if (target == NULL)
        return MODE_NO_SUCH_NICK;

    if (!_channels.has_Member(channelName, target->io.fd))
        return MODE_USER_NOT_IN_CHANNEL;

    if (sign == '+')
    {
        _channels.add_Operator(channelName, target->io.fd);
        appliedMode = "+o";
        appliedParam = targetNick;
    }
    else
    {
        if (would_Remove_Last_Operator(channelName, target->io.fd))
            return MODE_LAST_OPERATOR;

        _channels.remove_Operator(channelName, target->io.fd);
        appliedMode = "-o";
        appliedParam = targetNick;
    }

    return MODE_APPLY_OK;
}

bool IrcCore::mode_Requires_Param( char sign,
                                   char modeChar ) const
{
    if (modeChar == 'o')
        return true;

    if (modeChar == 'k' && sign == '+')
        return true;

    if (modeChar == 'l' && sign == '+')
        return true;

    return false;
}

bool IrcCore::would_Remove_Last_Operator( const std::string& channelName,
                                          int targetFd ) const
{
    const ChannelEntry* ch = _channels.find_By_Name(channelName);

    if (ch == NULL)
        return false;

    if (ch->operators.find(targetFd) == ch->operators.end())
        return false;

    return ch->operators.size() <= 1;
}

void IrcCore::append_Mode_Change( std::string& outModes,
                                  std::string& outParams,
                                  char sign,
                                  char modeChar,
                                  const std::string& appliedParam ) const
{
    if (outModes.empty() || outModes[outModes.size() - 1] != sign)
        outModes += sign;

    outModes += modeChar;

    if (!appliedParam.empty())
    {
        if (!outParams.empty())
            outParams += " ";
        outParams += appliedParam;
    }
}

void IrcCore::try_Register( ClientEntry& entry,
                            std::vector<IrcAction>& out )
{
    if (_clients.try_Register(entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Welcome_Reply(_clients.get_Nick(entry.io.fd)));
        //debug_Short("[REGISTER] completed\n");
    }
}

void IrcCore::handle_Command( ClientEntry& entry,
                              const IrcCommand& cmd,
                              std::vector<IrcAction>& out )
{
    if (!_clients.is_Registered(entry.io.fd))
    {
        if (!allowed_Before_Register(cmd.verb))
        {
            if (is_Known_Command(cmd.verb))
            {
                push_Send(out, entry.io.fd,
                          build_Err_Not_registered(_clients.get_Nick(entry.io.fd)));
                debug_Full(entry, cmd, "[ERROR] not registered\n");
                return;
            }
            return handle_Unknown(entry, cmd, out);
        }
    }

    if (cmd.verb == "CAP")
        return handle_Cap(entry, cmd, out);
    if (cmd.verb == "PING")
        return handle_Ping(entry, cmd, out);
    if (cmd.verb == "WHO")
        return handle_Who(entry, cmd, out);
    if (cmd.verb == "PONG")
        return handle_Pong(entry, cmd, out);
    if (cmd.verb == "PASS")
        return handle_Pass(entry, cmd, out);
    if (cmd.verb == "NICK")
        return handle_Nick(entry, cmd, out);
    if (cmd.verb == "USER")
        return handle_User(entry, cmd, out);
    if (cmd.verb == "JOIN")
        return handle_Join(entry, cmd, out);
    if (cmd.verb == "PRIVMSG")
        return handle_Privmsg(entry, cmd, out);
    if (cmd.verb == "KICK")
        return handle_Kick(entry, cmd, out);
    if (cmd.verb == "INVITE")
        return handle_Invite(entry, cmd, out);
    if (cmd.verb == "TOPIC")
        return handle_Topic(entry, cmd, out);
    if (cmd.verb == "MODE")
        return handle_Mode(entry, cmd, out);
    if (cmd.verb == "PART")
        return handle_Part(entry, cmd, out);
    if (cmd.verb == "QUIT")
        return handle_Quit(entry, cmd, out);

    return handle_Unknown(entry, cmd, out);
}

void IrcCore::handle_Cap( ClientEntry& entry,
                          const IrcCommand& cmd,
                          std::vector<IrcAction>& out )
{
    const std::string target = nick_Or_star(entry.io.fd);

    if (cmd.params.empty())
    {
        debug_Full(entry, cmd, "[CAP] subcommand missing\n");
        return;
    }

    const std::string& subCommand = cmd.params[0];

    if (subCommand == "LS" || subCommand == "LIST")
    {
        push_Send(out, entry.io.fd,
                  build_Cap_Message(target, subCommand, ""));
        debug_Full(entry, cmd, "[CAP] empty capability list sent\n");
        return;
    }

    if (subCommand == "REQ")
    {
        push_Send(out, entry.io.fd,
                  build_Cap_Message(target, "NAK", cmd.trailing));
        debug_Full(entry, cmd, "[CAP] capability request rejected\n");
        return;
    }

    if (subCommand == "END")
    {
        debug_Full(entry, cmd, "[CAP] capability negotiation ended\n");
        return;
    }

    debug_Full(entry, cmd, "[CAP] unsupported subcommand ignored\n");
}

void IrcCore::handle_Ping( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    std::string token;

    if (!cmd.trailing.empty())
        token = cmd.trailing;
    else if (!cmd.params.empty())
        token = cmd.params[0];

    if (token.empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(nick_Or_star(entry.io.fd), "PING"));
        debug_Full(entry, cmd, "[PING] token missing\n");
        return;
    }

    push_Send(out, entry.io.fd,
              build_Pong_Message(token));

    debug_Full(entry, cmd, "[PING] pong sent\n");
}

void IrcCore::handle_Who( ClientEntry& entry,
                          const IrcCommand& cmd,
                          std::vector<IrcAction>& out )
{
    const std::string requesterNick = nick_Or_star(entry.io.fd);

    if (cmd.params.empty())
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_End_of_who(requesterNick, "*"));
        debug_Full(entry, cmd, "[WHO] no mask, end of who\n");
        return;
    }

    const std::string& mask = cmd.params[0];

    if (mask.empty() || mask[0] != '#')
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_End_of_who(requesterNick, mask.empty() ? "*" : mask));
        debug_Full(entry, cmd, "[WHO] unsupported non-channel who\n");
        return;
    }

    if (!_channels.has_Channel(mask))
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_End_of_who(requesterNick, mask));
        debug_Full(entry, cmd, "[WHO] no such channel, end of who\n");
        return;
    }

    std::vector<int> members;
    _channels.collect_Channel_Members(mask, members);

    for (size_t i = 0; i < members.size(); ++i)
    {
        const std::string reply =
            build_Rpl_Who_reply(requesterNick, mask, members[i]);

        if (!reply.empty())
            push_Send(out, entry.io.fd, reply);
    }

    push_Send(out, entry.io.fd,
              build_Rpl_End_of_who(requesterNick, mask));

    debug_Full(entry, cmd, "[WHO] channel who replied\n");
}

void IrcCore::handle_Pong( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    (void)out;
    debug_Full(entry, cmd, "[PONG] pong received\n");
}

void IrcCore::handle_Error( ClientEntry& entry,
                            const IrcCommand& cmd,
                            std::vector<IrcAction>& out )
{
    (void)out;
    debug_Full(entry, cmd, debug_Message(HANDLE_ERROR));
}

void IrcCore::handle_Pass( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    handleResult result = check_Pass(entry.io.fd, cmd);

    if (result == PASS_PASSWORD_OK)
    {
        _clients.set_Pass_Ok(entry.io.fd, true);
        try_Register(entry, out);
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    if (result == PASS_PASSWORD_BAD)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Password_mismatch(_clients.get_Nick(entry.io.fd)));
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    if (result == PASS_PARAM_MISSING)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(_clients.get_Nick(entry.io.fd), "PASS"));
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    // PASS_ALREADY_REGISTERED
    push_Send(out, entry.io.fd,
              build_Err_Already_registered(_clients.get_Nick(entry.io.fd)));
    debug_Full(entry, cmd, debug_Message(result));
}

void IrcCore::handle_Nick( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (!_clients.has_Pass_Ok(entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_registered(_clients.get_Nick(entry.io.fd)));
        debug_Full(entry, cmd, "[NICK] password required first\n");
        return;
    }

    handleResult result = check_Nick(cmd);

    if (result == NICK_PARAM_MISSING)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(_clients.get_Nick(entry.io.fd), "NICK"));
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    if (result == NICK_ERRONEUS)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Erroneus_nickname(
                      _clients.get_Nick(entry.io.fd),
                      cmd.params.empty() ? "*" : cmd.params[0]));
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    const std::string& newNick = cmd.params[0];
    const bool hadNick = _clients.has_Nick(entry.io.fd);

    if (!is_Nick_available(entry.io.fd, newNick))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Nickname_in_use(_clients.get_Nick(entry.io.fd), newNick));
        debug_Full(entry, cmd, debug_Message(NICK_IN_USE));
        return;
    }

    std::string nickMsg;
    if (hadNick)
        nickMsg = build_Nick_Message(entry.io.fd, newNick);

    _clients.set_Nick(entry.io.fd, newNick);
    try_Register(entry, out);

    if (hadNick)
    {
        push_Send(out, entry.io.fd, nickMsg);
        send_To_Shared_Peers(entry.io.fd, nickMsg, out);
        debug_Full(entry, cmd, "[NICK] nickname changed\n");
    }
    else
    {
        debug_Full(entry, cmd, "[NICK] nickname set\n");
    }
}

void IrcCore::handle_User( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (!_clients.has_Pass_Ok(entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_registered(_clients.get_Nick(entry.io.fd)));
        debug_Full(entry, cmd, "[USER] password required first\n");
        return;
    }

    handleResult result = check_User(entry.io.fd, cmd);

    if (result == USER_OK)
    {
        _clients.set_User(entry.io.fd, cmd.params[0], cmd.trailing);
        try_Register(entry, out);
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    if (result == USER_PARAM_MISSING || result == USER_REALNAME_MISSING)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(_clients.get_Nick(entry.io.fd), "USER"));
        debug_Full(entry, cmd, debug_Message(result));
        return;
    }

    // USER_ALREADY_REGISTERED
    push_Send(out, entry.io.fd,
              build_Err_Already_registered(_clients.get_Nick(entry.io.fd)));
    debug_Full(entry, cmd, debug_Message(result));
}

void IrcCore::handle_Join( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 1)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(_clients.get_Nick(entry.io.fd), "JOIN"));
        debug_Full(entry, cmd, "[JOIN] channel name missing\n");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string nick = _clients.get_Nick(entry.io.fd);

    if (channelName.empty() || channelName[0] != '#')
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[JOIN] bad channel name\n");
        return;
    }

    std::string providedKey;
    if (cmd.params.size() >= 2)
        providedKey = cmd.params[1];

    handleResult result = check_Join(channelName, entry.io.fd, providedKey);

    if (result == JOIN_ALREADY_MEMBER)
    {
        debug_Full(entry, cmd, "[JOIN] already joined\n");
        return;
    }

    if (result == JOIN_INVITE_ONLY)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Invite_only_chan(nick, channelName));
        debug_Full(entry, cmd, "[JOIN] invite only channel\n");
        return;
    }

    if (result == JOIN_BAD_KEY)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Bad_channel_key(nick, channelName));
        debug_Full(entry, cmd, "[JOIN] bad key\n");
        return;
    }

    if (result == JOIN_CHANNEL_FULL)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Channel_is_full(nick, channelName));
        debug_Full(entry, cmd, "[JOIN] channel full\n");
        return;
    }

    if (!_channels.join_Channel(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[JOIN] join channel failed\n");
        return;
    }

    const std::string joinMsg = build_Join_Message(entry.io.fd, channelName);

    send_To_Channel(channelName, joinMsg, out, -1);

    const std::string topic = _channels.get_Topic(channelName);

    if (topic.empty())
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_No_topic(nick, channelName));
    }
    else
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_Topic(nick, channelName, topic));
    }

    push_Send(out, entry.io.fd,
              build_Rpl_Name_reply(nick, channelName));

    push_Send(out, entry.io.fd,
              build_Rpl_End_of_names(nick, channelName));

    debug_Full(entry, cmd, "[JOIN] joined channel\n");
}

void IrcCore::handle_Part( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 1 || cmd.params[0].empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(_clients.get_Nick(entry.io.fd), "PART"));
        debug_Full(entry, cmd, "[PART] channel name missing\n");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string nick = _clients.get_Nick(entry.io.fd);

    if (!_channels.has_Channel(channelName))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[PART] no such channel\n");
        return;
    }

    if (!_channels.has_Member(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[PART] not a member\n");
        return;
    }

    std::string reason;
    if (!cmd.trailing.empty())
        reason = cmd.trailing;

    const std::string partMsg =
        build_Part_Message(entry.io.fd, channelName, reason);

    if (!_channels.remove_Member_And_Cleanup(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[PART] remove member failed\n");
        return;
    }

    send_To_Channel(channelName, partMsg, out, -1);
    push_Send(out, entry.io.fd, partMsg);

    debug_Full(entry, cmd, "[PART] left channel\n");
}

void IrcCore::handle_Privmsg( ClientEntry& entry,
                              const IrcCommand& cmd,
                              std::vector<IrcAction>& out )
{
    const std::string senderNick = _clients.get_Nick(entry.io.fd);

    if (cmd.params.size() < 1 || cmd.params[0].empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_recipient(senderNick, "PRIVMSG"));
        debug_Full(entry, cmd, "[PRIVMSG] recipient missing\n");
        return;
    }

    if (cmd.trailing.empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_text_to_send(senderNick));
        debug_Full(entry, cmd, "[PRIVMSG] text missing\n");
        return;
    }

    const std::string& target = cmd.params[0];
    const std::string& text = cmd.trailing;

    const std::string msg =
        build_Privmsg_Message(entry.io.fd, target, text);

    if (target[0] == '#')
    {
        if (!_channels.has_Channel(target))
        {
            push_Send(out, entry.io.fd,
                      build_Err_No_such_channel(senderNick, target));
            debug_Full(entry, cmd, "[PRIVMSG] no such channel\n");
            return;
        }

        if (!_channels.has_Member(target, entry.io.fd))
        {
            push_Send(out, entry.io.fd,
                      build_Err_Cannot_send_to_chan(senderNick, target));
            debug_Full(entry, cmd, "[PRIVMSG] sender not in channel\n");
            return;
        }

        send_To_Channel(target, msg, out, entry.io.fd);
        debug_Full(entry, cmd, "[PRIVMSG] sent to channel\n");
        return;
    }

    ClientEntry* targetClient = _clients.find_By_Nick(target);
    if (targetClient == NULL)
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_nick(senderNick, target));
        debug_Full(entry, cmd, "[PRIVMSG] no such nick\n");
        return;
    }

    push_Send(out, targetClient->io.fd, msg);
    debug_Full(entry, cmd, "[PRIVMSG] sent to user\n");
}

void IrcCore::handle_Topic( ClientEntry& entry,
                            const IrcCommand& cmd,
                            std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 1)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(
                      _clients.get_Nick(entry.io.fd), "TOPIC"));
        debug_Full(entry, cmd, "[TOPIC] channel name missing\n");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string  nick = _clients.get_Nick(entry.io.fd);

    if (!_channels.has_Channel(channelName))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[TOPIC] no such channel\n");
        return;
    }

    if (!_channels.has_Member(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[TOPIC] not on channel\n");
        return;
    }

    if (!cmd.hasTrailing)
    {
        const std::string topic = _channels.get_Topic(channelName);

        if (topic.empty())
        {
            push_Send(out, entry.io.fd,
                    build_Rpl_No_topic(nick, channelName));
            debug_Full(entry, cmd, "[TOPIC] no topic\n");
            return;
        }

        push_Send(out, entry.io.fd,
                build_Rpl_Topic(nick, channelName, topic));
        debug_Full(entry, cmd, "[TOPIC] topic viewed\n");
        return;
    }

    if (!_channels.can_Change_Topic(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Chan_oprivs_needed(nick, channelName));
        debug_Full(entry, cmd, "[TOPIC] operator privilege required\n");
        return;
    }

    if (!_channels.set_Topic(channelName, cmd.trailing))
    {
        debug_Full(entry, cmd, "[TOPIC] set topic failed\n");
        return;
    }

    const std::string topicMsg =
        build_Topic_Message(entry.io.fd, channelName, cmd.trailing);

    send_To_Channel(channelName, topicMsg, out, -1);

    debug_Full(entry, cmd, "[TOPIC] topic changed\n");
}

void IrcCore::handle_Invite( ClientEntry& entry,
                             const IrcCommand& cmd,
                             std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 2 ||
        cmd.params[0].empty() ||
        cmd.params[1].empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(
                      _clients.get_Nick(entry.io.fd), "INVITE"));
        debug_Full(entry, cmd, "[INVITE] need more params\n");
        return;
    }

    const std::string& targetNick = cmd.params[0];
    const std::string& channelName = cmd.params[1];
    const std::string  nick = _clients.get_Nick(entry.io.fd);

    ClientEntry* targetClient = _clients.find_By_Nick(targetNick);
    if (targetClient == NULL)
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_nick(nick, targetNick));
        debug_Full(entry, cmd, "[INVITE] no such nick\n");
        return;
    }

    if (!_channels.has_Channel(channelName))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[INVITE] no such channel\n");
        return;
    }

    if (!_channels.has_Member(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[INVITE] requester not on channel\n");
        return;
    }

    if (!_channels.can_Invite(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Chan_oprivs_needed(nick, channelName));
        debug_Full(entry, cmd, "[INVITE] operator privilege required\n");
        return;
    }

    if (_channels.has_Member(channelName, targetClient->io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_User_on_channel(nick, targetNick, channelName));
        debug_Full(entry, cmd, "[INVITE] target already on channel\n");
        return;
    }

    _channels.add_Invite(channelName, targetClient->io.fd);

    push_Send(out, entry.io.fd,
              build_Rpl_Inviting(nick, targetNick, channelName));

    push_Send(out, targetClient->io.fd,
              build_Invite_Message(entry.io.fd, targetNick, channelName));

    debug_Full(entry, cmd, "[INVITE] invite processed\n");
}

void IrcCore::handle_Kick( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 2 ||
        cmd.params[0].empty() ||
        cmd.params[1].empty())
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(
                      _clients.get_Nick(entry.io.fd), "KICK"));
        debug_Full(entry, cmd, "[KICK] need more params\n");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string& targetNick = cmd.params[1];
    const std::string  nick = _clients.get_Nick(entry.io.fd);

    if (!_channels.has_Channel(channelName))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[KICK] no such channel\n");
        return;
    }

    if (!_channels.has_Member(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[KICK] requester not on channel\n");
        return;
    }

    if (!_channels.can_Kick(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Chan_oprivs_needed(nick, channelName));
        debug_Full(entry, cmd, "[KICK] operator privilege required\n");
        return;
    }

    ClientEntry* targetClient = _clients.find_By_Nick(targetNick);
    if (targetClient == NULL)
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_nick(nick, targetNick));
        debug_Full(entry, cmd, "[KICK] no such nick\n");
        return;
    }

    if (!_channels.has_Member(channelName, targetClient->io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_User_not_in_channel(nick, targetNick, channelName));
        debug_Full(entry, cmd, "[KICK] target not on channel\n");
        return;
    }

    std::string comment = targetNick;
    if (!cmd.trailing.empty())
        comment = cmd.trailing;

    const std::string kickMsg =
        build_Kick_Message(entry.io.fd, channelName, targetNick, comment);

    if (!_channels.remove_Member_And_Cleanup(channelName, targetClient->io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_User_not_in_channel(nick, targetNick, channelName));
        debug_Full(entry, cmd, "[KICK] remove member failed\n");
        return;
    }

    send_To_Channel(channelName, kickMsg, out, entry.io.fd);
    push_Send(out, entry.io.fd, kickMsg);
    push_Send(out, targetClient->io.fd, kickMsg);

    debug_Full(entry, cmd, "[KICK] kick processed\n");
}

void IrcCore::handle_Mode( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    if (cmd.params.size() < 1)
    {
        push_Send(out, entry.io.fd,
                  build_Err_Need_more_params(
                      _clients.get_Nick(entry.io.fd), "MODE"));
        debug_Full(entry, cmd, "[MODE] channel name missing\n");
        return;
    }

    const std::string& channelName = cmd.params[0];
    const std::string  nick = _clients.get_Nick(entry.io.fd);

    if (!_channels.has_Channel(channelName))
    {
        push_Send(out, entry.io.fd,
                  build_Err_No_such_channel(nick, channelName));
        debug_Full(entry, cmd, "[MODE] no such channel\n");
        return;
    }

    if (!_channels.has_Member(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Not_on_channel(nick, channelName));
        debug_Full(entry, cmd, "[MODE] requester not on channel\n");
        return;
    }

    if (cmd.params.size() < 2)
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_Channel_mode_is(nick, channelName));
        debug_Full(entry, cmd, "[MODE] channel mode viewed\n");
        return;
    }

    if (!_channels.can_Change_Mode(channelName, entry.io.fd))
    {
        push_Send(out, entry.io.fd,
                  build_Err_Chan_oprivs_needed(nick, channelName));
        debug_Full(entry, cmd, "[MODE] operator privilege required\n");
        return;
    }

        const std::string& modeString = cmd.params[1];

    char        currentSign = 0;
    size_t      paramIndex = 2;
    std::string appliedModes;
    std::string appliedParams;

    for (size_t i = 0; i < modeString.size(); ++i)
    {
        const char token = modeString[i];

        if (token == '+' || token == '-')
        {
            currentSign = token;
            continue;
        }

        if (currentSign == 0)
        {
            push_Send(out, entry.io.fd,
                      build_Err_Unknown_mode(nick, token));
            debug_Full(entry, cmd, "[MODE] sign missing before mode char\n");
            return;
        }

        if (!is_Supported_Channel_Mode(token))
        {
            push_Send(out, entry.io.fd,
                      build_Err_Unknown_mode(nick, token));
            debug_Full(entry, cmd, "[MODE] unsupported mode char\n");
            return;
        }

        std::string modeParam;
        if (mode_Requires_Param(currentSign, token))
        {
            if (paramIndex >= cmd.params.size())
            {
                push_Send(out, entry.io.fd,
                          build_Err_Need_more_params(nick, "MODE"));
                debug_Full(entry, cmd, "[MODE] parameter missing\n");
                return;
            }

            modeParam = cmd.params[paramIndex];
            paramIndex++;
        }

        std::string oneMode;
        std::string oneParam;

        handleResult modeResult;

        modeResult = apply_Mode_Toggle(channelName, currentSign, token, oneMode);

        if (modeResult == MODE_UNKNOWN_CHAR)
        {
            modeResult = apply_Mode_Param(channelName, currentSign, token,
                                          modeParam, oneMode, oneParam);
        }

        if (modeResult == MODE_UNKNOWN_CHAR)
        {
            modeResult = apply_Mode_Member(channelName, currentSign, token,
                                           modeParam, oneMode, oneParam);
        }

        if (modeResult == MODE_APPLY_OK)
        {
            append_Mode_Change(appliedModes, appliedParams,
                               currentSign, token, oneParam);
            continue;
        }

        if (modeResult == MODE_PARAM_MISSING)
        {
            push_Send(out, entry.io.fd,
                      build_Err_Need_more_params(nick, "MODE"));
            debug_Full(entry, cmd, "[MODE] parameter missing\n");
            return;
        }

        if (modeResult == MODE_NO_SUCH_NICK)
        {
            push_Send(out, entry.io.fd,
                      build_Err_No_such_nick(nick, modeParam));
            debug_Full(entry, cmd, "[MODE] no such nick\n");
            return;
        }

        if (modeResult == MODE_USER_NOT_IN_CHANNEL)
        {
            push_Send(out, entry.io.fd,
                      build_Err_User_not_in_channel(nick, modeParam, channelName));
            debug_Full(entry, cmd, "[MODE] target not on channel\n");
            return;
        }

        if (modeResult == MODE_BAD_VALUE)
        {
            push_Send(out, entry.io.fd,
                      build_Err_Invalid_mode_param(nick, channelName, token, modeParam));
            debug_Full(entry, cmd, "[MODE] bad value\n");
            return;
        }

        if (modeResult == MODE_LAST_OPERATOR)
        {
            push_Send(out, entry.io.fd,
                      build_Err_Last_operator(nick, channelName));
            debug_Full(entry, cmd, "[MODE] cannot remove last operator\n");
            return;
        }

        push_Send(out, entry.io.fd,
                  build_Err_Unknown_mode(nick, token));
        debug_Full(entry, cmd, "[MODE] unsupported mode\n");
        return;
    }

    if (appliedModes.empty())
    {
        push_Send(out, entry.io.fd,
                  build_Rpl_Channel_mode_is(nick, channelName));
        debug_Full(entry, cmd, "[MODE] no applied mode\n");
        return;
    }

    const std::string modeMsg =
        build_Mode_Message(entry.io.fd, channelName, appliedModes, appliedParams);

    send_To_Channel(channelName, modeMsg, out, -1);

    debug_Full(entry, cmd, "[MODE] mode changed\n");
}

void IrcCore::handle_Quit( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out )
{
    std::string reason = "Client Quit";
    if (!cmd.trailing.empty())
        reason = cmd.trailing;

    std::set<int> peers;
    _channels.collect_Shared_Peers(entry.io.fd, peers);

    std::vector<std::string> joinedChannels;
    _channels.collect_User_Channels(entry.io.fd, joinedChannels);

    const std::string quitMsg = build_Quit_Message(entry.io.fd, reason);

    std::set<int>::const_iterator pit = peers.begin();
    for (; pit != peers.end(); ++pit)
        push_Send(out, *pit, quitMsg);

    for (size_t i = 0; i < joinedChannels.size(); ++i)
        _channels.remove_Member_And_Cleanup(joinedChannels[i], entry.io.fd);

    _clients.set_Closing(entry.io.fd, true);
    push_Close(out, entry.io.fd);

    debug_Full(entry, cmd, "[QUIT] quit processed\n");
}

void IrcCore::handle_Unknown( ClientEntry& entry,
                              const IrcCommand& cmd,
                              std::vector<IrcAction>& out )
{
    push_Send(out, entry.io.fd,
              build_Err_Unknown_command(_clients.get_Nick(entry.io.fd), cmd.verb));
    debug_Full(entry, cmd, debug_Message(HANDLE_UNKNOWN));
}

bool IrcCore::is_Supported_Channel_Mode( char modeChar ) const
{
    if (modeChar == 'i') return true;
    if (modeChar == 't') return true;
    if (modeChar == 'k') return true;
    if (modeChar == 'l') return true;
    if (modeChar == 'o') return true;
    return false;
}

bool IrcCore::allowed_Before_Register( const std::string& verb ) const
{
    if (verb == "CAP")      return true;
    if (verb == "PING")     return true;
    if (verb == "PONG")     return true;
    if (verb == "PASS")     return true;
    if (verb == "NICK")     return true;
    if (verb == "USER")     return true;
    if (verb == "QUIT")     return true;
    return false;
}

bool IrcCore::is_Known_Command( const std::string& verb ) const
{
    if (verb == "CAP")      return true;
    if (verb == "PING")     return true;
    if (verb == "PONG")     return true;
    if (verb == "PASS")     return true;
    if (verb == "NICK")     return true;
    if (verb == "USER")     return true;
    if (verb == "JOIN")     return true;
    if (verb == "PRIVMSG")  return true;
    if (verb == "PART")     return true;
    if (verb == "QUIT")     return true;
    if (verb == "MODE")     return true;
    if (verb == "TOPIC")    return true;
    if (verb == "INVITE")   return true;
    if (verb == "KICK")     return true;
    if (verb == "WHO")      return true;
    return false;
}

bool IrcCore::is_Nick_available( int fd,
                                 const std::string& nick ) const
{
    return !_clients.is_Nick_In_Use(nick, fd);
}

bool IrcCore::is_Number_String( const std::string& value ) const
{
    if (value.empty())
        return false;

    for (size_t i = 0; i < value.size(); ++i)
    {
        if (value[i] < '0' || value[i] > '9')
            return false;
    }

    return true;
}

std::string IrcCore::nick_Or_star( int fd ) const
{
    std::string nick = _clients.get_Nick(fd);
    return nick.empty() ? "*" : nick;
}

std::string IrcCore::build_Reply( const std::string& code,
                                  const std::string& target,
                                  const std::string& params,
                                  const std::string& text ) const
{
    std::string msg = ":irc.local "; // 추후에 다른걸로 뺄수도?
    msg += code;
    msg += " ";
    msg += target;

    if (!params.empty())
    {
        msg += " ";
        msg += params;
    }

    if (!text.empty())
    {
        msg += " :";
        msg += text;
    }

    msg += "\r\n";

    return msg;
}

std::string IrcCore::build_Welcome_Reply( const std::string& nick ) const
{
    std::string target = nick.empty() ? "*" : nick;

    return build_Reply(
        "001",
        target,
        "",
        "Welcome to the Internet Relay Network " + target
    );
}

std::string IrcCore::build_Err_Need_more_params( const std::string& nick,
                                                 const std::string& command ) const
{
    return build_Reply(
        "461",
        nick.empty() ? "*" : nick,
        command,
        "Not enough parameters"
    );
}

std::string IrcCore::build_Err_Already_registered( const std::string& nick ) const
{
    return build_Reply(
        "462",
        nick.empty() ? "*" : nick,
        "",
        "You may not reregister"
    );
}

std::string IrcCore::build_Err_Password_mismatch( const std::string& nick ) const
{
    return build_Reply(
        "464",
        nick.empty() ? "*" : nick,
        "",
        "Password incorrect"
    );
}

std::string IrcCore::build_Err_Unknown_command( const std::string& nick,
                                                const std::string& command ) const
{
    return build_Reply(
        "421",
        nick.empty() ? "*" : nick,
        command,
        "Unknown command"
    );
}

std::string IrcCore::build_Err_Not_registered( const std::string& nick ) const
{
    return build_Reply(
        "451",
        nick.empty() ? "*" : nick,
        "",
        "You have not registered"
    );
}

std::string IrcCore::build_Err_Erroneus_nickname( const std::string& nick,
                                                  const std::string& badNick ) const
{
    return build_Reply(
        "432",
        nick.empty() ? "*" : nick,
        badNick,
        "Erroneous nickname"
    );
}

std::string IrcCore::build_Err_Nickname_in_use( const std::string& nick,
                                                const std::string& badNick ) const
{
    return build_Reply(
        "433",
        nick.empty() ? "*" : nick,
        badNick,
        "Nickname is already in use"
    );
}

std::string IrcCore::build_Err_No_recipient( const std::string& nick,
                                             const std::string& command ) const
{
    return build_Reply(
        "411",
        nick.empty() ? "*" : nick,
        "",
        "No recipient given (" + command + ")"
    );
}

std::string IrcCore::build_Err_No_text_to_send( const std::string& nick ) const
{
    return build_Reply(
        "412",
        nick.empty() ? "*" : nick,
        "",
        "No text to send"
    );
}

std::string IrcCore::build_Err_No_such_nick( const std::string& nick,
                                             const std::string& targetNick ) const
{
    return build_Reply(
        "401",
        nick.empty() ? "*" : nick,
        targetNick,
        "No such nick/channel"
    );
}

std::string IrcCore::build_Err_No_such_channel( const std::string& nick,
                                                const std::string& channelName ) const
{
    return build_Reply(
        "403",
        nick.empty() ? "*" : nick,
        channelName,
        "No such channel"
    );
}

std::string IrcCore::build_Err_Cannot_send_to_chan( const std::string& nick,
                                                    const std::string& channelName ) const
{
    return build_Reply(
        "404",
        nick.empty() ? "*" : nick,
        channelName,
        "Cannot send to channel"
    );
}

std::string IrcCore::build_Err_Not_on_channel( const std::string& nick,
                                               const std::string& channelName ) const
{
    return build_Reply(
        "442",
        nick.empty() ? "*" : nick,
        channelName,
        "You're not on that channel"
    );
}

std::string IrcCore::build_Err_Chan_oprivs_needed( const std::string& nick,
                                                   const std::string& channelName ) const
{
    return build_Reply(
        "482",
        nick.empty() ? "*" : nick,
        channelName,
        "You're not channel operator"
    );
}

std::string IrcCore::build_Err_User_on_channel( const std::string& nick,
                                                const std::string& targetNick,
                                                const std::string& channelName ) const
{
    return build_Reply(
        "443",
        nick.empty() ? "*" : nick,
        targetNick + " " + channelName,
        "is already on channel"
    );
}

std::string IrcCore::build_Err_User_not_in_channel( const std::string& nick,
                                                    const std::string& targetNick,
                                                    const std::string& channelName ) const
{
    return build_Reply(
        "441",
        nick.empty() ? "*" : nick,
        targetNick + " " + channelName,
        "They aren't on that channel"
    );
}

std::string IrcCore::build_Err_Invite_only_chan( const std::string& nick,
                                                 const std::string& channelName ) const
{
    return build_Reply(
        "473",
        nick.empty() ? "*" : nick,
        channelName,
        "Cannot join channel (+i)"
    );
}

std::string IrcCore::build_Err_Channel_is_full( const std::string& nick,
                                                const std::string& channelName ) const
{
    return build_Reply(
        "471",
        nick.empty() ? "*" : nick,
        channelName,
        "Cannot join channel (+l)"
    );
}

std::string IrcCore::build_Err_Bad_channel_key( const std::string& nick,
                                                const std::string& channelName ) const
{
    return build_Reply(
        "475",
        nick.empty() ? "*" : nick,
        channelName,
        "Cannot join channel (+k)"
    );
}

std::string IrcCore::build_Err_Unknown_mode( const std::string& nick,
                                             char modeChar ) const
{
    return build_Reply(
        "472",
        nick.empty() ? "*" : nick,
        std::string(1, modeChar),
        "is unknown mode char to me"
    );
}

std::string IrcCore::build_Err_Invalid_mode_param( const std::string& nick,
                                                   const std::string& channelName,
                                                   char modeChar,
                                                   const std::string& param ) const
{
    return build_Reply(
        "696",
        nick.empty() ? "*" : nick,
        channelName + " " + std::string(1, modeChar) + " " + param,
        "Invalid mode parameter"
    );

}

std::string IrcCore::build_Err_Last_operator( const std::string& nick,
                                              const std::string& channelName ) const
{
    return build_Reply(
        "482",
        nick.empty() ? "*" : nick,
        channelName,
        "Cannot remove the last channel operator"
    );
}

//

std::string IrcCore::build_Rpl_Who_reply( const std::string& requesterNick,
                                          const std::string& channelName,
                                          int targetFd ) const
{
    const ClientEntry* target = _clients.find_By_Fd(targetFd);

    if (target == NULL)
        return "";

    const std::string nick = target->state.hasNick ? target->state.nick : "*";
    const std::string user = target->state.hasUser ? target->state.user : "*";
    const std::string realName = target->state.realName.empty() ? "*" : target->state.realName;

    std::string flags = "H";
    if (_channels.is_Operator(channelName, targetFd))
        flags += "@";

    return build_Reply(
        "352",
        requesterNick.empty() ? "*" : requesterNick,
        channelName + " " + user + " localhost irc.local " + nick + " " + flags,
        "0 " + realName
    );
}

std::string IrcCore::build_Rpl_End_of_who( const std::string& requesterNick,
                                           const std::string& name ) const
{
    return build_Reply(
        "315",
        requesterNick.empty() ? "*" : requesterNick,
        name,
        "End of /WHO list"
    );
}

std::string IrcCore::build_Rpl_Channel_mode_is( const std::string& nick,
                                                const std::string& channelName ) const
{
    std::string modes = "+";
    std::string params;

    if (_channels.is_Invite_Only(channelName))
        modes += "i";

    if (_channels.is_Topic_Op_Only(channelName))
        modes += "t";

    if (_channels.has_Key(channelName))
    {
        std::string key;
        if (_channels.get_Key(channelName, key))
        {
            modes += "k";

            if (!params.empty())
                params += "-";
            params += "*";
        }
    }

    if (_channels.has_User_Limit(channelName))
    {
        size_t limit = 0;
        if (_channels.get_User_Limit(channelName, limit))
        {
            std::ostringstream oss;
            oss << limit;

            modes += "l";

            if (!params.empty())
                params += " ";
            params += oss.str();
        }
    }

    return build_Reply(
        "324",
        nick.empty() ? "*" : nick,
        channelName + " " + modes + (params.empty() ? "" : " " + params),
        ""
    );
}

std::string IrcCore::build_Rpl_No_topic( const std::string& nick,
                                         const std::string& channelName ) const
{
    return build_Reply(
        "331",
        nick.empty() ? "*" : nick,
        channelName,
        "No topic is set"
    );
}

std::string IrcCore::build_Rpl_Topic( const std::string& nick,
                                      const std::string& channelName,
                                      const std::string& topic ) const
{
    return build_Reply(
        "332",
        nick.empty() ? "*" : nick,
        channelName,
        topic
    );
}

std::string IrcCore::build_Rpl_Inviting( const std::string& nick,
                                            const std::string& targetNick,
                                            const std::string& channelName ) const
{
    return build_Reply(
        "341",
        nick.empty() ? "*" : nick,
        targetNick + " " + channelName,
        ""
    );
}

std::string IrcCore::build_Rpl_Name_reply( const std::string& nick,
                                           const std::string& channelName ) const
{
    const std::string names = build_Channel_Names(channelName);

    return build_Reply(
        "353",
        nick.empty() ? "*" : nick,
        "= " + channelName,
        names
    );
}

std::string IrcCore::build_Rpl_End_of_names( const std::string& nick,
                                             const std::string& channelName ) const
{
    return build_Reply(
        "366",
        nick.empty() ? "*" : nick,
        channelName,
        "End of /NAMES list"
    );
}

// 텍스트 생성

std::string IrcCore::build_User_Prefix( int fd ) const
{
    const ClientEntry* client = _clients.find_By_Fd(fd);
    if (client == NULL)
        return "*!*@localhost";

    std::string nick = client->state.hasNick ? client->state.nick : "*";
    std::string user = client->state.hasUser ? client->state.user : "*";

    return nick + "!" + user + "@localhost";
}

void IrcCore::send_To_Channel( const std::string& channelName,
                               const std::string& message,
                               std::vector<IrcAction>& out,
                               int exceptFd ) const
{
    const ChannelEntry* channel = _channels.find_By_Name(channelName);
    if (channel == NULL)
        return;

    for (std::set<int>::const_iterator it = channel->members.begin();
         it != channel->members.end(); ++it)
    {
        if (*it == exceptFd)
            continue;
        push_Send(out, *it, message);
    }
}

void IrcCore::send_To_Shared_Peers( int fd,
                                    const std::string& message,
                                    std::vector<IrcAction>& out ) const
{
    std::set<int> peers;
    _channels.collect_Shared_Peers(fd, peers);

    std::set<int>::const_iterator it = peers.begin();
    for (; it != peers.end(); ++it)
        push_Send(out, *it, message);
}

std::string IrcCore::build_Join_Message( int fd,
                                         const std::string& channelName ) const
{
    return ":" + build_User_Prefix(fd) + " JOIN " + channelName + "\r\n";
}

std::string IrcCore::build_Part_Message( int fd,
                                         const std::string& channelName,
                                         const std::string& reason ) const
{
    std::string msg = ":" + build_User_Prefix(fd) + " PART " + channelName;
    if (!reason.empty())
        msg += " :" + reason;
    msg += "\r\n";
    return msg;
}

std::string IrcCore::build_Quit_Message( int fd, const std::string& reason ) const
{
    std::string msg = ":" + build_User_Prefix(fd) + " QUIT";
    if (!reason.empty())
        msg += " :" + reason;
    msg += "\r\n";
    return msg;
}

std::string IrcCore::build_Nick_Message( int fd, const std::string& newNick ) const
{
    return ":" + build_User_Prefix(fd) + " NICK :" + newNick + "\r\n";
}

std::string IrcCore::build_Privmsg_Message( int fd,
                                            const std::string& target,
                                            const std::string& text ) const
{
    return ":" + build_User_Prefix(fd) + " PRIVMSG " + target + " :" + text + "\r\n";
}

std::string IrcCore::build_Topic_Message( int fd,
                                          const std::string& channelName,
                                          const std::string& topic ) const
{
    return ":" + build_User_Prefix(fd) + " TOPIC " + channelName + " :" + topic + "\r\n";
}

std::string IrcCore::build_Invite_Message( int fd,
                                           const std::string& targetNick,
                                           const std::string& channelName ) const
{
    return ":" + build_User_Prefix(fd)
         + " INVITE " + targetNick + " :" + channelName + "\r\n";
}

std::string IrcCore::build_Channel_Names( const std::string& channelName ) const
{
    std::vector<int> members;
    _channels.collect_Channel_Members(channelName, members);

    std::string names;

    for (size_t i = 0; i < members.size(); ++i)
    {
        if (!names.empty())
            names += " ";

        if (_channels.is_Operator(channelName, members[i]))
            names += "@";

        names += nick_Or_star(members[i]);
    }

    return names;
}

std::string IrcCore::build_Kick_Message( int fd,
                                         const std::string& channelName,
                                         const std::string& targetNick,
                                         const std::string& comment ) const
{
    std::string msg = ":" + build_User_Prefix(fd)
                    + " KICK " + channelName + " " + targetNick;

    if (!comment.empty())
        msg += " :" + comment;

    msg += "\r\n";
    return msg;
}

std::string IrcCore::build_Mode_Message( int fd,
                                         const std::string& channelName,
                                         const std::string& modes,
                                         const std::string& param ) const
{
    std::string msg = ":" + build_User_Prefix(fd)
                    + " MODE " + channelName + " " + modes;

    if (!param.empty())
        msg += " " + param;

    msg += "\r\n";
    return msg;
}

std::string IrcCore::build_Cap_Message( const std::string& target,
                                        const std::string& subCommand,
                                        const std::string& text ) const
{
    std::string msg = ":irc.local CAP ";
    msg += target.empty() ? "*" : target;
    msg += " ";
    msg += subCommand;

    msg += " :";
    msg += text;

    msg += "\r\n";
    return msg;
}

std::string IrcCore::build_Pong_Message( const std::string& token ) const
{
    std::string msg = ":irc.local PONG :";
    msg += token;
    msg += "\r\n";
    return msg;
}