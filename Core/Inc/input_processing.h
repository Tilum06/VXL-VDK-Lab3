#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include "global.h"

void input_processing_init(void);
/* Mode controller and exclusive dispatch to Automatic/Setting/Manual. */
void fsm_for_input_processing(void);

#endif /* INC_INPUT_PROCESSING_H_ */
