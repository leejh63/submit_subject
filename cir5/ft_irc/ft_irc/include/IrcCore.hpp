#ifndef IRC_CORE_HPP
#define IRC_CORE_HPP

#include <string>
#include <vector>

#include "ClientEntry.hpp"
#include "ClientRegistry.hpp"
#include "ChannelRegistry.hpp"
#include "IrcCommand.hpp"
#include "IrcAction.hpp"

// 에러 관련 / 메세지 관려 함수들 전부 따로 빼는게 좋을듯?

class IrcCore
{
private:
enum handleResult
{
    // join
    JOIN_OK,
    JOIN_ALREADY_MEMBER,
    JOIN_INVITE_ONLY,
    JOIN_BAD_KEY,
    JOIN_CHANNEL_FULL,

    // password
    PASS_ALREADY_REGISTERED,
    PASS_PARAM_MISSING,
    PASS_PASSWORD_OK,
    PASS_PASSWORD_BAD,

    // nickname
    NICK_PARAM_MISSING,
    NICK_ERRONEUS,
    NICK_IN_USE,
    NICK_OK,

    // user
    USER_PARAM_MISSING,
    USER_REALNAME_MISSING,
    USER_ALREADY_REGISTERED,
    USER_OK,

    // MODE
    MODE_APPLY_OK,
    MODE_PARAM_MISSING,
    MODE_NO_SUCH_NICK,
    MODE_USER_NOT_IN_CHANNEL,
    MODE_BAD_VALUE,
    MODE_BAD_FORMAT,
    MODE_UNKNOWN_CHAR,
    MODE_LAST_OPERATOR,
    MODE_SIGN_MISSING,

    // etc
    HANDLE_ERROR,
    HANDLE_UNKNOWN,
};

private:
    const std::string _server_password;
    ClientRegistry&   _clients;
    ChannelRegistry&  _channels;

public:
    IrcCore( const std::string& password,
             ClientRegistry& clients,
             ChannelRegistry&  channels);

    ~IrcCore( void );

    void    handle_Line( ClientEntry& entry,
                         const std::string& raw_Line,
                         std::vector<IrcAction>& out );

private:
    void    push_Send( std::vector<IrcAction>& out,
                       int fd,
                       const std::string& message ) const;

    void    push_Close( std::vector<IrcAction>& out,
                        int fd ) const;

private: // 임시 디버깅용
    void        debug_Short( const char* msg ) const;
    void        debug_Full( const ClientEntry& entry,
                            const IrcCommand& cmd,
                            const char* msg ) const;

    const char* debug_Message( handleResult result ) const;

private:
    handleResult    check_Pass( int fd, const IrcCommand& cmd ) const;
    handleResult    check_Nick( const IrcCommand& cmd ) const;
    handleResult    check_User( int fd, const IrcCommand& cmd ) const;
    handleResult    check_Join( const std::string& channelName, int fd, const std::string& providedKey ) const;
    handleResult    apply_Mode_Toggle( const std::string& channelName,
                                       char sign,
                                       char modeChar,
                                       std::string& appliedMode );

    handleResult    apply_Mode_Param( const std::string& channelName,
                                      char sign,
                                      char modeChar,
                                      const std::string& modeParam,
                                      std::string& appliedMode,
                                      std::string& appliedParam );

    handleResult    apply_Mode_Member( const std::string& channelName,
                                       char sign,
                                       char modeChar,
                                       const std::string& modeParam,
                                       std::string& appliedMode,
                                       std::string& appliedParam );

    bool            mode_Requires_Param( char sign,
                                         char modeChar ) const;

    bool            would_Remove_Last_Operator( const std::string& channelName,
                                                int targetFd ) const;

    void            append_Mode_Change( std::string& outModes,
                                        std::string& outParams,
                                        char sign,
                                        char modeChar,
                                        const std::string& appliedParam ) const;
//
    void    try_Register( ClientEntry& entry,
                          std::vector<IrcAction>& out );

    void    handle_Command( ClientEntry& entry,
                            const IrcCommand& cmd,
                            std::vector<IrcAction>& out );

    void    handle_Cap( ClientEntry& entry,
                        const IrcCommand& cmd,
                        std::vector<IrcAction>& out );

    void    handle_Ping( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Who( ClientEntry& entry,
                        const IrcCommand& cmd,
                        std::vector<IrcAction>& out );

    void    handle_Pong( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Error( ClientEntry& entry,
                          const IrcCommand& cmd,
                          std::vector<IrcAction>& out );

    void    handle_Pass( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Nick( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_User( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Join( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Part( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Privmsg( ClientEntry& entry,
                            const IrcCommand& cmd,
                            std::vector<IrcAction>& out );

    void    handle_Topic( ClientEntry& entry,
                          const IrcCommand& cmd,
                          std::vector<IrcAction>& out );

    void    handle_Invite( ClientEntry& entry,
                           const IrcCommand& cmd,
                           std::vector<IrcAction>& out );

    void    handle_Kick( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Mode( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Quit( ClientEntry& entry,
                         const IrcCommand& cmd,
                         std::vector<IrcAction>& out );

    void    handle_Unknown( ClientEntry& entry,
                            const IrcCommand& cmd,
                            std::vector<IrcAction>& out );

    bool    allowed_Before_Register( const std::string& verb ) const;
    bool    is_Known_Command( const std::string& verb ) const;
    bool    is_Nick_available( int fd, const std::string& nick ) const;
    bool    is_Number_String( const std::string& value ) const;
    bool    is_Supported_Channel_Mode( char modeChar ) const;
    
    std::string         nick_Or_star( int fd ) const;

    std::string         build_Reply( const std::string& code,
                                     const std::string& target,
                                     const std::string& params,
                                     const std::string& text ) const;

    std::string build_Welcome_Reply( const std::string& nick ) const;

    std::string build_Err_Need_more_params( const std::string& nick, const std::string& command ) const;
    std::string build_Err_Already_registered( const std::string& nick ) const;
    std::string build_Err_Password_mismatch( const std::string& nick ) const;
    std::string build_Err_Unknown_command( const std::string& nick, const std::string& command ) const;
    std::string build_Err_Not_registered( const std::string& nick ) const;
    std::string build_Err_Erroneus_nickname( const std::string& nick, const std::string& badNick ) const;
    std::string build_Err_Nickname_in_use( const std::string& nick, const std::string& badNick ) const;
    std::string build_Err_No_recipient( const std::string& nick, const std::string& command ) const;
    std::string build_Err_No_text_to_send( const std::string& nick ) const;
    std::string build_Err_No_such_nick( const std::string& nick, const std::string& targetNick ) const;
    std::string build_Err_No_such_channel( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_Cannot_send_to_chan( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_Not_on_channel( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_Chan_oprivs_needed( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_User_on_channel( const std::string& nick, const std::string& targetNick, const std::string& channelName ) const;
    std::string build_Err_Invite_only_chan( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_User_not_in_channel( const std::string& nick, const std::string& targetNick, const std::string& channelName ) const;
    std::string build_Err_Channel_is_full( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_Bad_channel_key( const std::string& nick, const std::string& channelName ) const;
    std::string build_Err_Unknown_mode( const std::string& nick, char modeChar ) const;
    std::string build_Err_Invalid_mode_param( const std::string& nick, const std::string& channelName, char modeChar, const std::string& param ) const;
    std::string build_Err_Last_operator( const std::string& nick, const std::string& channelName ) const;
    
    
    std::string build_Rpl_Who_reply( const std::string& requesterNick, const std::string& channelName, int targetFd ) const;
    std::string build_Rpl_End_of_who( const std::string& requesterNick, const std::string& name ) const;
    std::string build_Rpl_Channel_mode_is( const std::string& nick, const std::string& channelName ) const;
    std::string build_Rpl_No_topic( const std::string& nick, const std::string& channelName ) const;
    std::string build_Rpl_Topic( const std::string& nick, const std::string& channelName, const std::string& topic ) const;
    std::string build_Rpl_Inviting( const std::string& nick, const std::string& targetNick, const std::string& channelName ) const;
    std::string build_Rpl_Name_reply( const std::string& nick, const std::string& channelName) const;
    std::string build_Rpl_End_of_names( const std::string& nick, const std::string& channelName ) const;

private: // 텍스트 생성
    std::string build_User_Prefix( int fd ) const;

    void        send_To_Channel( const std::string& channelName,
                                 const std::string& message,
                                 std::vector<IrcAction>& out,
                                 int exceptFd ) const;

    void        send_To_Shared_Peers( int fd,
                                      const std::string& message,
                                      std::vector<IrcAction>& out ) const;

    std::string build_Cap_Message( const std::string& target,
                                   const std::string& subCommand,
                                   const std::string& text ) const;

    std::string build_Pong_Message( const std::string& token ) const;

    std::string build_Join_Message( int fd,
                                    const std::string& channelName ) const;

    std::string build_Part_Message( int fd,
                                    const std::string& channelName,
                                    const std::string& reason ) const;

    std::string build_Quit_Message( int fd,
                                    const std::string& reason ) const;

    std::string build_Nick_Message( int fd,
                                    const std::string& newNick ) const;

    std::string build_Privmsg_Message( int fd,
                                       const std::string& target,
                                       const std::string& text ) const;

    std::string build_Topic_Message( int fd,
                                     const std::string& channelName,
                                     const std::string& topic ) const;

    std::string build_Invite_Message( int fd,
                                      const std::string& targetNick,
                                      const std::string& channelName ) const;

    std::string build_Kick_Message( int fd,
                                    const std::string& channelName,
                                    const std::string& targetNick,
                                    const std::string& comment ) const;

    std::string build_Mode_Message( int fd,
                                    const std::string& channelName,
                                    const std::string& modes,
                                    const std::string& param ) const;
    
    std::string build_Channel_Names( const std::string& channelName ) const;

private: // 금지
    IrcCore( void );
    IrcCore( const IrcCore& );
    IrcCore& operator=( const IrcCore& );
};

#endif