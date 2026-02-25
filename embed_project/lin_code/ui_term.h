// ui_term.h
#ifndef UI_TERM_H_
#define UI_TERM_H_

#pragma once

#include <stdint.h>
#include "ui_model.h"

void ui_init(void);
void ui_clear_input_line(void);

// 모델을 화면에 렌더링 + 커서 복원
void ui_render(const ui_model_t *m, uint32_t cmd_len);

// (선택) 커서 show/hide 같은 터미널 제어는 그대로 유지
void term_show_cursor(void);

#endif /* UI_TERM_H_ */
