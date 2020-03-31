#include "include.h"

//extern uint8_t Image_Use[LCDH][LCDW];           //压缩后之后用于存放屏幕显示数据

void myfollow(uint8_t Img[LCDH][LCDW])
{
  uint32_t sum_t=0;//总像素和
  uint8_t threshold=0;//阈值
  int i=0,j=0;//循环变量
  uint8_t pixel[LCDH][LCDW]={0};
  int  width = LCDW,height = LCDH;//赋值，防止LCDH变了

    typedef struct 
  {
    int coordinate[LCDH];//数组下标为行坐标，存储宽坐标
    int fg_suces[LCDH];//该行边线是否提取成功，寻完边线存在fg_suces为0应该是整行无边界
 
  }EDGE;
  EDGE edge_L={0,0};
  EDGE edge_R={0,0};
  
  //求和  
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
     sum_t+=Img[i][j];
  threshold=sum_t/(width*height);
 
  //二值化
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
      if(Img[i][j]>=threshold)   
        pixel[i][j]=T_White;
      else
        pixel[i][j]=F_Black;
      
      
      
    // 滤波，有点特殊先二值化再滤波
    //用1*6的模板，外边四个若一样，里面的也一样
    for( i=0;i<height;++i)//横向滤波
      for( j=2;j<width-3;++j)
      {
        if(pixel[i][j-2]==T_White&&pixel[i][j-2]==pixel[i][j-1]&&pixel[i][j+2]==pixel[i][j+3]&&pixel[i][j-2]==pixel[i][j+2])
        {pixel[i][j]=T_White;pixel[i][j+1]=T_White;}
        else if (pixel[i][j-2]==F_Black&&pixel[i][j-2]==pixel[i][j-1]&&pixel[i][j+2]==pixel[i][j+3]&&pixel[i][j-2]==pixel[i][j+2])
        {pixel[i][j]=F_Black;pixel[i][j+1]=F_Black;}
      }  
    
    for (j=0;j<width;++j)//纵向滤波
      for (i=2;i<height-3;++i)
      {
        if(pixel[i-2][j]==T_White&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=T_White;pixel[i+1][j]=T_White;}
        else if (pixel[i-2][j]==F_Black&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=F_Black;pixel[i+1][j]=F_Black;}
      } 
    
      
  int mid=width/2;
  int16 a_temp=0,b_temp=0;
// step1 //取左边缘提取
for (i=height-1;i>=0;i--)
{
    for (j=mid;j>=2;--j)
    {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//注意了一定要加int16 哭。。。,好像c语言不用
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp<2*T_White && b_temp>T_White )
        {
            edge_L.coordinate[i]=j;//最左边白色点在j处
            edge_L.fg_suces[i]=1;
            break;
        }
    }

    if(edge_L.fg_suces[i]==0)//对最左边2列单独判断，因为上面的判法左2列是不处理的
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
    
    if(edge_L.fg_suces[i]==0)//还为0，可能是边界在右半边，重新判一下右半边是否存在左边缘
    {                                                                           //不和上面>=mid循环合并因为在右半边是低概率情况
      for (j=width-1;j>mid;--j)
      {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//注意了一定要加int16 哭。。。,好像c语言不用
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp<2*T_White && b_temp>T_White )
        {
          edge_L.coordinate[i]=j;//最左边白色点在j处
          edge_L.fg_suces[i]=1;
          break;
        }
      }
    }
   
}

// step2 //取右边缘提取
for (i=height-1;i>=0;i--)
{
    for (j=mid;j<=width-4;++j)
    {
        a_temp=pixel[i][j-2]+pixel[i][j-1]+pixel[i][j];//注意了一定要加int16 哭。。。,好像c语言不用
        b_temp=pixel[i][j+1]+pixel[i][j+2]+pixel[i][j+3];
        if(a_temp>T_White && b_temp<2*T_White )
        {
            edge_R.coordinate[i]=j+1;//最右边白色点在j+1处
            edge_R.fg_suces[i]=1;
            break;
        }
    }

    if(edge_R.fg_suces[i]==0)//对最右边2列单独判断，因为上面的判法左2列是不处理的
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
    
    if(edge_R.fg_suces[i]==0)//还为0，可能是边界在左半边，重新判一下左半边是否存在右边缘
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

