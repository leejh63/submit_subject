#include "platform/s32k_board.h"
#include "platform/s32k_bindings.h"
#include "platform/s32k_tick.h"

#define LIN_XCVR_ENABLE_PIN        (9UL)
#define LIN_XCVR_ENABLE_MASK       (0x1u << LIN_XCVR_ENABLE_PIN)
#define LIN_XCVR_ENABLE_GPIO_PORT  (PTE)

void LPTMR_ISR(void)
{
    S32kTick_IrqHandler();
}

static EmbResult S32kBoard_InitTickLptmr(uint32_t instance,
                                         IRQn_Type irq,
                                         const lptmr_user_config_t *config)
{
    S32kTick_Init();
    LPTMR_DRV_Init(instance, config, false);
    INT_SYS_InstallHandler(irq, LPTMR_ISR, 0);
    INT_SYS_EnableIRQ(irq);
    LPTMR_DRV_StartCounter(instance);
    return EMB_OK;
}

EmbResult S32kBoard_InitCommon(void)
{
    CLOCK_SYS_Init(S32K_LIN_SENSOR_CLOCK_CONFIGS,
                   S32K_LIN_SENSOR_CLOCK_CONFIG_COUNT,
                   S32K_LIN_SENSOR_CLOCK_CALLBACKS,
                   S32K_LIN_SENSOR_CLOCK_CALLBACK_COUNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PINS_DRV_Init(S32K_LIN_SENSOR_PIN_CONFIG_COUNT,
                  S32K_LIN_SENSOR_PIN_CONFIG_ARRAY);

    return EMB_OK;
}

EmbResult S32kBoard_InitLinSensorSlave(void)
{
    PINS_DRV_SetPinsDirection(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);
    PINS_DRV_SetPins(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);

#ifdef S32K_LIN_SENSOR_LPTMR_INSTANCE
    return S32kBoard_InitTickLptmr(S32K_LIN_SENSOR_LPTMR_INSTANCE,
                                   S32K_LIN_SENSOR_LPTMR_IRQn,
                                   &S32K_LIN_SENSOR_LPTMR_CONFIG);
#else
    return EMB_OK;
#endif
}

EmbResult S32kBoard_InitMasterNode(void)
{
#ifdef S32K_MASTER_CLOCK_CONFIGS
    CLOCK_SYS_Init(S32K_MASTER_CLOCK_CONFIGS,
                   S32K_MASTER_CLOCK_CONFIG_COUNT,
                   S32K_MASTER_CLOCK_CALLBACKS,
                   S32K_MASTER_CLOCK_CALLBACK_COUNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
#endif
#ifdef S32K_MASTER_PIN_CONFIG_ARRAY
    PINS_DRV_Init(S32K_MASTER_PIN_CONFIG_COUNT,
                  S32K_MASTER_PIN_CONFIG_ARRAY);
#endif
#ifdef S32K_MASTER_LPTMR_INSTANCE
    return S32kBoard_InitTickLptmr(S32K_MASTER_LPTMR_INSTANCE,
                                   S32K_MASTER_LPTMR_IRQn,
                                   &S32K_MASTER_LPTMR_CONFIG);
#else
    S32kTick_Init();
    return EMB_OK;
#endif
}

EmbResult S32kBoard_InitCanButtonSlave(void)
{
#ifdef S32K_BUTTON_CLOCK_CONFIGS
    CLOCK_SYS_Init(S32K_BUTTON_CLOCK_CONFIGS,
                   S32K_BUTTON_CLOCK_CONFIG_COUNT,
                   S32K_BUTTON_CLOCK_CALLBACKS,
                   S32K_BUTTON_CLOCK_CALLBACK_COUNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
#endif
#ifdef S32K_BUTTON_PIN_CONFIG_ARRAY
    PINS_DRV_Init(S32K_BUTTON_PIN_CONFIG_COUNT,
                  S32K_BUTTON_PIN_CONFIG_ARRAY);
#endif
#ifdef S32K_BUTTON_LPTMR_INSTANCE
    return S32kBoard_InitTickLptmr(S32K_BUTTON_LPTMR_INSTANCE,
                                   S32K_BUTTON_LPTMR_IRQn,
                                   &S32K_BUTTON_LPTMR_CONFIG);
#else
    S32kTick_Init();
    return EMB_OK;
#endif
}
