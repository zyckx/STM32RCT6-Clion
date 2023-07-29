//
// Created by Galaxy on 2023/7/29.
//
#include "delay.h"
#include "mpuiic.h"

//MPU IIC 延时函数
void MPU_IIC_Delay(void) {
    delay_us(2);
}

//初始化IIC
void MPU_IIC_Init(void) {
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();            //开启GPIOB时钟

    GPIO_Initure.Pin = GPIO_PIN_6 | GPIO_PIN_7;    //PB6、7
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;    //推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;            //上拉
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;    //高速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    MPU_IIC_SCL = 1;
    MPU_IIC_SDA = 1;
}

//产生IIC起始信号
void MPU_IIC_Start(void) {
    MPU_SDA_OUT();     //sda线输出
    MPU_IIC_SDA = 1;
    MPU_IIC_SCL = 1;
    MPU_IIC_Delay();
    MPU_IIC_SDA = 0;//START:when CLK is high,DATA change form high to low
    MPU_IIC_Delay();
    MPU_IIC_SCL = 0;//钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void MPU_IIC_Stop(void) {
    MPU_SDA_OUT();//sda线输出
    MPU_IIC_SCL = 0;
    MPU_IIC_SDA = 0;//STOP:when CLK is high DATA change form low to high
    MPU_IIC_Delay();
    MPU_IIC_SCL = 1;
    MPU_IIC_SDA = 1;//发送I2C总线结束信号
    MPU_IIC_Delay();
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 MPU_IIC_Wait_Ack(void) {
    u8 ucErrTime = 0;
    MPU_SDA_IN();      //SDA设置为输入
    MPU_IIC_SDA = 1;
    MPU_IIC_Delay();
    MPU_IIC_SCL = 1;
    MPU_IIC_Delay();
    while (MPU_READ_SDA) {
        ucErrTime++;
        if (ucErrTime > 250) {
            MPU_IIC_Stop();
            return 1;
        }
    }
    MPU_IIC_SCL = 0;//时钟输出0
    return 0;
}

//产生ACK应答
void MPU_IIC_Ack(void) {
    MPU_IIC_SCL = 0;
    MPU_SDA_OUT();
    MPU_IIC_SDA = 0;
    MPU_IIC_Delay();
    MPU_IIC_SCL = 1;
    MPU_IIC_Delay();
    MPU_IIC_SCL = 0;
}

//不产生ACK应答
void MPU_IIC_NAck(void) {
    MPU_IIC_SCL = 0;
    MPU_SDA_OUT();
    MPU_IIC_SDA = 1;
    MPU_IIC_Delay();
    MPU_IIC_SCL = 1;
    MPU_IIC_Delay();
    MPU_IIC_SCL = 0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void MPU_IIC_Send_Byte(u8 txd) {
    u8 t;
    MPU_SDA_OUT();
    MPU_IIC_SCL = 0;//拉低时钟开始数据传输
    for (t = 0; t < 8; t++) {
        MPU_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        MPU_IIC_SCL = 1;
        MPU_IIC_Delay();
        MPU_IIC_SCL = 0;
        MPU_IIC_Delay();
    }
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 MPU_IIC_Read_Byte(unsigned char ack) {
    unsigned char i, receive = 0;
    MPU_SDA_IN();//SDA设置为输入
    for (i = 0; i < 8; i++) {
        MPU_IIC_SCL = 0;
        MPU_IIC_Delay();
        MPU_IIC_SCL = 1;
        receive <<= 1;
        if (MPU_READ_SDA)receive++;
        MPU_IIC_Delay();
    }
    if (!ack)
        MPU_IIC_NAck();//发送nACK
    else
        MPU_IIC_Ack(); //发送ACK
    return receive;
}





