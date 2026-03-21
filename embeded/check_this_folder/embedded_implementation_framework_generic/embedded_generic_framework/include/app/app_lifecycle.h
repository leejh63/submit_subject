#ifndef APP_LIFECYCLE_H
#define APP_LIFECYCLE_H

#include "core/emb_status.h"
#include "core/emb_time.h"

emb_status_t app_init(void);
void app_process(emb_tick_t now);

#endif
