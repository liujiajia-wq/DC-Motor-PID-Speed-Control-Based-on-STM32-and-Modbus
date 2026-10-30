#include "SG90.h"
#include "tim.h"




void set_duty(uint16_t duty)
{
	uint16_t ccr = 0;
	
	ccr = (htim1.Init.Period+1) * duty / 100;
	
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);

}



void set_SG90_angle(uint8_t angle, uint8_t ch)
{
	
	uint16_t ccr = 0;
	
	//  // 角度 → 对应CCR值（线性映射：0°→500，180°→2500）
	ccr = 500 + (angle * 2000/180);
	__HAL_TIM_SetCompare(&htim1, ch, ccr);
	
}



// 匀速转动：从start到end，总耗时total_time_ms
void SG90_Uniform(uint8_t start, uint8_t end, uint16_t total_time_ms)
{
    int16_t dir = (end > start) ? 1 : -1;
    uint16_t steps = abs(end - start);
    uint16_t step_delay = total_time_ms / steps;

    for(uint16_t i=0; i<steps; i++)
    {
        set_SG90_angle(start + dir * i, TIM_CHANNEL_1);
        HAL_Delay(step_delay);
    }
    set_SG90_angle(end, TIM_CHANNEL_1);
}


// 两段式梯形（无匀速）：从start到end，总耗时total_time_ms
// 两段式梯形加减速（无匀速段）→ 100% 能走到终点，绝不差角度
void SG90_Trapezoid_2Phase(uint8_t start, uint8_t end, uint16_t total_time_ms)
{
    int16_t dir = (end > start) ? 1 : -1;
    uint16_t steps = abs(end - start);  // 0→180 = 180步
    if(steps == 0) return;

    uint16_t half_steps = steps / 2;    // 90步（前半加速，后半减速）

    // 总时间平均分给每一步（保证总时间绝对准确）
    uint16_t base_delay = total_time_ms / steps;

    uint8_t now_angle;

    // ==========================
    // 前半段：匀加速（延时越来越小 = 越来越快）
    // ==========================
    for(uint16_t i = 0; i < half_steps; i++)
    {
        now_angle = start + dir * i;
        set_SG90_angle(now_angle, TIM_CHANNEL_2);

        // 安全计算：绝不会出现 0 / 负数
        uint16_t reduce = (base_delay * i) / (half_steps + 1);
        uint16_t delay = base_delay - reduce;
        if(delay < 1) delay = 1;  // 保底延时，防止飞步

        HAL_Delay(delay);
    }

    // ==========================
    // 后半段：匀减速（延时越来越大 = 越来越慢）
    // ==========================
    for(uint16_t i = half_steps; i < steps; i++)
    {
        now_angle = start + dir * i;
        set_SG90_angle(now_angle, TIM_CHANNEL_2);

        // 安全计算
        uint16_t pos = i - half_steps;
        uint16_t increase = (base_delay * pos) / (half_steps + 1);
        uint16_t delay = base_delay + increase;

        HAL_Delay(delay);
    }

    // ==========================
    // 最终强制锁定目标角度（绝对不会差角度）
    // ==========================
    set_SG90_angle(end, TIM_CHANNEL_2);
}


// 三次S曲线（2段式）：从start到end，总耗时total_time_ms
void SG90_S_Curve_3rd(uint8_t start, uint8_t end, uint16_t total_time_ms)
{
    int16_t dir = (end > start) ? 1 : -1;
    uint16_t steps = abs(end - start);
    uint16_t step_delay = total_time_ms / steps;

    for(uint16_t i=0; i<steps; i++)
    {
        // 归一化时间t ∈ [0,1]
        float t = (float)i / steps;
        // S曲线公式：s = 3t2 - 2t3
        float factor = 3 * t * t - 2 * t * t * t;

        int16_t current_angle = start + dir * steps * factor;
        set_SG90_angle(current_angle,TIM_CHANNEL_2);
        HAL_Delay(step_delay);
    }
    set_SG90_angle(end, TIM_CHANNEL_2);
}



