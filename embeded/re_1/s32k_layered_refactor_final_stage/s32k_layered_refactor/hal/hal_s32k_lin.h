#ifndef HAL_S32K_LIN_H
#define HAL_S32K_LIN_H

#include <stdint.h>
#include "core/emb_result.h"

typedef void (*HalS32kLinCallback)(uint32_t instance, void *linStatePtr);

typedef enum
{
    HAL_S32K_LIN_BIND_SENSOR_SLAVE = 0,
    HAL_S32K_LIN_BIND_MASTER = 1,
    HAL_S32K_LIN_BIND_CUSTOM = 2
} HalS32kLinBinding;

typedef struct
{
    HalS32kLinBinding binding;
    uint32_t instance;
    uint16_t timeoutTicks;
} HalS32kLinPort;

EmbResult HalS32kLin_Init(HalS32kLinPort *port);
EmbResult HalS32kLin_InstallCallback(HalS32kLinPort *port, HalS32kLinCallback callback);
EmbResult HalS32kLin_Send(HalS32kLinPort *port, uint8_t *data, uint8_t size);
EmbResult HalS32kLin_Receive(HalS32kLinPort *port, uint8_t *data, uint8_t size);
EmbResult HalS32kLin_GoIdle(HalS32kLinPort *port);
EmbResult HalS32kLin_SetTimeout(HalS32kLinPort *port, uint16_t timeout);
EmbResult HalS32kLin_MasterSendHeader(HalS32kLinPort *port, uint8_t pid);

#endif
