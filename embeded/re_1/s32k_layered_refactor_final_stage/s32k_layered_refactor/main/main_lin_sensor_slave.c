#include "platform/s32k_board.h"
#include "platform/s32k_bindings.h"
#include "platform/s32k_tick.h"
#include "app/app_lin_sensor_slave.h"
#include "hal/hal_s32k_lin.h"

#define SLAVE2_LED_RED_PIN         (15U)
#define SLAVE2_LED_GREEN_PIN       (16U)
#define SLAVE2_LED_GPIO_PORT       (PTD)
#define SLAVE2_LED_RED_MASK        (1UL << SLAVE2_LED_RED_PIN)
#define SLAVE2_LED_GREEN_MASK      (1UL << SLAVE2_LED_GREEN_PIN)
#define SLAVE2_LED_ON              (0U)
#define SLAVE2_LED_OFF             (1U)
#define SLAVE2_ADC_RANGE_MAX       (4096U)
#define SLAVE2_ADC_SAFE_MAX        (SLAVE2_ADC_RANGE_MAX / 3U)
#define SLAVE2_ADC_WARNING_MAX     ((SLAVE2_ADC_RANGE_MAX * 2U) / 3U)
#define SLAVE2_ADC_EMERGENCY_MIN   ((SLAVE2_ADC_RANGE_MAX * 5U) / 6U)

static AppLinSensorSlave g_app;

int main(void)
{
    AppLinSensorSlaveConfig appConfig;
    DrvAdcConfig adcConfig;
    DrvLedConfig ledConfig;
    DrvLinSlaveConfig linConfig;

    (void)S32kBoard_InitCommon();
    (void)S32kBoard_InitLinSensorSlave();

    appConfig.adcSamplePeriod500us = 40U;
    appConfig.zoneConfig.rangeMax = SLAVE2_ADC_RANGE_MAX;
    appConfig.zoneConfig.safeMax = SLAVE2_ADC_SAFE_MAX;
    appConfig.zoneConfig.warningMax = SLAVE2_ADC_WARNING_MAX;
    appConfig.zoneConfig.emergencyMin = SLAVE2_ADC_EMERGENCY_MIN;

    adcConfig.channel.instance = S32K_LIN_SENSOR_ADC_INSTANCE;
    adcConfig.channel.group = S32K_LIN_SENSOR_ADC_GROUP;

    ledConfig.gpioBase = (void *)SLAVE2_LED_GPIO_PORT;
    ledConfig.redPin = SLAVE2_LED_RED_PIN;
    ledConfig.greenPin = SLAVE2_LED_GREEN_PIN;
    ledConfig.pinMask = SLAVE2_LED_RED_MASK | SLAVE2_LED_GREEN_MASK;
    ledConfig.activeLevel = SLAVE2_LED_ON;
    ledConfig.inactiveLevel = SLAVE2_LED_OFF;

    linConfig.port.binding = HAL_S32K_LIN_BIND_SENSOR_SLAVE;
    linConfig.port.instance = S32K_LIN_SENSOR_LIN_INSTANCE;
    linConfig.port.timeoutTicks = 500U;
    linConfig.txSize = 8U;
    linConfig.rxSize = 1U;

    if (AppLinSensorSlave_Init(&g_app,
                               &appConfig,
                               &adcConfig,
                               &ledConfig,
                               &linConfig) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    if (AppLinSensorSlave_Start(&g_app) != EMB_OK)
    {
        for (;;)
        {
        }
    }

    for (;;)
    {
        AppLinSensorSlave_Process(&g_app, S32kTick_Get500us());
    }
}
