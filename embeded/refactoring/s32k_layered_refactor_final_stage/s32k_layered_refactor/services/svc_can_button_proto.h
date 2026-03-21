#ifndef SVC_CAN_BUTTON_PROTO_H
#define SVC_CAN_BUTTON_PROTO_H

#include <stdint.h>
#include "core/emb_result.h"
#include "hal/hal_s32k_can.h"

#define SVC_CAN_BUTTON_EVENT_ID  0x301U

typedef enum
{
    SVC_CAN_BUTTON_ACTION_NONE = 0,
    SVC_CAN_BUTTON_ACTION_PRESSED = 1,
    SVC_CAN_BUTTON_ACTION_RELEASED = 2,
    SVC_CAN_BUTTON_ACTION_LONG_PRESS = 3,
    SVC_CAN_BUTTON_ACTION_ACK_REQUEST = 4
} SvcCanButtonAction;

typedef struct
{
    uint8_t buttonId;
    SvcCanButtonAction action;
    uint8_t sequence;
} SvcCanButtonEvent;

EmbResult SvcCanButtonProto_BuildEventFrame(const SvcCanButtonEvent *event,
                                            HalCanFrame *frame);
EmbResult SvcCanButtonProto_ParseEventFrame(const HalCanFrame *frame,
                                            SvcCanButtonEvent *event);

#endif
