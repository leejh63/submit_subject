#include "isosdk_tick.h"

#include <stddef.h>

#include "interrupt_manager.h"

#include "isosdk_sdk_bindings.h"

uint8_t IsoSdk_TickInit(IsoSdkTickHandler handler)
{
    if (handler == NULL)
    {
        return 0U;
    }

    LPTMR_DRV_Init(ISOSDK_SDK_LPTMR_INSTANCE, &ISOSDK_SDK_LPTMR_CONFIG, false);
    INT_SYS_InstallHandler(ISOSDK_SDK_LPTMR_IRQ, (isr_t)handler, (isr_t *)NULL);
    INT_SYS_EnableIRQ(ISOSDK_SDK_LPTMR_IRQ);
    LPTMR_DRV_StartCounter(ISOSDK_SDK_LPTMR_INSTANCE);
    return 1U;
}

void IsoSdk_TickClearCompareFlag(void)
{
    LPTMR_DRV_ClearCompareFlag(ISOSDK_SDK_LPTMR_INSTANCE);
}
