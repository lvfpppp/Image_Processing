
#ifndef MYFOLLOW_H
#define MYFOLLOW_H

#define T_White 255 //白色  灰度用
#define F_Black 0 //黑色
#define R_H 0xF8 //红色高位，rgb565
#define R_L 0x00 //红色低位
#define G_H 0x04 //绿色
#define G_L 0xE0
#define B_H 0x00 //蓝色
#define B_L 0x1F

#define PROSH 50//120//处理图像的高，宽
#define PROSW 80//160

void myfollow(uint8_t Img[PROSH][PROSW],float* AB);
void myfollow_Test(void);

#endif

