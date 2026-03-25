#include <string.h>
#include "app/app_can_button_slave.h"

EmbResult AppCanButtonSlave_Init(AppCanButtonSlave *app,
                                 const DrvCanConfig *canConfig)
{
    if (app == 0 || canConfig == 0)
        return EMB_EINVAL;

    (void)memset(app, 0, sizeof(*app));
    return DrvCan_Init(&app->can, canConfig);
}

EmbResult AppCanButtonSlave_Start(AppCanButtonSlave *app)
{
    if (app == 0)
        return EMB_EINVAL;

    return DrvCan_Start(&app->can);
}

EmbResult AppCanButtonSlave_ReportButtonEvent(AppCanButtonSlave *app,
                                              uint8_t buttonId,
                                              SvcCanButtonAction action)
{
    if (app == 0)
        return EMB_EINVAL;
    if (action == SVC_CAN_BUTTON_ACTION_NONE)
        return EMB_EINVAL;

    app->pendingEvent.buttonId = buttonId;
    app->pendingEvent.action = action;
    app->pendingEvent.sequence = app->nextSequence++;
    app->pendingValid = 1U;
    return EMB_OK;
}

void AppCanButtonSlave_Process(AppCanButtonSlave *app, uint32_t nowMs)
{
    HalCanFrame frame;

    if (app == 0)
        return;

    (void)DrvCan_Process(&app->can);

    if (app->pendingValid != 0U)
    {
        if (SvcCanButtonProto_BuildEventFrame(&app->pendingEvent, &frame) == EMB_OK)
        {
            if (DrvCan_Send(&app->can, &frame) == EMB_OK)
                app->pendingValid = 0U;
        }
    }

    app->lastSampleTick = nowMs;
}
