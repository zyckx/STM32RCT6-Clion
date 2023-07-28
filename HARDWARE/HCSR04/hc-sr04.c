//
// Created by Galaxy on 2023/7/28.
//

#include "hc-sr04.h"
#include "delay.h"
float HC_SR04_Read(void)
{
    uint32_t i = 0;
    float Distance;
    HAL_GPIO_WritePin(HC_SR04_Trig_GPIO_Port,HC_SR04_Trig_Pin,GPIO_PIN_SET);//输出15us高电平
    delay_us(15);
    HAL_GPIO_WritePin(HC_SR04_Trig_GPIO_Port,HC_SR04_Trig_Pin,GPIO_PIN_RESET);//高电平输出结束，设置为低电平

    while(HAL_GPIO_ReadPin(HC_SR04_Echo_GPIO_Port,HC_SR04_Echo_Pin) == GPIO_PIN_RESET)//等待回响高电平
    {
        i++;
        delay_us(1);
        if(i>100000) return -1;//超时退出循环、防止程序卡死这里
    }
    i = 0;
    while(HAL_GPIO_ReadPin(HC_SR04_Echo_GPIO_Port,HC_SR04_Echo_Pin) == GPIO_PIN_SET)//下面的循环是2us
    {
        i = i+1;
        delay_us(1);//1us 延时，但是整个循环大概2us左右
        if(i >100000) return -2;//超时退出循环
    }
    Distance = i*2*0.033/2;//这里乘2的原因是上面是2微妙
    return Distance	;
}