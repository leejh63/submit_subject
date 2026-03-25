#ifndef APP_MASTER_NODE_H
#define APP_MASTER_NODE_H

#include "drivers/drv_can.h"
#include "drivers/drv_uart.h"
#include "drivers/drv_lin_master.h"
#include "services/svc_can_button_proto.h"
#include "services/svc_lin_sensor_proto.h"
#include "services/svc_gateway.h"
#include "services/svc_uart_console.h"
#include "core/emb_result.h"

typedef struct
{
    DrvCan can;
    DrvUart uart;
    DrvLinMaster lin;
    SvcGateway gateway;
    SvcUartConsole console;
    uint32_t lastProcessTickMs;
    uint32_t buttonEventCount;
    uint32_t linPollIssueCount;
    uint32_t linAckIssueCount;
    uint32_t linFaultCount;
    uint32_t linDeferredRxCount;
    uint32_t linDeferredTxCount;
    uint8_t lastLinErrorEventId;
    uint8_t lastButtonId;
    SvcCanButtonAction lastButtonAction;
    uint8_t lastButtonSequence;
    SvcLinSensorStatus lastLinStatus;
    uint8_t linStatusValid;
} AppMasterNode;

EmbResult AppMasterNode_Init(AppMasterNode *app,
                             const DrvCanConfig *canConfig,
                             const DrvUartConfig *uartConfig,
                             const DrvLinMasterConfig *linConfig,
                             const SvcGatewayConfig *gatewayConfig);
EmbResult AppMasterNode_Start(AppMasterNode *app);
void AppMasterNode_Process(AppMasterNode *app, uint32_t nowMs);
void AppMasterNode_OnLinEvent(AppMasterNode *app, uint32_t instance, void *linStatePtr);

#endif
