#ifndef CAN_TRANSPORT_H
#define CAN_TRANSPORT_H

#include <stdint.h>

#include "can_types.h"
#include "can_hw.h"
#include "sdk_project_config.h"

typedef struct
{
    uint8_t localNodeId;
    uint8_t instance;
    uint8_t txMbIndex;
    uint8_t rxMbIndex;
    flexcan_state_t *driverState;
    const flexcan_user_config_t *userConfig;
} CanTransportConfig;

typedef struct
{
    uint8_t initialized;
    uint8_t txInFlight;
    uint8_t lastError;

    CanHw   hw;

    CanFrame txQueue[CAN_TRANSPORT_TX_QUEUE_SIZE];
    uint8_t  txHead;
    uint8_t  txTail;
    uint8_t  txCount;

    CanFrame rxQueue[CAN_TRANSPORT_RX_QUEUE_SIZE];
    uint8_t  rxHead;
    uint8_t  rxTail;
    uint8_t  rxCount;

    CanFrame currentTxFrame;

    uint32_t txQueuedCount;
    uint32_t txStartCount;
    uint32_t txRetryCount;
    uint32_t txDropCount;

    uint32_t rxQueuedCount;
    uint32_t rxDropCount;
} CanTransport;

uint8_t CanTransport_Init(CanTransport *transport,
                          const CanTransportConfig *config);

void    CanTransport_Task(CanTransport *transport, uint32_t nowMs);

uint8_t CanTransport_SendFrame(CanTransport *transport,
                               const CanFrame *frame);

uint8_t CanTransport_PopRx(CanTransport *transport,
                           CanFrame *outFrame);

uint8_t CanTransport_IsReady(const CanTransport *transport);
uint8_t CanTransport_GetLastError(const CanTransport *transport);

#endif
