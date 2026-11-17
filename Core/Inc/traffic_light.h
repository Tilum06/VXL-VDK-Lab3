#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H
#include <stdint.h>
typedef enum {LIGHT_OFF, LIGHT_RED, LIGHT_YELLOW, LIGHT_GREEN} Light;
void traffic_light_set(Light road1, Light road2);
#endif
