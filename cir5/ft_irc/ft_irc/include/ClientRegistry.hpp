#ifndef CLIENTREGISTRY_HPP
#define CLIENTREGISTRY_HPP

#include <map>
#include <string>

#include "ClientEntry.hpp"

class ClientRegistry
{
public:
    // ClientRegistry( void ); // 최종 목표
    ClientRegistry( std::map<int, ClientEntry>& clients ); // 임시 서버 호환용
    ~ClientRegistry( void );

public: // 디버깅
    void    debug_Print_All( void ) const;

private:
    // std::map<int, ClientEntry> _clients; // 최종 목표
    std::map<int, ClientEntry>& _clients; // 임시 서버 호환용

public:
    bool                has_Client( int fd ) const;

    ClientEntry*        find_By_Fd( int fd );
    const ClientEntry*  find_By_Fd( int fd ) const;

    ClientEntry*        find_By_Nick( const std::string& nick );
    const ClientEntry*  find_By_Nick( const std::string& nick ) const;
    int                 get_Fd_By_Nick( const std::string& nick ) const;

    bool                add_Client( int fd );
    bool                remove_Client( int fd );

    bool                is_Nick_In_Use( const std::string& nick,
                                        int exceptFd ) const;

    bool                is_Registered( int fd ) const;

    // 현재는 io,state가 구조체이다 보니 저장소에서 바로 건드림
    // 나중에는 클래스로 변경해서 좀더 객체지향적으로 변경예정
    bool                set_Pass_Ok( int fd, bool value );
    bool                set_Registered( int fd, bool value ); // 임시 유지, 사용은 지양
    bool                set_Nick( int fd, const std::string& newNick );
    bool                set_User( int fd,
                                  const std::string& user,
                                  const std::string& realName );
    bool                set_Closing( int fd, bool value );

    bool                update_Registered_State( int fd );
    bool                try_Register( int fd );

    bool                has_Pass_Ok( int fd ) const;
    bool                has_Nick( int fd ) const;
    bool                has_User( int fd ) const;

    std::string         get_Nick( int fd ) const;

private: // 금지
    ClientRegistry( void );
    ClientRegistry( const ClientRegistry& );
    ClientRegistry& operator=( const ClientRegistry& );
};

#endif