#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "scheduler.h"
#include "global.h"
#include "button.h"
#include "software_timer.h"
#include "mode_controller.h"
#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "fsm_setting.h"
#include "traffic_light.h"
#include "led_7seg.h"

GPIO_TypeDef porta, portb;
uint32_t primask;
static uint32_t now;
static unsigned writes;
#define TRAFFIC (RED_LED_Pin|YELLOW_LED_Pin|GREEN_LED_Pin|RED_LED2_Pin|YELLOW_LED2_Pin|GREEN_LED2_Pin)
#define ENABLES (EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin)
uint32_t HAL_GetTick(void) { return now; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return (port->idr & pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *gpio)
{
    assert(port == GPIOA && gpio->Pin == (MODE_Pin|SET_Pin|SAVE_Pin));
    assert(gpio->Pull == GPIO_PULLUP);
}
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pins, GPIO_PinState level)
{
    assert(primask == 0); /* No GPIO/application work in scheduler ISR lock. */
    if (port == GPIOA && level == GPIO_PIN_RESET && pins != 0)
        assert((porta.odr & TRAFFIC) == TRAFFIC); /* Old colours off first. */
    if (port == GPIOB && (pins & 0x7f))
        assert((portb.odr & ENABLES) == ENABLES ||
               (level == GPIO_PIN_SET && (pins & ENABLES) == ENABLES)); /* Atomic blank is safe. */
    if (level == GPIO_PIN_SET) port->odr |= pins;
    else port->odr &= (uint16_t)~pins;
    if (port == GPIOA && (input_mode == INPUT_MODE_NORMAL || input_mode == INPUT_MODE_MANUAL))
        assert((porta.odr & GREEN_LED_Pin) || (porta.odr & GREEN_LED2_Pin));
    ++writes;
}
static void roads(uint16_t on)
{
    assert(((uint16_t)~porta.odr & TRAFFIC) == on);
}
static void display(unsigned left, unsigned right)
{
    static const unsigned code[] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0,0x10};
    unsigned digits[4] = {99,99,99,99};
    for (unsigned n = 0; n < 4; ++n) {
        now += 2;
        led_7seg_scan();
        unsigned active = ((unsigned)~portb.odr >> 7) & 15;
        assert(active && !(active & (active-1)));
        unsigned i = 0;
        while ((active >> i) != 1) ++i;
        unsigned d = 0;
        while (d < 10 && code[d] != (portb.odr & 0x7f)) ++d;
        assert(d < 10);
        digits[i] = d;
    }
    assert(digits[0]*10 + digits[1] == left);
    assert(digits[2]*10 + digits[3] == right);
}
static void blank(void)
{
    for (unsigned i = 0; i < 8; ++i) { ++now; led_7seg_scan(); }
    assert((portb.odr & 0x7ff) == 0x7ff);
}
static void init_app(unsigned red)
{
    input_mode = INPUT_MODE_NORMAL;
    set_pressed = save_pressed = 0;
    red_time = red; yellow_time = 2; green_time = 3;
    porta.odr = portb.odr = porta.idr = 0xffff;
    primask = 0; now = 0;
    button_init(); software_timer_init(); led_7seg_init();
    fsm_automatic_start(); fsm_automatic_run(); SCH_Init();
    assert(SCH_Add_Task(button_run,1,1));
    assert(SCH_Add_Task(software_timer_run,1,1));
    assert(SCH_Add_Task(mode_controller_run,1,1));
    assert(SCH_Add_Task(fsm_automatic_run,1,1));
    assert(SCH_Add_Task(fsm_manual_run,1,1));
    assert(SCH_Add_Task(fsm_setting_run,1,1));
}
static void ticks(unsigned count)
{
    while (count--) {
        for (unsigned i=0;i<10;++i) { ++now; led_7seg_scan(); }
        SCH_Update(); SCH_Dispatch_Tasks();
    }
}
static void press(uint16_t pins)
{
    porta.idr &= (uint16_t)~pins;
    ticks(3);
}
static void release(uint16_t pins)
{
    porta.idr |= pins;
    ticks(3);
}
static void click(uint16_t pin) { press(pin); release(pin); }

static char logbuf[1000];
static unsigned logcount;
static uint8_t interrupt_in_a;
static void A(void) {
    assert(!primask); logbuf[logcount++]='A';
    if (interrupt_in_a) { interrupt_in_a=0; SCH_Update(); }
}
static void B(void) { assert(!primask); logbuf[logcount++]='B'; }
static void C(void) { assert(!primask); logbuf[logcount++]='C'; }
static void test_scheduler(void)
{
    SCH_Init(); logcount=0;
    assert(SCH_Add_Task(A,3,3)); assert(SCH_Add_Task(B,7,7)); assert(SCH_Add_Task(C,5,5));
    for (unsigned t=1;t<=105;++t) {
        unsigned start=logcount;
        SCH_Update(); SCH_Dispatch_Tasks();
        if(t%3==0) assert(logbuf[start++]=='A');
        if(t%7==0) assert(logbuf[start++]=='B');
        if(t%5==0) assert(logbuf[start++]=='C');
        assert(start==logcount);
        if(t==1) assert(logcount==0);
        if(t==4) assert(logcount==1);
        if(t==6) assert(logbuf[logcount-1]=='A');
    }
    puts("scheduler A(3,3),B(7,7),C(5,5): tick1=- tick4=- tick6=A tick15=AC tick105=ABC");
    SCH_Init(); logcount=0;
    uint32_t a=SCH_Add_Task(A,0,0), b=SCH_Add_Task(B,0,0);
    assert(a && b && a!=b); SCH_Dispatch_Tasks();
    assert(logcount==2 && memcmp(logbuf,"AB",2)==0);
    for(unsigned i=0;i<10;++i) SCH_Update();
    SCH_Dispatch_Tasks(); assert(logcount==2);
    assert(!SCH_Delete_Task(a)); assert(!SCH_Add_Task(0,1,1));
    SCH_Init(); logcount=0; assert(SCH_Add_Task(C,2,0));
    SCH_Update(); SCH_Dispatch_Tasks(); assert(logcount==0);
    SCH_Update(); SCH_Dispatch_Tasks(); assert(logcount==1 && logbuf[0]=='C');
    SCH_Update(); SCH_Dispatch_Tasks(); assert(logcount==1);
    SCH_Init(); logcount=0;
    a=SCH_Add_Task(A,1,1); b=SCH_Add_Task(B,1,1);
    assert(SCH_Delete_Task(a)); assert(SCH_Add_Task(C,1,1));
    SCH_Update(); SCH_Dispatch_Tasks(); assert(memcmp(logbuf,"BC",2)==0);
    assert(SCH_Delete_Task(b));
    SCH_Init(); logcount=0;
    assert(SCH_Add_Task(A,0,1)); assert(SCH_Add_Task(B,0,1)); assert(SCH_Add_Task(C,0,1));
    for(unsigned i=0;i<3;++i) SCH_Update();
    for(unsigned i=0;i<4;++i) SCH_Dispatch_Tasks();
    assert(logcount==12 && memcmp(logbuf,"ABCABCABCABC",12)==0);
    SCH_Init(); logcount=0; interrupt_in_a=1;
    assert(SCH_Add_Task(A,0,1)); assert(SCH_Add_Task(B,1,1));
    SCH_Dispatch_Tasks(); assert(logcount==1 && logbuf[0]=='A');
    SCH_Dispatch_Tasks(); assert(logcount==3 && memcmp(logbuf,"AAB",3)==0);
    SCH_Init();
    for(unsigned i=0;i<SCH_MAX_TASKS;++i) assert(SCH_Add_Task(A,1,1));
    assert(!SCH_Add_Task(A,1,1));
    primask=1; assert(SCH_Delete_Task(1)); assert(primask==1); primask=0;
    puts("scheduler: immediate/one-shot/periodic/delete/reinsert/capacity/backlog/ISR FIFO PASS");
}
static void test_automatic(void)
{
    for (unsigned red=5;red<=8;++red) {
        init_app(red);
        unsigned clearance=(red-5)*50;
        unsigned cycle=1000+2*clearance;
        int red_started[2]={-1,-1};
        for(unsigned t=0;t<cycle*4;++t) {
            unsigned p=t%cycle;
            if(p<300) { assert(traffic_phase==ROAD1_GREEN); roads(GREEN_LED_Pin|RED_LED2_Pin); }
            else if(p<500) { assert(traffic_phase==ROAD1_YELLOW); roads(YELLOW_LED_Pin|RED_LED2_Pin); }
            else if(p<500+clearance) { assert(traffic_phase==ALL_RED_TO_ROAD2); roads(RED_LED_Pin|RED_LED2_Pin); }
            else if(p<800+clearance) { assert(traffic_phase==ROAD2_GREEN); roads(RED_LED_Pin|GREEN_LED2_Pin); }
            else if(p<1000+clearance) { assert(traffic_phase==ROAD2_YELLOW); roads(RED_LED_Pin|YELLOW_LED2_Pin); }
            else { assert(traffic_phase==ALL_RED_TO_ROAD1); roads(RED_LED_Pin|RED_LED2_Pin); }
            for (unsigned road=0;road<2;++road) {
                unsigned pin=road==0 ? RED_LED_Pin : RED_LED2_Pin;
                if (!(porta.odr & pin)) {
                    if (red_started[road]<0 && t>0) red_started[road]=(int)t;
                } else if(red_started[road]>=0) {
                    /* Ignore the partial red interval at boot on road2. */
                    if(road==0 || (unsigned)red_started[road]>1)
                        assert(t-(unsigned)red_started[road]==red*100);
                    red_started[road]=-1;
                }
            }
            ticks(1);
        }
        assert(traffic_phase==ROAD1_GREEN && timerRemaining(TIMER_AUTO_PHASE)==3000);
    }
    init_app(5); display(3,5); ticks(100); display(2,4); ticks(200); display(2,2);
    ticks(200); display(5,3); ticks(300); display(2,2); ticks(200); display(3,5);
    init_app(7); display(3,6); ticks(300); display(2,3); ticks(200); display(7,1);
    ticks(100); display(6,3); ticks(300); display(3,2); ticks(200); display(1,7);
    ticks(100); display(3,6);
    init_app(5); ticks(360000); /* One hour, exact 10 s cycles, no phase drift. */
    assert(traffic_phase==ROAD1_GREEN && timerRemaining(TIMER_AUTO_PHASE)==3000);
    init_app(99); ticks(500); display(99,47); /* Maximum red countdown fits two digits. */
    puts("automatic: all six phases, clearance 0/500/1000/1500 ms, countdown, 1 h drift PASS");
}
static void test_buttons(void)
{
    init_app(5);
    porta.idr &= (uint16_t)~MODE_Pin; ticks(1);
    porta.idr |= MODE_Pin; ticks(1);
    porta.idr &= (uint16_t)~MODE_Pin; ticks(2);
    assert(input_mode==INPUT_MODE_NORMAL);
    ticks(1); assert(input_mode==INPUT_MODE_RED);
    ticks(100); assert(input_mode==INPUT_MODE_RED); /* Held: one event only. */
    release(MODE_Pin); click(MODE_Pin); assert(input_mode==INPUT_MODE_YELLOW);
    init_app(5); press(MODE_Pin|SET_Pin|SAVE_Pin);
    assert(input_mode==INPUT_MODE_RED && red_time==5); display(2,5);
    release(MODE_Pin|SET_Pin|SAVE_Pin);
    assert(button_take_events()==0);
    puts("buttons: 3 stable samples, bounce, hold, simultaneous MODE priority, one consumer PASS");
}
static void test_setting(void)
{
    init_app(5); press(MODE_Pin);
    assert(input_mode==INPUT_MODE_RED); display(2,5); roads(RED_LED_Pin|RED_LED2_Pin);
    ticks(24); roads(RED_LED_Pin|RED_LED2_Pin);
    ticks(1); roads(0); ticks(25); roads(RED_LED_Pin|RED_LED2_Pin);
    release(MODE_Pin); click(SET_Pin); click(SAVE_Pin);
    assert(red_time==6); display(2,6);
    click(MODE_Pin); assert(input_mode==INPUT_MODE_YELLOW); display(3,2);
    click(SET_Pin); click(SAVE_Pin); assert(yellow_time==3);
    click(MODE_Pin); assert(input_mode==INPUT_MODE_GREEN); display(4,3);
    click(SET_Pin); press(SAVE_Pin);
    assert(green_time==3); display(4,0);
    ticks(99); display(4,0); ticks(1); display(4,4);
    release(SAVE_Pin); click(MODE_Pin); assert(input_mode==INPUT_MODE_NORMAL);
    assert(traffic_phase==ROAD1_GREEN);
    init_app(99); click(MODE_Pin); click(SET_Pin); display(2,1);
    press(SAVE_Pin); assert(red_time==99); display(2,0);
    init_app(5); click(MODE_Pin); click(MODE_Pin);
    click(SET_Pin); press(SAVE_Pin); assert(yellow_time==2); display(3,0);
    /* Every candidate combination is checked independently of GPIO. */
    for(unsigned r=1;r<=99;++r)
        for(unsigned y=1;y<=99;++y)
            for(unsigned g=1;g<=99;++g)
                assert(durations_valid(r,y,g)==(r>=y+g));
    assert(!durations_valid(0,1,1)); assert(!durations_valid(100,1,1));
    puts("setting: 02/03/04, 500 ms blink, wrap99->1, valid/invalid SAVE, 1 s error PASS");
}
static void test_manual(void)
{
    const unsigned offsets[]={0,300,500,600,900,1100};
    const ManualState states[]={MAN_ROAD1_GREEN,MAN_ROAD1_YELLOW,MAN_ALL_RED_TO_ROAD2,
        MAN_ROAD2_GREEN,MAN_ROAD2_YELLOW,MAN_ALL_RED_TO_ROAD1};
    for(unsigned i=0;i<6;++i) {
        init_app(7); ticks(offsets[i]); uint16_t before=porta.odr & TRAFFIC;
        press(SAVE_Pin); assert(input_mode==INPUT_MODE_MANUAL && manual_state==states[i]);
        assert((porta.odr & TRAFFIC)==before); blank(); release(SAVE_Pin);
        press(MODE_Pin); assert(input_mode==INPUT_MODE_NORMAL);
        if(i==0 || i==3) assert(timerRemaining(TIMER_AUTO_PHASE)==3000);
        if(i==1 || i==4) assert(timerRemaining(TIMER_AUTO_PHASE)==2000);
        if(i==2 || i==5) { roads(RED_LED_Pin|RED_LED2_Pin); ticks(100); }
    }
    /* SAVE becomes debounced exactly when Automatic's green timer expires:
     * Mode Controller must preserve the green that was actually displayed.
     */
    init_app(5); ticks(297); press(SAVE_Pin);
    assert(manual_state==MAN_ROAD1_GREEN); roads(GREEN_LED_Pin|RED_LED2_Pin);
    release(SAVE_Pin); press(MODE_Pin);
    assert(traffic_phase==ROAD1_GREEN && timerRemaining(TIMER_AUTO_PHASE)==3000);
    init_app(5); click(SAVE_Pin); blank();
    click(SET_Pin); assert(manual_state==MAN_ROAD1_YELLOW); blank();
    click(SET_Pin); assert(manual_state==MAN_ROAD2_GREEN); blank();
    click(SET_Pin); assert(manual_state==MAN_ROAD2_YELLOW); blank();
    click(SET_Pin); assert(manual_state==MAN_ROAD1_GREEN); blank();
    ticks(996); assert(input_mode==INPUT_MODE_MANUAL);
    ticks(1); assert(input_mode==INPUT_MODE_NORMAL);
    assert(traffic_phase==ROAD1_GREEN && timerRemaining(TIMER_AUTO_PHASE)==3000);
    init_app(7); ticks(500); click(SAVE_Pin); click(SET_Pin);
    assert(manual_state==MAN_ALL_RED_TO_ROAD2); roads(RED_LED_Pin|RED_LED2_Pin);
    ticks(100); click(SET_Pin); assert(manual_state==MAN_ROAD2_GREEN);
    press(MODE_Pin); assert(traffic_phase==ROAD2_GREEN); roads(RED_LED_Pin|GREEN_LED2_Pin);
    init_app(7); ticks(1100); click(SAVE_Pin); ticks(1000);
    assert(input_mode==INPUT_MODE_NORMAL && traffic_phase==ROAD1_GREEN);
    init_app(5); press(SAVE_Pin); release(SAVE_Pin); ticks(997);
    assert(input_mode==INPUT_MODE_NORMAL); /* SAVE entry did not get reused. */
    puts("manual: preserve all phases, SET cycle, blank4digits, 10 s timeout, MODE, all-red guard PASS");
}
static void test_scan(void)
{
    init_app(5); unsigned before=writes;
    led_7seg_scan(); ++now; led_7seg_scan(); assert(writes==before);
    ++now; led_7seg_scan(); assert(writes>before);
    for(unsigned d=0;d<10;++d) { led_7seg_show(d*11,d*11); display(d*11,d*11); }
    led_7seg_blank(); blank();
    now=0xfffffffeu; led_7seg_init(); led_7seg_show(12,34);
    now=0; led_7seg_scan(); assert((portb.odr & ENABLES)!=ENABLES);
    display(12,34);
    setTimer(TIMER_ERROR,0); assert(isTimerExpired(TIMER_ERROR));
    setTimer(TIMER_ERROR,11); software_timer_run(); assert(!isTimerExpired(TIMER_ERROR));
    software_timer_run(); assert(isTimerExpired(TIMER_ERROR));
    puts("display/timer: active LOW patterns 0..9, 2 ms/digit, blank, tick wrap, zero timer PASS");
}
int main(void)
{
    test_scheduler(); test_buttons(); test_automatic(); test_setting(); test_manual(); test_scan();
    puts("ALL LAB4 HOST TESTS PASSED (mock HAL; not a Proteus/hardware test)");
    return 0;
}
