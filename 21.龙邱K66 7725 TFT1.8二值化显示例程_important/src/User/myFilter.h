
#ifndef MYFILTER_H
#define MYFILTER_H


#define SIZE 3//高斯滤波窗口大小
/*---------------------------------------------------------------
【函    数】Pixle_Filter
【功    能】过滤噪点
【参    数】无
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
void Pixle_Filter(uint8_t Pixle[LCDH][LCDW]);

void Median_filtering(int height,int width,uint8_t (*data)[LCDW],int windowsize);

void Gaussian_filtering(int height,int width,uint8_t (*data)[LCDW]);

void Gaussian_Init(void);
#endif

