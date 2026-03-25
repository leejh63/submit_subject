#ifndef SVC_LIN_SENSOR_PROTO_H
#define SVC_LIN_SENSOR_PROTO_H

#include <stdint.h>
#include "core/emb_result.h"
#include "services/svc_zone.h"

#define SVC_LIN_SENSOR_PID_ADC_STATUS  (0x24U)
#define SVC_LIN_SENSOR_PID_OK_CMD      (0x25U)
#define SVC_LIN_SENSOR_OK_TOKEN        (0xA5U)
#define SVC_LIN_SENSOR_TIMEOUT_TICKS   (500U)
#define SVC_LIN_SENSOR_STATUS_SIZE     (8U)
#define SVC_LIN_SENSOR_OK_CMD_SIZE     (1U)

typedef struct
{
    uint16_t adcRaw;
    SvcZone zone;
    uint8_t emergencyLatched;
} SvcLinSensorStatus;

void SvcLinSensorProto_BuildStatusFrame(const SvcLinSensorStatus *status,
                                        uint8_t *outFrame,
                                        uint8_t frameSize);
EmbResult SvcLinSensorProto_ParseStatusFrame(const uint8_t *frame,
                                             uint8_t frameSize,
                                             SvcLinSensorStatus *outStatus);
void SvcLinSensorProto_BuildOkCmdFrame(uint8_t *outFrame, uint8_t frameSize);
uint8_t SvcLinSensorProto_IsEmergencyClearAccepted(uint8_t rxByte,
                                                   uint8_t emergencyLatched,
                                                   SvcZone zone);

#endif
