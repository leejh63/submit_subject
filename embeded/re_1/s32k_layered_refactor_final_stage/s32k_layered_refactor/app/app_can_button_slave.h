#ifndef APP_CAN_BUTTON_SLAVE_H
#define APP_CAN_BUTTON_SLAVE_H

#include "drivers/drv_can.h"
#include "services/svc_can_button_proto.h"
#include "core/emb_result.h"

typedef struct
{
    DrvCan can;
    uint32_t lastSampleTick;
    uint8_t pendingValid;
    uint8_t nextSequence;
    SvcCanButtonEvent pendingEvent;
} AppCanButtonSlave;

EmbResult AppCanButtonSlave_Init(AppCanButtonSlave *app,
                                 const DrvCanConfig *canConfig);
EmbResult AppCanButtonSlave_Start(AppCanButtonSlave *app);
EmbResult AppCanButtonSlave_ReportButtonEvent(AppCanButtonSlave *app,
                                              uint8_t buttonId,
                                              SvcCanButtonAction action);
void AppCanButtonSlave_Process(AppCanButtonSlave *app, uint32_t nowMs);

#endif
