#include "encode.h"
#include "tim.h"
#include "oled.h"

static uint16_t last_cnt = 0;


void init_ec11_encode_tim()
{
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	
}



int16_t get_es11_encode_delte()
{
	uint16_t now_cnt = __HAL_TIM_GET_COUNTER(&htim2);
	int16_t delta = 0;
	char str[32] = {0};
	
	sprintf(str, "cnt:%05d", now_cnt);		
	oled_show_string(0, 2, str);
	

	delta = now_cnt - last_cnt;


	
	last_cnt = now_cnt;
	
	return delta;

}



void init_BMQ370_encode_tim()
{
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	
}



int16_t get_BMQ370_encode_delte()
{
	uint16_t now_cnt = __HAL_TIM_GET_COUNTER(&htim3);
	int16_t delta = 0;
	char str[32] = {0};
	
	sprintf(str, "cnt:%05d", now_cnt);		
	oled_show_string(0, 4, str);
	

	delta = (int16_t)(now_cnt - last_cnt);
	last_cnt = now_cnt;
	
	return abs(delta);

}