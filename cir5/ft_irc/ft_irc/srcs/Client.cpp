#include "Client.hpp"

Client::Client( void )
: inBuf()
, outBuf()
, nick()
, user()
, realName()
, wantWrite(false)
, closing(false)
, _fd(-1)
, passOk(false)
, hasNick(false)
, hasUser(false)
, registered(false)
{
}

Client::Client( int cfd )
: inBuf()
, outBuf()
, nick()
, user()
, realName()
, wantWrite(false)
, closing(false)
, _fd(cfd)
, passOk(false)
, hasNick(false)
, hasUser(false)
, registered(false)
{
}

Client::~Client( void )
{
}

int Client::fd( void ) const
{
    return _fd.get();   // Fd 클래스 인터페이스에 맞게 조정
}

bool Client::get_passOk( void ) const
{
    return passOk;
}

bool Client::get_hasNick( void ) const
{
    return hasNick;
}

bool Client::get_hasUser( void ) const
{
    return hasUser;
}

bool Client::get_registered( void ) const
{
    return registered;
}

void Client::set_passOk( bool v )
{
    passOk = v;
}

void Client::set_hasNick( bool v )
{
    hasNick = v;
}

void Client::set_hasUser( bool v )
{
    hasUser = v;
}

void Client::set_registered( bool v )
{
    registered = v;
}