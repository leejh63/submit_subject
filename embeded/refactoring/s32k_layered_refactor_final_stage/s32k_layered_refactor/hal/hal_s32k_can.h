#ifndef HAL_S32K_CAN_H
#define HAL_S32K_CAN_H

#include <stdint.h>
#include "core/emb_result.h"

typedef struct
{
    uint32_t id;
    uint8_t dlc;
    uint8_t isExtendedId;
    uint8_t data[8];
} HalCanFrame;

typedef enum
{
    HAL_S32K_CAN_BIND_NONE = 0,
    HAL_S32K_CAN_BIND_MASTER = 1,
    HAL_S32K_CAN_BIND_BUTTON = 2
} HalS32kCanBinding;

typedef struct
{
    HalS32kCanBinding binding;
    uint32_t instance;
    uint8_t txMb;
    uint8_t rxMb;
} HalS32kCanPort;

EmbResult HalS32kCan_Init(HalS32kCanPort *port);
EmbResult HalS32kCan_Start(HalS32kCanPort *port);
EmbResult HalS32kCan_IsTxBusy(HalS32kCanPort *port, uint8_t *outBusy);
EmbResult HalS32kCan_Tx(HalS32kCanPort *port, const HalCanFrame *frame);
EmbResult HalS32kCan_TryRead(HalS32kCanPort *port, HalCanFrame *frame);

#endif
