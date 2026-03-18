#ifndef CTRL_CMD_H
#define CTRL_CMD_H

#include <stdint.h>

#define CTRL_CMD_TEXT_SIZE        32U
#define CTRL_CMD_AUTH_TEXT_SIZE   16U

typedef enum
{
    CTRL_CMD_NONE = 0,
    CTRL_CMD_OPEN,
    CTRL_CMD_CLOSE,
    CTRL_CMD_OFF,
    CTRL_CMD_TEST,
    CTRL_CMD_TEXT
} CtrlCmdType;

typedef struct
{
    CtrlCmdType type;

    uint8_t     hasTargetId;
    uint8_t     targetId;

    uint8_t     hasText;
    char        text[CTRL_CMD_TEXT_SIZE];

    uint8_t     hasAuthText;
    char        authText[CTRL_CMD_AUTH_TEXT_SIZE];

    uint8_t     hasRequestId;
    uint16_t    requestId;
} CtrlCmd;

typedef enum
{
    CTRL_CMD_RESULT_OK = 0,
    CTRL_CMD_RESULT_EMPTY,
    CTRL_CMD_RESULT_INVALID,
    CTRL_CMD_RESULT_UNSUPPORTED
} CtrlCmdResult;

void CtrlCmd_Clear(CtrlCmd *cmd);
CtrlCmdResult CtrlCmd_Parse(const char *input, CtrlCmd *outCmd);

#endif
