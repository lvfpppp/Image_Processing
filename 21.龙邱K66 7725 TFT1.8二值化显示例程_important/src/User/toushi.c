#include "include.h"
#include "LQ_OV7725.h"
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

uint8 ** TouShi(uint8 grayImage[LCDH][LCDW])//LCDHҪ��heightһ��
{
  float height=945;//Ҫ��LCDHһ��
  float width=1134;
   double perspective_inv[3][3]={
              0.475206861530918,	-0.528779142881703,	466.545114756812,
              0.573640794781262,	0.153505741820254,	-69.5786551373927,
              0,	0,	1};//�任���������
   double min_x=-90.740049751243800;  
   double min_y=-928.413290689411;
   //TS_H,TS_WҲҪ��
   
   uint8 **wrapImg;//[TS_H][TS_W];
     wrapImg=(uint8 **)malloc(sizeof(uint8*)*TS_H);
   int i,j,k;
   for (i=0;i<TS_H;i++)
     wrapImg[i]=(uint8*)malloc(sizeof(uint8)*TS_W);
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
             wrapImg[i-1][j-1]=grayImage[(int)round(temp_point[1])-1][(int)round(temp_point[0])-1];//�����ڽ���ֵ,Ӧ��-1,�⺯��roundҪ��int
    }
   }
   return wrapImg;
}
