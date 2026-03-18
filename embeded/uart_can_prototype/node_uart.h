#ifndef NODE_UART_H
#define NODE_UART_H

#include "uart_structs.h"
#include "ctrl_mailbox.h"
#include "ctrl_result_box.h"

typedef struct
{
    uint32_t                     instance;
    lpuart_state_t              *driverState;
    const lpuart_user_config_t  *userConfig;
    CtrlMailbox                 *ctrlMailbox;
    CtrlResultBox               *ctrlResultBox;
    uint8_t          			nodeId;
} NodeUartConfig;

status_t NodeUart_Init(NodeUartContext *node,
                       const NodeUartConfig *config);

void NodeUart_Task(NodeUartContext *node);
void NodeUart_RenderTask(NodeUartContext *node);
void NodeUart_Clear(NodeUartContext *node);

void NodeUart_SetTaskText(NodeUartContext *node, const char *text);
void NodeUart_SetResultText(NodeUartContext *node, const char *text);

status_t NodeUart_Recover(NodeUartContext *node);

#endif
