/*
 * 하나의 펌웨어 역할에 대한 컴파일 타임 설정 모음이다.
 * 활성 노드 역할을 선택하고, 공통 task 주기와
 * 콘솔 버퍼 크기, 역할 식별값을 함께 정의한다.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

/*
 * 세 프로젝트가 공통으로 쓰는 역할 식별자다.
 * 빌드마다 하나의 역할만 활성화되지만,
 * 노드 관계를 일관되게 유지하기 위해 전체 집합을 모두 가진다.
 */
#define APP_ROLE_MASTER   1U
#define APP_ROLE_SLAVE1   2U
#define APP_ROLE_SLAVE2   3U

/*
 * 이 펌웨어 이미지의 빌드 시 역할 선택 지점이다.
 * 각 프로젝트는 다른 기본값을 사용해,
 * 같은 소스 트리가 master, slave1, slave2로 동작하게 만든다.
 */
#ifndef APP_ACTIVE_ROLE
#define APP_ACTIVE_ROLE   APP_ROLE_SLAVE2
#endif

#define APP_NODE_ID_MASTER    1U
#define APP_NODE_ID_SLAVE1    2U
#define APP_NODE_ID_SLAVE2    3U

/*
 * cooperative runtime task 테이블의 주기 정의다.
 * 스케줄러는 이 값을 공유 tick과 비교하여,
 * 실행 시점이 된 AppCore task를 호출한다.
 */
#define APP_TASK_UART_MS          1U
#define APP_TASK_LIN_FAST_MS      1U
#define APP_TASK_BUTTON_MS        10U
#define APP_TASK_CAN_MS           10U
#define APP_TASK_ADC_MS           20U
#define APP_TASK_LIN_POLL_MS      20U
#define APP_TASK_LED_MS           100U
#define APP_TASK_RENDER_MS        100U
#define APP_TASK_HEARTBEAT_MS     1000U

#define APP_SLAVE1_ACK_TOGGLES    6U

/*
 * UART HMI 계층이 사용하는 콘솔/UI 버퍼 크기다.
 * 동적 메모리 없이도 명령 길이와 렌더 가능한 상태 문자열을
 * 일정 범위 안에 제한하도록 만든다.
 */
#define APP_CONSOLE_LINE_BUFFER_SIZE   64U
#define APP_CONSOLE_INPUT_VIEW_SIZE    64U
#define APP_CONSOLE_TASK_VIEW_SIZE     160U
#define APP_CONSOLE_SOURCE_VIEW_SIZE   128U
#define APP_CONSOLE_RESULT_VIEW_SIZE   96U
#define APP_CONSOLE_VALUE_VIEW_SIZE    128U
#define APP_CONSOLE_CAN_CMD_QUEUE_SIZE 4U

#endif
