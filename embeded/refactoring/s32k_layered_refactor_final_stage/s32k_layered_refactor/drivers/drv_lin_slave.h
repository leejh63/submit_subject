#ifndef DRV_LIN_SLAVE_H
#define DRV_LIN_SLAVE_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_lin.h"
#include "core/emb_critical.h"

typedef enum
{
    DRV_LIN_SLAVE_IDLE = 0,
    DRV_LIN_SLAVE_WAIT_TX,
    DRV_LIN_SLAVE_WAIT_RX
} DrvLinSlaveState;

typedef struct
{
    HalS32kLinPort port;
    uint8_t txSize;
    uint8_t rxSize;
} DrvLinSlaveConfig;

typedef struct
{
    DrvLinSlaveConfig config;
    DrvLinSlaveState state;
    uint8_t txBuffer[8];
    uint8_t rxBuffer[8];
    volatile uint8_t rxFramePending;
    volatile uint8_t txDonePending;
    volatile uint8_t timeoutPending;
    volatile uint8_t errorPending;
    volatile uint8_t lastErrorEventId;
    uint32_t rxCompletedCount;
    uint32_t txCompletedCount;
    uint32_t timeoutCount;
    uint32_t errorCount;
} DrvLinSlave;

EmbResult DrvLinSlave_Init(DrvLinSlave *lin, const DrvLinSlaveConfig *config);
EmbResult DrvLinSlave_Start(DrvLinSlave *lin, HalS32kLinCallback callback);
EmbResult DrvLinSlave_SendResponse(DrvLinSlave *lin, const uint8_t *data, uint8_t size);
EmbResult DrvLinSlave_StartReceive(DrvLinSlave *lin, uint8_t size);
EmbResult DrvLinSlave_GoIdle(DrvLinSlave *lin);
uint8_t DrvLinSlave_TakeRxFramePending(DrvLinSlave *lin);
uint8_t DrvLinSlave_TakeTxDonePending(DrvLinSlave *lin);
uint8_t DrvLinSlave_TakeTimeoutPending(DrvLinSlave *lin);
uint8_t DrvLinSlave_TakeErrorPending(DrvLinSlave *lin, uint8_t *outEventId);

#endif
