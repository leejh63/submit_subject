
#include "Server.hpp"    // Server 클래스 선언, 멤버/메서드 시그니처
#include "ClientIo.hpp"
#include "ClientState.hpp"
#include "IrcAction.hpp"
#include "Signal.hpp"
#include "Error.hpp"    // err_word(), ErrFunc(EF_SOCKET 등)
#include "tmp_ClientAdapter.hpp"

#include <stdexcept>     // std::runtime_error 예외 던질 때 필요
#include <cerrno>        // errno 전역 변수, errno 값(EADDRINUSE 등) 읽을 때 필요
#include <cstddef>       // size_t
#include <cstring>       // std::memset (sockaddr_in 0으로 초기화)
#include <unistd.h>      // close
#include <fcntl.h>       // fcntl
#include <sys/types.h>   // (전통적으로 소켓 타입 정의에 필요, 일부 시스템에서 socklen_t 등)
#include <sys/socket.h>  // socket(), setsockopt(), bind() 같은 소켓 API 선언
#include <netinet/in.h>  // sockaddr_in, AF_INET, INADDR_ANY, htons/htonl(플랫폼에 따라)
#include <arpa/inet.h>   // htons/htonl, inet_pton/inet_ntop 등 주소 변환 함수들

#include <iostream>      // 디버깅용!
void Server::debug_Server_State( const std::string& msg, int fd ) const
{
    std::cout << "\n========================================\n";
    std::cout << msg << " fd=" << fd << "\n";

    _clientRegistry.debug_Print_All();
    _channelRegistry.debug_Print_All();

    std::cout << "========================================\n";
}


Server::Server( int port, const std::string& password )
: _port(port)
, _password(password)
, _listenFd()
, _running(false)
, _pfds()
, _clients()
, _new_clients()
, _clientRegistry(_new_clients)
, _channelRegistry()
, _core(password, _clientRegistry, _channelRegistry)
{
    // todo?
}
Server:: ~Server( void ) 
{
    // todo?
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();
}

static void setNonBlocking(int fd) { 
    int flags = fcntl(fd, F_GETFL, 0); 
    if (flags < 0) { 
        const int e = errno;
        throw std::runtime_error(err_word(e, EF_FCNTL));
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        const int e = errno;
        throw std::runtime_error(err_word(e, EF_FCNTL));
    }
}

void Server::init_Socket( void )
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    //int fd = -1; // 에러 테스트
    if (fd < 0) {
        const int e = errno;
        //const int e = EINVAL; // 에러 테스트
        throw std::runtime_error(err_word(e, EF_SOCKET));
    }
    _listenFd.reset(fd);
    setNonBlocking(_listenFd.get());
}

void Server::init_Setsockopt( void )
{
    int reuse = 1;
    // const int e = EBADF; // 에러 테스트
    // throw std::runtime_error(err_word(e, 2)); // 에러 테스트
    if (setsockopt(_listenFd.get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        const int e = errno;
        throw std::runtime_error(err_word(e, EF_SETSOCKOPT));
    }
}

void Server::init_Bind( void )
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(_port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(_listenFd.get(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        const int e = errno;
        throw std::runtime_error(err_word(e, EF_BIND));
    }
}

void Server::init_Listen( void )
{
    if (listen(_listenFd.get(), SOMAXCONN) < 0) {
        const int e = errno;
        throw std::runtime_error(err_word(e, EF_LISTEN));
    }
}

void Server::init_pfds( void )
{
    _pfds.clear();

    pollfd p;
    p.fd = _listenFd.get();
    p.events = POLLIN;   // “새 연결 요청” 감시
    p.revents = 0;

    _pfds.push_back(p);
}

bool Server::isBlock( int e )
{
#ifdef EWOULDBLOCK
    return (e == EAGAIN || e == EWOULDBLOCK);
#else
    return (e == EAGAIN);
#endif
}

void Server::acceptClient( void )
{
    while (true)
    {
        sockaddr_in cliaddr;
        socklen_t   len = sizeof(cliaddr);

        int cfd = accept(_listenFd.get(), (sockaddr*)&cliaddr, &len);
        if (cfd < 0)
        {
            const int e = errno;
            //if (errno == EAGAIN) break; // 더 이상 연결 없음
            if (isBlock(e)) break;
            throw std::runtime_error(err_word(e, EF_ACCEPT));
        }

        // 1) poll 대상에 추가
        pollfd p;
        p.fd = cfd;
        p.events = POLLIN;   // 아직은 읽기만(다음 단계에서 outBuf 생기면 POLLOUT 켬)
        p.revents = 0;
        
        // 2) Client 상태 저장소에 추가
        Client* cli_ent = NULL;
        try
        {
            setNonBlocking(cfd);
            cli_ent = new Client(cfd);
            _clients.insert(std::make_pair(cfd, cli_ent));
            _pfds.push_back(p);

            if (!_clientRegistry.add_Client(cfd))
                throw std::runtime_error("client registry add failed");

        }
        catch (const std::exception& e)
        {
            // 1) map에 들어갔으면 map 기준으로 정리 (가장 안전)
            std::map<int, Client*>::iterator it = _clients.find(cfd);
            if (it != _clients.end()) {
                delete it->second;       // delete client
                _clients.erase(it);
            } else if (cli_ent) {
                delete cli_ent;          // insert 전에 터진 경우
            } else {
                close(cfd);              // setNonBlocking에서 터진 경우
            }

            std::cout << e.what() << std::endl;
            continue;
        }

        // (선택) 디버그
        std::cout << "Server: Accepted fd=" << cfd << "\n";
    }
}

bool Server::removeClient( size_t idx )
{
    int fd = _pfds[idx].fd;
    _channelRegistry.remove_Client_From_All_Channels(fd);
    
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        delete it->second;
        _clients.erase(it);
    }
    else {
        if (fd >= 0) close(fd);
    }

    _clientRegistry.remove_Client(fd);
    _pfds.erase(_pfds.begin() + idx);
    debug_Server_State("[Server::removeClient] after channel cleanup", fd);
    std::cout << "Server: remove!  idx / left: " << _pfds.size() - 1 << "\n";
    return true;
}

bool Server::buf_Copy_in_to_out ( int fd, const std::string& msg )
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return false;

    Client* c = it->second;

    // outBuf 상한 체크 (오버플로 방지 형태로)
    // c->outBuf.size() + msg.size() > MAX_OUTBUF 를 안전하게 표현
    if (msg.size() > (MAX_OUTBUF - c->outBuf.size()))
        return false;

    c->outBuf += msg;
    return true;
}

// 함수명이 맘에안듬
bool Server::enqueue( int fd, const std::string& msg )
{
    if (!buf_Copy_in_to_out(fd, msg))
        return false;

    int idx = findPfdIndexByFd(fd);
    if (idx >= 0)
        _pfds[idx].events |= POLLOUT; // write 관심 등록
    return true;
}

bool Server::extractLine( std::string& buf, std::string& line )
{
    // 1) CRLF 우선
    std::string::size_type pos = buf.find("\r\n");
    if (pos != std::string::npos)
    {
        line = buf.substr(0, pos);
        buf.erase(0, pos + 2);
        return true;
    }

    // 2) LF
    pos = buf.find('\n');
    if (pos != std::string::npos)
    {
        line = buf.substr(0, pos);
        buf.erase(0, pos + 1);

        // 라인 끝에 \r이 남아있으면 제거 (방어)
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        return true;
    }

    return false; // 아직 한 줄 완성 안 됨
}

bool Server::recv_from_Client( size_t idx )
{
    const int fd = _pfds[idx].fd;

    std::map<int, Client*>::iterator lit = _clients.find(fd);
    if (lit == _clients.end()) {
        std::cout << "Server: recv internal desync legacy fd=" << fd << "\n";
        return removeClient(idx);
    }

    ClientEntry* entryPtr = _clientRegistry.find_By_Fd(fd);
    if (entryPtr == NULL) {
        std::cout << "Server: recv internal desync new fd=" << fd << "\n";
        return removeClient(idx);
    }

    
    Client* legacy = lit->second;
    ClientEntry& entry = *entryPtr;

    char buf[MAX_INBUF];

    while (true)
    {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);

        if (n > 0)
        {
            legacy->inBuf.append(buf, n);

            if (legacy->inBuf.size() > MAX_INBUF)
                return removeClient(idx);

            std::string line;
            while (extractLine(legacy->inBuf, line))
            {
                getClientEntry(*legacy, entry);

                std::vector<IrcAction> actions;
                // _core.handle_Line(entry.io, entry.state, line, actions);
                _core.handle_Line(entry, line, actions);
                
                setClientEntry(*legacy, entry);

                for (size_t i = 0; i < actions.size(); ++i)
                {
                    if (actions[i].type == ACT_SEND)
                    {
                        if (!enqueue(actions[i].fd, actions[i].message))
                            std::cout << "Server: enqueue failed fd="
                                      << actions[i].fd << "\n";
                    }
                    else if (actions[i].type == ACT_CLOSE)
                    {
                        if (actions[i].fd == fd)
                            return removeClient(idx);
                    }
                }
            }
            continue;
        }

        if (n == 0)
            return removeClient(idx);

        if (n < 0)
        {
            const int e = errno;
            if (isBlock(e)) break;
            if (e == EINTR) continue;
            return removeClient(idx);
        }
    }

    return false;
}
// 벡터에서 이진탐색방식으로 찾는 방식으로 변경 가능// find? 
int Server::findPfdIndexByFd( int fd )
{
    for (size_t i = 0; i < _pfds.size(); ++i)
    {
        if (_pfds[i].fd == fd)
            return static_cast<int>(i);
    }
    return -1;
}

bool Server::send_to_Client( size_t idx )
{
    const int fd = _pfds[idx].fd;

    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end()) { 
        std::cout << "Server: send internal desync, fd=" << fd << "\n";
        return removeClient(idx); 
    }

    Client* c = it->second;

    if (c->outBuf.empty()) {
        _pfds[idx].events &= ~POLLOUT;
        return false;
    }

    while (!c->outBuf.empty())
    {
        ssize_t n = send(fd, c->outBuf.c_str(), c->outBuf.size(), 0);

        if (n > 0) {
            c->outBuf.erase(0, static_cast<size_t>(n));
            continue;
        }
        if (n == 0)
        {
            std::cout << "Server: send n == 0 closed fd=" << fd << "\n";
            return removeClient(idx);
        }
        if (n < 0) {
            const int e = errno;
            if (isBlock(e)) return false;
            if (e == EINTR) continue;
            std::cout << "Server: send error fd=" << fd
                      << " err=" << err_word(e, EF_SEND) << "\n";
        }

        return removeClient(idx);
    }

    _pfds[idx].events &= ~POLLOUT;
    return false;
}

void Server::Poll(void)
{
    init_pfds();
    _running = true;
    while (_running)
    {
        // 임시 이쪽으로 시그널 출력 변경?
        if (Signal::getFlag()) { 

            _running = false;
            break;
            // 시그널 테스트용 
            // Signal::clearFlag();
            // continue;
        }
        
        int ret = poll(&_pfds[0], _pfds.size(), -1);
        if (ret < 0) {
            const int e = errno;
            if (e == EINTR) continue;
            throw std::runtime_error(err_word(e, EF_POLL));
        }

        if (_pfds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            if (_pfds[0].revents & POLLHUP) std::cout << "Server : POLLHUP!\n";
            else if (_pfds[0].revents & POLLERR) std::cout << "Server : POLLERR!\n";
            else if (_pfds[0].revents & POLLNVAL) std::cout << "Server : POLLNVAL!\n";
            throw std::runtime_error("listen fd poll error");
        }

        if (_pfds[0].revents & POLLIN) { acceptClient(); }

        for (size_t i = 1; i < _pfds.size(); /* no ++ */)
        {
            short rev = _pfds[i].revents;
            if (rev == 0) { ++i; continue; }

            if (rev & (POLLERR | POLLHUP | POLLNVAL)) {
                if (rev & POLLHUP) std::cout << "Server_c : POLLHUP!\n";
                if (rev & POLLERR) std::cout << "Server_c : POLLERR!\n";
                if (rev & POLLNVAL) std::cout << "Server_c : POLLNVAL!\n";
                removeClient(i);
                continue; // i 그대로, 당겨진 새 원소 재검사
            }

            // 삭제됐으니 i 증가 금지
            // 삭제 안 됐으면 그대로 다음 체크 진행
            if (rev & POLLIN) {
                if (recv_from_Client(i)) continue; 

            }

            // 여기서 rev를 다시 사용하지 말고, "현재 i가 살아있다"는 가정 하에
            // revents를 다시 읽지 않는 게 포인트.
            if (rev & POLLOUT) {
                if (send_to_Client(i)) continue;
            }

            ++i;
        }

        
        if (_pfds.size() == 1) _running = false; // 테스트용 임시 코드임
    }
}

void Server::initRoom( void )
{
    init_Socket();
    init_Setsockopt();
    init_Bind();
    init_Listen();
}