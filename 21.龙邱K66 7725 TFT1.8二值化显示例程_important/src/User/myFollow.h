
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
#define Y_H 0xFF //黄色 
#define Y_L 0xE0        
#define P_H 0x78 //紫色
#define P_L 0x0F

#define PROSH 102//处理图像的高，宽
#define PROSW 160

#define TSD_d2 4//判断二阶差分是否是阶跃点的阈值，用在提取peak值，判断上拐点
#define TSD_kind 6//配合none简单判断赛道类型的阈值

void myfollow(uint8_t Img[PROSH][PROSW],float* AB);
void myfollow_Test(void);

#endif

