#ifndef _SG90_H_
#define _SG90_H_




#include "main.h"



void set_duty(uint16_t duty);
void set_SG90_angle(uint8_t angle, uint8_t ch);

void SG90_SmoothTurn(uint8_t target_angle, uint16_t delay_ms);
void SG90_SmoothTurn_S_Curve(uint8_t target_angle, uint16_t total_time_ms);
void SG90_Trapezoid(uint8_t angleStart, uint8_t angleEnd, 
                    uint16_t t_acc, uint16_t t_dec, uint16_t stepDelay);























#endif