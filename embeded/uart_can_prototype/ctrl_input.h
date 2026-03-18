#ifndef CTRL_INPUT_H
#define CTRL_INPUT_H

#include <stdint.h>

#include "status.h"
#include "ctrl_mailbox.h"

#define CTRL_INPUT_TEXT_SIZE 64U

typedef struct
{
    char text[CTRL_INPUT_TEXT_SIZE];
} CtrlInputResult;

typedef struct
{
    uint8_t  nodeState;
    uint8_t  uartErrorFlag;
    uint16_t rxLineLength;
    uint8_t  txBusy;

    uint8_t  cmdQueueCount;
    uint8_t  cmdQueueCapacity;

    uint8_t  resultQueueCount;
    uint8_t  resultQueueCapacity;
} CtrlInputSnapshot;

void CtrlInputResult_Clear(CtrlInputResult *result);

status_t CtrlInput_HandleLine(const char              *line,
                              const CtrlInputSnapshot *snapshot,
                              CtrlMailbox             *mailbox,
                              CtrlInputResult         *outResult);

#endif
