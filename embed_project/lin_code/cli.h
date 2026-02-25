// cli.h

#ifndef CLI_H_
#define CLI_H_


#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t help;
    uint8_t status;
    uint8_t open;
    uint8_t close;
    uint8_t stuck;
} cli_req_t;

void     cli_init(void);
void     cli_reset(void);

// RX 바이트를 CLI에 투입 (echo/backspace 포함)
void     cli_feed_byte(uint8_t b, cli_req_t *out);

// 현재 입력 중인 커맨드 길이(커서 복원용)
uint32_t cli_get_cmd_len(void);

#endif /* CLI_H_ */
