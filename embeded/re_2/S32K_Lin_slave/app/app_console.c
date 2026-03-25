/*
 * UART 콘솔 구현부다.
 * 텍스트 UI를 렌더링하고 사용자 명령을 파싱하며,
 * AppCore가 CAN 작업으로 바꿀 때까지 operator 동작을 버퍼링한다.
 */
#include "app_console.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define APP_CONSOLE_ROW_TASK_LINE1      4U
#define APP_CONSOLE_ROW_TASK_LINE2      5U
#define APP_CONSOLE_ROW_TASK_LINE3      6U
#define APP_CONSOLE_ROW_TASK_LINE4      7U
#define APP_CONSOLE_ROW_VALUE_LINE1    10U
#define APP_CONSOLE_ROW_VALUE_LINE2    11U
#define APP_CONSOLE_ROW_VALUE_LINE3    12U
#define APP_CONSOLE_ROW_SOURCE_LINE1   15U
#define APP_CONSOLE_ROW_SOURCE_LINE2   16U
#define APP_CONSOLE_ROW_RESULT         19U
#define APP_CONSOLE_ROW_INPUT          22U
#define APP_CONSOLE_RENDER_BUFFER_SIZE 1024U

/*
 * 콘솔 메시지 영역에 표시되는 기본 help 문자열이다.
 * startup 힌트로도 쓰이고,
 * operator가 직접 입력한 `help` 명령의 응답으로도 사용된다.
 */
static const char g_app_console_help_text[] =
    "cmd: help hello ping status ok "
    "open <id|all> close <id|all> off <id|all> test <id|all> "
    "text <id|all> <msg> event <id|all> <eventCode> <arg0> <arg1>";

/*
 * 콘솔 전체 view를 dirty 상태로 표시한다.
 * 초기화나 recover 이후에는 full refresh를 사용해,
 * 다음 렌더 시 전체 terminal layout을 다시 그리게 한다.
 */
static void AppConsole_RequestFullRefresh(AppConsole *console);
static void AppConsole_RequestInputRefresh(AppConsole *console);
static void AppConsole_RequestTaskRefresh(AppConsole *console);
static void AppConsole_RequestSourceRefresh(AppConsole *console);
static void AppConsole_RequestResultRefresh(AppConsole *console);
static void AppConsole_RequestValueRefresh(AppConsole *console);
/*
 * cached view 문자열이 실제로 바뀌었을 때만 갱신한다.
 * 불필요한 UART 트래픽을 줄이고,
 * dirty flag가 의미 있는 내용 변경과 맞물리게 유지한다.
 */
static void AppConsole_SetViewText(char *dst, uint16_t dst_size, const char *src, uint8_t *out_changed);
static void AppConsole_UpdateInputView(AppConsole *console);
/*
 * 고정 terminal layout을 한 번 그린다.
 * section header와 cursor 위치 지정은 여기서 출력하고,
 * 이후 렌더는 바뀐 내용 줄만 갱신하면 되게 만든다.
 */
static void AppConsole_RenderLayout(AppConsole *console);
/*
 * 내용이 바뀐 콘솔 줄만 출력한다.
 * 매 렌더 주기마다 전체 terminal을 다시 그리지 않아도 되어,
 * UART UI가 더 responsive하게 동작한다.
 */
static void AppConsole_RenderDirtyLines(AppConsole *console);
static const char *AppConsole_GetUartErrorText(UartErrorCode error_code);
static uint8_t AppConsole_IsSpaceChar(char ch);
/*
 * 콘솔 입력에서 작은 unsigned integer를 파싱한다.
 * parser를 의도적으로 엄격하게 두어,
 * 잘못된 명령 인자가 CAN 계층까지 내려가기 전에 걸러지게 한다.
 */
static uint8_t AppConsole_ParseU8(const char *text, uint8_t *out_value);
static uint8_t AppConsole_ParseTarget(const char *text, uint8_t *out_node_id, uint8_t *out_is_broadcast);
static uint8_t AppConsole_Tokenize(char *buffer, char *argv[], uint8_t argv_capacity, uint8_t *out_argc);
/*
 * 완성된 콘솔 입력 한 줄을 파싱한다.
 * 일부 명령은 로컬에서 처리하고,
 * 나머지는 app 계층이 보낼 CAN 요청으로 queue에 넣는다.
 */
static void AppConsole_HandleLine(AppConsole *console, const char *line);
static void AppConsole_ExtractTextLine(const char *src,
                                       uint8_t line_index,
                                       char *out_buffer,
                                       uint16_t out_buffer_size);

static const char *AppConsole_GetUartErrorText(UartErrorCode error_code)
{
    switch (error_code)
    {
        case UART_ERROR_HW_INIT:
            return "[error] uart hw init failed";
        case UART_ERROR_RX_DRIVER:
            return "[error] uart rx driver failed";
        case UART_ERROR_RX_PENDING_OVERFLOW:
            return "[error] uart rx pending overflow";
        case UART_ERROR_RX_LINE_OVERFLOW:
            return "[error] uart rx line overflow";
        case UART_ERROR_TX_QUEUE_FULL:
            return "[error] uart tx queue full";
        case UART_ERROR_TX_DRIVER:
            return "[error] uart tx driver failed";
        case UART_ERROR_TX_TIMEOUT:
            return "[error] uart tx timeout";
        case UART_ERROR_NONE:
        default:
            return "[error] uart failure";
    }
}

static void AppConsole_RequestFullRefresh(AppConsole *console)
{
    if (console == NULL)
    {
        return;
    }

    console->view.full_refresh_required = 1U;
    console->view.input_dirty = 1U;
    console->view.task_dirty = 1U;
    console->view.source_dirty = 1U;
    console->view.result_dirty = 1U;
    console->view.value_dirty = 1U;
}

static void AppConsole_RequestInputRefresh(AppConsole *console)
{
    if (console != NULL)
    {
        console->view.input_dirty = 1U;
    }
}

static void AppConsole_RequestTaskRefresh(AppConsole *console)
{
    if (console != NULL)
    {
        console->view.task_dirty = 1U;
    }
}

static void AppConsole_RequestSourceRefresh(AppConsole *console)
{
    if (console != NULL)
    {
        console->view.source_dirty = 1U;
    }
}

static void AppConsole_RequestResultRefresh(AppConsole *console)
{
    if (console != NULL)
    {
        console->view.result_dirty = 1U;
    }
}

static void AppConsole_RequestValueRefresh(AppConsole *console)
{
    if (console != NULL)
    {
        console->view.value_dirty = 1U;
    }
}

static void AppConsole_SetViewText(char *dst, uint16_t dst_size, const char *src, uint8_t *out_changed)
{
    char temp[APP_CONSOLE_RENDER_BUFFER_SIZE];

    if ((dst == NULL) || (src == NULL) || (out_changed == NULL) || (dst_size == 0U))
    {
        return;
    }

    (void)snprintf(temp, sizeof(temp), "%s", src);
    if (strncmp(dst, temp, dst_size) == 0)
    {
        *out_changed = 0U;
        return;
    }

    (void)snprintf(dst, dst_size, "%s", temp);
    *out_changed = 1U;
}

void AppConsole_SetResultText(AppConsole *console, const char *text)
{
    uint8_t changed;

    if ((console == NULL) || (text == NULL))
    {
        return;
    }

    AppConsole_SetViewText(console->view.result_text,
                           (uint16_t)sizeof(console->view.result_text),
                           text,
                           &changed);
    if (changed != 0U)
    {
        AppConsole_RequestResultRefresh(console);
    }
}

void AppConsole_SetTaskText(AppConsole *console, const char *text)
{
    uint8_t changed;

    if ((console == NULL) || (text == NULL))
    {
        return;
    }

    AppConsole_SetViewText(console->view.task_text,
                           (uint16_t)sizeof(console->view.task_text),
                           text,
                           &changed);
    if (changed != 0U)
    {
        AppConsole_RequestTaskRefresh(console);
    }
}

void AppConsole_SetSourceText(AppConsole *console, const char *text)
{
    uint8_t changed;

    if ((console == NULL) || (text == NULL))
    {
        return;
    }

    AppConsole_SetViewText(console->view.source_text,
                           (uint16_t)sizeof(console->view.source_text),
                           text,
                           &changed);
    if (changed != 0U)
    {
        AppConsole_RequestSourceRefresh(console);
    }
}

void AppConsole_SetValueText(AppConsole *console, const char *text)
{
    uint8_t changed;

    if ((console == NULL) || (text == NULL))
    {
        return;
    }

    AppConsole_SetViewText(console->view.value_text,
                           (uint16_t)sizeof(console->view.value_text),
                           text,
                           &changed);
    if (changed != 0U)
    {
        AppConsole_RequestValueRefresh(console);
    }
}

static void AppConsole_UpdateInputView(AppConsole *console)
{
    char     input_text[APP_CONSOLE_INPUT_VIEW_SIZE];
    uint8_t  changed;

    if (console == NULL)
    {
        return;
    }

    if (UartService_GetCurrentInputText(&console->uart,
                                        input_text,
                                        (uint16_t)sizeof(input_text)) != INFRA_STATUS_OK)
    {
        return;
    }

    AppConsole_SetViewText(console->view.input_text,
                           (uint16_t)sizeof(console->view.input_text),
                           input_text,
                           &changed);
    if (changed != 0U)
    {
        AppConsole_RequestInputRefresh(console);
    }
}

static void AppConsole_RenderLayout(AppConsole *console)
{
    char        render_buffer[APP_CONSOLE_RENDER_BUFFER_SIZE];
    const char *title_text;

    if (console == NULL)
    {
        return;
    }

    title_text = (console->node_id == APP_NODE_ID_MASTER) ? "MASTER NODE" : "FIELD NODE";
    (void)snprintf(render_buffer,
                   sizeof(render_buffer),
                   "\033[2J\033[H"
                   "[ %s ]\r\n"
                   "\r\n"
                   "[Connection Status]\r\n"
                   "\r\n"
                   "\r\n"
                   "\r\n"
                   "\r\n"
                   "[Status]\r\n"
                   "\r\n"
                   "\r\n"
                   "\r\n"
                   "\r\n"
                   "[Input]\r\n"
                   "\r\n"
                   "\r\n"
                   "\r\n"
                   "[Message]\r\n"
                   "\r\n"
                   "\r\n"
                   "[Command]\r\n"
                   "\r\n"
                   "\033[J",
                   title_text);

    if (UartService_RequestTx(&console->uart, render_buffer) == INFRA_STATUS_OK)
    {
        console->view.layout_drawn = 1U;
    }
}

static void AppConsole_ExtractTextLine(const char *src,
                                       uint8_t line_index,
                                       char *out_buffer,
                                       uint16_t out_buffer_size)
{
    uint8_t  current_line;
    uint16_t out_index;

    if ((src == NULL) || (out_buffer == NULL) || (out_buffer_size == 0U))
    {
        return;
    }

    current_line = 0U;
    out_index = 0U;
    out_buffer[0] = '\0';

    while (*src != '\0')
    {
        if ((current_line == line_index) && (*src != '\r') && (*src != '\n'))
        {
            if (out_index < (uint16_t)(out_buffer_size - 1U))
            {
                out_buffer[out_index] = *src;
                out_index++;
            }
        }

        if ((*src == '\r') || (*src == '\n'))
        {
            if ((*src == '\r') && (*(src + 1) == '\n'))
            {
                src++;
            }

            if (current_line == line_index)
            {
                break;
            }

            current_line++;
        }

        src++;
    }

    out_buffer[out_index] = '\0';
}

static void AppConsole_RenderDirtyLines(AppConsole *console)
{
    char render_buffer[APP_CONSOLE_RENDER_BUFFER_SIZE];
    char input_update[128];
    char task_update[224];
    char source_update[160];
    char result_update[112];
    char value_update[192];
    char line1[64];
    char line2[64];
    char line3[64];
    char line4[64];

    if (console == NULL)
    {
        return;
    }

    input_update[0] = '\0';
    task_update[0] = '\0';
    source_update[0] = '\0';
    result_update[0] = '\0';
    value_update[0] = '\0';

    if (console->view.input_dirty != 0U)
    {
        (void)snprintf(input_update,
                       sizeof(input_update),
                       "\033[%u;1H\033[2Knode%u> %s",
                       (unsigned int)APP_CONSOLE_ROW_INPUT,
                       (unsigned int)console->node_id,
                       console->view.input_text);
    }

    if (console->view.task_dirty != 0U)
    {
        AppConsole_ExtractTextLine(console->view.task_text, 0U, line1, (uint16_t)sizeof(line1));
        AppConsole_ExtractTextLine(console->view.task_text, 1U, line2, (uint16_t)sizeof(line2));
        AppConsole_ExtractTextLine(console->view.task_text, 2U, line3, (uint16_t)sizeof(line3));
        AppConsole_ExtractTextLine(console->view.task_text, 3U, line4, (uint16_t)sizeof(line4));

        (void)snprintf(task_update,
                       sizeof(task_update),
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_CONSOLE_ROW_TASK_LINE1, line1,
                       (unsigned int)APP_CONSOLE_ROW_TASK_LINE2, line2,
                       (unsigned int)APP_CONSOLE_ROW_TASK_LINE3, line3,
                       (unsigned int)APP_CONSOLE_ROW_TASK_LINE4, line4);
    }

    if (console->view.source_dirty != 0U)
    {
        AppConsole_ExtractTextLine(console->view.source_text, 0U, line1, (uint16_t)sizeof(line1));
        AppConsole_ExtractTextLine(console->view.source_text, 1U, line2, (uint16_t)sizeof(line2));

        (void)snprintf(source_update,
                       sizeof(source_update),
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_CONSOLE_ROW_SOURCE_LINE1, line1,
                       (unsigned int)APP_CONSOLE_ROW_SOURCE_LINE2, line2);
    }

    if (console->view.result_dirty != 0U)
    {
        (void)snprintf(result_update,
                       sizeof(result_update),
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_CONSOLE_ROW_RESULT,
                       console->view.result_text);
    }

    if (console->view.value_dirty != 0U)
    {
        AppConsole_ExtractTextLine(console->view.value_text, 0U, line1, (uint16_t)sizeof(line1));
        AppConsole_ExtractTextLine(console->view.value_text, 1U, line2, (uint16_t)sizeof(line2));
        AppConsole_ExtractTextLine(console->view.value_text, 2U, line3, (uint16_t)sizeof(line3));

        (void)snprintf(value_update,
                       sizeof(value_update),
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s"
                       "\033[%u;1H\033[2K%s",
                       (unsigned int)APP_CONSOLE_ROW_VALUE_LINE1, line1,
                       (unsigned int)APP_CONSOLE_ROW_VALUE_LINE2, line2,
                       (unsigned int)APP_CONSOLE_ROW_VALUE_LINE3, line3);
    }

    (void)snprintf(render_buffer,
                   sizeof(render_buffer),
                   "%s%s%s%s%s",
                   input_update,
                   task_update,
                   source_update,
                   result_update,
                   value_update);

    if (render_buffer[0] == '\0')
    {
        return;
    }

    if (UartService_RequestTx(&console->uart, render_buffer) == INFRA_STATUS_OK)
    {
        console->view.input_dirty = 0U;
        console->view.task_dirty = 0U;
        console->view.source_dirty = 0U;
        console->view.result_dirty = 0U;
        console->view.value_dirty = 0U;
    }
}

static uint8_t AppConsole_IsSpaceChar(char ch)
{
    return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}

static uint8_t AppConsole_ParseU8(const char *text, uint8_t *out_value)
{
    uint16_t value;

    if ((text == NULL) || (out_value == NULL) || (text[0] == '\0'))
    {
        return 0U;
    }

    value = 0U;
    while (*text != '\0')
    {
        if ((*text < '0') || (*text > '9'))
        {
            return 0U;
        }

        value = (uint16_t)((value * 10U) + (uint16_t)(*text - '0'));
        if (value > 255U)
        {
            return 0U;
        }

        text++;
    }

    *out_value = (uint8_t)value;
    return 1U;
}

static uint8_t AppConsole_ParseTarget(const char *text, uint8_t *out_node_id, uint8_t *out_is_broadcast)
{
    uint8_t node_id;

    if ((text == NULL) || (out_node_id == NULL) || (out_is_broadcast == NULL))
    {
        return 0U;
    }

    if (strcmp(text, "all") == 0)
    {
        *out_node_id = CAN_NODE_ID_BROADCAST;
        *out_is_broadcast = 1U;
        return 1U;
    }

    if (AppConsole_ParseU8(text, &node_id) == 0U)
    {
        return 0U;
    }

    if ((node_id < CAN_NODE_ID_MIN) || (node_id > CAN_NODE_ID_MAX))
    {
        return 0U;
    }

    *out_node_id = node_id;
    *out_is_broadcast = 0U;
    return 1U;
}

static uint8_t AppConsole_Tokenize(char *buffer, char *argv[], uint8_t argv_capacity, uint8_t *out_argc)
{
    uint8_t argc;
    char   *cursor;

    if ((buffer == NULL) || (argv == NULL) || (out_argc == NULL) || (argv_capacity == 0U))
    {
        return 0U;
    }

    argc = 0U;
    cursor = buffer;

    while (*cursor != '\0')
    {
        while ((*cursor != '\0') && (AppConsole_IsSpaceChar(*cursor) != 0U))
        {
            cursor++;
        }

        if (*cursor == '\0')
        {
            break;
        }

        if (argc >= argv_capacity)
        {
            return 0U;
        }

        argv[argc] = cursor;
        argc++;

        while ((*cursor != '\0') && (AppConsole_IsSpaceChar(*cursor) == 0U))
        {
            cursor++;
        }

        if (*cursor == '\0')
        {
            break;
        }

        *cursor = '\0';
        cursor++;
    }

    *out_argc = argc;
    return 1U;
}

/*
 * 파싱이 끝난 CAN 명령 하나를 콘솔 요청 큐에 넣는다.
 * AppCore는 CAN task에서 이 큐를 비우므로,
 * 사용자 입력 처리와 실제 통신 작업이 느슨하게 분리된다.
 */
static void AppConsole_QueueCanCommand(AppConsole *console, const AppConsoleCanCommand *command)
{
    if ((console == NULL) || (command == NULL))
    {
        return;
    }

    if (InfraQueue_Push(&console->can_cmd_queue, command) != INFRA_STATUS_OK)
    {
        AppConsole_SetResultText(console, "[busy] can cmd queue full");
        return;
    }

    switch (command->type)
    {
        case APP_CONSOLE_CAN_CMD_OPEN:
            AppConsole_SetResultText(console, "[queued] open");
            break;
        case APP_CONSOLE_CAN_CMD_CLOSE:
            AppConsole_SetResultText(console, "[queued] close");
            break;
        case APP_CONSOLE_CAN_CMD_OFF:
            AppConsole_SetResultText(console, "[queued] off");
            break;
        case APP_CONSOLE_CAN_CMD_TEST:
            AppConsole_SetResultText(console, "[queued] test");
            break;
        case APP_CONSOLE_CAN_CMD_TEXT:
            AppConsole_SetResultText(console, "[queued] text");
            break;
        case APP_CONSOLE_CAN_CMD_EVENT:
            AppConsole_SetResultText(console, "[queued] event");
            break;
        default:
            AppConsole_SetResultText(console, "[queued] command");
            break;
    }
}

static void AppConsole_HandleLine(AppConsole *console, const char *line)
{
    char                 parse_buffer[APP_CONSOLE_LINE_BUFFER_SIZE];
    char                *argv[16];
    uint8_t              argc;
    AppConsoleCanCommand command;
    uint8_t              index;
    uint8_t              target_is_broadcast;

    if ((console == NULL) || (line == NULL))
    {
        return;
    }

    if (strcmp(line, "help") == 0)
    {
        AppConsole_SetResultText(console, g_app_console_help_text);
        return;
    }

    if (strcmp(line, "hello") == 0)
    {
        AppConsole_SetResultText(console, "hello");
        return;
    }

    if (strcmp(line, "ping") == 0)
    {
        AppConsole_SetResultText(console, "pong");
        return;
    }

    if (strcmp(line, "status") == 0)
    {
        char status_text[APP_CONSOLE_RESULT_VIEW_SIZE];

        (void)snprintf(status_text,
                       sizeof(status_text),
                       "state=%u uart_err=%u rx_len=%u tx_busy=%u cmd_q=%u/%u",
                       (unsigned int)console->state,
                       (unsigned int)UartService_HasError(&console->uart),
                       (unsigned int)UartService_GetCurrentInputLength(&console->uart),
                       (unsigned int)UartService_IsTxBusy(&console->uart),
                       (unsigned int)InfraQueue_GetCount(&console->can_cmd_queue),
                       (unsigned int)InfraQueue_GetCapacity(&console->can_cmd_queue));
        AppConsole_SetResultText(console, status_text);
        return;
    }

    if (strcmp(line, "ok") == 0)
    {
        console->local_ok_pending = 1U;
        AppConsole_SetResultText(console, "[queued] local ok");
        return;
    }

    (void)snprintf(parse_buffer, sizeof(parse_buffer), "%s", line);
    if (AppConsole_Tokenize(parse_buffer,
                            argv,
                            (uint8_t)(sizeof(argv) / sizeof(argv[0])),
                            &argc) == 0U)
    {
        AppConsole_SetResultText(console, "[error] invalid command");
        return;
    }

    if (argc == 0U)
    {
        return;
    }

    (void)memset(&command, 0, sizeof(command));
    if ((strcmp(argv[0], "open") == 0) ||
        (strcmp(argv[0], "close") == 0) ||
        (strcmp(argv[0], "off") == 0) ||
        (strcmp(argv[0], "test") == 0))
    {
        if ((argc != 2U) || (AppConsole_ParseTarget(argv[1], &command.target_node_id, &target_is_broadcast) == 0U))
        {
            AppConsole_SetResultText(console, "[error] invalid command");
            return;
        }

        command.target_is_broadcast = target_is_broadcast;
        if (strcmp(argv[0], "open") == 0)
        {
            command.type = APP_CONSOLE_CAN_CMD_OPEN;
        }
        else if (strcmp(argv[0], "close") == 0)
        {
            command.type = APP_CONSOLE_CAN_CMD_CLOSE;
        }
        else if (strcmp(argv[0], "off") == 0)
        {
            command.type = APP_CONSOLE_CAN_CMD_OFF;
        }
        else
        {
            command.type = APP_CONSOLE_CAN_CMD_TEST;
        }

        AppConsole_QueueCanCommand(console, &command);
        return;
    }

    if (strcmp(argv[0], "text") == 0)
    {
        size_t remaining;
        char  *write_ptr;
        int    written;

        if ((argc < 3U) || (AppConsole_ParseTarget(argv[1], &command.target_node_id, &target_is_broadcast) == 0U))
        {
            AppConsole_SetResultText(console, "[error] invalid command");
            return;
        }

        command.type = APP_CONSOLE_CAN_CMD_TEXT;
        command.target_is_broadcast = target_is_broadcast;
        command.text[0] = '\0';
        write_ptr = command.text;
        remaining = sizeof(command.text);

        for (index = 2U; index < argc; index++)
        {
            if (index > 2U)
            {
                if (remaining <= 1U)
                {
                    AppConsole_SetResultText(console, "[error] text too long");
                    return;
                }

                *write_ptr = ' ';
                write_ptr++;
                *write_ptr = '\0';
                remaining--;
            }

            written = snprintf(write_ptr, remaining, "%s", argv[index]);
            if ((written <= 0) || ((size_t)written >= remaining))
            {
                AppConsole_SetResultText(console, "[error] text too long");
                return;
            }

            write_ptr += written;
            remaining -= (size_t)written;
        }

        AppConsole_QueueCanCommand(console, &command);
        return;
    }

    if (strcmp(argv[0], "event") == 0)
    {
        if ((argc != 5U) ||
            (AppConsole_ParseTarget(argv[1], &command.target_node_id, &target_is_broadcast) == 0U) ||
            (AppConsole_ParseU8(argv[2], &command.event_code) == 0U) ||
            (AppConsole_ParseU8(argv[3], &command.arg0) == 0U) ||
            (AppConsole_ParseU8(argv[4], &command.arg1) == 0U))
        {
            AppConsole_SetResultText(console, "[error] invalid command");
            return;
        }

        command.type = APP_CONSOLE_CAN_CMD_EVENT;
        command.target_is_broadcast = target_is_broadcast;
        AppConsole_QueueCanCommand(console, &command);
        return;
    }

    AppConsole_SetResultText(console, "[error] unsupported command");
}

/*
 * 콘솔 상태와 UART transport를 초기화한다.
 * live 데이터가 오기 전에도 operator가 구조화된 화면을 보도록,
 * view에 기본 placeholder 문자열을 미리 채워둔다.
 */
InfraStatus AppConsole_Init(AppConsole *console, uint8_t node_id)
{
    if (console == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(console, 0, sizeof(*console));
    console->node_id = node_id;
    console->state = APP_CONSOLE_STATE_IDLE;

    if (InfraQueue_Init(&console->can_cmd_queue,
                        console->can_cmd_storage,
                        (uint16_t)sizeof(AppConsoleCanCommand),
                        APP_CONSOLE_CAN_CMD_QUEUE_SIZE) != INFRA_STATUS_OK)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    if (UartService_Init(&console->uart) != INFRA_STATUS_OK)
    {
        console->state = APP_CONSOLE_STATE_ERROR;
        return INFRA_STATUS_IO_ERROR;
    }

    (void)snprintf(console->view.task_text,
                   sizeof(console->view.task_text),
                   "HeartBeat : Waiting...\r\n"
                   "CAN       : Waiting...\r\n"
                   "LIN       : Waiting...\r\n"
                   "UART      : Waiting...");
    (void)snprintf(console->view.source_text,
                   sizeof(console->view.source_text),
                   "from [can] \"waiting\"\r\n"
                   "from [lin] \"waiting\"");
    (void)snprintf(console->view.result_text,
                   sizeof(console->view.result_text),
                   "%s",
                   g_app_console_help_text);
    (void)snprintf(console->view.value_text,
                   sizeof(console->view.value_text),
                   "Mode   : waiting\r\n"
                   "Button : waiting\r\n"
                   "ADC    : waiting");

    AppConsole_RequestFullRefresh(console);
    console->initialized = 1U;
    return INFRA_STATUS_OK;
}

/*
 * UART RX/TX 작업을 진행시키고 완성된 입력 줄을 처리한다.
 * 오류 처리와 `recover` 명령도 여기 들어 있어,
 * 펌웨어 재시작 없이 콘솔을 복구할 수 있게 한다.
 */
void AppConsole_Task(AppConsole *console, uint32_t now_ms)
{
    char line_buffer[APP_CONSOLE_LINE_BUFFER_SIZE];

    if ((console == NULL) || (console->initialized == 0U))
    {
        return;
    }

    UartService_ProcessRx(&console->uart);

    if (UartService_HasError(&console->uart) != 0U)
    {
        console->state = APP_CONSOLE_STATE_ERROR;
        AppConsole_SetResultText(console,
                                 AppConsole_GetUartErrorText(UartService_GetErrorCode(&console->uart)));

        if (UartService_HasLine(&console->uart) != 0U)
        {
            if (UartService_ReadLine(&console->uart,
                                     line_buffer,
                                     (uint16_t)sizeof(line_buffer)) == INFRA_STATUS_OK)
            {
                if (strcmp(line_buffer, "recover") == 0)
                {
                    if (UartService_Recover(&console->uart) == INFRA_STATUS_OK)
                    {
                        console->state = APP_CONSOLE_STATE_IDLE;
                        AppConsole_SetResultText(console, "[ok] uart recover complete");
                        AppConsole_RequestFullRefresh(console);
                    }
                }
                else
                {
                    AppConsole_SetResultText(console, "[error] uart stopped (type: recover)");
                }
            }
        }
    }
    else if (UartService_HasLine(&console->uart) != 0U)
    {
        if (UartService_ReadLine(&console->uart,
                                 line_buffer,
                                 (uint16_t)sizeof(line_buffer)) == INFRA_STATUS_OK)
        {
            AppConsole_HandleLine(console, line_buffer);
        }
    }

    UartService_ProcessTx(&console->uart, now_ms);
}

/*
 * 현재 dirty flag에 따라 콘솔 UI를 갱신한다.
 * layout과 바뀐 줄을 분리해서 보내므로,
 * 작은 텍스트 필드 하나만 변해도 효율적으로 렌더링할 수 있다.
 */
void AppConsole_Render(AppConsole *console)
{
    if ((console == NULL) || (console->initialized == 0U))
    {
        return;
    }

    if (console->state != APP_CONSOLE_STATE_ERROR)
    {
        AppConsole_UpdateInputView(console);
    }

    if ((console->view.layout_drawn == 0U) || (console->view.full_refresh_required != 0U))
    {
        AppConsole_RenderLayout(console);
        if (console->view.layout_drawn == 0U)
        {
            return;
        }

        console->view.full_refresh_required = 0U;
    }

    AppConsole_RenderDirtyLines(console);
}

uint8_t AppConsole_TryPopCanCommand(AppConsole *console, AppConsoleCanCommand *out_cmd)
{
    if ((console == NULL) || (out_cmd == NULL))
    {
        return 0U;
    }

    return (InfraQueue_Pop(&console->can_cmd_queue, out_cmd) == INFRA_STATUS_OK) ? 1U : 0U;
}

uint8_t AppConsole_ConsumeLocalOk(AppConsole *console)
{
    uint8_t pending;

    if (console == NULL)
    {
        return 0U;
    }

    pending = console->local_ok_pending;
    console->local_ok_pending = 0U;
    return pending;
}

uint8_t AppConsole_IsError(const AppConsole *console)
{
    if (console == NULL)
    {
        return 1U;
    }

    return (console->state == APP_CONSOLE_STATE_ERROR) ? 1U : 0U;
}
