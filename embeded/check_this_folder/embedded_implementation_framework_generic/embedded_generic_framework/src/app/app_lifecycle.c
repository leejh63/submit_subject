#include "app/app_lifecycle.h"

/*
 * 실제 프로젝트에서는 여기에서
 * - board/platform init
 * - driver init/start
 * - service init
 * - app FSM init
 * 순으로 조립한다.
 */

emb_status_t app_init(void)
{
    return EMB_OK;
}

void app_process(emb_tick_t now)
{
    (void)now;
    /* periodic jobs, service processing, app FSM dispatch */
}
