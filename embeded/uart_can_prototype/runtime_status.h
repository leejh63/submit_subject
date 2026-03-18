#ifndef RUNTIME_STATUS_H
#define RUNTIME_STATUS_H

#include <stdint.h>

typedef struct
{
    uint8_t  heartbeatAlive;
    uint8_t  canAlive;
    uint8_t  uartOk;
    uint32_t tickMs;

    uint32_t tmp_check_heartbeat;
    uint32_t tmp_check_uart;
    uint32_t tmp_check_can;
} RuntimeStatus;

void RuntimeStatus_Init(RuntimeStatus *status);

void RuntimeStatus_SetHeartbeatAlive(RuntimeStatus *status, uint8_t alive);
void RuntimeStatus_SetCanAlive(RuntimeStatus *status, uint8_t alive);
void RuntimeStatus_SetUartOk(RuntimeStatus *status, uint8_t ok);
void RuntimeStatus_SetTickMs(RuntimeStatus *status, uint32_t tickMs);

void RuntimeStatus_BuildTaskText(const RuntimeStatus *status,
                                 char *outBuffer,
                                 uint16_t outBufferSize);

#endif
