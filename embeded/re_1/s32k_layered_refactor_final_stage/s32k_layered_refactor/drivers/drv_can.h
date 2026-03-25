#ifndef DRV_CAN_H
#define DRV_CAN_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_can.h"

#define DRV_CAN_TX_QUEUE_SIZE  8U
#define DRV_CAN_RX_QUEUE_SIZE  8U

typedef struct
{
    HalS32kCanPort port;
} DrvCanConfig;

typedef struct
{
    DrvCanConfig config;
    uint8_t started;
    uint8_t txInFlight;
    uint8_t txHead;
    uint8_t txTail;
    uint8_t txCount;
    uint8_t rxHead;
    uint8_t rxTail;
    uint8_t rxCount;
    HalCanFrame txQueue[DRV_CAN_TX_QUEUE_SIZE];
    HalCanFrame rxQueue[DRV_CAN_RX_QUEUE_SIZE];
    uint32_t txStartCount;
    uint32_t txOkCount;
    uint32_t txDropCount;
    uint32_t rxOkCount;
    uint32_t rxDropCount;
    uint32_t errorCount;
    EmbResult lastError;
} DrvCan;

EmbResult DrvCan_Init(DrvCan *can, const DrvCanConfig *config);
EmbResult DrvCan_Start(DrvCan *can);
EmbResult DrvCan_Process(DrvCan *can);
EmbResult DrvCan_Send(DrvCan *can, const HalCanFrame *frame);
EmbResult DrvCan_TryReceive(DrvCan *can, HalCanFrame *frame);

#endif
