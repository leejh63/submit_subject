#ifndef SVC_GATEWAY_H
#define SVC_GATEWAY_H

#include <stdint.h>
#include "core/emb_result.h"
#include "services/svc_can_button_proto.h"
#include "services/svc_lin_sensor_proto.h"

typedef struct
{
    uint32_t sensorPollPeriodMs;
    uint32_t sensorStaleMs;
    uint32_t ackRetryPeriodMs;
    uint8_t ackMaxRetryCount;
} SvcGatewayConfig;

typedef struct
{
    SvcGatewayConfig config;
    SvcLinSensorStatus sensorStatus;
    uint8_t sensorStatusValid;
    uint8_t sensorStatusStale;
    uint8_t forceSensorPoll;
    uint8_t linAckPending;
    uint8_t linAckAwaitingClear;
    uint8_t linAckRetryCount;
    uint32_t lastSensorPollRequestTick;
    uint32_t lastSensorStatusTick;
    uint32_t lastLinAckTick;
    uint32_t sensorPollRequestCount;
    uint32_t sensorUpdateCount;
    uint32_t sensorTimeoutCount;
    uint32_t linAckRequestCount;
    uint32_t linAckSendCount;
    uint32_t linAckConfirmCount;
    uint32_t linAckRetryGiveUpCount;
    uint8_t lastButtonId;
    SvcCanButtonAction lastButtonAction;
} SvcGateway;

EmbResult SvcGateway_Init(SvcGateway *gateway, const SvcGatewayConfig *config);
void SvcGateway_Process(SvcGateway *gateway, uint32_t nowMs);
uint8_t SvcGateway_ShouldPollSensor(const SvcGateway *gateway, uint32_t nowMs);
void SvcGateway_MarkSensorPollIssued(SvcGateway *gateway, uint32_t nowMs);
void SvcGateway_OnSensorStatus(SvcGateway *gateway,
                               const SvcLinSensorStatus *status,
                               uint32_t nowMs);
void SvcGateway_OnButtonEvent(SvcGateway *gateway,
                              const SvcCanButtonEvent *event);
void SvcGateway_RequestLinAck(SvcGateway *gateway);
void SvcGateway_RequestImmediatePoll(SvcGateway *gateway);
uint8_t SvcGateway_ShouldSendLinAck(const SvcGateway *gateway, uint32_t nowMs);
void SvcGateway_MarkLinAckSent(SvcGateway *gateway, uint32_t nowMs);
uint8_t SvcGateway_HasFreshSensorStatus(const SvcGateway *gateway);
uint8_t SvcGateway_IsEmergencyActive(const SvcGateway *gateway);

#endif
