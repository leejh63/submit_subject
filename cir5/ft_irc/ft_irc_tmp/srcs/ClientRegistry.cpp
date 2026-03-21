#include "ClientRegistry.hpp"


#include <iostream> // 디버깅용
void ClientRegistry::debug_Print_All( void ) const
{
    std::cout << "\n====== CLIENT STATE ======\n";

    if (_clients.empty())
    {
        std::cout << "(no clients)\n";
        std::cout << "==========================\n";
        return;
    }

    for (std::map<int, ClientEntry>::const_iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        const ClientEntry& entry = it->second;

        std::cout << "fd=" << entry.io.fd << "\n"
                  << " passOk=" << entry.state.passOk << "\n"
                  << " hasNick=" << entry.state.hasNick << "\n"
                  << " hasUser=" << entry.state.hasUser << "\n"
                  << " registered=" << entry.state.registered << "\n"
                  << " nick='" << entry.state.nick << "'" << "\n"
                  << " user='" << entry.state.user << "'" << "\n"
                  << " realName='" << entry.state.realName << "'" << "\n"
                  << " wantWrite=" << entry.io.wantWrite << "\n"
                  << " closing=" << entry.io.closing
                  << "\n";
    }

    std::cout << "==========================\n";
}

ClientRegistry::ClientRegistry( std::map<int, ClientEntry>& clients )
: _clients(clients)
{
}

ClientRegistry::~ClientRegistry( void )
{
}

bool ClientRegistry::has_Client( int fd ) const
{
    return (_clients.find(fd) != _clients.end());
}

ClientEntry* ClientRegistry::find_By_Fd( int fd )
{
    std::map<int, ClientEntry>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return NULL;
    return &(it->second);
}

const ClientEntry* ClientRegistry::find_By_Fd( int fd ) const
{
    std::map<int, ClientEntry>::const_iterator it = _clients.find(fd);
    if (it == _clients.end())
        return NULL;
    return &(it->second);
}

ClientEntry* ClientRegistry::find_By_Nick( const std::string& nick )
{
    std::map<int, ClientEntry>::iterator it = _clients.begin();
    std::map<int, ClientEntry>::iterator end = _clients.end();

    while (it != end)
    {
        ClientState& state = it->second.state;
        if (state.hasNick && state.nick == nick)
            return &(it->second);
        ++it;
    }
    return NULL;
}

const ClientEntry* ClientRegistry::find_By_Nick( const std::string& nick ) const
{
    std::map<int, ClientEntry>::const_iterator it = _clients.begin();
    std::map<int, ClientEntry>::const_iterator end = _clients.end();

    while (it != end)
    {
        const ClientState& state = it->second.state;
        if (state.hasNick && state.nick == nick)
            return &(it->second);
        ++it;
    }
    return NULL;
}

int ClientRegistry::get_Fd_By_Nick( const std::string& nick ) const
{
    std::map<int, ClientEntry>::const_iterator it = _clients.begin();
    for (; it != _clients.end(); ++it)
    {
        if (it->second.state.hasNick && it->second.state.nick == nick)
            return it->first;
    }
    return -1;
}

bool ClientRegistry::add_Client( int fd )
{
    if (has_Client(fd))
        return false;

    ClientEntry entry;

    entry.io.fd = fd;
    entry.io.inBuf = "";
    entry.io.outBuf = "";
    entry.io.wantWrite = false;
    entry.io.closing = false;

    entry.state.passOk = false;
    entry.state.hasNick = false;
    entry.state.hasUser = false;
    entry.state.registered = false;
    entry.state.nick = "";
    entry.state.user = "";
    entry.state.realName = "";

    _clients.insert(std::make_pair(fd, entry));
    return true;
}

bool ClientRegistry::remove_Client( int fd )
{
    std::map<int, ClientEntry>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return false;

    _clients.erase(it);
    return true;
}

bool ClientRegistry::is_Nick_In_Use( const std::string& nick,
                                     int exceptFd ) const
{
    std::map<int, ClientEntry>::const_iterator it = _clients.begin();
    std::map<int, ClientEntry>::const_iterator end = _clients.end();

    while (it != end)
    {
        if (it->first != exceptFd)
        {
            const ClientState& state = it->second.state;
            // 
            if (state.hasNick && state.nick == nick)
                return true;
        }
        ++it;
    }
    return false;
}

bool ClientRegistry::is_Registered( int fd ) const
{
    const ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;
    // 
    return entry->state.registered;
}

bool ClientRegistry::update_Registered_State( int fd )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    if (entry->state.passOk &&
        entry->state.hasNick &&
        entry->state.hasUser)
    {
        entry->state.registered = true;
    }
    else
    {
        entry->state.registered = false;
    }
    return true;
}

bool ClientRegistry::try_Register( int fd )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    if (entry->state.registered)
        return false;

    if (!entry->state.passOk)
        return false;
    if (!entry->state.hasNick)
        return false;
    if (!entry->state.hasUser)
        return false;

    entry->state.registered = true;
    return true;
}

bool ClientRegistry::set_Pass_Ok( int fd, bool value )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    entry->state.passOk = value;
    return true;
}

bool ClientRegistry::set_Registered( int fd, bool value )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    entry->state.registered = value;
    return true;
}

bool ClientRegistry::set_Nick( int fd, const std::string& newNick )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    entry->state.nick = newNick;
    entry->state.hasNick = !newNick.empty();
    return true;
}

bool ClientRegistry::set_User( int fd,
                               const std::string& user,
                               const std::string& realName )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    entry->state.user = user;
    entry->state.realName = realName;
    entry->state.hasUser = !user.empty();
    return true;
}

bool ClientRegistry::set_Closing( int fd, bool value )
{
    ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;

    entry->io.closing = value;
    return true;
}

bool ClientRegistry::has_Pass_Ok( int fd ) const
{
    const ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;
    return entry->state.passOk;
}

bool ClientRegistry::has_Nick( int fd ) const
{
    const ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;
    return entry->state.hasNick;
}

bool ClientRegistry::has_User( int fd ) const
{
    const ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return false;
    return entry->state.hasUser;
}

std::string ClientRegistry::get_Nick( int fd ) const
{
    const ClientEntry* entry = find_By_Fd(fd);
    if (entry == NULL)
        return "";
    return entry->state.nick;
}
