// ui_term.c
#include "ui_term.h"
#include "uart_io.h"

#include <stdio.h>
#include <string.h>

#define ANSI_ESC "\x1b"

static void term_clear_screen(void) { uart_write_str(ANSI_ESC "[2J"); }
static void term_cursor_home(void)  { uart_write_str(ANSI_ESC "[H");  }
static void term_clear_line(void)   { uart_write_str(ANSI_ESC "[2K"); }
static void term_hide_cursor(void)  { uart_write_str(ANSI_ESC "[?25l"); }
void term_show_cursor(void)         { uart_write_str(ANSI_ESC "[?25h"); }

static void term_cursor_pos(uint8_t r, uint8_t c)
{
    char b[24];
    (void)snprintf(b, sizeof(b), ANSI_ESC "[%u;%uH", (unsigned)r, (unsigned)c);
    uart_write_str(b);
}

// --- UI row layout (1-indexed) ---
#define UI_ROW_TITLE_INPUT   (1u)
#define UI_ROW_INPUT_LINE    (2u)
#define UI_ROW_BLANK_1       (3u)

#define UI_ROW_TITLE_LIN     (4u)
#define UI_ROW_LIN_1         (5u)
#define UI_ROW_LIN_2         (6u)
#define UI_ROW_LIN_3         (7u)
#define UI_ROW_BLANK_2       (8u)

#define UI_ROW_TITLE_SLAVE   (9u)
#define UI_ROW_SLAVE_1       (10u)
#define UI_ROW_SLAVE_2       (11u)
#define UI_ROW_SLAVE_3       (12u)

#define UI_INPUT_COL_PROMPT  (1u)
#define UI_INPUT_COL_TEXT    (3u)   // after "> "

static const char *range_str(uint8_t r)
{
    switch (r)
    {
        case RANGE_OPEN:  return "OPEN";
        case RANGE_STUCK: return "STUCK";
        case RANGE_CLOSE: return "CLOSE";
        default:          return "UNK";
    }
}

static const char *control_str(uint8_t ctrl)
{
    switch (ctrl)
    {
        case CTRL_BTN: return "BTN";
        case CTRL_CMD: return "CMD";
        case CTRL_POT: return "POT";
        default:       return "UNK";
    }
}

static const char *lin_err_str(uint8_t e)
{
    switch (e)
    {
        case 0: return "NONE";
        case 1: return "PID";
        case 2: return "CHECKSUM";
        case 3: return "READBACK";
        case 4: return "FRAME";
        case 5: return "TIMEOUT";
        default:return "UNK";
    }
}

void ui_init(void)
{
    term_clear_screen();
    term_cursor_home();
    term_hide_cursor();

    uart_write_str("[1] UART INPUT\r\n");
    uart_write_str("> \r\n");
    uart_write_str("\r\n");

    uart_write_str("[2] LIN STATE\r\n");
    uart_write_str("comm : \r\n");
    uart_write_str("state: \r\n");
    uart_write_str("req  : \r\n");
    uart_write_str("\r\n");

    uart_write_str("[3] SLAVE STATUS\r\n");
    uart_write_str("pos  : \r\n");
    uart_write_str("range: \r\n");
    uart_write_str("ctrl : \r\n");

    term_cursor_pos(UI_ROW_INPUT_LINE, UI_INPUT_COL_TEXT);
}

void ui_clear_input_line(void)
{
    term_cursor_pos(UI_ROW_INPUT_LINE, 1);
    term_clear_line();
    uart_write_str("> ");
    term_cursor_pos(UI_ROW_INPUT_LINE, UI_INPUT_COL_TEXT);
}

void ui_render(const ui_model_t *m, uint32_t cmd_len)
{
    char line[120];

    // [2] LIN STATE
    term_cursor_pos(UI_ROW_LIN_1, 1); term_clear_line();
    {
        const char *comm = (m->err_code == 0) ? "OK" : "ERR";
        (void)snprintf(line, sizeof(line),
                       "comm : %s   lastErr=%s   lastEvent=%u   msg=%s",
                       comm, lin_err_str(m->err_code),
                       (unsigned)m->last_event, m->msg);
        uart_write_str(line);
    }

    term_cursor_pos(UI_ROW_LIN_2, 1); term_clear_line();
    {
        const char *st =
            (m->eng_state == ENG_IDLE)     ? "IDLE" :
            (m->eng_state == ENG_WAIT_PID) ? "WAIT_PID" :
            (m->eng_state == ENG_WAIT_TX)  ? "WAIT_TX" : "WAIT_RX";

        (void)snprintf(line, sizeof(line),
                       "state: %s   tick=%lu",
                       st, (unsigned long)m->tick500us);
        uart_write_str(line);
    }

    term_cursor_pos(UI_ROW_LIN_3, 1); term_clear_line();
    {
        if (m->eng_state != ENG_IDLE)
        {
            const char *t = (m->active_req.type == REQ_TYPE_CMD) ? "CMD" :
                            (m->active_req.type == REQ_TYPE_STATUS) ? "STATUS" : "UNK";
            (void)snprintf(line, sizeof(line),
                           "req  : %s   retry=%u/%u   deadline=%lu",
                           t,
                           (unsigned)m->active_req.retryCount,
                           (unsigned)m->active_req.maxRetry,
                           (unsigned long)m->active_req.deadlineTick);
        }
        else
        {
            uint32_t now = m->tick500us;
            uint32_t ok_age_ms = 0U;
            uint32_t fail_age_ms = 0U;

            if (m->last_ok_tick != 0U)   ok_age_ms   = (uint32_t)((now - m->last_ok_tick) / 2U);
            if (m->last_fail_tick != 0U) fail_age_ms = (uint32_t)((now - m->last_fail_tick) / 2U);

            (void)snprintf(line, sizeof(line),
                           "req  : (none)   lastOK=%lums ago   lastFAIL=%lums ago",
                           (unsigned long)ok_age_ms,
                           (unsigned long)fail_age_ms);
        }
        uart_write_str(line);
    }

    // [3] SLAVE STATUS
    term_cursor_pos(UI_ROW_SLAVE_1, 1); term_clear_line();
    if (m->slave_valid)
        (void)snprintf(line, sizeof(line), "pos  : %3u %%", (unsigned)m->pos);
    else
        (void)snprintf(line, sizeof(line), "pos  : (no data)");
    uart_write_str(line);

    term_cursor_pos(UI_ROW_SLAVE_2, 1); term_clear_line();
    if (m->slave_valid)
        (void)snprintf(line, sizeof(line), "range: %s", range_str(m->range));
    else
        (void)snprintf(line, sizeof(line), "range: (no data)");
    uart_write_str(line);

    term_cursor_pos(UI_ROW_SLAVE_3, 1); term_clear_line();
    if (m->slave_valid)
        (void)snprintf(line, sizeof(line), "ctrl : %s   err=0x%02X",
                       control_str(m->ctrl), (unsigned)m->err);
    else
        (void)snprintf(line, sizeof(line), "ctrl : (no data)");
    uart_write_str(line);

    // restore cursor to input
    term_cursor_pos(UI_ROW_INPUT_LINE, (uint8_t)(UI_INPUT_COL_TEXT + cmd_len));
}
