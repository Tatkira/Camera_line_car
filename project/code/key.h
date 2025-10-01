#ifndef KEY_H
#define KEY_H

#include "zf_common_headfile.h"

#define KEY_3       "/dev/zf_driver_gpio_key_3"

// 声明按键检测函数
bool isKey3Pressed();  // 检测按键是否按下

// 启动按键控制逻辑，按下 KEY_3 触发视觉巡线
void startKeyControl();

#endif