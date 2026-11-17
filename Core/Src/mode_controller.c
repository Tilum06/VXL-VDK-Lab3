#include "mode_controller.h"
#include "global.h"
#include "button.h"
#include "software_timer.h"
#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "fsm_setting.h"

void mode_controller_run(void)
{
    uint8_t events = button_take_events();
    set_pressed = (events & BUTTON_SET) != 0;
    save_pressed = (events & BUTTON_SAVE) != 0;
    /* MODE wins simultaneous presses. Transition edges belong to the old
     * mode and must not be reused as commands by the destination FSM.
     */
    switch (input_mode) {
    case MODE_AUTO:
        if (events & BUTTON_MODE) {
            input_mode = MODE_SET_RED;
            fsm_setting_enter();
        } else if (save_pressed) {
            input_mode = MODE_MANUAL;
            fsm_manual_enter();
        } else return;
        break;
    case MODE_SET_RED:
        if (!(events & BUTTON_MODE)) return;
        input_mode = MODE_SET_YELLOW;
        fsm_setting_enter();
        break;
    case MODE_SET_YELLOW:
        if (!(events & BUTTON_MODE)) return;
        input_mode = MODE_SET_GREEN;
        fsm_setting_enter();
        break;
    case MODE_SET_GREEN:
        if (!(events & BUTTON_MODE)) return;
        input_mode = MODE_AUTO;
        fsm_automatic_start();
        break;
    case MODE_MANUAL:
        if (events & BUTTON_MODE) {
            input_mode = MODE_AUTO;
            fsm_automatic_resume();
        } else if (set_pressed || save_pressed) {
            setTimer(TIMER_MANUAL, 10000);
            return;
        } else if (isTimerExpired(TIMER_MANUAL)) {
            input_mode = MODE_AUTO;
            fsm_automatic_resume();
        } else return;
        break;
    }
    set_pressed = save_pressed = 0;
}
