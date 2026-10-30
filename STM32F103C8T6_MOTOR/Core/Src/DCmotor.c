#include "DCmotor.h"
#include "tim.h"
#include "modbus.h"

// TB6612 引脚定义
#define AIN1_Pin    GPIO_PIN_0
#define AIN1_Port   GPIOB
#define AIN2_Pin    GPIO_PIN_1
#define AIN2_Port   GPIOB



void Motor_init()
{
	// 开启PWMA
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	
}


// 电机正转
void Motor_Forward(void)
{
    HAL_GPIO_WritePin(AIN1_Port, AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN2_Port, AIN2_Pin, GPIO_PIN_RESET);
}

// 电机反转
void Motor_Backward(void)
{
    HAL_GPIO_WritePin(AIN1_Port, AIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AIN2_Port, AIN2_Pin, GPIO_PIN_SET);
}

// 电机刹车（强力停止）
void Motor_Brake(void)
{
    HAL_GPIO_WritePin(AIN1_Port, AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(AIN2_Port, AIN2_Pin, GPIO_PIN_SET);
}

// 电机停止（自由停止）
void Motor_Stop(void)
{
    HAL_GPIO_WritePin(AIN1_Port, AIN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AIN2_Port, AIN2_Pin, GPIO_PIN_RESET);
}




// 速度：0~100
void Motor_SetSpeed(uint8_t speed)
{
    if(speed > 100) speed = 100;

    // 自动重装载值ARR，假设你的TIM4是10kHz PWM，arr=999
    uint16_t arr = 999;
    uint16_t ccr = (speed * arr) / 100;

    // 设置 TIM4 通道1 占空比
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ccr);
}



// 模式：1=正转  2=反转  0=停止
// speed：0~100
void Motor_Run(int8_t mode, uint8_t speed)
{
    if(mode == 1)
    {
        Motor_Forward();   // 正转
        Motor_SetSpeed(speed);
    }
    else if(mode == 2)
    {
        Motor_Backward();  // 反转
        Motor_SetSpeed(speed);
    }
    else
    {
        Motor_Stop();      // 停止
    }
}






// ===================== PID 最终配置 =====================
#define PWM_MAX        99      // 全速
#define PWM_MIN        0
#define MOTOR_DEAD_ZONE 22     // 电机启动死区

//// 高速、强响应、不震荡参数
//float Kp = 0.5f;
//float Ki = 0.15f;
//float Kd = 0.05f;     //D = P * 0.05


//float Kp = 0.35f;  // 降30%，先把超调压下去
//float Ki = 0.08f;  // 减半，消除积分震荡
//float Kd = 0.08f;  // 提升60%，增加阻尼


float Kp = 0.40f;  // 从 0.35 → 0.40，提升响应速度
float Ki = 0.08f;  // 保持不变，消除静差
float Kd = 0.09f;  // 从 0.08 → 0.09，加一点阻尼防止超调




float pwm_out = 0;
float err = 0;
float last_err = 0;
float last_last_err = 0;



// 带死区补偿的增量式PID
float PID_Inc_Calc(uint8_t target, uint8_t real)
{
    err = target - real;

    // 真正增量式公式
    float output = Kp * (err - last_err)
                 + Ki * err
                 + Kd * (err - 2 * last_err + last_last_err);

    last_last_err = last_err;
    last_err = err;

    // 累加增量
    pwm_out += output;

    // 限幅
    if(pwm_out > PWM_MAX) pwm_out = PWM_MAX;
    if(pwm_out < PWM_MIN) pwm_out = PWM_MIN;

    // 低速死区
//    if(target > 0 && target < 50 && pwm_out < MOTOR_DEAD_ZONE)
//        pwm_out = MOTOR_DEAD_ZONE;

    return pwm_out;
}

// 放在定时器100ms中断 或者 主循环精确定时
void Motor_PID_Loop(void)
{
 

    //1. 电机停止直接清PID
    if(motor_cmd == 0)
    {
        pwm_out = 0;
        err = 0;
        last_err = 0;
        last_last_err = 0;
        Motor_Stop();
        return;
    }
		
		// 1. 先读编码器实际转速
    motor_real_speed = get_BMQ370_encode_delte();

    // 3. PID计算目标转速→实际转速
    pwm_out = PID_Inc_Calc(motor_target_speed, motor_real_speed);
		
			char str[32] = {0};
	
	sprintf(str, "pwm:%05.2f", pwm_out);		
	oled_show_string(0, 6, str);

    // 4. 控制方向 + 设置PWM速度
    if(motor_cmd == 1)
    {
        Motor_Forward();
    }
    else if(motor_cmd == 2)
    {
        Motor_Backward();
    }

    // 把PID输出设置到TB6612的PWM
    //Motor_SetSpeed((uint8_t)pid_out);
		Motor_Run(motor_cmd, (uint8_t)pwm_out);
}






























