#include "modbus.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>



uint8_t  modbus_rx_buf[BUF_MAX_LEN];
uint16_t modbus_rx_len = 0;
uint32_t modbus_rx_tick = 0;

// 电机控制变量（Modbus 控制这两个）
uint8_t  motor_target_speed = 0;     // 0~100
int16_t  motor_real_speed = 0; 
uint8_t  motor_cmd   = 0;     // 0=停止 1=正转 2=反转



uint16_t Modbus_CRC16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for(uint16_t i=0; i<len; i++)
    {
        crc ^= buf[i];
        for(uint8_t j=0; j<8; j++)
        {
            if(crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}

uint8_t Modbus_Parse(void)
{
    // 1. 帧未结束 → 不处理
    if(HAL_GetTick() - modbus_rx_tick < MODBUS_TIMEOUT)
        return -1;

    // 2. 长度太短 → 清空退出
    if(modbus_rx_len < 8)
    {
        modbus_rx_len = 0;
        return -1;
    }

    // 3. 取一帧数据，立即清零！！！（关键）
    uint8_t buf[BUF_MAX_LEN];
    uint16_t len = modbus_rx_len;
    memcpy(buf, modbus_rx_buf, len);
    modbus_rx_len = 0; // 收到立刻清零！！！

    // 4. 校验CRC
    uint16_t recv_crc = (buf[len-1] << 8) | buf[len-2];
    uint16_t calc_crc = Modbus_CRC16(buf, len-2);
    if(recv_crc != calc_crc) 
			return -1;

    // 5. 地址校验
    if(buf[0] != MODBUS_ADDR) 
			return -1;

   // ======================================================
		// 0x03 读寄存器 【完美适配上位机：目标速度 + 当前速度】
		// ======================================================
		if(buf[1] == 0x03 && len == 8)
		{
				uint16_t reg   = (buf[2] << 8) | buf[3];  // 起始寄存器
				uint16_t cnt   = (buf[4] << 8) | buf[5];  // 寄存器数量

				// --------------------------
				// 上位机要求：读 2 个寄存器（0 和 1）
				// 0 = 目标速度
				// 1 = 当前速度
				// --------------------------
				if(cnt == 2 && reg == 0)   
				{
						uint8_t resp[9];  // 固定 9 字节
						resp[0] = MODBUS_ADDR;
						resp[1] = 0x03;
						resp[2] = 0x04;  // 4字节数据（2个寄存器）

						// ===== 寄存器0：目标速度 =====
						resp[3] = (motor_target_speed >> 8) & 0xFF;
						resp[4] = motor_target_speed & 0xFF;

						// ===== 寄存器1：当前速度（PID反馈值）=====
						resp[5] = (motor_real_speed >> 8) & 0xFF;
						resp[6] = motor_real_speed & 0xFF;

						// CRC 校验
						uint16_t crc = Modbus_CRC16(resp, 7);
						resp[7] = crc & 0xFF;
						resp[8] = (crc >> 8) & 0xFF;

						// 发送回复
						HAL_UART_Transmit(&huart2, resp, 9, 10);
				}

				// --------------------------
				// 兼容旧指令：读单个寄存器
				// --------------------------
				else if(cnt == 1 && (reg == 0 || reg == 1))
				{
						uint8_t resp[8];
						resp[0] = MODBUS_ADDR;
						resp[1] = 0x03;
						resp[2] = 0x02;
						resp[3] = 0x00;

						if(reg == 0) resp[4] = motor_target_speed;
						else         resp[4] = motor_real_speed;

						uint16_t crc = Modbus_CRC16(resp, 5);
						resp[5] = crc & 0xFF;
						resp[6] = crc >> 8;

						HAL_UART_Transmit(&huart2, resp, 7, 10);
				}
		}

    // ======================================================
    // 0x06 写寄存器
    // ======================================================
    else if(buf[1] == 0x06 && len == 8)
    {
        uint16_t reg   = (buf[2] << 8) | buf[3];
        uint16_t val   = (buf[4] << 8) | buf[5];

        if(reg == 0)
        {
            if(val > 100) val = 100;
            motor_target_speed = val;
        }
        else if(reg == 1)
        {
            if(val > 2) val = 0;
            motor_cmd = val;
        }

        // 回传
        HAL_UART_Transmit(&huart2, buf, 8, 10);
    }
		
		return 0;
}


