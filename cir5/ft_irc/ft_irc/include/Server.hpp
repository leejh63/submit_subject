#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>

#include <poll.h>

#include "Fd.hpp"
#include "IrcCore.hpp"
#include "Client.hpp"
#include "ClientEntry.hpp"
#include "ClientRegistry.hpp"
#include "ChannelRegistry.hpp"

class Client;

class Server {
private:
    int                         _port;
    std::string                 _password;
    Fd                          _listenFd;
    bool                        _running;

    std::vector<pollfd>         _pfds;
    std::map<int, Client*>      _clients; // fd -> Client
    std::map<int, ClientEntry>  _new_clients;

    ClientRegistry              _clientRegistry;
    ChannelRegistry             _channelRegistry;
    IrcCore                     _core;

public:
    Server( int port, const std::string& password );
    ~Server( void );

    void initRoom( void );
    void Poll( void );

public: // 디버깅용
    void    debug_Server_State( const std::string& msg, int fd ) const;

private: 
    static const size_t MAX_INBUF  = 4 * 1024;
    static const size_t MAX_OUTBUF = 64 * 1024;
    static const size_t MAX_IRC_LINE = 510;

    // Server::initRoom
    void init_Socket( void );
    void init_Setsockopt( void );
    void init_Bind( void );
    void init_Listen( void );

    // Server::Poll
    void init_pfds( void );
    void acceptClient( void );
    bool removeClient( size_t idx );

    bool recv_from_Client( size_t idx );
    bool extractLine( std::string& buf, std::string& line );
   
    bool send_to_Client( size_t idx );
    bool buf_Copy_in_to_out ( int fd, const std::string& msg );
    bool enqueue( int fd, const std::string& msg );
    int  findPfdIndexByFd( int fd );
    bool isBlock( int e );

private: // 임시
    // Server::recv_to_Client
    // bool onLine( int fd, const std::string& line );

private: // 금지
    Server( void );
    Server( const Server& );
    Server& operator=( const Server& );
};

#endif