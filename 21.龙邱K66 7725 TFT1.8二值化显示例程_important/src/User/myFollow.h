
#ifndef MYFOLLOW_H
#define MYFOLLOW_H

#define T_White 255 //��ɫ  �Ҷ���
#define F_Black 0 //��ɫ
#define R_H 0xF8 //��ɫ��λ��rgb565
#define R_L 0x00 //��ɫ��λ
#define G_H 0x04 //��ɫ
#define G_L 0xE0
#define B_H 0x00 //��ɫ
#define B_L 0x1F

#define PROSH 50//120//����ͼ��ĸߣ���
#define PROSW 80//160

void myfollow(uint8_t Img[PROSH][PROSW],float* AB);
void myfollow_Test(void);

#endif

