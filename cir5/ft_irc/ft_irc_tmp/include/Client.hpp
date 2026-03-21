#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "Fd.hpp"

class Client
{
public:
    // recv 누적 버퍼
    std::string inBuf;

    // send 누적 버퍼
    std::string outBuf;

    // IRC state
    std::string nick;
    std::string user;
    std::string realName;

    // IO state
    bool wantWrite;
    bool closing;

private:
    Fd          _fd;

    // registration state
    bool        passOk;
    bool        hasNick;
    bool        hasUser;
    bool        registered;

public:
    Client( void );
    Client( int cfd );
    ~Client( void );

    int fd( void ) const;

    bool get_passOk( void ) const;
    bool get_hasNick( void ) const;
    bool get_hasUser( void ) const;
    bool get_registered( void ) const;

    void set_passOk( bool v );
    void set_hasNick( bool v );
    void set_hasUser( bool v );
    void set_registered( bool v );

private: // 금지
    Client( const Client& );
    Client& operator=( const Client& );
};

#endif