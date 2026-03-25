#include <string.h>
#include "services/svc_gateway.h"
#include "core/emb_time.h"

EmbResult SvcGateway_Init(SvcGateway *gateway, const SvcGatewayConfig *config)
{
    if (gateway == 0 || config == 0)
        return EMB_EINVAL;

    (void)memset(gateway, 0, sizeof(*gateway));
    gateway->config = *config;
    return EMB_OK;
}

void SvcGateway_Process(SvcGateway *gateway, uint32_t nowMs)
{
    if (gateway == 0)
        return;

    if ((gateway->sensorStatusValid != 0U) &&
        (gateway->sensorStatusStale == 0U) &&
        EmbTime_IsExpired(nowMs,
                          gateway->lastSensorStatusTick,
                          gateway->config.sensorStaleMs))
    {
        gateway->sensorStatusStale = 1U;
        gateway->sensorTimeoutCount++;
    }

    if ((gateway->linAckPending != 0U) &&
        (gateway->linAckAwaitingClear != 0U) &&
        (gateway->linAckRetryCount >= gateway->config.ackMaxRetryCount) &&
        EmbTime_IsExpired(nowMs,
                          gateway->lastLinAckTick,
                          gateway->config.ackRetryPeriodMs))
    {
        gateway->linAckPending = 0U;
        gateway->linAckAwaitingClear = 0U;
        gateway->linAckRetryGiveUpCount++;
    }
}

uint8_t SvcGateway_ShouldPollSensor(const SvcGateway *gateway, uint32_t nowMs)
{
    if (gateway == 0)
        return 0U;
    if (gateway->forceSensorPoll != 0U)
        return 1U;

    return EmbTime_IsExpired(nowMs,
                             gateway->lastSensorPollRequestTick,
                             gateway->config.sensorPollPeriodMs);
}

void SvcGateway_MarkSensorPollIssued(SvcGateway *gateway, uint32_t nowMs)
{
    if (gateway == 0)
        return;

    gateway->forceSensorPoll = 0U;
    gateway->lastSensorPollRequestTick = nowMs;
    gateway->sensorPollRequestCount++;
}

void SvcGateway_OnSensorStatus(SvcGateway *gateway,
                               const SvcLinSensorStatus *status,
                               uint32_t nowMs)
{
    if (gateway == 0 || status == 0)
        return;

    gateway->sensorStatus = *status;
    gateway->sensorStatusValid = 1U;
    gateway->sensorStatusStale = 0U;
    gateway->lastSensorStatusTick = nowMs;
    gateway->sensorUpdateCount++;

    if ((gateway->linAckPending != 0U) &&
        (status->emergencyLatched == 0U))
    {
        gateway->linAckPending = 0U;
        gateway->linAckAwaitingClear = 0U;
        gateway->linAckRetryCount = 0U;
        gateway->linAckConfirmCount++;
    }
}

void SvcGateway_RequestLinAck(SvcGateway *gateway)
{
    if (gateway == 0)
        return;

    gateway->linAckPending = 1U;
    gateway->linAckRequestCount++;
}

void SvcGateway_RequestImmediatePoll(SvcGateway *gateway)
{
    if (gateway == 0)
        return;

    gateway->forceSensorPoll = 1U;
}

void SvcGateway_OnButtonEvent(SvcGateway *gateway,
                              const SvcCanButtonEvent *event)
{
    if (gateway == 0 || event == 0)
        return;

    gateway->lastButtonId = event->buttonId;
    gateway->lastButtonAction = event->action;

    if (event->action == SVC_CAN_BUTTON_ACTION_ACK_REQUEST)
        SvcGateway_RequestLinAck(gateway);
}

uint8_t SvcGateway_ShouldSendLinAck(const SvcGateway *gateway, uint32_t nowMs)
{
    if (gateway == 0)
        return 0U;
    if (gateway->linAckPending == 0U)
        return 0U;
    if (gateway->sensorStatusValid == 0U || gateway->sensorStatusStale != 0U)
        return 0U;
    if (gateway->sensorStatus.emergencyLatched == 0U)
        return 0U;
    if (gateway->sensorStatus.zone == SVC_ZONE_EMERGENCY)
        return 0U;
    if (gateway->linAckAwaitingClear == 0U)
        return 1U;
    if (gateway->linAckRetryCount >= gateway->config.ackMaxRetryCount)
        return 0U;

    return EmbTime_IsExpired(nowMs,
                             gateway->lastLinAckTick,
                             gateway->config.ackRetryPeriodMs);
}

void SvcGateway_MarkLinAckSent(SvcGateway *gateway, uint32_t nowMs)
{
    if (gateway == 0)
        return;

    if (gateway->linAckAwaitingClear != 0U)
        gateway->linAckRetryCount++;
    else
        gateway->linAckRetryCount = 0U;

    gateway->linAckAwaitingClear = 1U;
    gateway->lastLinAckTick = nowMs;
    gateway->linAckSendCount++;
}

uint8_t SvcGateway_HasFreshSensorStatus(const SvcGateway *gateway)
{
    if (gateway == 0)
        return 0U;

    if ((gateway->sensorStatusValid == 0U) || (gateway->sensorStatusStale != 0U))
        return 0U;

    return 1U;
}

uint8_t SvcGateway_IsEmergencyActive(const SvcGateway *gateway)
{
    if (gateway == 0)
        return 0U;
    if (SvcGateway_HasFreshSensorStatus(gateway) == 0U)
        return 0U;

    return gateway->sensorStatus.emergencyLatched;
}
