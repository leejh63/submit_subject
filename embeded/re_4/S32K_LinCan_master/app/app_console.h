/*
 * 사람이 읽을 수 있는 제어와 상태 표시용 UART 콘솔 인터페이스다.
 * 콘솔은 화면 렌더 상태를 유지하고 명령을 파싱하며,
 * queue에 쌓인 CAN 요청을 app 계층에 노출한다.
 */
#ifndef APP_CONSOLE_H
#define APP_CONSOLE_H

#include "app_config.h"
#include "../services/can_types.h"
#include "../core/infra_queue.h"
#include "../services/uart_service.h"

typedef enum
{
    APP_CONSOLE_CAN_CMD_NONE = 0,
    APP_CONSOLE_CAN_CMD_OPEN,
    APP_CONSOLE_CAN_CMD_CLOSE,
    APP_CONSOLE_CAN_CMD_OFF,
    APP_CONSOLE_CAN_CMD_TEST,
    APP_CONSOLE_CAN_CMD_TEXT,
    APP_CONSOLE_CAN_CMD_EVENT
} AppConsoleCanCommandType;

/*
 * 파싱된 CAN 지향 콘솔 명령 구조체다.
 * 콘솔은 operator 의도를 이 중립적인 형태로 저장해 두고,
 * AppCore가 나중에 CAN 모듈 요청으로 변환한다.
 */
typedef struct
{
    uint8_t type;
    uint8_t target_node_id;
    uint8_t target_is_broadcast;
    char    text[CAN_TEXT_MAX_LEN + 1U];
    uint8_t event_code;
    uint8_t arg0;
    uint8_t arg1;
} AppConsoleCanCommand;

typedef enum
{
    APP_CONSOLE_STATE_IDLE = 0,
    APP_CONSOLE_STATE_ERROR
} AppConsoleState;

/*
 * 콘솔 view 문자열과 dirty flag를 캐시하는 구조체다.
 * 렌더링은 이 구조를 사용해,
 * 마지막 UART 전송 이후 바뀐 부분만 다시 그린다.
 */
typedef struct
{
    char    input_text[APP_CONSOLE_INPUT_VIEW_SIZE];
    char    task_text[APP_CONSOLE_TASK_VIEW_SIZE];
    char    source_text[APP_CONSOLE_SOURCE_VIEW_SIZE];
    char    result_text[APP_CONSOLE_RESULT_VIEW_SIZE];
    char    value_text[APP_CONSOLE_VALUE_VIEW_SIZE];
    uint8_t full_refresh_required;
    uint8_t input_dirty;
    uint8_t task_dirty;
    uint8_t source_dirty;
    uint8_t result_dirty;
    uint8_t value_dirty;
    uint8_t layout_drawn;
} AppConsoleView;

/*
 * 하나의 펌웨어 이미지가 소유하는 전체 UART 콘솔 상태다.
 * UART transport와 operator 요청 queue,
 * 그리고 현재 화면에 표시된 task/source/value view를 모두 가진다.
 */
typedef struct
{
    uint8_t        initialized;
    uint8_t        node_id;
    AppConsoleState state;
    uint8_t        local_ok_pending;
    UartService    uart;
    InfraQueue     can_cmd_queue;
    AppConsoleCanCommand can_cmd_storage[APP_CONSOLE_CAN_CMD_QUEUE_SIZE];
    AppConsoleView view;
} AppConsole;

InfraStatus AppConsole_Init(AppConsole *console, uint8_t node_id);
void        AppConsole_Task(AppConsole *console, uint32_t now_ms);
void        AppConsole_Render(AppConsole *console);
uint8_t     AppConsole_TryPopCanCommand(AppConsole *console, AppConsoleCanCommand *out_cmd);
uint8_t     AppConsole_ConsumeLocalOk(AppConsole *console);
void        AppConsole_SetTaskText(AppConsole *console, const char *text);
void        AppConsole_SetSourceText(AppConsole *console, const char *text);
void        AppConsole_SetResultText(AppConsole *console, const char *text);
void        AppConsole_SetValueText(AppConsole *console, const char *text);
uint8_t     AppConsole_IsError(const AppConsole *console);

#endif
