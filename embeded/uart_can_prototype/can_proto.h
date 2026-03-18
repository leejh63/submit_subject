#ifndef CAN_PROTO_H
#define CAN_PROTO_H

#include <stdint.h>

#include "can_types.h"

typedef struct
{
    uint8_t localNodeId;
} CanProtoConfig;

typedef struct
{
    CanFrame frames[CAN_PROTO_MAX_ENCODED_FRAMES];
    uint8_t  count;
} CanEncodedFrameList;

typedef struct
{
    uint8_t  initialized;
    uint8_t  localNodeId;
    uint8_t  lastError;

    uint32_t decodeOkCount;
    uint32_t decodeIgnoredCount;
    uint32_t decodeInvalidCount;
    uint32_t decodeUnsupportedCount;
} CanProto;

uint8_t              CanProto_Init(CanProto *proto, const CanProtoConfig *config);
void                 CanProto_Task(CanProto *proto, uint32_t nowMs);
uint8_t              CanProto_EncodeMessage(CanProto *proto,
                                            const CanMessage *message,
                                            CanEncodedFrameList *outList);
CanProtoDecodeStatus CanProto_DecodeFrame(CanProto *proto,
                                          const CanFrame *frame,
                                          uint32_t nowMs,
                                          CanMessage *outMessage);

#endif
