#include "runtime_status.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

void RuntimeStatus_Init(RuntimeStatus *status)
{
    if (status == NULL)
        return;

    memset(status, 0, sizeof(RuntimeStatus));

    status->heartbeatAlive = 0U;
    status->canAlive = 0U;
    status->uartOk = 1U;
    status->tickMs = 0U;

    status->tmp_check_uart = 0U;
    status->tmp_check_heartbeat = 0U;
    status->tmp_check_can = 0U;
}

void RuntimeStatus_SetHeartbeatAlive(RuntimeStatus *status, uint8_t alive)
{
    if (status == NULL)
        return;

    status->heartbeatAlive = alive;
}

void RuntimeStatus_SetCanAlive(RuntimeStatus *status, uint8_t alive)
{
    if (status == NULL)
        return;

    status->canAlive = alive;
}

void RuntimeStatus_SetUartOk(RuntimeStatus *status, uint8_t ok)
{
    if (status == NULL)
        return;

    status->uartOk = ok;
}

void RuntimeStatus_SetTickMs(RuntimeStatus *status, uint32_t tickMs)
{
    if (status == NULL)
        return;

    status->tickMs = tickMs;
}

void RuntimeStatus_BuildTaskText(const RuntimeStatus *status,
                                 char *outBuffer,
                                 uint16_t outBufferSize)
{
    const char *heartbeatText;
    const char *canText;
    const char *uartText;

    if (status == NULL || outBuffer == NULL || outBufferSize == 0U)
        return;

    heartbeatText = (status->heartbeatAlive != 0U) ? "alive" : "idle";
    canText = (status->canAlive != 0U) ? "active" : "idle";
    uartText = (status->uartOk != 0U) ? "ok" : "error";

    (void)snprintf(outBuffer,
                   outBufferSize,
                   "heartbeat : %s / %lu\r\n"
                   "can       : %s / %lu\r\n"
                   "uart      : %s / %lu",
                   heartbeatText,
                   (unsigned long)status->tmp_check_heartbeat,
                   canText,
				   (unsigned long)status->tmp_check_can,
                   uartText,
				   (unsigned long)status->tmp_check_uart);
}
