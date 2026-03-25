#ifndef DRV_LIN_MASTER_H
#define DRV_LIN_MASTER_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_lin.h"
#include "core/emb_critical.h"

typedef enum
{
    DRV_LIN_MASTER_IDLE = 0,
    DRV_LIN_MASTER_WAIT_HEADER,
    DRV_LIN_MASTER_WAIT_RX,
    DRV_LIN_MASTER_WAIT_TX
} DrvLinMasterState;

typedef enum
{
    DRV_LIN_MASTER_OP_NONE = 0,
    DRV_LIN_MASTER_OP_READ_STATUS,
    DRV_LIN_MASTER_OP_SEND_OK_CMD
} DrvLinMasterOperation;

typedef struct
{
    HalS32kLinPort port;
    uint8_t statusPid;
    uint8_t okCmdPid;
    uint8_t rxSize;
    uint8_t txSize;
    uint16_t timeoutTicks;
} DrvLinMasterConfig;

typedef struct
{
    DrvLinMasterConfig config;
    DrvLinMasterState state;
    DrvLinMasterOperation operation;
    uint8_t txBuffer[8];
    uint8_t rxBuffer[8];
    volatile uint8_t statusFrameReady;
    volatile uint8_t txDonePending;
    volatile uint8_t timeoutPending;
    volatile uint8_t errorPending;
    volatile uint8_t lastErrorEventId;
    uint32_t headerRequestCount;
    uint32_t txCompletedCount;
    uint32_t rxCompletedCount;
    uint32_t timeoutCount;
    uint32_t errorCount;
    uint32_t callbackCount;
    uint8_t lastPid;
    uint32_t lastCallbackEventId;
} DrvLinMaster;

EmbResult DrvLinMaster_Init(DrvLinMaster *lin, const DrvLinMasterConfig *config);
EmbResult DrvLinMaster_Start(DrvLinMaster *lin, HalS32kLinCallback callback);
EmbResult DrvLinMaster_RequestStatus(DrvLinMaster *lin);
EmbResult DrvLinMaster_RequestOkCmd(DrvLinMaster *lin, const uint8_t *data, uint8_t size);
EmbResult DrvLinMaster_StartStatusReceive(DrvLinMaster *lin);
EmbResult DrvLinMaster_StartOkCmdSend(DrvLinMaster *lin);
EmbResult DrvLinMaster_GoIdle(DrvLinMaster *lin);
EmbResult DrvLinMaster_TryGetStatusFrame(DrvLinMaster *lin, uint8_t *outData, uint8_t *outSize);
void DrvLinMaster_OnCallback(DrvLinMaster *lin, void *linStatePtr);
uint8_t DrvLinMaster_TakeTxDonePending(DrvLinMaster *lin);
uint8_t DrvLinMaster_TakeTimeoutPending(DrvLinMaster *lin);
uint8_t DrvLinMaster_TakeErrorPending(DrvLinMaster *lin, uint8_t *outEventId);

#endif
