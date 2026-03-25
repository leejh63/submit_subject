#include "app/app_master_node.h"
#include "platform/s32k_board.h"
#include "hal/hal_s32k_can.h"
#include "hal/hal_s32k_lin.h"
#include "hal/hal_s32k_uart.h"
#include "platform/s32k_bindings.h"
#include "platform/s32k_tick.h"

static AppMasterNode g_master;

int main(void)
{
    DrvCanConfig canConfig;
    DrvUartConfig uartConfig;
    DrvLinMasterConfig linConfig;
    SvcGatewayConfig gatewayConfig;

    (void)S32kBoard_InitMasterNode();

    canConfig.port.binding = HAL_S32K_CAN_BIND_MASTER;
#ifdef S32K_MASTER_CAN_TX_MB
    canConfig.port.txMb = S32K_MASTER_CAN_TX_MB;
#else
    canConfig.port.txMb = 1U;
#endif
#ifdef S32K_MASTER_CAN_RX_MB
    canConfig.port.rxMb = S32K_MASTER_CAN_RX_MB;
#else
    canConfig.port.rxMb = 0U;
#endif

    uartConfig.port.binding = HAL_S32K_UART_BIND_MASTER;
#ifdef S32K_MASTER_UART_INSTANCE
    uartConfig.port.instance = S32K_MASTER_UART_INSTANCE;
#else
    uartConfig.port.instance = 0U;
#endif

    linConfig.port.binding = HAL_S32K_LIN_BIND_MASTER;
#ifdef S32K_MASTER_LIN_INSTANCE
    linConfig.port.instance = S32K_MASTER_LIN_INSTANCE;
#else
    linConfig.port.instance = 0U;
#endif
#ifdef S32K_MASTER_LIN_STATUS_PID
    linConfig.statusPid = S32K_MASTER_LIN_STATUS_PID;
#else
    linConfig.statusPid = SVC_LIN_SENSOR_PID_ADC_STATUS;
#endif
#ifdef S32K_MASTER_LIN_OK_CMD_PID
    linConfig.okCmdPid = S32K_MASTER_LIN_OK_CMD_PID;
#else
    linConfig.okCmdPid = SVC_LIN_SENSOR_PID_OK_CMD;
#endif
    linConfig.rxSize = SVC_LIN_SENSOR_STATUS_SIZE;
    linConfig.txSize = SVC_LIN_SENSOR_OK_CMD_SIZE;
    linConfig.timeoutTicks = SVC_LIN_SENSOR_TIMEOUT_TICKS;

#ifdef S32K_MASTER_LIN_POLL_PERIOD_MS
    gatewayConfig.sensorPollPeriodMs = S32K_MASTER_LIN_POLL_PERIOD_MS;
#else
    gatewayConfig.sensorPollPeriodMs = 50U;
#endif
#ifdef S32K_MASTER_LIN_STATUS_STALE_MS
    gatewayConfig.sensorStaleMs = S32K_MASTER_LIN_STATUS_STALE_MS;
#else
    gatewayConfig.sensorStaleMs = 150U;
#endif
#ifdef S32K_MASTER_LIN_ACK_RETRY_PERIOD_MS
    gatewayConfig.ackRetryPeriodMs = S32K_MASTER_LIN_ACK_RETRY_PERIOD_MS;
#else
    gatewayConfig.ackRetryPeriodMs = 80U;
#endif
#ifdef S32K_MASTER_LIN_ACK_MAX_RETRY
    gatewayConfig.ackMaxRetryCount = S32K_MASTER_LIN_ACK_MAX_RETRY;
#else
    gatewayConfig.ackMaxRetryCount = 3U;
#endif

    if (AppMasterNode_Init(&g_master,
                           &canConfig,
                           &uartConfig,
                           &linConfig,
                           &gatewayConfig) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    if (AppMasterNode_Start(&g_master) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    for (;;)
    {
        AppMasterNode_Process(&g_master, S32kTick_GetMs());
    }
}
