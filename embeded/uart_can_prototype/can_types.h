#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#include <stdint.h>

/*
 * CAN 스택 전체(can_hw / can_transport / can_proto / can_service / can_app)가
 * 공통으로 사용하는 상수, enum, 기본 데이터 모델 모음.
 * 이 파일이 CAN 계층의 "공통 언어" 역할을 한다.
 */

// -----------------------------------------------------------------------------
// 노드 ID 규칙
// -----------------------------------------------------------------------------

#define CAN_NODE_ID_INVALID              0U    /* 유효하지 않은 ID */
#define CAN_NODE_ID_MIN                  1U    /* 사용 가능한 최소 노드 ID */
#define CAN_NODE_ID_MAX                  254U  /* 사용 가능한 최대 노드 ID */
#define CAN_NODE_ID_BROADCAST            255U  /* 전체 브로드캐스트 */

// -----------------------------------------------------------------------------
// 프레임 / payload 크기
// -----------------------------------------------------------------------------

#define CAN_FD_PAYLOAD_SIZE              16U   /* 현재 설계에서 사용하는 FD payload 길이 */
#define CAN_FRAME_DATA_SIZE              CAN_FD_PAYLOAD_SIZE

#define CAN_TEXT_MAX_LEN                 11U   /* 텍스트 payload 최대 길이 */
#define CAN_TEXT_FRAME_HEADER_SIZE       5U    /* text 프레임 메타데이터 바이트 수 */

// -----------------------------------------------------------------------------
// 하위 계층 큐/MB 기본값
// -----------------------------------------------------------------------------

#define CAN_HW_RX_MB_INDEX               0U
#define CAN_HW_TX_MB_INDEX               1U
#define CAN_HW_RX_QUEUE_SIZE             8U

#define CAN_TRANSPORT_TX_QUEUE_SIZE      8U
#define CAN_TRANSPORT_RX_QUEUE_SIZE      8U

#define CAN_SERVICE_QUEUE_SIZE           8U
#define CAN_SERVICE_PENDING_SIZE         4U

// -----------------------------------------------------------------------------
// 프로토콜 버전 / ID 매핑
// -----------------------------------------------------------------------------

#define CAN_PROTO_VERSION_V1             1U
#define CAN_PROTO_MAX_ENCODED_FRAMES     1U    /* 현재는 1 메시지 = 1 프레임 */

#define CAN_PROTO_STDID_COMMAND          0x120U
#define CAN_PROTO_STDID_RESPONSE         0x121U
#define CAN_PROTO_STDID_EVENT            0x122U
#define CAN_PROTO_STDID_TEXT             0x123U

#define CAN_MSG_FLAG_NEED_RESPONSE       (1U << 0) /* 응답 요구 */
#define CAN_MSG_FLAG_ERROR               (1U << 1) /* 오류 성격 표시 */

typedef enum
{
    CAN_APP_ROLE_NONE = 0,   /* 역할 미설정 */
    CAN_APP_ROLE_MASTER,     /* 마스터 */
    CAN_APP_ROLE_SLAVE,      /* 슬레이브 1 */
    CAN_APP_ROLE_SLAVE_TWO   /* 슬레이브 2 등 확장용 */
} CanAppRole;

typedef enum
{
    CAN_MSG_COMMAND = 0, /* 제어 명령 */
    CAN_MSG_RESPONSE,    /* 명령 응답 */
    CAN_MSG_EVENT,       /* 비동기 이벤트 */
    CAN_MSG_TEXT         /* 짧은 ASCII 텍스트 */
} CanMessageType;

typedef enum
{
    CAN_PAYLOAD_NONE = 0,      /* payload 미지정 */
    CAN_PAYLOAD_CTRL_CMD,      /* payload[0..]를 제어 명령으로 해석 */
    CAN_PAYLOAD_CTRL_RESULT,   /* payload[0..]를 결과 코드로 해석 */
    CAN_PAYLOAD_EVENT_DATA,    /* payload[0..]를 이벤트 데이터로 해석 */
    CAN_PAYLOAD_TEXT_DATA      /* text[]를 본문으로 사용 */
} CanPayloadKind;

typedef enum
{
    CAN_CMD_NONE = 0,
    CAN_CMD_OPEN,
    CAN_CMD_CLOSE,
    CAN_CMD_OFF,
    CAN_CMD_TEST,
    CAN_CMD_STATUS_REQ
} CanCommandCode;

typedef enum
{
    CAN_RES_NONE = 0,
    CAN_RES_OK,
    CAN_RES_FAIL,
    CAN_RES_INVALID_TARGET,
    CAN_RES_BUSY,
    CAN_RES_TIMEOUT,
    CAN_RES_NOT_SUPPORTED
} CanResultCode;

typedef enum
{
    CAN_EVENT_NONE = 0,
    CAN_EVENT_BOOT,
    CAN_EVENT_ONLINE,
    CAN_EVENT_OFFLINE,
    CAN_EVENT_HEARTBEAT,
    CAN_EVENT_WARNING,
    CAN_EVENT_ERROR
} CanEventCode;

typedef enum
{
    CAN_TEXT_NONE = 0,
    CAN_TEXT_USER,
    CAN_TEXT_LOG,
    CAN_TEXT_DEBUG,
    CAN_TEXT_RESPONSE,
    CAN_TEXT_EVENT
} CanTextType;

typedef enum
{
    CAN_PROTO_DECODE_OK = 0,   /* 정상 디코딩 */
    CAN_PROTO_DECODE_IGNORED,  /* 관심 없는 ID라 무시 */
    CAN_PROTO_DECODE_INVALID,  /* 형식 오류 */
    CAN_PROTO_DECODE_UNSUPPORTED /* 지원하지 않는 형태 */
} CanProtoDecodeStatus;

typedef enum
{
    CAN_HW_ERROR_NONE = 0,
    CAN_HW_ERROR_NOT_READY,
    CAN_HW_ERROR_INIT_FAIL,
    CAN_HW_ERROR_RX_CONFIG_FAIL,
    CAN_HW_ERROR_TX_CONFIG_FAIL,
    CAN_HW_ERROR_TX_START_FAIL,
    CAN_HW_ERROR_TX_STATUS_FAIL,
    CAN_HW_ERROR_RX_STATUS_FAIL,
    CAN_HW_ERROR_RX_QUEUE_FULL,
    CAN_HW_ERROR_RX_RESTART_FAIL
} CanHwError;

typedef enum
{
    CAN_TRANSPORT_ERROR_NONE = 0,
    CAN_TRANSPORT_ERROR_NOT_READY,
    CAN_TRANSPORT_ERROR_TX_QUEUE_FULL,
    CAN_TRANSPORT_ERROR_RX_QUEUE_FULL,
    CAN_TRANSPORT_ERROR_HW_TX_BUSY,
    CAN_TRANSPORT_ERROR_HW_TX_FAIL,
    CAN_TRANSPORT_ERROR_HW_RX_FAIL
} CanTransportError;

typedef enum
{
    CAN_SERVICE_ERROR_NONE = 0,
    CAN_SERVICE_ERROR_NOT_READY,
    CAN_SERVICE_ERROR_INVALID_TARGET,
    CAN_SERVICE_ERROR_PENDING_FULL,
    CAN_SERVICE_ERROR_TX_QUEUE_FULL,
    CAN_SERVICE_ERROR_PROTOCOL_ERROR,
    CAN_SERVICE_ERROR_UNSUPPORTED
} CanServiceError;

typedef enum
{
    CAN_SERVICE_RESULT_NONE = 0, /* 초기 상태 */
    CAN_SERVICE_RESULT_RESPONSE, /* 정상 응답 수신 */
    CAN_SERVICE_RESULT_TIMEOUT,  /* timeout 발생 */
    CAN_SERVICE_RESULT_SEND_FAIL /* 송신 자체 실패 */
} CanServiceResultKind;

/* 실제 CAN HW 레이어가 주고받는 원시 프레임 표현 */
typedef struct
{
    uint32_t id;                              /* 표준/확장 CAN ID */
    uint8_t  dlc;                             /* 데이터 길이 */
    uint8_t  data[CAN_FRAME_DATA_SIZE];       /* payload 바이트 */
    uint8_t  isExtendedId;                    /* 확장 ID 여부 */
    uint8_t  isRemoteFrame;                   /* RTR 여부 */
    uint32_t timestampMs;                     /* 수신 시각 */
} CanFrame;

/*
 * 프로토콜 해석이 끝난 논리 메시지.
 * 상위 계층은 보통 CanFrame 대신 이 구조를 사용한다.
 */
typedef struct
{
    uint8_t  version;                         /* 프로토콜 버전 */
    uint8_t  messageType;                     /* CanMessageType */
    uint8_t  sourceNodeId;                    /* 보낸 노드 */
    uint8_t  targetNodeId;                    /* 받는 노드 */
    uint8_t  requestId;                       /* request/response 매칭용 ID */
    uint8_t  flags;                           /* CAN_MSG_FLAG_* */
    uint8_t  payloadKind;                     /* payload 해석 방식 */
    uint8_t  payloadLength;                   /* payload 유효 길이 */
    uint8_t  payload[CAN_FRAME_DATA_SIZE];    /* command/result/event 바이트 */

    uint8_t  textType;                        /* text 메시지 종류 */
    uint8_t  textLength;                      /* text 길이 */
    char     text[CAN_TEXT_MAX_LEN + 1U];     /* NUL 종료 텍스트 */
} CanMessage;

/* service 계층이 상위로 올려주는 응답/timeout 결과 */
typedef struct
{
    uint8_t  kind;         /* CanServiceResultKind */
    uint8_t  requestId;    /* 매칭된 requestId */
    uint8_t  sourceNodeId; /* 응답을 보낸 노드 */
    uint8_t  targetNodeId; /* 원래 수신 대상 */
    uint8_t  commandCode;  /* 어떤 명령에 대한 결과인지 */
    uint8_t  resultCode;   /* CanResultCode */
    uint8_t  detailCode;   /* 세부 정보 */
} CanServiceResult;

#endif
