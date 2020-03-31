
#ifndef MYFILTER_H
#define MYFILTER_H


#define SIZE 3//��˹�˲����ڴ�С
/*---------------------------------------------------------------
����    ����Pixle_Filter
����    �ܡ��������
����    ������
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
void Pixle_Filter(uint8_t Pixle[LCDH][LCDW]);

void Median_filtering(int height,int width,uint8_t (*data)[LCDW],int windowsize);

void Gaussian_filtering(int height,int width,uint8_t (*data)[LCDW]);

void Gaussian_Init(void);
#endif

