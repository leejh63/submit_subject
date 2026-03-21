#ifndef HAL_DMA_H
#define HAL_DMA_H

#include <stddef.h>
#include <stdint.h>
#include "core/emb_status.h"

typedef struct hal_dma hal_dma_t;

typedef struct
{
    emb_status_t (*start_memcpy)(hal_dma_t *hal, void *dst, const void *src, size_t length);
    emb_status_t (*start_periph_to_mem)(hal_dma_t *hal, void *dst, const void *srcReg, size_t length);
    emb_status_t (*start_mem_to_periph)(hal_dma_t *hal, void *dstReg, const void *src, size_t length);
    emb_status_t (*abort)(hal_dma_t *hal, uint32_t channel);
} hal_dma_ops_t;

struct hal_dma
{
    const hal_dma_ops_t *ops;
    void *context;
};

#endif
