#include "include.h"

uint8 toushi[TS_H][TS_W];
uint8 compress_cut[LCDH][LCDW];


//int round(double x)//��������,�п⺯��
//{
//  if (x>=0)
//  {
//    if(x-(int)x>=0.5)
//      return (int)x+1;
//    else return (int)x;
//  }
//  else
//  {
//    x=-x;
//    if(x-(int)x>=0.5)
//      return -((int)x+1);
//    else return -(int)x;
//  }
//}
//ѹ��,���к���
void toushi_compress_cut(void)
{
  int height=TS_H;//������һ��
  int width=TS_W;
  int i,j,ai,aj; 
   
   
   for(i=0,ai=0;i<height/CPS;i++,ai=ai+CPS)
   {
     for(j=0,aj=0;i<width/CPS;j++,aj=aj+CPS)
     {
       
       toushi[i][j]=toushi[ai][aj];
     }
   }
   
   for(i=150,ai=0;ai<LCDH;i++,ai++)//��40,150��ʼ����
   {
     for(j=40,aj=0;aj<LCDW;j++,aj++)
     {
       compress_cut[ai][aj]=toushi[i][j];
     }
   }

}


//�������ƣ�toushi
//����˵������͸�ӱ任
//����˵����grayImage��ԭͼ�����飬result�����ͼ������
//�������أ�result�����ͼ������
//�޸�ʱ�䣺2020/2/21
//�� ע�� �޸ı任����perspective_mat����,     �Ĳ������ʼ�
void TouShi(uint8 grayImage[661][835])//Ҫ��heightһ��,�����˸�ͷ�ļ����
{
  float height=661;//������һ��
  float width=835;
   double perspective_inv[3][3]={
1.28974316321220,	-1.30359006359121,	395.076851477222,
1.54459897908404,	0.383280271311732,	4.01093250230692,
8.02668698460085e-06,	-9.25008537258943e-06,	1.00273312557670};//�任���������perspective_mat������
   double min_x=-61.792556000604094;  
   double min_y=-2.754861420044677e+02;
   //TS_H,TS_WҲҪ��
     int i,j,k;
//   uint8 **wrapImg;//[TS_H][TS_W];
//     wrapImg=(uint8 **)malloc(sizeof(uint8*)*TS_H);
//   for (i=0;i<TS_H;i++)
//     wrapImg[i]=(uint8*)malloc(sizeof(uint8)*TS_W);
   double x,y,moving_point[3],temp_point[2];
   
   for ( i=1;i<=TS_H;i++)
   {
    for( j=1;j<=TS_W;j++)
    {
      x = min_x+j; // ʹ��x��y�ķ�Χ��ԭ���귶Χ��
        y = min_y+i;
        for ( k=0;k<3;k++)
          moving_point[k]=perspective_inv[k][0]*x+perspective_inv[k][1]*y+perspective_inv[k][2];//��ӳ��ص�ԭ����λ��
        for (k=0;k<2;k++)
          temp_point[k]=moving_point[k]/moving_point[2];//ȥ��һ��ά��
        if (temp_point[0]>=1 && temp_point[0]<width && temp_point[1]>=1 && temp_point[1]<height)
             toushi[i-1][j-1]=grayImage[(int)round(temp_point[1])-1][(int)round(temp_point[0])-1];//�����ڽ���ֵ,Ӧ��-1,�⺯��roundҪ��int
    }
   }
   
   toushi_compress_cut();
//   return compress(wrapImg);
}
