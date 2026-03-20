#ifndef RUNTIME_TASK_H
#define RUNTIME_TASK_H

#include <stdint.h>

/*
 * 단순 주기 태스크 스케줄러용 정의.
 * 각 태스크는 periodMs 간격으로 실행된다.
 */

typedef void (*RuntimeTaskFunc)(void *context);

/* 태스크 테이블 한 줄 */
typedef struct
{
    const char      *name;        /* 디버깅용 이름 */
    uint32_t         periodMs;    /* 실행 주기 */
    uint32_t         lastRunTick; /* 마지막 실행 tick */
    RuntimeTaskFunc  taskFunc;    /* 호출할 함수 */
    void            *context;     /* 태스크별 사용자 컨텍스트 */
} RuntimeTaskEntry;

/* 현재 tick을 기준으로 실행 시점이 된 태스크들을 순회 실행한다. */
void RuntimeTask_RunTable(RuntimeTaskEntry *table,
                          uint32_t taskCount,
                          uint32_t currentTickMs);

#endif
