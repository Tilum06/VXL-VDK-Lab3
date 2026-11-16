#ifndef INC_FSM_SETTING_H_
#define INC_FSM_SETTING_H_

#include "global.h"

void fsm_setting_enter(InputMode mode);
void fsm_setting_run(uint8_t set_edge, uint8_t save_edge);

#endif
