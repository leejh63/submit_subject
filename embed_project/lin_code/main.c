// main.c
#include "sdk_project_config.h"

// ===================================
#include "app_types.h"
#include "uart_io.h"
#include "ui_model.h"
#include "ui_term.h"
#include "app_time.h"
#include "cli.h"
#include "lin_engine.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
// ===================================

volatile int exit_code = 0;

// ============================================================
// UI-term compatibility mirrors (ui_term.c externs)
// ============================================================

static ui_model_t g_ui;

// ============================================================
// EVB LED (active-low)
// ============================================================
#define PORT_LED0_INDEX           (0u)    // PTD0 (Blue)  active-low
#define PORT_LED1_INDEX           (15u)   // PTD15 (Red)   active-low
#define PORT_LED2_INDEX           (16u)   // PTD16 (Green) active-low
#define LED_GPIO_PORT             (PTD)

// ============================================================
// Buttons: PTC12 / PTC13
// ============================================================
#define PORT_BTN_OPEN_INDEX       (12u)   // PTC12
#define PORT_BTN_CLOSE_INDEX      (13u)   // PTC13
#define BTN_PORT_NAME             (PORTC)
#define BTN_PORT_IRQn             (PORTC_IRQn)

// ============================================================
// LIN XCVR enable
// ============================================================
#define USE_LIN_XCVR              (1U)
#define LIN_XCVR_ENABLE_PIN       (9UL)
#define LIN_XCVR_ENABLE_MASK      (0x1u << LIN_XCVR_ENABLE_PIN)
#define LIN_XCVR_ENABLE_GPIO_PORT (PTE)

// ============================================================
// UI refresh
// ============================================================
#define UI_REFRESH_TICKS          (200u) // 100ms (200 * 0.5ms)

// ============================================================
// LED helpers (active-low)
// ============================================================
static void led_all_off(void)
{
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED0_INDEX, 1U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED1_INDEX, 1U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED2_INDEX, 1U);
}

static void led_blue(void)
{
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED0_INDEX, 0U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED1_INDEX, 1U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED2_INDEX, 1U);
}

static void led_red(void)
{
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED0_INDEX, 1U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED1_INDEX, 0U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED2_INDEX, 1U);
}

static void led_yellow(void)
{
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED0_INDEX, 1U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED1_INDEX, 0U);
    PINS_DRV_WritePin(LED_GPIO_PORT, PORT_LED2_INDEX, 0U);
}

static void apply_led_by_range(uint8_t range)
{
    switch (range)
    {
        case RANGE_OPEN:  led_blue();    break;
        case RANGE_STUCK: led_yellow();  break;
        case RANGE_CLOSE: led_red();     break;
        default:          led_all_off(); break;
    }
}

// ============================================================
// Button ISR
// ============================================================
static volatile bool g_btnOpen  = false;
static volatile bool g_btnClose = false;

void BTNPORT_IRQHandler(void)
{
    uint32_t flags = PINS_DRV_GetPortIntFlag(BTN_PORT_NAME);

    if (flags & (1UL << PORT_BTN_OPEN_INDEX))
    {
        g_btnOpen = true;
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_NAME, PORT_BTN_OPEN_INDEX);
    }

    if (flags & (1UL << PORT_BTN_CLOSE_INDEX))
    {
        g_btnClose = true;
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT_NAME, PORT_BTN_CLOSE_INDEX);
    }
}


// ============================================================
// LIN done callback
// ============================================================
static void on_lin_done(const lin_req_t *req, bool ok)
{
    uint32_t now = app_time_now_tick500us();

    if (!ok)
    {
        g_ui.last_fail_tick = now;
        ui_model_set_msg(&g_ui, "LIN FAIL");
        led_yellow();
        return;
    }

    g_ui.last_ok_tick = now;

    if (req->type == REQ_TYPE_STATUS)
    {
        const uint8_t *st = lin_engine_status_buf();

        g_ui.pos   = st[0];
        g_ui.range = st[1];
        g_ui.err   = st[2];
        g_ui.ctrl  = st[3];
        g_ui.slave_valid = true;

        apply_led_by_range(g_ui.range);
        ui_model_set_msg(&g_ui, "STATUS updated");
        return;
    }

    if (req->type == REQ_TYPE_CMD)
    {
        lin_engine_request_status(REQ_SRC_UART);
        ui_model_set_msg(&g_ui, "CMD ok -> sync status");
        return;
    }
}
// ============================================================
// Master task
// ============================================================
static void lin_master_task(void)
{
    uart_io_init();

    // LIN init
    (void)LIN_DRV_Init(INST_LIN2, &lin2_InitConfig0, &lin2_State);

    uint32_t now0 = app_time_now_tick500us();
    lin_engine_init(on_lin_done, now0);
    lin_engine_install_callback();

    led_all_off();

    // UI init
    cli_init();
    ui_init();
    ui_model_init(&g_ui);
    ui_model_set_msg(&g_ui, "type: open/close/stuck/status/help");
    ui_render(&g_ui, cli_get_cmd_len());

    
    uint32_t lastBtnOpenTick  = 0U;
    uint32_t lastBtnCloseTick = 0U;
    const uint32_t debounceTicks = 40U; // 20ms

    uint32_t lastUi = app_time_now_tick500us();

    for (;;)
    {
        // UART progress
        uart_io_poll();

        // time first
        uint32_t now = app_time_now_tick500us();

        // engine
        lin_engine_step(now);
        lin_engine_poll(now);

        // model update
        g_ui.tick500us = now;
        g_ui.eng_state  = lin_engine_state();
        g_ui.active_req = lin_engine_active_req();
        g_ui.last_event = lin_engine_last_event();
        g_ui.err_code   = lin_engine_err_code();

        // CLI parse
        cli_req_t req = {0};
        uint8_t b;

        while (uart_read_byte(&b))
        {
            cli_req_t one = {0};
            cli_feed_byte(b, &one);

            req.help   |= one.help;
            req.status |= one.status;
            req.open   |= one.open;
            req.close  |= one.close;
            req.stuck  |= one.stuck;
        }

        // handle commands
        if (req.help)
            ui_model_set_msg(&g_ui, "cmd: open(o) close(c) stuck(st) status(s) help(h)");

        if (req.open)
        {
            ui_model_set_msg(&g_ui, "CMD: OPEN");
            lin_engine_request_cmd(REQ_SRC_UART, MOTOR_TARGET_LEFT);
        }
        if (req.close)
        {
            ui_model_set_msg(&g_ui, "CMD: CLOSE");
            lin_engine_request_cmd(REQ_SRC_UART, MOTOR_TARGET_RIGHT);
        }
        if (req.stuck)
        {
            ui_model_set_msg(&g_ui, "CMD: STUCK");
            lin_engine_request_cmd(REQ_SRC_UART, MOTOR_TARGET_STUCK);
        }
        if (req.status)
        {
            ui_model_set_msg(&g_ui, "STATUS: requested");
            lin_engine_request_status(REQ_SRC_UART);
        }

        // Buttons -> requests (with debounce)
        if (g_btnOpen)
        {
            g_btnOpen = false;
            if ((uint32_t)(now - lastBtnOpenTick) >= debounceTicks)
            {
                lastBtnOpenTick = now;
                ui_model_set_msg(&g_ui, "BTN: OPEN");
                lin_engine_request_cmd(REQ_SRC_BTN, MOTOR_TARGET_LEFT);
            }
        }

        if (g_btnClose)
        {
            g_btnClose = false;
            if ((uint32_t)(now - lastBtnCloseTick) >= debounceTicks)
            {
                lastBtnCloseTick = now;
                ui_model_set_msg(&g_ui, "BTN: CLOSE");
                lin_engine_request_cmd(REQ_SRC_BTN, MOTOR_TARGET_RIGHT);
            }
        }

        // UI refresh periodic (ONLY HERE)
        if ((uint32_t)(now - lastUi) >= UI_REFRESH_TICKS)
        {
            lastUi = now;
            ui_render(&g_ui, cli_get_cmd_len());
        }

        if (exit_code != 0) break;
    }

    term_show_cursor();
}

// ============================================================
// main
// ============================================================
int main(void)
{
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                   g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

#if USE_LIN_XCVR
    PINS_DRV_SetPinsDirection(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);
    PINS_DRV_SetPins(LIN_XCVR_ENABLE_GPIO_PORT, LIN_XCVR_ENABLE_MASK);
#endif

    // Button IRQ (PORTC)
    INT_SYS_InstallHandler(BTN_PORT_IRQn, BTNPORT_IRQHandler, (isr_t *)NULL);
    INT_SYS_EnableIRQ(BTN_PORT_IRQn);

    // LPTMR
    LPTMR_DRV_Init(INST_LPTMR_1, &lptmr_1_config0, false);
    INT_SYS_InstallHandler(LPTMR0_IRQn, LPTMR_ISR, (isr_t *)NULL);
    INT_SYS_EnableIRQ(LPTMR0_IRQn);
    LPTMR_DRV_StartCounter(INST_LPTMR_1);

    lin_master_task();

    for (;;)
    {
        if (exit_code != 0) break;
    }
    return exit_code;
}
