#include <string.h>
#include "services/svc_can_button_proto.h"

EmbResult SvcCanButtonProto_BuildEventFrame(const SvcCanButtonEvent *event,
                                            HalCanFrame *frame)
{
    if (event == 0 || frame == 0)
        return EMB_EINVAL;
    if (event->action == SVC_CAN_BUTTON_ACTION_NONE)
        return EMB_EINVAL;

    (void)memset(frame, 0, sizeof(*frame));
    frame->id = SVC_CAN_BUTTON_EVENT_ID;
    frame->dlc = 4U;
    frame->isExtendedId = 0U;
    frame->data[0] = 0x42U;
    frame->data[1] = event->buttonId;
    frame->data[2] = (uint8_t)event->action;
    frame->data[3] = event->sequence;
    return EMB_OK;
}

EmbResult SvcCanButtonProto_ParseEventFrame(const HalCanFrame *frame,
                                            SvcCanButtonEvent *event)
{
    if (frame == 0 || event == 0)
        return EMB_EINVAL;
    if (frame->id != SVC_CAN_BUTTON_EVENT_ID)
        return EMB_EINVAL;
    if (frame->dlc < 4U)
        return EMB_EINVAL;
    if (frame->data[0] != 0x42U)
        return EMB_EINVAL;

    event->buttonId = frame->data[1];
    event->action = (SvcCanButtonAction)frame->data[2];
    event->sequence = frame->data[3];
    return EMB_OK;
}
