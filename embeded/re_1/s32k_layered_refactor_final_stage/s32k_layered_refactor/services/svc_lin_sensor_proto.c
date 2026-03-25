#include <string.h>
#include "services/svc_lin_sensor_proto.h"

void SvcLinSensorProto_BuildStatusFrame(const SvcLinSensorStatus *status,
                                        uint8_t *outFrame,
                                        uint8_t frameSize)
{
    if (status == 0 || outFrame == 0 || frameSize == 0U)
        return;

    (void)memset(outFrame, 0, frameSize);
    outFrame[0] = (uint8_t)(status->adcRaw & 0xFFU);
    if (frameSize > 1U)
        outFrame[1] = (uint8_t)((status->adcRaw >> 8U) & 0x0FU);
    if (frameSize > 2U)
        outFrame[2] = (uint8_t)status->zone;
    if (frameSize > 3U)
        outFrame[3] = status->emergencyLatched;
}

EmbResult SvcLinSensorProto_ParseStatusFrame(const uint8_t *frame,
                                             uint8_t frameSize,
                                             SvcLinSensorStatus *outStatus)
{
    if (frame == 0 || outStatus == 0)
        return EMB_EINVAL;
    if (frameSize < 4U)
        return EMB_EINVAL;

    outStatus->adcRaw = (uint16_t)frame[0] | ((uint16_t)(frame[1] & 0x0FU) << 8U);
    outStatus->zone = (SvcZone)frame[2];
    outStatus->emergencyLatched = frame[3];
    return EMB_OK;
}

void SvcLinSensorProto_BuildOkCmdFrame(uint8_t *outFrame, uint8_t frameSize)
{
    if (outFrame == 0 || frameSize == 0U)
        return;

    (void)memset(outFrame, 0, frameSize);
    outFrame[0] = SVC_LIN_SENSOR_OK_TOKEN;
}

uint8_t SvcLinSensorProto_IsEmergencyClearAccepted(uint8_t rxByte,
                                                   uint8_t emergencyLatched,
                                                   SvcZone zone)
{
    if (rxByte != SVC_LIN_SENSOR_OK_TOKEN)
        return 0U;
    if (emergencyLatched == 0U)
        return 0U;
    if (zone == SVC_ZONE_EMERGENCY)
        return 0U;

    return 1U;
}
