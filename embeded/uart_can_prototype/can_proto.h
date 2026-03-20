#ifndef CAN_PROTO_H
#define CAN_PROTO_H

#include <stdint.h>

#include "can_types.h"

/*
 * CAN frame <-> 논리 메시지 변환 계층.
 * transport는 프레임을 다루고, service/app은 메시지를 다루기 때문에
 * 그 사이 변환을 이 모듈이 맡는다.
 */

typedef struct
{
    uint8_t localNodeId; /* 현재 노드 ID */
} CanProtoConfig;

/* 1개의 논리 메시지가 몇 개의 프레임으로 인코딩되었는지 담는 결과 구조체 */
typedef struct
{
    CanFrame frames[CAN_PROTO_MAX_ENCODED_FRAMES]; /* 인코딩된 프레임 목록 */
    uint8_t  count;                                /* 유효 프레임 수 */
} CanEncodedFrameList;

typedef struct
{
    uint8_t  initialized;          /* 초기화 여부 */
    uint8_t  localNodeId;          /* 현재 노드 ID */
    uint8_t  lastError;            /* 향후 확장용 마지막 오류 */

    uint32_t decodeOkCount;        /* 정상 디코딩 횟수 */
    uint32_t decodeIgnoredCount;   /* 관심 없는 ID 무시 횟수 */
    uint32_t decodeInvalidCount;   /* 형식 오류 횟수 */
    uint32_t decodeUnsupportedCount; /* 확장/RTR 등 미지원 횟수 */
} CanProto;

/* proto 컨텍스트를 초기화한다. */
uint8_t              CanProto_Init(CanProto *proto, const CanProtoConfig *config);

/* 현재 구현에서는 실질 작업이 거의 없지만, 계층 정합성을 위해 유지되는 주기 함수다. */
void                 CanProto_Task(CanProto *proto, uint32_t nowMs);

/* 논리 메시지 1개를 원시 CAN frame 목록으로 인코딩한다. */
uint8_t              CanProto_EncodeMessage(CanProto *proto,
                                            const CanMessage *message,
                                            CanEncodedFrameList *outList);

/* 원시 CAN frame 1개를 논리 메시지로 디코딩한다. */
CanProtoDecodeStatus CanProto_DecodeFrame(CanProto *proto,
                                          const CanFrame *frame,
                                          uint32_t nowMs,
                                          CanMessage *outMessage);

/*
 * -----------------------------------------------------------------------------
 * 내부 static 함수 정리 (can_proto.c 전용, 참고용 메모)
 * -----------------------------------------------------------------------------
 *
 * [can_proto.c]
 * - static void CanProto_ClearFrame(CanFrame *frame);
 *   : 인코딩 전에 frame을 0 초기화한다.
 *
 * - static void CanProto_ClearMessage(CanMessage *message);
 *   : 디코딩 전에 message를 0 초기화한다.
 *
 * - static void CanProto_ClearEncodedList(CanEncodedFrameList *list);
 *   : 인코딩 결과 목록을 초기화한다.
 *
 * - static uint8_t CanProto_IsValidNodeId(uint8_t nodeId);
 *   : node ID 범위 및 broadcast 허용 여부를 검사한다.
 *
 * - static uint8_t CanProto_IsPrintableAscii(const char *text, uint8_t length);
 *   : text payload가 출력 가능한 ASCII 범위인지 검사한다.
 *
 * - static uint32_t CanProto_MessageTypeToId(uint8_t messageType);
 *   : 논리 메시지 타입을 CAN 표준 ID로 바꾼다.
 *
 * - static uint8_t CanProto_IdToMessageType(uint32_t id, uint8_t *outMessageType);
 *   : CAN 표준 ID를 논리 메시지 타입으로 역변환한다.
 */
#endif
