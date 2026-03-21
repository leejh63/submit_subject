#include "platform/s32k_board.h"
#include "platform/s32k_tick.h"
#include "app/app_can_button_slave.h"
#include "hal/hal_s32k_can.h"

static AppCanButtonSlave g_button;

int main(void)
{
    DrvCanConfig canConfig;

    (void)S32kBoard_InitCanButtonSlave();

    canConfig.port.binding = HAL_S32K_CAN_BIND_BUTTON;
#ifdef S32K_BUTTON_CAN_TX_MB
    canConfig.port.txMb = S32K_BUTTON_CAN_TX_MB;
#else
    canConfig.port.txMb = 1U;
#endif
#ifdef S32K_BUTTON_CAN_RX_MB
    canConfig.port.rxMb = S32K_BUTTON_CAN_RX_MB;
#else
    canConfig.port.rxMb = 0U;
#endif

    if (AppCanButtonSlave_Init(&g_button, &canConfig) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    if (AppCanButtonSlave_Start(&g_button) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    for (;;)
    {
        AppCanButtonSlave_Process(&g_button, S32kTick_GetMs());
    }
}
