#ifndef CAN_APP_H
#define CAN_APP_H

#include <stdint.h>

#include "can_types.h"
#include "can_service.h"
#include "ctrl_cmd.h"
#include "ctrl_result_box.h"
#include "sdk_project_config.h"

#define CAN_APP_RESULT_QUEUE_SIZE   8U

typedef struct
{
    uint8_t     localNodeId;
    uint8_t     role;
    uint8_t     defaultTargetNodeId;
    uint8_t     instance;
    uint8_t     txMbIndex;
    uint8_t     rxMbIndex;
    uint32_t    defaultTimeoutMs;
    flexcan_state_t *driverState;
    const flexcan_user_config_t *userConfig;
} CanAppConfig;

typedef struct
{
    uint8_t     initialized;
    uint8_t     localNodeId;
    uint8_t     role;
    uint8_t     defaultTargetNodeId;

    CanService  service;

    CtrlResult  resultQueue[CAN_APP_RESULT_QUEUE_SIZE];
    uint8_t     resultHead;
    uint8_t     resultTail;
    uint8_t     resultCount;

    uint32_t    localSubmitOkCount;
    uint32_t    localSubmitFailCount;
    uint32_t    remoteCommandCount;
    uint32_t    resultConvertCount;
    uint32_t    eventConvertCount;
    uint32_t    textConvertCount;
    uint32_t    resultDropCount;
} CanApp;

uint8_t CanApp_Init(CanApp *app, const CanAppConfig *config);
void    CanApp_Task(CanApp *app, uint32_t nowMs);
void    CanApp_FlushTx(CanApp *app, uint32_t nowMs);

uint8_t CanApp_SubmitCtrlCmd(CanApp *app, const CtrlCmd *cmd);
uint8_t CanApp_PopCtrlResult(CanApp *app, CtrlResult *outResult);

#endif
