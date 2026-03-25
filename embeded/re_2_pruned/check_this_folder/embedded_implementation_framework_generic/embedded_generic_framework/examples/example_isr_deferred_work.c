#include "services/svc_deferred_work.h"

static svc_deferred_work_t g_deferred;
static svc_deferred_item_t g_storage[16];

static void process_sensor(void *context)
{
    (void)context;
    /* heavy work here */
}

void adc_dma_complete_isr(void)
{
    /* ISR에서는 등록만 */
    (void)svc_deferred_work_post(&g_deferred, process_sensor, 0);
}

void deferred_init(void)
{
    svc_deferred_work_config_t config =
    {
        .storage = g_storage,
        .capacity = 16U
    };

    (void)svc_deferred_work_init(&g_deferred, &config);
}

void deferred_process(void)
{
    svc_deferred_work_process(&g_deferred);
}
