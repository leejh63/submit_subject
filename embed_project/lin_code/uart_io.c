// uart_io.c
#include "sdk_project_config.h"
#include "uart_io.h"

#include <string.h>

// ============================================================
// UART config
// ============================================================
#define UART_INST             (INST_LPUART_1)

// ============================================================
// RX chunk + RX ring, TX ring
// ============================================================
#define UART_RX_CHUNK_SIZE    (1u)       //
#define UART_RX_RING_SIZE     (256u)
#define UART_TX_RING_SIZE     (512u)

static uint8_t       s_rx_chunk[UART_RX_CHUNK_SIZE];
static volatile bool s_rx_armed = false;

static volatile uint8_t  s_rx_ring[UART_RX_RING_SIZE];
static volatile uint16_t s_rx_w = 0u;
static volatile uint16_t s_rx_r = 0u;

static volatile uint8_t  s_tx_ring[UART_TX_RING_SIZE];
static volatile uint16_t s_tx_w = 0u;
static volatile uint16_t s_tx_r = 0u;

static volatile bool     s_tx_busy = false;
static volatile uint16_t s_tx_inflight = 0u;

static inline uint16_t rb_next_u16(uint16_t x, uint16_t mod)
{
    return (uint16_t)((x + 1u) % mod);
}

// ============================================================
// RX ring
// ============================================================
static void rx_ring_push(uint8_t b)
{
    uint16_t nw = rb_next_u16(s_rx_w, UART_RX_RING_SIZE);
    if (nw == s_rx_r) return; // overflow drop
    s_rx_ring[s_rx_w] = b;
    s_rx_w = nw;
}

int uart_read_byte(uint8_t *out)
{
    if (out == NULL) return 0;
    if (s_rx_r == s_rx_w) return 0;

    *out = s_rx_ring[s_rx_r];
    s_rx_r = rb_next_u16(s_rx_r, UART_RX_RING_SIZE);
    return 1;
}

// ============================================================
// TX ring
// ============================================================
static void tx_ring_push(uint8_t b)
{
    uint16_t nw = rb_next_u16(s_tx_w, UART_TX_RING_SIZE);
    if (nw == s_tx_r) return; // overflow drop
    s_tx_ring[s_tx_w] = b;
    s_tx_w = nw;
}

static uint16_t tx_ring_count(void)
{
    if (s_tx_w >= s_tx_r) return (uint16_t)(s_tx_w - s_tx_r);
    return (uint16_t)(UART_TX_RING_SIZE - (s_tx_r - s_tx_w));
}

static uint16_t tx_ring_contig_count(void)
{
    if (s_tx_w >= s_tx_r) return (uint16_t)(s_tx_w - s_tx_r);
    return (uint16_t)(UART_TX_RING_SIZE - s_tx_r);
}

void uart_write_char(char c)
{
    tx_ring_push((uint8_t)c);
}

void uart_write_str(const char *s)
{
    if (s == NULL) return;
    while (*s) uart_write_char(*s++);
}

// ============================================================
// TX progress
// ============================================================
static void tx_poll_complete(void)
{
    if (!s_tx_busy) return;

    uint32_t rem = 0u;
    status_t st = LPUART_DRV_GetTransmitStatus(UART_INST, &rem);

    if (st == STATUS_SUCCESS)
    {
        s_tx_r = (uint16_t)((s_tx_r + s_tx_inflight) % UART_TX_RING_SIZE);
        s_tx_inflight = 0u;
        s_tx_busy = false;
    }
    else if (st != STATUS_BUSY)
    {
        // error/abort: drop inflight
        s_tx_r = (uint16_t)((s_tx_r + s_tx_inflight) % UART_TX_RING_SIZE);
        s_tx_inflight = 0u;
        s_tx_busy = false;
    }
}

static void tx_drain_step(void)
{
    if (s_tx_busy) return;

    uint16_t cnt = tx_ring_count();
    if (cnt == 0u) return;

    uint32_t rem = 0u;
    if (LPUART_DRV_GetTransmitStatus(UART_INST, &rem) == STATUS_BUSY)
    {
        s_tx_busy = true;
        return;
    }

    uint16_t contig = tx_ring_contig_count();
    uint16_t sendLen = contig;
    if (sendLen > 64u) sendLen = 64u;

    status_t st = LPUART_DRV_SendData(UART_INST, (uint8_t *)&s_tx_ring[s_tx_r], sendLen);
    if (st == STATUS_SUCCESS)
    {
        s_tx_inflight = sendLen;
        s_tx_busy = true;
    }
}

// ============================================================
// RX progress
// ============================================================
static void rx_arm_chunk(void)
{
    if (s_rx_armed) return;

    status_t st = LPUART_DRV_ReceiveData(UART_INST, s_rx_chunk, UART_RX_CHUNK_SIZE);
    if (st == STATUS_SUCCESS)
        s_rx_armed = true;
}

static void rx_collect_and_rearm(void)
{
    if (!s_rx_armed)
    {
        rx_arm_chunk();
        return;
    }

    uint32_t rem = 0u;
    status_t st = LPUART_DRV_GetReceiveStatus(UART_INST, &rem);

    if (st == STATUS_SUCCESS)
    {
        for (uint32_t i = 0; i < UART_RX_CHUNK_SIZE; i++)
            rx_ring_push(s_rx_chunk[i]);

        s_rx_armed = false;
        rx_arm_chunk();
    }
    else if (st == STATUS_BUSY)
    {
        // still receiving...
    }
    else
    {
        (void)LPUART_DRV_AbortReceivingData(UART_INST);
        s_rx_armed = false;
        rx_arm_chunk();
    }
}

// ============================================================
// Public API
// ============================================================
void uart_io_init(void)
{
    (void)LPUART_DRV_Init(UART_INST, &lpUartState1, &lpuart_1_InitConfig0);

    s_rx_w = s_rx_r = 0u;
    s_tx_w = s_tx_r = 0u;

    s_tx_busy = false;
    s_tx_inflight = 0u;

    s_rx_armed = false;
    rx_arm_chunk();
}

void uart_io_poll(void)
{
    rx_collect_and_rearm();
    tx_poll_complete();
    tx_drain_step();
}
