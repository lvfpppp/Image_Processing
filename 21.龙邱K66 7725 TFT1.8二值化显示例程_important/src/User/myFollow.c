#include "include.h"

//extern uint8_t Image_Use[LCDH][LCDW];           //ѹ����֮�����ڴ����Ļ��ʾ����

void myfollow(uint8_t Img[LCDH][LCDW])
{
  uint32_t sum_t=0;//�����غ�
  uint8_t threshold=0;//��ֵ
  int i=0,j=0;//ѭ������
  uint8_t pixel[LCDH][LCDW]={0};
  int  width = LCDW,height = LCDH;//��ֵ����ֹLCDH����

    typedef struct 
  {
    int coordinate[LCDH];//�����±�Ϊ�����꣬�洢������
    int fg_suces[LCDH];//���б����Ƿ���ȡ�ɹ���Ѱ����ߴ���fg_sucesΪ0Ӧ���������ޱ߽�
 
  }EDGE;
  EDGE edge_L={0,0};
  EDGE edge_R={0,0};
  
  //���  
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
     sum_t+=Img[i][j];
  threshold=sum_t/(width*height);
 
  //��ֵ��
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
      if(Img[i][j]>=threshold)   
        pixel[i][j]=T_White;
      else
        pixel[i][j]=F_Black;
      
      
      
    // �˲����е������ȶ�ֵ�����˲�
    //��1*6��ģ�壬����ĸ���һ���������Ҳһ��
    for( i=0;i<height;++i)//�����˲�
      for( j=2;j<width-3;++j)
      {
        if(pixel[i][j-2]==T_White&&pixel[i][j-2]==pixel[i][j-1]&&pixel[i][j+2]==pixel[i][j+3]&&pixel[i][j-2]==pixel[i][j+2])
        {pixel[i][j]=T_White;pixel[i][j+1]=T_White;}
        else if (pixel[i][j-2]==F_Black&&pixel[i][j-2]==pixel[i][j-1]&&pixel[i][j+2]==pixel[i][j+3]&&pixel[i][j-2]==pixel[i][j+2])
        {pixel[i][j]=F_Black;pixel[i][j+1]=F_Black;}
      }  
    
    for (j=0;j<width;++j)//�����˲�
      for (i=2;i<height-3;++i)
      {
        if(pixel[i-2][j]==T_White&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=T_White;pixel[i+1][j]=T_White;}
        else if (pixel[i-2][j]==F_Black&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=F_Black;pixel[i+1][j]=F_Black;}
      } 
    
      
  int mid=width/2;
  int16 a_temp=0,b_temp=0;
// step1 //ȡ���Ե��ȡ
for (i=height-1;i>=0;i--)
{
    for (j=mid;j>=2;--j)
    {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//ע����һ��Ҫ��int16 �ޡ�����,����c���Բ���
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp<2*T_White && b_temp>T_White )
        {
            edge_L.coordinate[i]=j;//����߰�ɫ����j��
            edge_L.fg_suces[i]=1;
            break;
        }
    }

    if(edge_L.fg_suces[i]==0)//�������2�е����жϣ���Ϊ������з���2���ǲ������
    {
      if(pixel[i][0]==F_Black && pixel[i][1]+pixel[i][2]+pixel[i][3]>T_White)
      {
        edge_L.coordinate[i]=1;
        edge_L.fg_suces[i]=1;
      }
      else if(pixel[i][0]==T_White && pixel[i][0]+pixel[i][1]+pixel[i][2]>T_White)
      {
        edge_L.coordinate[i]=0;
        edge_L.fg_suces[i]=1;
      }
    }
    
    if(edge_L.fg_suces[i]==0)//��Ϊ0�������Ǳ߽����Ұ�ߣ�������һ���Ұ���Ƿ�������Ե
    {                                                                           //��������>=midѭ���ϲ���Ϊ���Ұ���ǵ͸������
      for (j=width-1;j>mid;--j)
      {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//ע����һ��Ҫ��int16 �ޡ�����,����c���Բ���
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp<2*T_White && b_temp>T_White )
        {
          edge_L.coordinate[i]=j;//����߰�ɫ����j��
          edge_L.fg_suces[i]=1;
          break;
        }
      }
    }
   
}

// step2 //ȡ�ұ�Ե��ȡ
for (i=height-1;i>=0;i--)
{
    for (j=mid;j<=width-4;++j)
    {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//ע����һ��Ҫ��int16 �ޡ�����,����c���Բ���
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp>T_White && b_temp<2*T_White )
        {
            edge_R.coordinate[i]=j+1;//���ұ߰�ɫ����j+1��
            edge_R.fg_suces[i]=1;
            break;
        }
    }

    if(edge_R.fg_suces[i]==0)//�����ұ�2�е����жϣ���Ϊ������з���2���ǲ������
    {
      if(pixel[i][width-1]==F_Black && pixel[i][width-2]+pixel[i][width-3]+pixel[i][width-4]>T_White)
      {
        edge_R.coordinate[i]=width-2;
        edge_R.fg_suces[i]=1;
      }
      else if(pixel[i][width-1]==T_White && pixel[i][width-1]+pixel[i][width-2]+pixel[i][width-3]>T_White)
      {
        edge_R.coordinate[i]=width-1;
        edge_R.fg_suces[i]=1;
      }
    }
    
    if(edge_R.fg_suces[i]==0)//��Ϊ0�������Ǳ߽������ߣ�������һ�������Ƿ�����ұ�Ե
    {                                                                           
      for (j=0;j<mid;++j)
      {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp>T_White && b_temp<2*T_White)
        {
          edge_R.coordinate[i]=j+1;
          edge_R.fg_suces[i]=1;
          break;
        }
      }
    }
   
}

   
            
            
          
      
      
  
}

