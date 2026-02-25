// lin_engine.c
#include "lin_engine.h"
#include "sdk_project_config.h"

#include <string.h>

// ============================================================
// local critical
// ============================================================
static inline void ENTER_CRITICAL(void) { INT_SYS_DisableIRQGlobal(); }
static inline void EXIT_CRITICAL(void)  { INT_SYS_EnableIRQGlobal();  }

// ============================================================
// buffers (engine-owned)
// ============================================================
static uint8_t s_txCmd[DATA_SIZE]    = {0};
static uint8_t s_rxStatus[DATA_SIZE] = {0};

const uint8_t *lin_engine_status_buf(void) { return s_rxStatus; }

// ============================================================
// frames (engine-owned)
// ============================================================
enum
{
    FRAME_IDX_CMD = 0,
    FRAME_IDX_STATUS,
    FRAME_IDX_COUNT
};

static lin_frame_def_t s_frames[FRAME_IDX_COUNT] =
{
    {
        .id = FRAME_MOTOR_CMD,
        .dir = LIN_DIR_TX,
        .buf = s_txCmd,
        .len = DATA_SIZE,
        .timeoutCnt = TIMEOUT_LIN_CNT
    },
    {
        .id = FRAME_SLAVE_STATUS,
        .dir = LIN_DIR_RX,
        .buf = s_rxStatus,
        .len = DATA_SIZE,
        .timeoutCnt = TIMEOUT_LIN_CNT
    }
};

// ============================================================
// scheduler (engine-owned)
// ============================================================
typedef struct
{
    uint8_t   frameIndex;
    uint32_t  periodTicks;
    uint32_t  nextDueTick;
    uint8_t   priority;
    bool      enabled;
} lin_sched_entry_t;

// 200ms = 200000us / 500us = 400 ticks
#define TICKS_200MS (400U)

enum
{
    SCHED_STATUS_POLL = 0,
    SCHED_COUNT
};

static lin_sched_entry_t s_sched[SCHED_COUNT] =
{
    {
        .frameIndex = FRAME_IDX_STATUS,
        .periodTicks = TICKS_200MS,
        .nextDueTick = 0U,
        .priority = 10U,
        .enabled = true
    }
};

static void scheduler_init(uint32_t nowTick)
{
    for (uint32_t i = 0; i < SCHED_COUNT; i++)
    {
        if (!s_sched[i].enabled) continue;
        s_sched[i].nextDueTick = nowTick + s_sched[i].periodTicks;
    }
}

static int32_t scheduler_pick_due(uint32_t nowTick)
{
    int32_t best = -1;
    uint8_t bestPrio = 0xFFU;

    for (uint32_t i = 0; i < SCHED_COUNT; i++)
    {
        if (!s_sched[i].enabled) continue;
        if (s_sched[i].periodTicks == 0U) continue;

        if ((int32_t)(nowTick - s_sched[i].nextDueTick) >= 0)
        {
            if (s_sched[i].priority < bestPrio)
            {
                bestPrio = s_sched[i].priority;
                best = (int32_t)i;
            }
        }
    }
    return best;
}

static void scheduler_on_fired(uint32_t schedIndex, uint32_t nowTick)
{
    if (s_sched[schedIndex].periodTicks != 0U)
        s_sched[schedIndex].nextDueTick = nowTick + s_sched[schedIndex].periodTicks;
}

// ============================================================
// engine runtime (engine-owned)
// ============================================================
static volatile lin_eng_state_t s_engState = ENG_IDLE;

static volatile uint8_t   s_activeFrameIndex = 0U;
static volatile lin_req_t s_activeReq;

// callback flags
#define LIN_F_PID_OK   (1UL << 0)
#define LIN_F_TX_DONE  (1UL << 1)
#define LIN_F_RX_DONE  (1UL << 2)
#define LIN_F_ERROR    (1UL << 3)
#define LIN_F_TIMEOUT  (1UL << 4)

static volatile uint32_t s_linFlags = 0U;

static volatile uint8_t s_lastEvent  = 0U;
static volatile uint8_t s_linErrCode = 0U;

// done callback
static lin_engine_done_cb_t s_done_cb = 0;

// ============================================================
// request coalescing (last-wins) (engine-owned)
// ============================================================
static volatile bool         s_cmdPending = false;
static volatile uint8_t      s_cmdTarget  = MOTOR_TARGET_LEFT;
static volatile req_source_t s_cmdSrc     = REQ_SRC_UART;

static volatile bool         s_userStatusPending = false;
static volatile req_source_t s_userStatusSrc     = REQ_SRC_UART;

// ============================================================
// public getters
// ============================================================
bool lin_engine_is_idle(void) { return (s_engState == ENG_IDLE); }
lin_eng_state_t lin_engine_state(void) { return (lin_eng_state_t)s_engState; }

lin_req_t lin_engine_active_req(void)
{
    lin_req_t r;
    ENTER_CRITICAL();
    r = s_activeReq;
    EXIT_CRITICAL();
    return r;
}

uint8_t lin_engine_last_event(void) { return (uint8_t)s_lastEvent; }
uint8_t lin_engine_err_code(void)   { return (uint8_t)s_linErrCode; }

// ============================================================
// LIN Callback: flags only (engine-owned)
// ============================================================
static lin_callback_t lin_engine_cb(uint32_t instance, lin_state_t *linState)
{
    lin_callback_t ret = linState->Callback;
    (void)instance;

    if (linState->timeoutCounterFlag)
    {
        linState->timeoutCounterFlag = false;
        s_linFlags |= (LIN_F_TIMEOUT | LIN_F_ERROR);
        s_linErrCode = 5;
        return ret;
    }

    s_lastEvent = (uint8_t)linState->currentEventId;

    switch (linState->currentEventId)
    {
        case LIN_PID_OK:        s_linFlags |= LIN_F_PID_OK;  break;
        case LIN_TX_COMPLETED:  s_linFlags |= LIN_F_TX_DONE; break;
        case LIN_RX_COMPLETED:  s_linFlags |= LIN_F_RX_DONE; break;

        case LIN_PID_ERROR:        s_linFlags |= LIN_F_ERROR; s_linErrCode = 1; break;
        case LIN_CHECKSUM_ERROR:   s_linFlags |= LIN_F_ERROR; s_linErrCode = 2; break;
        case LIN_READBACK_ERROR:   s_linFlags |= LIN_F_ERROR; s_linErrCode = 3; break;
        case LIN_FRAME_ERROR:      s_linFlags |= LIN_F_ERROR; s_linErrCode = 4; break;

        default: break;
    }

    return ret;
}

void lin_engine_install_callback(void)
{
    (void)LIN_DRV_InstallCallback(INST_LIN2, (lin_callback_t)lin_engine_cb);
}

// ============================================================
// engine core
// ============================================================
static void engine_reset_to_idle(void)
{
    (void)LIN_DRV_GotoIdleState(INST_LIN2);
    s_engState = ENG_IDLE;

    ENTER_CRITICAL();
    s_linFlags = 0U;
    EXIT_CRITICAL();
}

static bool engine_start_request(const lin_req_t *req)
{
    if (s_engState != ENG_IDLE) return false;

    s_linErrCode = 0;
    s_lastEvent  = 0;

    s_activeReq = *req;
    s_activeFrameIndex = req->frameIndex;

    (void)LIN_DRV_MasterSendHeader(INST_LIN2, s_frames[s_activeFrameIndex].id);
    s_engState = ENG_WAIT_PID;
    return true;
}

static void engine_done(bool ok)
{
    if (s_done_cb) s_done_cb((const lin_req_t *)&s_activeReq, ok);
}

void lin_engine_step(uint32_t nowTick)
{
    uint32_t flags;
    ENTER_CRITICAL();
    flags = s_linFlags;
    s_linFlags = 0U;
    EXIT_CRITICAL();

    // deadline
    if (s_engState != ENG_IDLE && s_activeReq.deadlineTick != 0U)
    {
        if ((int32_t)(nowTick - s_activeReq.deadlineTick) >= 0)
        {
            engine_done(false);
            engine_reset_to_idle();
            return;
        }
    }

    // error + retry
    if (flags & LIN_F_ERROR)
    {
        if (s_activeReq.retryCount < s_activeReq.maxRetry)
        {
            s_activeReq.retryCount++;
            engine_reset_to_idle();
            (void)engine_start_request((const lin_req_t *)&s_activeReq);
            return;
        }

        engine_done(false);
        engine_reset_to_idle();
        return;
    }

    // WAIT_PID -> send/recv
    if (s_engState == ENG_WAIT_PID)
    {
        if (flags & LIN_F_PID_OK)
        {
            LIN_DRV_SetTimeoutCounter(INST_LIN2, s_frames[s_activeFrameIndex].timeoutCnt);

            if (s_frames[s_activeFrameIndex].dir == LIN_DIR_TX)
            {
                (void)LIN_DRV_SendFrameData(INST_LIN2,
                                            s_frames[s_activeFrameIndex].buf,
                                            s_frames[s_activeFrameIndex].len);
                s_engState = ENG_WAIT_TX;
            }
            else
            {
                (void)LIN_DRV_ReceiveFrameData(INST_LIN2,
                                               s_frames[s_activeFrameIndex].buf,
                                               s_frames[s_activeFrameIndex].len);
                s_engState = ENG_WAIT_RX;
            }
        }
        return;
    }

    // WAIT_TX
    if (s_engState == ENG_WAIT_TX)
    {
        if (flags & LIN_F_TX_DONE)
        {
            engine_done(true);
            engine_reset_to_idle();
        }
        return;
    }

    // WAIT_RX
    if (s_engState == ENG_WAIT_RX)
    {
        if (flags & LIN_F_RX_DONE)
        {
            engine_done(true);
            engine_reset_to_idle();
        }
        return;
    }
}

// ============================================================
// dispatcher (CMD > user STATUS > periodic STATUS)
// ============================================================
static bool dispatcher_make_next_request(lin_req_t *outReq, uint32_t nowTick, int32_t *outSchedIndex)
{
    *outSchedIndex = -1;
    memset(outReq, 0, sizeof(*outReq));

    if (s_cmdPending)
    {
        s_cmdPending = false;

        memset(s_txCmd, 0, sizeof(s_txCmd));
        s_txCmd[0] = s_cmdTarget;

        outReq->type = REQ_TYPE_CMD;
        outReq->src  = s_cmdSrc;
        outReq->frameIndex = FRAME_IDX_CMD;
        outReq->maxRetry = 1U;
        outReq->retryCount = 0U;
        outReq->deadlineTick = nowTick + 300U; // 150ms
        return true;
    }

    if (s_userStatusPending)
    {
        s_userStatusPending = false;

        outReq->type = REQ_TYPE_STATUS;
        outReq->src  = s_userStatusSrc;
        outReq->frameIndex = FRAME_IDX_STATUS;
        outReq->maxRetry = 2U;
        outReq->retryCount = 0U;
        outReq->deadlineTick = nowTick + 400U; // 200ms
        return true;
    }

    int32_t si = scheduler_pick_due(nowTick);
    if (si >= 0)
    {
        *outSchedIndex = si;

        outReq->type = REQ_TYPE_STATUS;
        outReq->src  = REQ_SRC_PERIODIC;
        outReq->frameIndex = s_sched[si].frameIndex;
        outReq->maxRetry = 1U;
        outReq->retryCount = 0U;
        outReq->deadlineTick = nowTick + 400U;
        return true;
    }

    return false;
}

void lin_engine_poll(uint32_t nowTick)
{
    if (s_engState != ENG_IDLE) return;

    lin_req_t req;
    int32_t schedIndex = -1;

    if (dispatcher_make_next_request(&req, nowTick, &schedIndex))
    {
        if (engine_start_request(&req))
        {
            if (schedIndex >= 0)
                scheduler_on_fired((uint32_t)schedIndex, nowTick);
        }
    }
}

// ============================================================
// requests API
// ============================================================
void lin_engine_request_cmd(req_source_t src, uint8_t target)
{
    if (target > MOTOR_TARGET_STUCK) target = MOTOR_TARGET_STUCK;

    s_cmdSrc = src;
    s_cmdTarget = target;
    s_cmdPending = true;
}

void lin_engine_request_status(req_source_t src)
{
    s_userStatusSrc = src;
    s_userStatusPending = true;
}

// ============================================================
// init
// ============================================================
void lin_engine_init(lin_engine_done_cb_t done_cb, uint32_t nowTick)
{
    s_done_cb = done_cb;

    s_cmdPending = false;
    s_userStatusPending = false;

    s_linFlags = 0U;
    s_lastEvent = 0U;
    s_linErrCode = 0U;

    s_engState = ENG_IDLE;
    memset((void *)s_txCmd, 0, sizeof(s_txCmd));
    memset((void *)s_rxStatus, 0, sizeof(s_rxStatus));

    scheduler_init(nowTick);
    engine_reset_to_idle();
}
