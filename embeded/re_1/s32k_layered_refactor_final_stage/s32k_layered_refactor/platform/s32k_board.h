#ifndef S32K_BOARD_H
#define S32K_BOARD_H

#include "core/emb_result.h"

EmbResult S32kBoard_InitCommon(void);
EmbResult S32kBoard_InitLinSensorSlave(void);
EmbResult S32kBoard_InitMasterNode(void);
EmbResult S32kBoard_InitCanButtonSlave(void);

#endif
