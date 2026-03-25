#include "sdk_project_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Enable this after ADC SDK files and generated ADC peripheral headers
 * are added to sdk_slave_red.
 */
#ifndef SLAVE2_ENABLE_ADC
#define SLAVE2_ENABLE_ADC  1U
#endif

#if (SLAVE2_ENABLE_ADC != 0U)
#include "adc_driver.h"
#include "peripherals_adc_config_1.h"
#endif

#define LIN_XCVR_ENABLE_PIN        (9UL)
#define LIN_XCVR_ENABLE_MASK       (0x1u << LIN_XCVR_ENABLE_PIN)
#define LIN_XCVR_ENABLE_GPIO_PORT  (PTE)

#define TIMER_COMPARE_VAL          (uint16_t)(2000U)
#define TIMER_TICKS_1US            (uint16_t)(4U)

#define DATA_SIZE                  (8U)
#define TIMEOUT                    (500U)

#define PID_SLAVE2_ADC_STATUS      (0x24U)
#define PID_SLAVE2_OK_CMD          (0x25U)
#define SLAVE2_OK_TOKEN            (0xA5U)
#define SLAVE2_ENABLE_AUTOBAUD     (0U)

#define SLAVE2_ADC_RANGE_MAX       (4096U)
#define SLAVE2_ADC_SAFE_MAX        (SLAVE2_ADC_RANGE_MAX / 3U)
#define SLAVE2_ADC_WARNING_MAX     ((SLAVE2_ADC_RANGE_MAX * 2U) / 3U)
#define SLAVE2_ADC_EMERGENCY_MIN   ((SLAVE2_ADC_RANGE_MAX * 5U) / 6U)

#define SLAVE2_LED_RED_PIN         (15U)
#define SLAVE2_LED_GREEN_PIN       (16U)
#define SLAVE2_LED_GPIO_PORT       (PTD)
#define SLAVE2_LED_RED_MASK        (1UL << SLAVE2_LED_RED_PIN)
#define SLAVE2_LED_GREEN_MASK      (1UL << SLAVE2_LED_GREEN_PIN)
#define SLAVE2_LED_ON              (0U)
#define SLAVE2_LED_OFF             (1U)

#define TICKS_FROM_MS(ms)          ((uint32_t)((ms) * 2U))
#define ADC_SAMPLE_PERIOD_TICKS    TICKS_FROM_MS(20U)
#define EMERGENCY_BLINK_PERIOD_TICKS TICKS_FROM_MS(200U)

#if (SLAVE2_ENABLE_ADC != 0U)
#define ADC_INST                   (INST_ADC_CONFIG_1)
#define ADC_GROUP                  (0U)
#endif

typedef enum
{
    LIN_SLAVE_IDLE = 0,
    LIN_SLAVE_WAIT_TX,
    LIN_SLAVE_WAIT_RX
} LinSlaveState;

typedef enum
{
    SLAVE2_ZONE_SAFE = 0U,
    SLAVE2_ZONE_WARNING,
    SLAVE2_ZONE_DANGER,
    SLAVE2_ZONE_EMERGENCY
} Slave2Zone;

static uint8_t            g_linTxBuffer[DATA_SIZE];
static uint8_t            g_linRxBuffer[1];
static uint16_t           g_timerOverflowInterruptCount = 0U;
static volatile uint32_t  g_tick500us = 0U;
static volatile uint16_t  g_adcLatestValue = 0U;
static LinSlaveState      g_linState = LIN_SLAVE_IDLE;
static Slave2Zone         g_currentZone = SLAVE2_ZONE_SAFE;
static uint8_t            g_emergencyLatched = 0U;
static uint8_t            g_emergencyBlinkOn = 0U;
static uint32_t           g_lastAdcSampleTick = 0U;
static uint32_t           g_lastEmergencyBlinkTick = 0U;

static void Slave2_AdcInit(void);
static uint16_t Slave2_AdcReadBlocking(void);
static void Slave2_LedInit(void);
static Slave2Zone Slave2_ClassifyZone(uint16_t adcValue);
static void Slave2_SetLedSafe(void);
static void Slave2_SetLedWarning(void);
static void Slave2_SetLedDanger(void);
static void Slave2_SetLedEmergencyBlink(uint8_t on);
static void Slave2_ApplyUnlockedZoneLed(Slave2Zone zone);
static void Slave2_UpdateLedFromAdc(uint16_t adcValue);
static void Slave2_RunEmergencyBlinkTask(void);
static void Slave2_RunAdcTask(void);
static void Slave2_PrepareAdcFrame(void);
static void Slave2_GoIdle(void);
static void Slave2_LinFsmStep(void);
static void Slave2_LinCallback(uint32_t instance, void *linStatePtr);

void LPTMR_ISR(void)
{
    LIN_DRV_TimeoutService(INST_LIN2);
    g_timerOverflowInterruptCount++;
    g_tick500us++;
    LPTMR_DRV_ClearCompareFlag(INST_LPTMR_1);
}

uint32_t lin2_slave_callback(uint32_t *ns)
{
    static uint32_t previousCountValue = 0UL;
    uint32_t counterValue;

    counterValue = LPTMR_DRV_GetCounterValueByCount(INST_LPTMR_1);
    if (ns != NULL)
    {
        *ns = ((uint32_t)(counterValue +
                (uint32_t)g_timerOverflowInterruptCount * TIMER_COMPARE_VAL -
                previousCountValue)) * 1000UL / TIMER_TICKS_1US;
    }

    g_timerOverflowInterruptCount = 0U;
    previousCountValue = counterValue;
    return 0UL;
}

#if (SLAVE2_ENABLE_AUTOBAUD != 0U)
void RXPIN_IRQHandler(void)
{
    static bool autoBaudComplete = false;

    if ((PINS_DRV_GetPortIntFlag(PORTD) & (1UL << 6)) != 0U)
    {
        if (autoBaudComplete == false)
        {
            if (LIN_DRV_AutoBaudCapture(INST_LIN2) == STATUS_SUCCESS)
                autoBaudComplete = true;
        }
    }

    PINS_DRV_ClearPortIntFlagCmd(PORTD);
}
#endif

static void Slave2_AdcInit(void)
{
#if (SLAVE2_ENABLE_ADC != 0U)
    (void)ADC_DRV_ConfigConverter(ADC_INST, &adc_config_1_ConvConfig0);
    (void)ADC_DRV_AutoCalibration(ADC_INST);
#endif
}

static uint16_t Slave2_AdcReadBlocking(void)
{
#if (SLAVE2_ENABLE_ADC != 0U)
    uint16_t raw = 0U;

    (void)ADC_DRV_ConfigChan(ADC_INST, ADC_GROUP, &adc_config_1_ChnConfig0);
    (void)ADC_DRV_WaitConvDone(ADC_INST);
    (void)ADC_DRV_GetChanResult(ADC_INST, ADC_GROUP, &raw);

    if (raw > 4095U)
        raw = 4095U;

    return raw;
#else
    return 0U;
#endif
}

static void Slave2_LedInit(void)
{
    PINS_DRV_SetPinsDirection(SLAVE2_LED_GPIO_PORT,
                              SLAVE2_LED_RED_MASK | SLAVE2_LED_GREEN_MASK);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_RED_PIN, SLAVE2_LED_OFF);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_GREEN_PIN, SLAVE2_LED_OFF);
}

static Slave2Zone Slave2_ClassifyZone(uint16_t adcValue)
{
    if (adcValue < SLAVE2_ADC_SAFE_MAX)
        return SLAVE2_ZONE_SAFE;

    if (adcValue < SLAVE2_ADC_WARNING_MAX)
        return SLAVE2_ZONE_WARNING;

    if (adcValue < SLAVE2_ADC_EMERGENCY_MIN)
        return SLAVE2_ZONE_DANGER;

    return SLAVE2_ZONE_EMERGENCY;
}

static void Slave2_SetLedSafe(void)
{
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_RED_PIN, SLAVE2_LED_OFF);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_GREEN_PIN, SLAVE2_LED_ON);
}

static void Slave2_SetLedWarning(void)
{
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_RED_PIN, SLAVE2_LED_ON);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_GREEN_PIN, SLAVE2_LED_ON);
}

static void Slave2_SetLedDanger(void)
{
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_RED_PIN, SLAVE2_LED_ON);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_GREEN_PIN, SLAVE2_LED_OFF);
}

static void Slave2_SetLedEmergencyBlink(uint8_t on)
{
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT,
                      SLAVE2_LED_RED_PIN,
                      (on != 0U) ? SLAVE2_LED_ON : SLAVE2_LED_OFF);
    PINS_DRV_WritePin(SLAVE2_LED_GPIO_PORT, SLAVE2_LED_GREEN_PIN, SLAVE2_LED_OFF);
}

static void Slave2_ApplyUnlockedZoneLed(Slave2Zone zone)
{
    switch (zone)
    {
        case SLAVE2_ZONE_SAFE:
            Slave2_SetLedSafe();
            break;

        case SLAVE2_ZONE_WARNING:
            Slave2_SetLedWarning();
            break;

        case SLAVE2_ZONE_DANGER:
            Slave2_SetLedDanger();
            break;

        case SLAVE2_ZONE_EMERGENCY:
        default:
            g_emergencyLatched = 1U;
            g_emergencyBlinkOn = 1U;
            g_lastEmergencyBlinkTick = g_tick500us;
            Slave2_SetLedEmergencyBlink(g_emergencyBlinkOn);
            break;
    }
}

static void Slave2_UpdateLedFromAdc(uint16_t adcValue)
{
    g_currentZone = Slave2_ClassifyZone(adcValue);

    if (g_emergencyLatched != 0U)
        return;

    Slave2_ApplyUnlockedZoneLed(g_currentZone);
}

static void Slave2_RunEmergencyBlinkTask(void)
{
    uint32_t nowTicks;

    if (g_emergencyLatched == 0U)
        return;

    nowTicks = g_tick500us;
    if ((nowTicks - g_lastEmergencyBlinkTick) < EMERGENCY_BLINK_PERIOD_TICKS)
        return;

    g_lastEmergencyBlinkTick = nowTicks;
    g_emergencyBlinkOn = (g_emergencyBlinkOn == 0U) ? 1U : 0U;
    Slave2_SetLedEmergencyBlink(g_emergencyBlinkOn);
}

static void Slave2_RunAdcTask(void)
{
    uint32_t nowTicks;

    nowTicks = g_tick500us;
    if ((nowTicks - g_lastAdcSampleTick) < ADC_SAMPLE_PERIOD_TICKS)
        return;

    g_lastAdcSampleTick = nowTicks;
    g_adcLatestValue = Slave2_AdcReadBlocking();
    Slave2_UpdateLedFromAdc(g_adcLatestValue);
}

static void Slave2_PrepareAdcFrame(void)
{
    uint16_t adcValue;

    adcValue = g_adcLatestValue;
    (void)memset(g_linTxBuffer, 0, sizeof(g_linTxBuffer));
    g_linTxBuffer[0] = (uint8_t)(adcValue & 0xFFU);
    g_linTxBuffer[1] = (uint8_t)((adcValue >> 8U) & 0x0FU);
    g_linTxBuffer[2] = (uint8_t)g_currentZone;
    g_linTxBuffer[3] = g_emergencyLatched;
}

static void Slave2_GoIdle(void)
{
    (void)LIN_DRV_GotoIdleState(INST_LIN2);
    g_linState = LIN_SLAVE_IDLE;
}

static void Slave2_LinCallback(uint32_t instance, void *linStatePtr)
{
    lin_state_t *linState;
    status_t status;

    (void)instance;

    linState = (lin_state_t *)linStatePtr;
    if (linState == NULL)
        return;

    if (linState->timeoutCounterFlag != false)
    {
        linState->timeoutCounterFlag = false;
        Slave2_GoIdle();
        return;
    }

    switch (linState->currentEventId)
    {
        case LIN_PID_OK:
            LIN_DRV_SetTimeoutCounter(INST_LIN2, TIMEOUT);
            if (linState->currentId == PID_SLAVE2_ADC_STATUS)
            {
                Slave2_PrepareAdcFrame();
                status = LIN_DRV_SendFrameData(INST_LIN2,
                                               g_linTxBuffer,
                                               (uint8_t)sizeof(g_linTxBuffer));
                if (status == STATUS_SUCCESS)
                {
                    g_linState = LIN_SLAVE_WAIT_TX;
                }
                else
                {
                    Slave2_GoIdle();
                }
            }
            else if (linState->currentId == PID_SLAVE2_OK_CMD)
            {
                status = LIN_DRV_ReceiveFrameData(INST_LIN2,
                                                  g_linRxBuffer,
                                                  (uint8_t)sizeof(g_linRxBuffer));
                if (status == STATUS_SUCCESS)
                {
                    g_linState = LIN_SLAVE_WAIT_RX;
                }
                else
                {
                    Slave2_GoIdle();
                }
            }
            else
            {
                Slave2_GoIdle();
            }
            break;

        case LIN_TX_COMPLETED:
            Slave2_GoIdle();
            break;

        case LIN_RX_COMPLETED:
            if ((linState->currentId == PID_SLAVE2_OK_CMD) &&
                (g_linRxBuffer[0] == SLAVE2_OK_TOKEN) &&
                (g_emergencyLatched != 0U) &&
                (g_currentZone != SLAVE2_ZONE_EMERGENCY))
            {
                g_emergencyLatched = 0U;
                g_emergencyBlinkOn = 0U;
                Slave2_ApplyUnlockedZoneLed(g_currentZone);
            }

            Slave2_GoIdle();
            break;

        case LIN_PID_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RX_OVERRUN:
            Slave2_GoIdle();
            break;

        case LIN_RECV_BREAK_FIELD_OK:
            LIN_DRV_SetTimeoutCounter(INST_LIN2, TIMEOUT);
            break;

        default:
            break;
    }
}

static void Slave2_LinFsmStep(void)
{
    /* LIN response is handled directly inside the callback for tighter timing. */
}

int main(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr,
                   CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr,
                   CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    PINS_DRV_SetPinsDirection(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);
    PINS_DRV_SetPins(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);
    Slave2_LedInit();

#if (SLAVE2_ENABLE_AUTOBAUD != 0U)
    PINS_DRV_SetPinIntSel(PORTD, 6UL, PORT_INT_EITHER_EDGE);
    INT_SYS_InstallHandler(PORTD_IRQn, RXPIN_IRQHandler, NULL);
    INT_SYS_EnableIRQ(PORTD_IRQn);
#endif

    LPTMR_DRV_Init(INST_LPTMR_1, &lptmr_1_config0, false);
    INT_SYS_InstallHandler(LPTMR0_IRQn, LPTMR_ISR, NULL);
    INT_SYS_EnableIRQ(LPTMR0_IRQn);
    LPTMR_DRV_StartCounter(INST_LPTMR_1);

    Slave2_AdcInit();
    g_adcLatestValue = Slave2_AdcReadBlocking();
    Slave2_UpdateLedFromAdc(g_adcLatestValue);

    lin2_InitConfig0.autobaudEnable = (SLAVE2_ENABLE_AUTOBAUD != 0U) ? true : false;
    (void)LIN_DRV_Init(INST_LIN2, &lin2_InitConfig0, &lin2_State);
    (void)LIN_DRV_InstallCallback(INST_LIN2, Slave2_LinCallback);

    for (;;)
    {
        Slave2_LinFsmStep();
        Slave2_RunAdcTask();
        Slave2_RunEmergencyBlinkTask();
    }
}
