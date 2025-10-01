/**
 * @file key.cpp
 * @brief  按键检测模块
 * @author  Tatkira
 * @version 1.1
 * @date    2025-05-27
 * @copyright Copyright (c) 2025
 * 函数列表：
 *
 * 1. Bin_Image_Filter()
 *
 * @par 修改日志
 * <table>
 * <tr><th>日期           <th>版本               <th>作者          <th>修改内容
 * <tr><td>2025-05-26    <td>1.1               <th>Tatkira       <th> 添加一键起跑
 *
 */

#include "key.h"

bool isKey3Pressed() {
    return gpio_get_level(KEY_3) == 0; // 假设低电平表示按下
}

void startKeyControl() {
    while (1) {
        if (isKey3Pressed()) {
            system_delay_ms(50); // 消抖延时
            if (isKey3Pressed()) {
                binary_ipsdrawline(); // 触发视觉巡线
            }
        }

        system_delay_ms(10); // 防止 CPU 占用过高
    }
}