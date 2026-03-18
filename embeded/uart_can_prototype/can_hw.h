#ifndef CAN_HW_H
#define CAN_HW_H

#include <stdint.h>

#include "can_types.h"
#include "sdk_project_config.h"

typedef struct
{
    uint8_t localNodeId;
    uint8_t instance;
    uint8_t txMbIndex;
    uint8_t rxMbIndex;
    flexcan_state_t *driverState;
    const flexcan_user_config_t *userConfig;
} CanHwConfig;

typedef struct
{
    uint8_t initialized;
    uint8_t localNodeId;
    uint8_t instance;
    uint8_t txMbIndex;
    uint8_t rxMbIndex;

    uint8_t txBusy;
    uint8_t lastError;

    flexcan_state_t *driverState;
    const flexcan_user_config_t *userConfig;

    flexcan_msgbuff_t rxMsg;

    CanFrame rxQueue[CAN_HW_RX_QUEUE_SIZE];
    uint8_t  rxHead;
    uint8_t  rxTail;
    uint8_t  rxCount;

    uint32_t txOkCount;
    uint32_t txErrorCount;
    uint32_t rxOkCount;
    uint32_t rxErrorCount;
    uint32_t rxDropCount;
} CanHw;

uint8_t CanHw_Init(CanHw *hw, const CanHwConfig *config);
void    CanHw_Task(CanHw *hw, uint32_t nowMs);

uint8_t CanHw_StartTx(CanHw *hw, const CanFrame *frame);
uint8_t CanHw_TryPopRx(CanHw *hw, CanFrame *outFrame);

uint8_t CanHw_IsReady(const CanHw *hw);
uint8_t CanHw_IsTxBusy(const CanHw *hw);
uint8_t CanHw_GetLastError(const CanHw *hw);

#endif
