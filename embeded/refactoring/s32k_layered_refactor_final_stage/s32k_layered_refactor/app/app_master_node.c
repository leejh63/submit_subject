#include <string.h>
#include "app/app_master_node.h"
#include "services/svc_lin_sensor_proto.h"

static AppMasterNode *g_masterNodeInstance = 0;

static void AppMasterNode_LinCallbackThunk(uint32_t instance, void *linStatePtr)
{
    if (g_masterNodeInstance != 0)
        AppMasterNode_OnLinEvent(g_masterNodeInstance, instance, linStatePtr);
}

static void AppMasterNode_BuildConsoleSnapshot(const AppMasterNode *app,
                                               SvcUartConsoleSnapshot *snapshot)
{
    if (app == 0 || snapshot == 0)
        return;

    (void)memset(snapshot, 0, sizeof(*snapshot));
    snapshot->nowMs = app->lastProcessTickMs;
    snapshot->buttonEventCount = app->buttonEventCount;
    snapshot->linPollIssueCount = app->linPollIssueCount;
    snapshot->linAckIssueCount = app->linAckIssueCount;
    snapshot->linFaultCount = app->linFaultCount;
    snapshot->uartRxOverflowCount = app->uart.rxOverflowCount;
    snapshot->uartTxDropCount = app->uart.txDropCount;
    snapshot->gatewaySensorTimeoutCount = app->gateway.sensorTimeoutCount;
    snapshot->gatewayAckRequestCount = app->gateway.linAckRequestCount;
    snapshot->gatewayAckSendCount = app->gateway.linAckSendCount;
    snapshot->gatewayAckConfirmCount = app->gateway.linAckConfirmCount;
    snapshot->gatewayAckGiveUpCount = app->gateway.linAckRetryGiveUpCount;
    snapshot->linStatusValid = app->linStatusValid;
    snapshot->linAdcRaw = app->lastLinStatus.adcRaw;
    snapshot->linZone = (uint8_t)app->lastLinStatus.zone;
    snapshot->linEmergencyLatched = app->lastLinStatus.emergencyLatched;
    snapshot->lastButtonId = app->lastButtonId;
    snapshot->lastButtonAction = (uint8_t)app->lastButtonAction;
    snapshot->lastButtonSequence = app->lastButtonSequence;
}

static void AppMasterNode_ProcessLinDeferred(AppMasterNode *app, uint32_t nowMs)
{
    uint8_t frame[8];
    uint8_t frameSize = 0U;
    uint8_t errorEventId = 0U;
    SvcLinSensorStatus status;

    if (DrvLinMaster_TakeTimeoutPending(&app->lin) != 0U)
        app->linFaultCount++;

    if (DrvLinMaster_TakeErrorPending(&app->lin, &errorEventId) != 0U)
    {
        app->linFaultCount++;
        app->lastLinErrorEventId = errorEventId;
    }

    if (DrvLinMaster_TakeTxDonePending(&app->lin) != 0U)
        app->linDeferredTxCount++;

    if (DrvLinMaster_TryGetStatusFrame(&app->lin, frame, &frameSize) == EMB_OK)
    {
        if (SvcLinSensorProto_ParseStatusFrame(frame, frameSize, &status) == EMB_OK)
        {
            app->linDeferredRxCount++;
            app->lastLinStatus = status;
            app->linStatusValid = 1U;
            SvcGateway_OnSensorStatus(&app->gateway, &status, nowMs);
        }
        else
        {
            app->linFaultCount++;
        }
    }
}

EmbResult AppMasterNode_Init(AppMasterNode *app,
                             const DrvCanConfig *canConfig,
                             const DrvUartConfig *uartConfig,
                             const DrvLinMasterConfig *linConfig,
                             const SvcGatewayConfig *gatewayConfig)
{
    SvcUartConsoleConfig consoleConfig;

    if (app == 0 || canConfig == 0 || uartConfig == 0 || linConfig == 0 || gatewayConfig == 0)
        return EMB_EINVAL;

    (void)memset(app, 0, sizeof(*app));
    if (DrvCan_Init(&app->can, canConfig) != EMB_OK)
        return EMB_EIO;
    if (DrvUart_Init(&app->uart, uartConfig) != EMB_OK)
        return EMB_EIO;
    if (DrvLinMaster_Init(&app->lin, linConfig) != EMB_OK)
        return EMB_EIO;
    if (SvcGateway_Init(&app->gateway, gatewayConfig) != EMB_OK)
        return EMB_EIO;

    consoleConfig.prompt = "master> ";
    consoleConfig.banner = "master console ready\r\n";
    if (SvcUartConsole_Init(&app->console, &consoleConfig) != EMB_OK)
        return EMB_EIO;

    return EMB_OK;
}

EmbResult AppMasterNode_Start(AppMasterNode *app)
{
    if (app == 0)
        return EMB_EINVAL;

    g_masterNodeInstance = app;

    if (DrvCan_Start(&app->can) != EMB_OK)
        return EMB_EIO;
    if (DrvUart_Start(&app->uart) != EMB_OK)
        return EMB_EIO;
    if (DrvLinMaster_Start(&app->lin, AppMasterNode_LinCallbackThunk) != EMB_OK)
        return EMB_EIO;
    if (SvcUartConsole_Start(&app->console, &app->uart) != EMB_OK)
        return EMB_EIO;

    return EMB_OK;
}

void AppMasterNode_Process(AppMasterNode *app, uint32_t nowMs)
{
    HalCanFrame frame;
    SvcCanButtonEvent event;
    SvcUartConsoleSnapshot consoleSnapshot;
    uint8_t okCmdFrame[SVC_LIN_SENSOR_OK_CMD_SIZE];

    if (app == 0)
        return;

    app->lastProcessTickMs = nowMs;

    (void)DrvCan_Process(&app->can);
    (void)DrvUart_Process(&app->uart);
    AppMasterNode_ProcessLinDeferred(app, nowMs);

    while (DrvCan_TryReceive(&app->can, &frame) == EMB_OK)
    {
        if (SvcCanButtonProto_ParseEventFrame(&frame, &event) == EMB_OK)
        {
            app->buttonEventCount++;
            app->lastButtonId = event.buttonId;
            app->lastButtonAction = event.action;
            app->lastButtonSequence = event.sequence;
            SvcGateway_OnButtonEvent(&app->gateway, &event);
        }
    }

    AppMasterNode_BuildConsoleSnapshot(app, &consoleSnapshot);
    SvcUartConsole_Process(&app->console, &app->uart, &consoleSnapshot);

    if (SvcUartConsole_TakeAckRequest(&app->console) != 0U)
        SvcGateway_RequestLinAck(&app->gateway);
    if (SvcUartConsole_TakePollRequest(&app->console) != 0U)
        SvcGateway_RequestImmediatePoll(&app->gateway);

    SvcGateway_Process(&app->gateway, nowMs);

    if ((app->lin.state == DRV_LIN_MASTER_IDLE) &&
        (SvcGateway_ShouldSendLinAck(&app->gateway, nowMs) != 0U))
    {
        SvcLinSensorProto_BuildOkCmdFrame(okCmdFrame, sizeof(okCmdFrame));
        if (DrvLinMaster_RequestOkCmd(&app->lin, okCmdFrame, sizeof(okCmdFrame)) == EMB_OK)
        {
            app->linAckIssueCount++;
            SvcGateway_MarkLinAckSent(&app->gateway, nowMs);
        }
    }
    else if ((app->lin.state == DRV_LIN_MASTER_IDLE) &&
             (SvcGateway_ShouldPollSensor(&app->gateway, nowMs) != 0U))
    {
        if (DrvLinMaster_RequestStatus(&app->lin) == EMB_OK)
        {
            app->linPollIssueCount++;
            SvcGateway_MarkSensorPollIssued(&app->gateway, nowMs);
        }
    }
}

void AppMasterNode_OnLinEvent(AppMasterNode *app, uint32_t instance, void *linStatePtr)
{
    (void)instance;

    if (app == 0 || linStatePtr == 0)
        return;

    DrvLinMaster_OnCallback(&app->lin, linStatePtr);
}
