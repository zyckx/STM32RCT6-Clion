#ifndef __TEXT_H__
#define __TEXT_H__
#include <stm32f1xx.h>


void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size);			//得到汉字的点阵码
void Show_Font(u16 x, u16 y, u16 fc, u16 bc, u8 *font, u8 size, u8 mode, int i, int i1, int i2, int i3, int i4, int i5);		//在指定位置显示一个汉字
void Show_Str(u16 x,u16 y,u16 fc, u16 bc,u8*str,u8 size,u8 mode);		//在指定位置显示一个字符串
#endif
