#include <string.h>
#include "app/app_lin_sensor_slave.h"
#include "hal/hal_s32k_lin.h"

static AppLinSensorSlave *g_linSensorAppInstance = 0;

static void AppLinSensorSlave_LinCallbackThunk(uint32_t instance, void *linStatePtr)
{
    if (g_linSensorAppInstance != 0)
        AppLinSensorSlave_OnLinEvent(g_linSensorAppInstance, instance, linStatePtr);
}

static void AppLinSensorSlave_UpdateStatusFrame(AppLinSensorSlave *app)
{
    SvcLinSensorStatus statusPayload;

    statusPayload.adcRaw = app->adcLatestValue;
    statusPayload.zone = app->currentZone;
    statusPayload.emergencyLatched = SvcLedPattern_IsEmergencyLatched(&app->ledPattern);
    SvcLinSensorProto_BuildStatusFrame(&statusPayload, app->statusFrame, sizeof(app->statusFrame));
}

static void AppLinSensorSlave_ProcessLinDeferred(AppLinSensorSlave *app)
{
    uint8_t errorEventId = 0U;

    if (DrvLinSlave_TakeRxFramePending(&app->lin) != 0U)
    {
        if (SvcLinSensorProto_IsEmergencyClearAccepted(app->lin.rxBuffer[0],
                                                       SvcLedPattern_IsEmergencyLatched(&app->ledPattern),
                                                       app->currentZone) != 0U)
        {
            (void)SvcLedPattern_ClearEmergency(&app->ledPattern);
            app->okCmdAcceptedCount++;
            AppLinSensorSlave_UpdateStatusFrame(app);
        }
    }

    if (DrvLinSlave_TakeTimeoutPending(&app->lin) != 0U)
        app->linFaultCount++;

    if (DrvLinSlave_TakeErrorPending(&app->lin, &errorEventId) != 0U)
    {
        (void)errorEventId;
        app->linFaultCount++;
    }

    (void)DrvLinSlave_TakeTxDonePending(&app->lin);
}

EmbResult AppLinSensorSlave_Init(AppLinSensorSlave *app,
                                 const AppLinSensorSlaveConfig *appConfig,
                                 const DrvAdcConfig *adcConfig,
                                 const DrvLedConfig *ledConfig,
                                 const DrvLinSlaveConfig *linConfig)
{
    SvcLedPatternConfig ledPatternConfig;

    if (app == 0 || appConfig == 0 || adcConfig == 0 || ledConfig == 0 || linConfig == 0)
        return EMB_EINVAL;

    (void)memset(app, 0, sizeof(*app));
    app->config = *appConfig;
    app->currentZone = SVC_ZONE_SAFE;

    if (DrvAdc_Init(&app->adc, adcConfig) != EMB_OK)
        return EMB_EIO;
    if (DrvLed_Init(&app->led, ledConfig) != EMB_OK)
        return EMB_EIO;
    if (DrvLinSlave_Init(&app->lin, linConfig) != EMB_OK)
        return EMB_EIO;

    ledPatternConfig.led = &app->led;
    ledPatternConfig.blinkPeriod500us = 400U;
    if (SvcLedPattern_Init(&app->ledPattern, &ledPatternConfig) != EMB_OK)
        return EMB_EIO;

    AppLinSensorSlave_UpdateStatusFrame(app);
    return EMB_OK;
}

EmbResult AppLinSensorSlave_Start(AppLinSensorSlave *app)
{
    uint16_t raw = 0U;

    if (app == 0)
        return EMB_EINVAL;

    g_linSensorAppInstance = app;

    if (DrvLed_Start(&app->led) != EMB_OK)
        return EMB_EIO;
    if (DrvAdc_Start(&app->adc) != EMB_OK)
        return EMB_EIO;
    if (DrvLinSlave_Start(&app->lin, AppLinSensorSlave_LinCallbackThunk) != EMB_OK)
        return EMB_EIO;

    if (DrvAdc_Read(&app->adc, &raw) == EMB_OK)
    {
        app->adcLatestValue = raw;
        app->currentZone = SvcZone_Classify(&app->config.zoneConfig, raw);
        (void)SvcLedPattern_ApplyZone(&app->ledPattern, app->currentZone, 0U);
        AppLinSensorSlave_UpdateStatusFrame(app);
    }

    return EMB_OK;
}

void AppLinSensorSlave_Process(AppLinSensorSlave *app, uint32_t now500us)
{
    uint16_t raw = 0U;

    if (app == 0)
        return;

    AppLinSensorSlave_ProcessLinDeferred(app);

    if (EmbTime_IsExpired(now500us,
                          app->lastAdcSampleTick,
                          app->config.adcSamplePeriod500us) != 0U)
    {
        app->lastAdcSampleTick = now500us;
        if (DrvAdc_Read(&app->adc, &raw) == EMB_OK)
        {
            app->adcLatestValue = raw;
            app->currentZone = SvcZone_Classify(&app->config.zoneConfig, raw);
            (void)SvcLedPattern_ApplyZone(&app->ledPattern, app->currentZone, now500us);
            AppLinSensorSlave_UpdateStatusFrame(app);
        }
    }

    SvcLedPattern_Process(&app->ledPattern, now500us);
}

void AppLinSensorSlave_OnLinEvent(AppLinSensorSlave *app,
                                  uint32_t instance,
                                  void *linStatePtr)
{
    lin_state_t *linState;

    (void)instance;

    if (app == 0 || linStatePtr == 0)
        return;

    linState = (lin_state_t *)linStatePtr;

    if (linState->timeoutCounterFlag != false)
    {
        linState->timeoutCounterFlag = false;
        app->lin.timeoutPending = 1U;
        app->lin.timeoutCount++;
        (void)DrvLinSlave_GoIdle(&app->lin);
        return;
    }

    switch (linState->currentEventId)
    {
        case LIN_PID_OK:
            (void)HalS32kLin_SetTimeout(&app->lin.config.port,
                                        SVC_LIN_SENSOR_TIMEOUT_TICKS);
            if (linState->currentId == SVC_LIN_SENSOR_PID_ADC_STATUS)
            {
                if (DrvLinSlave_SendResponse(&app->lin,
                                             app->statusFrame,
                                             sizeof(app->statusFrame)) != EMB_OK)
                {
                    app->lin.errorPending = 1U;
                    app->lin.lastErrorEventId = LIN_PID_OK;
                    app->lin.errorCount++;
                    (void)DrvLinSlave_GoIdle(&app->lin);
                }
            }
            else if (linState->currentId == SVC_LIN_SENSOR_PID_OK_CMD)
            {
                if (DrvLinSlave_StartReceive(&app->lin, SVC_LIN_SENSOR_OK_CMD_SIZE) != EMB_OK)
                {
                    app->lin.errorPending = 1U;
                    app->lin.lastErrorEventId = LIN_PID_OK;
                    app->lin.errorCount++;
                    (void)DrvLinSlave_GoIdle(&app->lin);
                }
            }
            else
            {
                (void)DrvLinSlave_GoIdle(&app->lin);
            }
            break;

        case LIN_TX_COMPLETED:
            app->lin.txDonePending = 1U;
            app->lin.txCompletedCount++;
            (void)DrvLinSlave_GoIdle(&app->lin);
            break;

        case LIN_RX_COMPLETED:
            app->lin.rxFramePending = 1U;
            app->lin.rxCompletedCount++;
            (void)DrvLinSlave_GoIdle(&app->lin);
            break;

        case LIN_PID_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RX_OVERRUN:
            app->lin.errorPending = 1U;
            app->lin.lastErrorEventId = (uint8_t)linState->currentEventId;
            app->lin.errorCount++;
            (void)DrvLinSlave_GoIdle(&app->lin);
            break;

        case LIN_RECV_BREAK_FIELD_OK:
            (void)HalS32kLin_SetTimeout(&app->lin.config.port,
                                        SVC_LIN_SENSOR_TIMEOUT_TICKS);
            break;

        default:
            break;
    }
}
