// cli.c
#include "cli.h"
#include "uart_io.h"
#include "ui_term.h"

#include <string.h>

// ============================================================
// CLI buffer
// ============================================================
#define CMD_BUF_SIZE (32u)

static char     s_cmd_buf[CMD_BUF_SIZE];
static uint32_t s_cmd_len = 0u;

uint32_t cli_get_cmd_len(void) { return s_cmd_len; }

void cli_reset(void)
{
    s_cmd_len = 0u;
    s_cmd_buf[0] = '\0';
}

void cli_init(void)
{
    cli_reset();
}

static void normalize_and_append(char ch)
{
    // normalize case
    if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');

    // printable only
    if (ch < 0x20 || ch > 0x7E) return;

    if (s_cmd_len < (CMD_BUF_SIZE - 1u))
    {
        s_cmd_buf[s_cmd_len++] = ch;
        s_cmd_buf[s_cmd_len] = '\0';
        uart_write_char((char)ch);
    }
}

void cli_feed_byte(uint8_t b, cli_req_t *out)
{
    if (!out) return;

    // init outputs (caller can also memset)
    out->help = out->status = out->open = out->close = out->stuck = 0u;

    char ch = (char)b;

    // backspace/delete
    if (ch == '\b' || ch == 0x7F)
    {
        if (s_cmd_len > 0u)
        {
            s_cmd_len--;
            s_cmd_buf[s_cmd_len] = '\0';
            uart_write_str("\b \b");
        }
        return;
    }

    // endline => commit
    if (ch == '\r' || ch == '\n')
    {
        s_cmd_buf[s_cmd_len] = '\0';

        if (strcmp(s_cmd_buf, "help") == 0 || strcmp(s_cmd_buf, "h") == 0)
            out->help = 1u;
        else if (strcmp(s_cmd_buf, "status") == 0 || strcmp(s_cmd_buf, "s") == 0)
            out->status = 1u;
        else if (strcmp(s_cmd_buf, "open") == 0 || strcmp(s_cmd_buf, "o") == 0)
            out->open = 1u;
        else if (strcmp(s_cmd_buf, "close") == 0 || strcmp(s_cmd_buf, "c") == 0)
            out->close = 1u;
        else if (strcmp(s_cmd_buf, "stuck") == 0 || strcmp(s_cmd_buf, "st") == 0)
            out->stuck = 1u;
        else if (s_cmd_buf[0] != '\0')
        {
            // UI 메시지 set은 main이 소유(현재 구조)라면 여기서 직접 못 부른다.
            // 그래서 "unknown" 처리는 main에서 하거나, 콜백으로 빼야 한다.
            // -> 지금은 out에 아무것도 안 세팅해서 main이 처리하도록 두자.
        }

        cli_reset();
        ui_clear_input_line();
        return;
    }

    normalize_and_append(ch);
}
