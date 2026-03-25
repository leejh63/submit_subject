#include "tick_hw.h"

#include <stddef.h>

#include "../platform/s32k_sdk/isosdk_tick.h"

InfraStatus TickHw_Init(TickHwHandler handler)
{
    if (handler == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return (IsoSdk_TickInit(handler) != 0U) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

void TickHw_ClearCompareFlag(void)
{
    IsoSdk_TickClearCompareFlag();
}
