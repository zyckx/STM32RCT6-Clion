/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "w25qxx.h"
#include "oled.h"
#include "fontupd.h"
#include "text.h"
#include "gui.h"
#include "hc-sr04.h"
#include "MPU6050.h"
#include "inv_mpu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//串口1发送1个字符
//c:要发送的字符
void usart1_send_char(u8 c) {
    while (__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) == RESET) {}; //循环发送,直到发送完毕

    HAL_UART_Transmit(&UART1_Handler, &c, 1, 1000);
}

//传送数据给匿名四轴上位机软件(V2.6版本)
//fun:功能字. 0XA0~0XAF
//data:数据缓存区,最多28字节!!
//len:data区有效数据个数
void usart1_niming_report(u8 fun, u8 *data, u8 len) {
    u8 send_buf[32];
    u8 i;

    if (len > 28)return;    //最多28字节数据

    send_buf[len + 3] = 0;    //校验数置零
    send_buf[0] = 0X88;    //帧头
    send_buf[1] = fun;    //功能字
    send_buf[2] = len;    //数据长度

    for (i = 0; i < len; i++)send_buf[3 + i] = data[i];            //复制数据

    for (i = 0; i < len + 3; i++)send_buf[len + 3] += send_buf[i];    //计算校验和

    for (i = 0; i < len + 4; i++)usart1_send_char(send_buf[i]);    //发送数据到串口1
}

//发送加速度传感器数据和陀螺仪数据
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
void mpu6050_send_data(short aacx, short aacy, short aacz, short gyrox, short gyroy, short gyroz) {
    printf("aacx:%d\r\n", aacx);
    printf("aacy:%d\r\n", aacy);
    printf("aacz:%d\r\n", aacz);
    printf("gyrox:%d\r\n", gyrox);
    printf("gyroy:%d\r\n", gyroy);
    printf("gyroz:%d\r\n", gyroz);

    u8 tbuf[12];
    tbuf[0] = (aacx >> 8) & 0XFF;
    tbuf[1] = aacx & 0XFF;
    tbuf[2] = (aacy >> 8) & 0XFF;
    tbuf[3] = aacy & 0XFF;
    tbuf[4] = (aacz >> 8) & 0XFF;
    tbuf[5] = aacz & 0XFF;
    tbuf[6] = (gyrox >> 8) & 0XFF;
    tbuf[7] = gyrox & 0XFF;
    tbuf[8] = (gyroy >> 8) & 0XFF;
    tbuf[9] = gyroy & 0XFF;
    tbuf[10] = (gyroz >> 8) & 0XFF;
    tbuf[11] = gyroz & 0XFF;
    usart1_niming_report(0XA1, tbuf, 12); //自定义帧,0XA1
}

//通过串口1上报结算后的姿态数据给电脑
//aacx,aacy,aacz:x,y,z三个方向上面的加速度值
//gyrox,gyroy,gyroz:x,y,z三个方向上面的陀螺仪值
//roll:横滚角.单位0.01度。 -18000 -> 18000 对应 -180.00  ->  180.00度
//pitch:俯仰角.单位 0.01度。-9000 - 9000 对应 -90.00 -> 90.00 度
//yaw:航向角.单位为0.1度 0 -> 3600  对应 0 -> 360.0度
void
usart1_report_imu(short aacx, short aacy, short aacz, short gyrox, short gyroy, short gyroz, short roll, short pitch,
                  short yaw) {


    u8 tbuf[28];
    u8 i;

    for (i = 0; i < 28; i++)tbuf[i] = 0; //清0

    tbuf[0] = (aacx >> 8) & 0XFF;
    tbuf[1] = aacx & 0XFF;
    tbuf[2] = (aacy >> 8) & 0XFF;
    tbuf[3] = aacy & 0XFF;
    tbuf[4] = (aacz >> 8) & 0XFF;
    tbuf[5] = aacz & 0XFF;
    tbuf[6] = (gyrox >> 8) & 0XFF;
    tbuf[7] = gyrox & 0XFF;
    tbuf[8] = (gyroy >> 8) & 0XFF;
    tbuf[9] = gyroy & 0XFF;
    tbuf[10] = (gyroz >> 8) & 0XFF;
    tbuf[11] = gyroz & 0XFF;
    tbuf[18] = (roll >> 8) & 0XFF;
    tbuf[19] = roll & 0XFF;
    tbuf[20] = (pitch >> 8) & 0XFF;
    tbuf[21] = pitch & 0XFF;
    tbuf[22] = (yaw >> 8) & 0XFF;
    tbuf[23] = yaw & 0XFF;
    usart1_niming_report(0XAF, tbuf, 28); //飞控显示帧,0XAF
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
    /* USER CODE BEGIN 1 */
    float Distance;
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */
    delay_init(72);                    //初始化延时函数

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    /* USER CODE BEGIN 2 */
    u16 i = 0;
    W25QXX_Init();
    LCD_Init();                            //液晶屏初始化
    LCD_Clear(BLACK);                    //清屏
    POINT_COLOR = RED;            //设置字体为蓝色
    font_init();

    u8 t = 0, report = 1;            //默认开启上报
    u8 key;
    float pitch, roll, yaw;        //欧拉角
    short aacx, aacy, aacz;        //加速度传感器原始数据
    short gyrox, gyroy, gyroz;    //陀螺仪原始数据
    short temp;                    //温度
    MPU_Init();                            //初始化MPU6050
    while (mpu_dmp_init()) {
        printf("MPU6050 Error\r\n");
        delay_ms(200);
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        if (mpu_dmp_get_data(&pitch, &roll, &yaw) == 0) {
            temp = MPU_Get_Temperature();    //得到温度值
            printf("temp:%d\r\n", temp);
            MPU_Get_Accelerometer(&aacx, &aacy, &aacz);    //得到加速度传感器数据
            MPU_Get_Gyroscope(&gyrox, &gyroy, &gyroz);    //得到陀螺仪数据

            if (report)mpu6050_send_data(aacx, aacy, aacz, gyrox, gyroy, gyroz); //用自定义帧发送加速度和陀螺仪原始数据

            if (report)
                usart1_report_imu(aacx, aacy, aacz, gyrox, gyroy, gyroz, (int) (roll * 100), (int) (pitch * 100),
                                  (int) (yaw * 10));

            if ((t % 10) == 0) {
                if (temp < 0) {

                    Show_Str(30 + 48, 180, WHITE, BLACK, "-", 16, 0);        //显示负号
                    temp = -temp;        //转为正数
                } else Show_Str(30 + 48, 200, WHITE, BLACK, (u8 *) ' ', 16, 0);           //去掉负号

                LCD_ShowNum(30 + 48 + 8, 200, temp / 100, 3, 16);        //显示整数部分
                printf("整数部分:%d\r\n", temp / 100);
                LCD_ShowNum(30 + 48 + 40, 200, temp % 10, 1, 16);        //显示小数部分
                printf("小数部分:%d\r\n", temp % 10);
                temp = pitch * 10;

                if (temp < 0) {
                    Show_Str(30 + 48, 180, WHITE, BLACK, "-", 16, 0);        //显示负号
                    temp = -temp;        //转为正数
                } else Show_Str(30 + 48, 200, WHITE, BLACK, (u8 *) ' ', 16, 0);      //去掉负号

                LCD_ShowNum(30 + 48 + 8, 220, temp / 10, 3, 16);        //显示整数部分
                LCD_ShowNum(30 + 48 + 40, 220, temp % 10, 1, 16);        //显示小数部分
                temp = roll * 10;

                if (temp < 0) {
                    Show_Str(30 + 48, 180, WHITE, BLACK, "-", 16, 0);        //显示负号
                    temp = -temp;        //转为正数
                } else Show_Str(30 + 48, 200, WHITE, BLACK, (u8 *) ' ', 16, 0);      //去掉负号

                LCD_ShowNum(30 + 48 + 8, 240, temp / 10, 3, 16);        //显示整数部分
                LCD_ShowNum(30 + 48 + 40, 240, temp % 10, 1, 16);        //显示小数部分
                temp = yaw * 10;

                if (temp < 0) {
                    Show_Str(30 + 48, 180, WHITE, BLACK, "-", 16, 0);        //显示负号
                    temp = -temp;        //转为正数
                } else Show_Str(30 + 48, 200, WHITE, BLACK, (u8 *) ' ', 16, 0);       //去掉负号

                LCD_ShowNum(30 + 48 + 8, 260, temp / 10, 3, 16);        //显示整数部分
                LCD_ShowNum(30 + 48 + 40, 260, temp % 10, 1, 16);        //显示小数部分
                t = 0;
                HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
            }
        }

        t++;

    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
