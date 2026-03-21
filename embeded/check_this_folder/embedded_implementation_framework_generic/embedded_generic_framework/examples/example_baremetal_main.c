#include "app/app_lifecycle.h"
#include "core/emb_time.h"

/*
 * 가장 일반적인 bare-metal super loop 틀.
 * 필요하면 여기서 scheduler나 RTOS task entry로 치환하면 된다.
 */

extern emb_tick_t platform_time_now(void);
extern void platform_idle(void);

int main(void)
{
    emb_tick_t now;

    if (app_init() != EMB_OK)
    {
        for (;;)
        {
            /* safe state / blink / reset */
        }
    }

    for (;;)
    {
        now = platform_time_now();
        app_process(now);
        platform_idle();
    }

    return 0;
}
