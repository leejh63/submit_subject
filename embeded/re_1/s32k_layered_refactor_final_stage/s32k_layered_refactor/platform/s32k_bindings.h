#ifndef S32K_BINDINGS_H
#define S32K_BINDINGS_H

#include "sdk_project_config.h"

// ==============================
// LIN sensor slave project binds
// ==============================
#define S32K_LIN_SENSOR_LIN_INSTANCE         INST_LIN2
#define S32K_LIN_SENSOR_LPTMR_INSTANCE       INST_LPTMR_1
#define S32K_LIN_SENSOR_LPTMR_IRQn           LPTMR0_IRQn
#define S32K_LIN_SENSOR_LPTMR_CONFIG         lptmr_1_config0
#define S32K_LIN_SENSOR_LIN_INIT_CONFIG      lin2_InitConfig0
#define S32K_LIN_SENSOR_LIN_STATE            lin2_State
#define S32K_LIN_SENSOR_PIN_CONFIG_COUNT     NUM_OF_CONFIGURED_PINS0
#define S32K_LIN_SENSOR_PIN_CONFIG_ARRAY     g_pin_mux_InitConfigArr0
#define S32K_LIN_SENSOR_CLOCK_CONFIGS        g_clockManConfigsArr
#define S32K_LIN_SENSOR_CLOCK_CONFIG_COUNT   CLOCK_MANAGER_CONFIG_CNT
#define S32K_LIN_SENSOR_CLOCK_CALLBACKS      g_clockManCallbacksArr
#define S32K_LIN_SENSOR_CLOCK_CALLBACK_COUNT CLOCK_MANAGER_CALLBACK_CNT

/*
 * tick/LIN timeout service 는 각 노드 프로젝트에서 별도로 바꿔 끼운다.
 * 기본값은 LIN sensor slave 기준으로 맞춰 둔다.
 */
#define S32K_TICK_LPTMR_INSTANCE            S32K_LIN_SENSOR_LPTMR_INSTANCE
#define S32K_TICK_TIMEOUT_SERVICE_INSTANCE  S32K_LIN_SENSOR_LIN_INSTANCE


#define S32K_LIN_SENSOR_ADC_ENABLE           1U
#if (S32K_LIN_SENSOR_ADC_ENABLE != 0U)
#include "adc_driver.h"
#include "peripherals_adc_config_1.h"
#define S32K_LIN_SENSOR_ADC_INSTANCE         INST_ADC_CONFIG_1
#define S32K_LIN_SENSOR_ADC_GROUP            0U
#define S32K_LIN_SENSOR_ADC_CONV_CONFIG      adc_config_1_ConvConfig0
#define S32K_LIN_SENSOR_ADC_CH_CONFIG        adc_config_1_ChnConfig0

// =========================
// Master/Button node binds
// =========================
// 아래 generated 이름은 실제 프로젝트 값으로 교체하면 된다.
// mailbox 번호도 여기서 함께 고정하는 방식이 가장 관리하기 쉽다.
//
// 예시:
// #define S32K_MASTER_CAN_INSTANCE         INST_CANCOM1
// #define S32K_MASTER_CAN_STATE            flexcanState0
// #define S32K_MASTER_CAN_INIT_CONFIG      flexcanInitConfig0
// #define S32K_MASTER_CAN_TX_MB            1U
// #define S32K_MASTER_CAN_RX_MB            0U
// #define S32K_MASTER_UART_INSTANCE        INST_LPUART_1
// #define S32K_MASTER_UART_STATE           lpUartState1
// #define S32K_MASTER_UART_INIT_CONFIG     lpuart_1_InitConfig0

// #define S32K_MASTER_LIN_INSTANCE         INST_LIN1
// #define S32K_MASTER_LIN_STATE            lin1_State
// #define S32K_MASTER_LIN_INIT_CONFIG      lin1_InitConfig0
// #define S32K_MASTER_LIN_STATUS_PID          0x24U
// #define S32K_MASTER_LIN_OK_CMD_PID          0x25U
// #define S32K_MASTER_LIN_POLL_PERIOD_MS      50U
// #define S32K_MASTER_LIN_STATUS_STALE_MS     150U
// #define S32K_MASTER_LIN_ACK_RETRY_PERIOD_MS 80U
// #define S32K_MASTER_LIN_ACK_MAX_RETRY       3U
//
// #define S32K_BUTTON_CAN_INSTANCE         INST_CANCOM1
// #define S32K_BUTTON_CAN_STATE            flexcanState0
// #define S32K_BUTTON_CAN_INIT_CONFIG      flexcanInitConfig0
// #define S32K_BUTTON_CAN_TX_MB            1U
// #define S32K_BUTTON_CAN_RX_MB            0U
//
// #define S32K_BUTTON_UART_INSTANCE        INST_LPUART_0
// #define S32K_BUTTON_UART_STATE           lpUartState0
// #define S32K_BUTTON_UART_INIT_CONFIG     lpuart_0_InitConfig0


// #define S32K_MASTER_CLOCK_CONFIGS          g_clockManConfigsArr
// #define S32K_MASTER_CLOCK_CONFIG_COUNT      CLOCK_MANAGER_CONFIG_CNT
// #define S32K_MASTER_CLOCK_CALLBACKS        g_clockManCallbacksArr
// #define S32K_MASTER_CLOCK_CALLBACK_COUNT   CLOCK_MANAGER_CALLBACK_CNT
// #define S32K_MASTER_PIN_CONFIG_COUNT       NUM_OF_CONFIGURED_PINS0
// #define S32K_MASTER_PIN_CONFIG_ARRAY       g_pin_mux_InitConfigArr0
// #define S32K_MASTER_LPTMR_INSTANCE         INST_LPTMR_0
// #define S32K_MASTER_LPTMR_IRQn             LPTMR0_IRQn
// #define S32K_MASTER_LPTMR_CONFIG           lptmr_0_config0

// #define S32K_BUTTON_CLOCK_CONFIGS          g_clockManConfigsArr
// #define S32K_BUTTON_CLOCK_CONFIG_COUNT      CLOCK_MANAGER_CONFIG_CNT
// #define S32K_BUTTON_CLOCK_CALLBACKS        g_clockManCallbacksArr
// #define S32K_BUTTON_CLOCK_CALLBACK_COUNT   CLOCK_MANAGER_CALLBACK_CNT
// #define S32K_BUTTON_PIN_CONFIG_COUNT       NUM_OF_CONFIGURED_PINS0
// #define S32K_BUTTON_PIN_CONFIG_ARRAY       g_pin_mux_InitConfigArr0
// #define S32K_BUTTON_LPTMR_INSTANCE         INST_LPTMR_0
// #define S32K_BUTTON_LPTMR_IRQn             LPTMR0_IRQn
// #define S32K_BUTTON_LPTMR_CONFIG           lptmr_0_config0

#endif
