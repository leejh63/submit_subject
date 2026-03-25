/*
 * portable LIN 상태기계 구현부다.
 * protocol 흐름과 fresh status 저장, token 처리를 담당하고,
 * 보드 전용 동작은 runtime_io에 의존한다.
 */
#include "lin_module.h"

#include <stddef.h>
#include <string.h>

#define LIN_STATE_IDLE      0U
#define LIN_STATE_WAIT_PID  1U
#define LIN_STATE_WAIT_RX   2U
#define LIN_STATE_WAIT_TX   3U

#define LIN_FLAG_PID_OK     (1UL << 0)
#define LIN_FLAG_RX_DONE    (1UL << 1)
#define LIN_FLAG_TX_DONE    (1UL << 2)
#define LIN_FLAG_ERROR      (1UL << 3)

/*
 * LIN 상태기계를 중립 idle 상태로 되돌린다.
 * 이 helper는 하드웨어 바인딩에도 활성 전송 상태를 정리하라고 요청하여,
 * 이후 transaction이 깨끗하게 시작되도록 한다.
 */
static void LinModule_GotoIdle(LinModule *module)
{
    if (module == NULL)
    {
        return;
    }

    if (module->config.binding.goto_idle_fn != NULL)
    {
        module->config.binding.goto_idle_fn(module->config.binding.context);
    }

    module->state = LIN_STATE_IDLE;
    module->current_pid = 0U;
}

static void LinModule_SetTimeout(LinModule *module)
{
    if ((module == NULL) || (module->config.binding.set_timeout_fn == NULL))
    {
        return;
    }

    module->config.binding.set_timeout_fn(module->config.binding.context,
                                          module->config.timeout_ticks);
}

/*
 * cache된 status frame을 raw LIN payload buffer로 포장한다.
 * slave 모드는 master가 status PID를 poll하고,
 * 최신 센서 정보를 기대할 때 이 helper를 사용한다.
 */
static void LinModule_PrepareStatusTx(const LinStatusFrame *status, uint8_t *buffer, uint8_t size)
{
    uint16_t adc_value;

    if ((status == NULL) || (buffer == NULL) || (size == 0U))
    {
        return;
    }

    (void)memset(buffer, 0, size);
    adc_value = status->adc_value;
    buffer[0] = (uint8_t)(adc_value & 0xFFU);
    if (size > 1U)
    {
        buffer[1] = (uint8_t)((adc_value >> 8U) & 0x0FU);
    }
    if (size > 2U)
    {
        buffer[2] = status->zone;
    }
    if (size > 3U)
    {
        buffer[3] = status->emergency_latched;
    }
}

/*
 * 수신한 LIN status payload를 모듈 상태로 decode한다.
 * master 모드는 결과를 latest_status에 저장해,
 * app 계층이 나중에 fresh 센서 업데이트를 소비할 수 있게 한다.
 */
static void LinModule_ParseStatusRx(LinModule *module)
{
    uint16_t adc_value;

    if (module == NULL)
    {
        return;
    }

    adc_value = (uint16_t)module->rx_buffer[0] |
                (uint16_t)((uint16_t)module->rx_buffer[1] << 8U);
    if (adc_value > 4095U)
    {
        adc_value = 4095U;
    }

    module->latest_status.adc_value = adc_value;
    module->latest_status.zone = module->rx_buffer[2];
    module->latest_status.emergency_latched = (module->rx_buffer[3] != 0U) ? 1U : 0U;
    // 사실상 죽은 변수
    module->latest_status.valid = 1U;
    module->latest_status.fresh = 1U;
}

/*
 * header를 보내서 master 측 LIN transaction 하나를 시작한다.
 * 이 함수는 idle 상태에서만 성공하도록 제한해,
 * 상태기계가 한 번에 하나의 status/token 교환만 다루게 한다.
 */
static InfraStatus LinModule_MasterStart(LinModule *module, uint8_t pid)
{
    if ((module == NULL) || (module->config.binding.master_send_header_fn == NULL))
    {
        return INFRA_STATUS_NOT_READY;
    }

    if (module->state != LIN_STATE_IDLE)
    {
        return INFRA_STATUS_BUSY;
    }
    
    if (module->config.binding.master_send_header_fn(module->config.binding.context, pid) != INFRA_STATUS_OK)
    {
        LinModule_GotoIdle(module);
        return INFRA_STATUS_IO_ERROR;
    }

    module->current_pid = pid;
    module->state = LIN_STATE_WAIT_PID;
    return INFRA_STATUS_OK;
}

/*
 * portable LIN 상태기계를 초기화한다.
 * 먼저 설정을 복사하고,
 * 그다음 하드웨어 바인딩에 concrete peripheral 초기화를 요청한다.
 */
InfraStatus LinModule_Init(LinModule *module, const LinConfig *config)
{
    InfraStatus status;

    if ((module == NULL) || (config == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(module, 0, sizeof(*module));
    module->config = *config;
    module->state = LIN_STATE_IDLE;

    if (module->config.binding.init_fn == NULL)
    {
        return INFRA_STATUS_NOT_READY;
    }

    status = module->config.binding.init_fn(module->config.binding.context);
    if (status != INFRA_STATUS_OK)
    {
        return status;
    }

    module->initialized = 1U;
    return INFRA_STATUS_OK;
}

void LinModule_OnBaseTick(LinModule *module)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    if (module->config.binding.service_tick_fn != NULL)
    {
        module->config.binding.service_tick_fn(module->config.binding.context);
    }
}

/*
 * runtime_io가 전달한 하드웨어 event 하나를 소비한다.
 * master 모드는 fast task용 flag를 기록하고,
 * slave 모드는 일치하는 PID에 대해 바로 RX/TX 작업을 시작할 수 있다.
 */
void LinModule_OnEvent(LinModule *module, LinEventId event_id, uint8_t current_pid)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    module->current_pid = current_pid;
    // LIN_ROLE_MASTER
    if (module->config.role == LIN_ROLE_MASTER)
    {
        switch (event_id)
        {
            case LIN_EVENT_PID_OK:
                module->flags |= LIN_FLAG_PID_OK;
                break;

            case LIN_EVENT_RX_DONE:
                module->flags |= LIN_FLAG_RX_DONE;
                break;

            case LIN_EVENT_TX_DONE:
                module->flags |= LIN_FLAG_TX_DONE;
                break;

            case LIN_EVENT_ERROR:
                module->flags |= LIN_FLAG_ERROR;
                break;

            default:
                break;
        }

        return;
    }
    // LIN_ROLE_SLAVE
    switch (event_id)
    {
        case LIN_EVENT_PID_OK:
            LinModule_SetTimeout(module);
            if (current_pid == module->config.pid_status)
            {
                LinModule_PrepareStatusTx(&module->slave_status_cache,
                                          module->tx_buffer,
                                          module->config.status_frame_size);
                if (module->config.binding.start_send_fn != NULL)
                {
                    if (module->config.binding.start_send_fn(module->config.binding.context,
                                                             module->tx_buffer,
                                                             module->config.status_frame_size) == INFRA_STATUS_OK)
                    {
                        module->state = LIN_STATE_WAIT_TX;
                    }
                    else
                    {
                        LinModule_GotoIdle(module);
                    }
                }
            }
            else if (current_pid == module->config.pid_ok)
            {
                if (module->config.binding.start_receive_fn != NULL)
                {
                    if (module->config.binding.start_receive_fn(module->config.binding.context,
                                                                module->rx_buffer,
                                                                module->config.ok_frame_size) == INFRA_STATUS_OK)
                    {
                        module->state = LIN_STATE_WAIT_RX;
                    }
                    else
                    {
                        LinModule_GotoIdle(module);
                    }
                }
            }
            else
            {
                LinModule_GotoIdle(module);
            }
            break;

        case LIN_EVENT_RX_DONE:
            if ((current_pid == module->config.pid_ok) &&
                (module->rx_buffer[0] == module->config.ok_token))
            {
                module->ok_token_pending = 1U;
            }
            LinModule_GotoIdle(module);
            break;

        case LIN_EVENT_TX_DONE:
            LinModule_GotoIdle(module);
            break;

        case LIN_EVENT_ERROR:
            LinModule_GotoIdle(module);
            break;

        default:
            break;
    }
}

/*
 * master 측 LIN 상태기계를 빠르게 진행시킨다.
 * 이 task는 기록된 PID, RX, TX, error event에 반응하여,
 * 느린 poll 주기 사이에도 status polling이 responsive하게 유지되도록 한다.
 */
void LinModule_TaskFast(LinModule *module, uint32_t now_ms)
{
    (void)now_ms;

    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    if (module->config.role != LIN_ROLE_MASTER)
    {
        return;
    }

    if ((module->flags & LIN_FLAG_ERROR) != 0U)
    {
        module->flags &= ~LIN_FLAG_ERROR;
        if (module->current_pid == module->config.pid_ok)
        {
            module->ok_tx_pending = 0U;
        }
        LinModule_GotoIdle(module);
        return;
    }

    if ((module->state == LIN_STATE_WAIT_PID) && ((module->flags & LIN_FLAG_PID_OK) != 0U))
    {
        module->flags &= ~LIN_FLAG_PID_OK;
        LinModule_SetTimeout(module);

        if (module->current_pid == module->config.pid_status)
        {
            if ((module->config.binding.start_receive_fn != NULL) &&
                (module->config.binding.start_receive_fn(module->config.binding.context,
                                                         module->rx_buffer,
                                                         module->config.status_frame_size) == INFRA_STATUS_OK))
            {
                module->state = LIN_STATE_WAIT_RX;
                return;
            }
        }
        else if (module->current_pid == module->config.pid_ok)
        {
            module->tx_buffer[0] = module->config.ok_token;
            if ((module->config.binding.start_send_fn != NULL) &&
                (module->config.binding.start_send_fn(module->config.binding.context,
                                                      module->tx_buffer,
                                                      module->config.ok_frame_size) == INFRA_STATUS_OK))
            {
                module->state = LIN_STATE_WAIT_TX;
                return;
            }

            module->ok_tx_pending = 0U;
        }

        LinModule_GotoIdle(module);
        return;
    }

    if ((module->state == LIN_STATE_WAIT_RX) && ((module->flags & LIN_FLAG_RX_DONE) != 0U))
    {
        module->flags &= ~LIN_FLAG_RX_DONE;
        LinModule_ParseStatusRx(module);
        LinModule_GotoIdle(module);
        return;
    }

    if ((module->state == LIN_STATE_WAIT_TX) && ((module->flags & LIN_FLAG_TX_DONE) != 0U))
    {
        module->flags &= ~LIN_FLAG_TX_DONE;
        module->ok_tx_pending = 0U;
        LinModule_GotoIdle(module);
    }
}

/*
 * 주기적인 master 측 LIN 작업을 시작한다.
 * poll task는 현재 application 필요에 따라,
 * status 요청과 queue에 쌓인 OK token 전송 중 하나를 선택한다.
 */
void LinModule_TaskPoll(LinModule *module, uint32_t now_ms)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return;
    }

    if (module->config.role != LIN_ROLE_MASTER)
    {
        return;
    }

    if (Infra_TimeIsDue(now_ms, module->last_poll_ms, module->config.poll_period_ms) == 0U)
    {
        return;
    }

    module->last_poll_ms = now_ms;
    if (module->ok_tx_pending != 0U)
    {
        (void)LinModule_MasterStart(module, module->config.pid_ok);
        return;
    }

    (void)LinModule_MasterStart(module, module->config.pid_status);
}

/*
 * 다음 master poll 슬롯에 보낼 OK token 전송을 queue에 올린다.
 * 승인 로직은 즉시 보내지 않고 이 경로를 사용해,
 * 버스 동작이 일반 상태기계 흐름 안에서 진행되게 한다.
 */
InfraStatus LinModule_RequestOk(LinModule *module)
{
    if ((module == NULL) || (module->initialized == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (module->config.role != LIN_ROLE_MASTER)
    {
        return INFRA_STATUS_UNSUPPORTED;
    }

    module->ok_tx_pending = 1U;
    module->tx_buffer[0] = module->config.ok_token;
    return INFRA_STATUS_OK;
}

uint8_t LinModule_GetLatestStatus(const LinModule *module, LinStatusFrame *out_status)
{
    if ((module == NULL) || (out_status == NULL) || (module->latest_status.valid == 0U))
    {
        return 0U;
    }

    *out_status = module->latest_status;
    return 1U;
}

uint8_t LinModule_ConsumeFreshStatus(LinModule *module, LinStatusFrame *out_status)
{
    if ((module == NULL) || (out_status == NULL) || (module->latest_status.fresh == 0U))
    {
        return 0U;
    }

    *out_status = module->latest_status;
    module->latest_status.fresh = 0U;
    return 1U;
}

void LinModule_SetSlaveStatus(LinModule *module, const LinStatusFrame *status)
{
    if ((module == NULL) || (status == NULL) || (module->initialized == 0U))
    {
        return;
    }

    module->slave_status_cache = *status;
}

uint8_t LinModule_ConsumeSlaveOkToken(LinModule *module)
{
    uint8_t pending;

    if (module == NULL)
    {
        return 0U;
    }

    pending = module->ok_token_pending;
    module->ok_token_pending = 0U;
    return pending;
}
