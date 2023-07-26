//
// Created by Galaxy on 2023/7/24.
//

#ifndef MAIN_DELAY_H
#define MAIN_DELAY_H
#include "sys.h"

void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
#endif //MAIN_DELAY_H
