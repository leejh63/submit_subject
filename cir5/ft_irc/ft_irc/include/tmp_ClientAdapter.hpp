#ifndef CLIENT_ADAPTER_HPP
#define CLIENT_ADAPTER_HPP

#include "Client.hpp"
#include "ClientIo.hpp"
#include "ClientState.hpp"
#include "ClientEntry.hpp"

void getClientEntry( const Client& src, ClientEntry& dst );
void setClientEntry( Client& dst, const ClientEntry& src );

void getClientIo( const Client& c, ClientIo& out );
void setClientIo( Client& c, const ClientIo& in );

void getClientState( const Client& c, ClientState& out );
void setClientState( Client& c, const ClientState& in );

#endif