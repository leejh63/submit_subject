#ifndef APP_LIN_SENSOR_SLAVE_H
#define APP_LIN_SENSOR_SLAVE_H

#include <stdint.h>
#include "core/emb_result.h"
#include "drivers/drv_adc.h"
#include "drivers/drv_led.h"
#include "drivers/drv_lin_slave.h"
#include "services/svc_led_pattern.h"
#include "services/svc_zone.h"
#include "services/svc_lin_sensor_proto.h"

typedef struct
{
    uint32_t adcSamplePeriod500us;
    SvcZoneConfig zoneConfig;
} AppLinSensorSlaveConfig;

typedef struct
{
    AppLinSensorSlaveConfig config;
    DrvAdc adc;
    DrvLed led;
    DrvLinSlave lin;
    SvcLedPattern ledPattern;
    uint16_t adcLatestValue;
    SvcZone currentZone;
    uint32_t lastAdcSampleTick;
    uint8_t statusFrame[SVC_LIN_SENSOR_STATUS_SIZE];
    uint32_t okCmdAcceptedCount;
    uint32_t linFaultCount;
} AppLinSensorSlave;

EmbResult AppLinSensorSlave_Init(AppLinSensorSlave *app,
                                 const AppLinSensorSlaveConfig *appConfig,
                                 const DrvAdcConfig *adcConfig,
                                 const DrvLedConfig *ledConfig,
                                 const DrvLinSlaveConfig *linConfig);
EmbResult AppLinSensorSlave_Start(AppLinSensorSlave *app);
void AppLinSensorSlave_Process(AppLinSensorSlave *app, uint32_t now500us);
void AppLinSensorSlave_OnLinEvent(AppLinSensorSlave *app,
                                  uint32_t instance,
                                  void *linStatePtr);

#endif
