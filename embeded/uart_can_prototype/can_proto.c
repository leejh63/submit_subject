#include "can_proto.h"

#include <stddef.h>
#include <string.h>

static void CanProto_ClearFrame(CanFrame *frame)
{
    if (frame == NULL)
        return;

    (void)memset(frame, 0, sizeof(*frame));
}

static void CanProto_ClearMessage(CanMessage *message)
{
    if (message == NULL)
        return;

    (void)memset(message, 0, sizeof(*message));
}

static void CanProto_ClearEncodedList(CanEncodedFrameList *list)
{
    if (list == NULL)
        return;

    (void)memset(list, 0, sizeof(*list));
}

static uint8_t CanProto_IsValidNodeId(uint8_t nodeId)
{
    if (nodeId == CAN_NODE_ID_BROADCAST)
        return 1U;

    if (nodeId < CAN_NODE_ID_MIN || nodeId > CAN_NODE_ID_MAX)
        return 0U;

    return 1U;
}

static uint8_t CanProto_IsPrintableAscii(const char *text, uint8_t length)
{
    uint8_t i;
    unsigned char ch;

    if (text == NULL)
        return 0U;

    for (i = 0U; i < length; i++)
    {
        ch = (unsigned char)text[i];
        if (ch < 32U || ch > 126U)
            return 0U;
    }

    return 1U;
}

static uint32_t CanProto_MessageTypeToId(uint8_t messageType)
{
    switch (messageType)
    {
        case CAN_MSG_COMMAND:
            return CAN_PROTO_STDID_COMMAND;
        case CAN_MSG_RESPONSE:
            return CAN_PROTO_STDID_RESPONSE;
        case CAN_MSG_EVENT:
            return CAN_PROTO_STDID_EVENT;
        case CAN_MSG_TEXT:
            return CAN_PROTO_STDID_TEXT;
        default:
            return 0U;
    }
}

static uint8_t CanProto_IdToMessageType(uint32_t id, uint8_t *outMessageType)
{
    if (outMessageType == NULL)
        return 0U;

    switch (id)
    {
        case CAN_PROTO_STDID_COMMAND:
            *outMessageType = CAN_MSG_COMMAND;
            return 1U;
        case CAN_PROTO_STDID_RESPONSE:
            *outMessageType = CAN_MSG_RESPONSE;
            return 1U;
        case CAN_PROTO_STDID_EVENT:
            *outMessageType = CAN_MSG_EVENT;
            return 1U;
        case CAN_PROTO_STDID_TEXT:
            *outMessageType = CAN_MSG_TEXT;
            return 1U;
        default:
            return 0U;
    }
}

uint8_t CanProto_Init(CanProto *proto, const CanProtoConfig *config)
{
    if (proto == NULL || config == NULL)
        return 0U;

    (void)memset(proto, 0, sizeof(*proto));
    proto->localNodeId = config->localNodeId;
    proto->initialized = 1U;
    return 1U;
}

void CanProto_Task(CanProto *proto, uint32_t nowMs)
{
    (void)proto;
    (void)nowMs;
}

uint8_t CanProto_EncodeMessage(CanProto *proto,
                               const CanMessage *message,
                               CanEncodedFrameList *outList)
{
    CanFrame *frame;
    uint32_t frameId;
    uint8_t textLength;

    if (proto == NULL || message == NULL || outList == NULL || proto->initialized == 0U)
        return 0U;

    CanProto_ClearEncodedList(outList);

    if (message->version != CAN_PROTO_VERSION_V1)
        return 0U;

    if (CanProto_IsValidNodeId(message->sourceNodeId) == 0U)
        return 0U;

    if (CanProto_IsValidNodeId(message->targetNodeId) == 0U)
        return 0U;

    frameId = CanProto_MessageTypeToId(message->messageType);
    if (frameId == 0U)
        return 0U;

    frame = &outList->frames[0];
    CanProto_ClearFrame(frame);

    frame->id = frameId;
    frame->isExtendedId = 0U;
    frame->isRemoteFrame = 0U;

    if (message->messageType == CAN_MSG_TEXT)
    {
        textLength = message->textLength;
        if (textLength == 0U || textLength > CAN_TEXT_MAX_LEN)
            return 0U;

        if ((uint8_t)(CAN_TEXT_FRAME_HEADER_SIZE + textLength) > CAN_FRAME_DATA_SIZE)
            return 0U;

        if (CanProto_IsPrintableAscii(message->text, textLength) == 0U)
            return 0U;

        frame->dlc = (uint8_t)(CAN_TEXT_FRAME_HEADER_SIZE + textLength);
        frame->data[0] = message->version;
        frame->data[1] = message->sourceNodeId;
        frame->data[2] = message->targetNodeId;
        frame->data[3] = message->textType;
        frame->data[4] = textLength;
        (void)memcpy(&frame->data[5], message->text, textLength);

        outList->count = 1U;
        return 1U;
    }

    frame->dlc = 8U;
    frame->data[0] = message->version;
    frame->data[1] = message->sourceNodeId;
    frame->data[2] = message->targetNodeId;
    frame->data[3] = message->requestId;
    frame->data[4] = (message->payloadLength > 0U) ? message->payload[0] : 0U;
    frame->data[5] = (message->payloadLength > 1U) ? message->payload[1] : 0U;
    frame->data[6] = (message->payloadLength > 2U) ? message->payload[2] : 0U;
    frame->data[7] = message->flags;

    outList->count = 1U;
    return 1U;
}

CanProtoDecodeStatus CanProto_DecodeFrame(CanProto *proto,
                                          const CanFrame *frame,
                                          uint32_t nowMs,
                                          CanMessage *outMessage)
{
    uint8_t messageType;
    uint8_t textLength;

    (void)nowMs;
    // 중간중간 헬퍼함수로 따로 빼서 관리하게 편하게한느게 좋아보임
    // 지금 사실상 그냥 구조도 없이 임시코드 수준으로 만들어져있음
    if (proto == NULL || frame == NULL || outMessage == NULL || proto->initialized == 0U)
        return CAN_PROTO_DECODE_INVALID;

    CanProto_ClearMessage(outMessage);

    if (frame->isExtendedId != 0U || frame->isRemoteFrame != 0U)
    {
        proto->decodeUnsupportedCount++;
        return CAN_PROTO_DECODE_UNSUPPORTED;
    }

    if (CanProto_IdToMessageType(frame->id, &messageType) == 0U)
    {
        proto->decodeIgnoredCount++;
        return CAN_PROTO_DECODE_IGNORED;
    }
    // text_decode 헬퍼함수로 빼는게 좋을듯
    if (messageType == CAN_MSG_TEXT)
    {
        if (frame->dlc < CAN_TEXT_FRAME_HEADER_SIZE)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        if (frame->data[0] != CAN_PROTO_VERSION_V1)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        outMessage->version = frame->data[0];
        outMessage->messageType = messageType;
        outMessage->sourceNodeId = frame->data[1];
        outMessage->targetNodeId = frame->data[2];
        outMessage->textType = frame->data[3];
        outMessage->textLength = frame->data[4];
        outMessage->payloadKind = CAN_PAYLOAD_TEXT_DATA;

        if (CanProto_IsValidNodeId(outMessage->sourceNodeId) == 0U ||
            CanProto_IsValidNodeId(outMessage->targetNodeId) == 0U)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        textLength = outMessage->textLength;
        if (textLength == 0U || textLength > CAN_TEXT_MAX_LEN)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        if ((uint8_t)(CAN_TEXT_FRAME_HEADER_SIZE + textLength) > frame->dlc)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        (void)memcpy(outMessage->text, &frame->data[5], textLength);
        outMessage->text[textLength] = '\0';

        if (CanProto_IsPrintableAscii(outMessage->text, textLength) == 0U)
        {
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
        }

        proto->decodeOkCount++;
        return CAN_PROTO_DECODE_OK;
    }

    // 텍스트 명령어가 아닌 다른명령어
    if (frame->dlc < 8U)
    {
        proto->decodeInvalidCount++;
        return CAN_PROTO_DECODE_INVALID;
    }

    if (frame->data[0] != CAN_PROTO_VERSION_V1)
    {
        proto->decodeInvalidCount++;
        return CAN_PROTO_DECODE_INVALID;
    }

    outMessage->version = frame->data[0];
    outMessage->messageType = messageType;
    outMessage->sourceNodeId = frame->data[1];
    outMessage->targetNodeId = frame->data[2];
    outMessage->requestId = frame->data[3];
    outMessage->flags = frame->data[7];

    if (CanProto_IsValidNodeId(outMessage->sourceNodeId) == 0U ||
        CanProto_IsValidNodeId(outMessage->targetNodeId) == 0U)
    {
        proto->decodeInvalidCount++;
        return CAN_PROTO_DECODE_INVALID;
    }

    switch (messageType)
    {
        case CAN_MSG_COMMAND:
            outMessage->payloadKind = CAN_PAYLOAD_CTRL_CMD;
            outMessage->payloadLength = 3U;
            break;
        case CAN_MSG_RESPONSE:
            outMessage->payloadKind = CAN_PAYLOAD_CTRL_RESULT;
            outMessage->payloadLength = 3U;
            break;
        case CAN_MSG_EVENT:
            outMessage->payloadKind = CAN_PAYLOAD_EVENT_DATA;
            outMessage->payloadLength = 3U;
            break;
        default:
            proto->decodeInvalidCount++;
            return CAN_PROTO_DECODE_INVALID;
    }

    outMessage->payload[0] = frame->data[4];
    outMessage->payload[1] = frame->data[5];
    outMessage->payload[2] = frame->data[6];

    proto->decodeOkCount++;
    return CAN_PROTO_DECODE_OK;
}
