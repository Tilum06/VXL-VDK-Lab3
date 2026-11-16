#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include "global.h"

void input_processing_init(void);
/* Consume events once and coordinate mode transitions; no FSM run calls. */
void fsm_for_input_processing(void);
void Task_Button(void);
void Task_SoftwareTimer(void);
void Task_ModeController(void);
void Task_Automatic(void);
void Task_Manual(void);
void Task_Setting(void);

#endif /* INC_INPUT_PROCESSING_H_ */
