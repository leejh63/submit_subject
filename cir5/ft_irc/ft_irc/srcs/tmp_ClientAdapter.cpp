#include "tmp_ClientAdapter.hpp"

void getClientIo(const Client& c, ClientIo& out)
{
    out.fd        = c.fd();
    out.inBuf     = c.inBuf;
    out.outBuf    = c.outBuf;
    out.wantWrite = c.wantWrite;
    out.closing   = c.closing;
}

void setClientIo(Client& c, const ClientIo& in)
{
    c.inBuf     = in.inBuf;
    c.outBuf    = in.outBuf;
    c.wantWrite = in.wantWrite;
    c.closing   = in.closing;
}

void getClientState(const Client& c, ClientState& out)
{
    out.passOk     = c.get_passOk();
    out.hasNick    = c.get_hasNick();
    out.hasUser    = c.get_hasUser();
    out.registered = c.get_registered();

    out.nick       = c.nick;
    out.user       = c.user;
    out.realName   = c.realName;
}

void setClientState(Client& c, const ClientState& in)
{
    c.set_passOk(in.passOk);
    c.set_hasNick(in.hasNick);
    c.set_hasUser(in.hasUser);
    c.set_registered(in.registered);

    c.nick      = in.nick;
    c.user      = in.user;
    c.realName  = in.realName;
}

void getClientEntry( const Client& src, ClientEntry& dst )
{
    dst.io.fd        = src.fd();
    dst.io.inBuf     = src.inBuf;
    dst.io.outBuf    = src.outBuf;
    dst.io.wantWrite = src.wantWrite;
    dst.io.closing   = src.closing;

    dst.state.passOk     = src.get_passOk();
    dst.state.hasNick    = src.get_hasNick();
    dst.state.hasUser    = src.get_hasUser();
    dst.state.registered = src.get_registered();

    dst.state.nick     = src.nick;
    dst.state.user     = src.user;
    dst.state.realName = src.realName;
}

void setClientEntry( Client& dst, const ClientEntry& src )
{
    dst.inBuf      = src.io.inBuf;
    dst.outBuf     = src.io.outBuf;
    dst.wantWrite  = src.io.wantWrite;
    dst.closing    = src.io.closing;

    dst.nick       = src.state.nick;
    dst.user       = src.state.user;
    dst.realName   = src.state.realName;

    dst.set_passOk(src.state.passOk);
    dst.set_hasNick(src.state.hasNick);
    dst.set_hasUser(src.state.hasUser);
    dst.set_registered(src.state.registered);
}