#ifndef _DCMOTOR_H_
#define _DCMOTOR_H_


#include "main.h"
void Motor_init();
void Motor_Run(int8_t mode, uint8_t speed);

void Motor_PID_Loop(void);
#endif