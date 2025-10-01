/**
 * @file servo.cpp
 * @brief   舵机控制模块
 * @author  Tatkira
 * @version 1.1
 * @date    2025-05-26
 * @copyright Copyright (c) 2025
 * 函数列表：
 *
 * 1.void servo_init(ServoController *servo)
 * 2.void servo_set_angle(ServoController *servo, float angle)
 * 3.void servo_control_by_error(ServoController *servo, int error, int max_error)
 *
 * @par 修改日志
 * <table>
 * <tr><th>日期           <th> 版 本               <th> 作 者          <th> 修改内容
 * <tr><td>2025-05-26    <td> 1.1                <th> Tatkira        <th> 加入微分系数抑制误差
 *
 */

#include "zf_common_headfile.h"
// 计算舵机占空比
static uint16_t calculate_duty(float angle, const pwm_info_t *pwm_cfg) {
    return (uint16_t)((float)pwm_cfg->duty_max / (1000.0 / (float)pwm_cfg->freq) * (0.5 + (180-angle) / 90.0));
}

void servo_init(ServoController *servo) {
    // 获取PWM设备信息
    pwm_get_dev_info(SERVO_PWM_DEVICE, &servo->pwm_cfg);
    
    // 初始化舵机到中位
    servo->current_angle = SERVO_CENTER_ANGLE;
    pwm_set_duty(SERVO_PWM_DEVICE, calculate_duty(servo->current_angle, &servo->pwm_cfg));
    
//    printf("Servo initialized: Freq=%dHz, DutyMax=%d\n",
//           servo->pwm_cfg.freq, servo->pwm_cfg.duty_max);
}

void servo_set_angle(ServoController *servo, float angle) {
    // 限制角度范围
    if (angle < SERVO_LEFT_MAX) angle = SERVO_LEFT_MAX;
    if (angle > SERVO_RIGHT_MAX) angle = SERVO_RIGHT_MAX;
    
    // 更新角度并设置PWM
    servo->current_angle = angle;
    pwm_set_duty(SERVO_PWM_DEVICE, calculate_duty(angle, &servo->pwm_cfg));
}


void servo_control_by_error(ServoController *servo, int error, int max_error) {
    // 分段比例系数
    float kp_small = 1.1f;  // 小偏差时的比例系数
    float kp_large = 1.0f;  // 大偏差时的比例系数
    float kd_small = 0.15f;  // 小偏差时的微分系数
    float kd_large = 0.1f;  // 大偏差时的微分系数
    float threshold = max_error * 0.3f; // 阈值
/*
小弯道响应不足：

增大kp_small（每次增加0.1）

或减小threshold（每次减小5%）

急弯转不过去：

增大kp_large（每次增加0.05）

或增大threshold（每次增加5%）

直道震荡：

减小kp_small（每次减小0.1）

或增大threshold（每次增加5%）

急弯过冲：

减小kp_large（每次减小0.05）

或考虑微分项
*/

// ---- 计算微分项 ----
    float error_derivative = error - servo->last_error; // 误差变化率 = 当前误差 - 上次误差
    servo->last_error = error; // 更新为本次误差，供下次使用

    // ---- 动态选择Kp和Kd ----
    float kp, kd;
    if(fabs(error) < threshold) {
        // 小偏差区：高灵敏度比例 + 弱微分抑制
        kp = kp_small;
        kd = kd_small;
    } else {
        // 大偏差区：低灵敏度比例 + 强微分制动
        kp = kp_large;
        kd = kd_large;
    }

    // ---- 计算角度偏移（P + D）----
    float angle_offset;
    
    if(fabs(error) < threshold) {
        // 小偏差区域 - 更灵敏
        angle_offset = (float)error * (SERVO_RIGHT_MAX - SERVO_CENTER_ANGLE) / threshold * kp;
    } else {
        // 大偏差区域 - 更平缓
        angle_offset = copysignf((SERVO_RIGHT_MAX - SERVO_CENTER_ANGLE) * kp, error);
    }

    // 加入微分项（注意符号通常为负，根据实际效果调整）
    angle_offset -= kd * error_derivative; // D项抑制震荡, 可尝试将-变为+

    //  ---- 计算最终目标角度并设置舵机 ----
    float target_angle = SERVO_CENTER_ANGLE + angle_offset;
    servo_set_angle(servo, target_angle);
}

