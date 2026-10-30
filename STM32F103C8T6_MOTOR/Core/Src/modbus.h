#ifndef  _MODBUS_H_
#define  _MODBUS_H_

#include "main.h"

#define MODBUS_ADDR    0x01    // 从机地址
#define BUF_MAX_LEN       64      // 接收缓冲区
#define MODBUS_TIMEOUT     5    // 5ms 帧间隔超时


extern uint8_t  modbus_rx_buf[BUF_MAX_LEN];
extern uint16_t modbus_rx_len;
extern uint32_t modbus_rx_tick;

// 电机控制变量（Modbus 控制这两个）
extern uint8_t  motor_target_speed;     // 0~100
extern int16_t  motor_real_speed;
extern uint8_t  motor_cmd;     // 0=停止 1=正转 2=反转

uint8_t Modbus_Parse(void);


#endif