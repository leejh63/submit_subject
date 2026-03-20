#ifndef APP_CTRL_COMMAND_H
#define APP_CTRL_COMMAND_H

#include <stdint.h>

/*
 * UART 입력 문자열을 CAN 전송용 제어 명령 구조로 변환하기 위한 정의 모음.
 * 이 헤더는 "문자열 입력"과 "정규화된 명령 데이터" 사이의 경계를 담당한다.
 */

#define APP_CTRL_COMMAND_TEXT_SIZE      32U   /* text 명령 본문 최대 길이 */
#define APP_CTRL_COMMAND_INPUT_SIZE     96U   /* 파싱용 입력 임시 버퍼 최대 길이 */
#define APP_CMD_ARGS_UNBOUNDED          255U  /* 가변 인자 개수 표기용 */

typedef enum
{
    APP_CTRL_COMMAND_NONE = 0,  /* 초기 상태 / 미지정 */
    APP_CTRL_COMMAND_OPEN,      /* open <id|all> */
    APP_CTRL_COMMAND_CLOSE,     /* close <id|all> */
    APP_CTRL_COMMAND_OFF,       /* off <id|all> */
    APP_CTRL_COMMAND_TEST,      /* test <id|all> */
    APP_CTRL_COMMAND_TEXT,      /* text <id|all> <msg> */
    APP_CTRL_COMMAND_EVENT      /* event <id|all> <eventCode> <arg0> <arg1> */
} AppCtrlCommandType;

typedef enum
{
    APP_CMD_GRAMMAR_NONE = 0,
    APP_CMD_GRAMMAR_TARGET_ONLY,
    APP_CMD_GRAMMAR_TARGET_TEXT,
    APP_CMD_GRAMMAR_TARGET_U8_U8_U8,
    APP_CMD_GRAMMAR_LOCAL_ONLY
} AppCmdGrammar;

typedef struct
{
    uint8_t hasValue;       /* target 사용 여부 */
    uint8_t nodeId;         /* 실제 target node id */
    uint8_t isBroadcast;    /* broadcast 여부 */
} AppCmdTarget;

typedef struct
{
    char text[APP_CTRL_COMMAND_TEXT_SIZE];
} AppCmdTextArgs;

typedef struct
{
    uint8_t eventCode;
    uint8_t arg0;
    uint8_t arg1;
} AppCmdEventArgs;

typedef union
{
    AppCmdTextArgs  textArgs;
    AppCmdEventArgs eventArgs;
} AppCmdPayload;

typedef struct
{
    AppCtrlCommandType type;
    AppCmdTarget       target;
    AppCmdPayload      payload;
} AppCtrlCommand;

typedef struct
{
    const char         *name;
    AppCtrlCommandType  type;
    AppCmdGrammar       grammar;
    uint8_t             minArgs;   /* command 이름 뒤 인자 최소 개수 */
    uint8_t             maxArgs;   /* command 이름 뒤 인자 최대 개수 */
} AppCmdSpec;

typedef enum
{
    APP_CTRL_COMMAND_PARSE_OK = 0,
    APP_CTRL_COMMAND_PARSE_EMPTY,
    APP_CTRL_COMMAND_PARSE_INVALID,
    APP_CTRL_COMMAND_PARSE_UNSUPPORTED
} AppCtrlCommandParseResult;

void AppCtrlCommand_Clear(AppCtrlCommand *cmd);
AppCtrlCommandParseResult AppCtrlCommand_Parse(const char *input, AppCtrlCommand *outCmd);
const char *AppCtrlCommand_TypeToString(AppCtrlCommandType type);
const char *AppCtrlCommand_GetHelpText(void);
uint8_t AppCtrlCommand_FormatSummary(const AppCtrlCommand *cmd,
                                     char                 *buffer,
                                     uint16_t              bufferSize,
                                     const char           *prefix);

#endif
