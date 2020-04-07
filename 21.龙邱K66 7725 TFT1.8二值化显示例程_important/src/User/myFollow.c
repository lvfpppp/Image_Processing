#include "include.h"

//extern uint8_t saidao_1[51][70];
extern uint8_t wandao[PROSH][PROSW] ;
extern void mykey();

/*********************************************************************************************
* 
* 函数名称：SendPicture_s 
* 功能说明：发送图像到上位机 ，不同的上位机注意修改对应的数据接收协议
* 注  意  ：修改了原SendPicture的传送图像大小
**********************************************************************************************/ 
static void UARTSendPicture_s(uint8_t  tmImage[PROSH][PROSW]) 
{ 
  int i = 0, j = 0; 
  UART_PutChar(UART4,0x01); //发送帧头标志 WindowsFormsApplication1.exe
  UART_PutChar(UART4,0xfe); //发送帧头标志 WindowsFormsApplication1.exe
  for(i=0;i < PROSH; i++) 
  { 
    for(j=0;j <PROSW ;j++) 
    { 
      if(tmImage[i][j]==0xfe) 
      { 
        tmImage[i][j]=0xff; //防止发送标志位 
      } 
      UART_PutChar(UART4,tmImage[i][j]); 
    } 
  }
  UART_PutChar(UART4,0xfe); //发送帧尾标志 
  UART_PutChar(UART4,0x01); //发送帧尾标志 
} 


/************************************最小二乘法计算中线斜率***********************************
* 函数名称：regression, y = A+Bx
* 参数说明：输入Middle_black中线坐标，startline数组Middle_black开始行
* 返 回 值：AB[0]是截距，AB[1]是斜率，也是输入
*  注  意 ：正常斜率应该在0附近，截距应该在LCDW/2附近
*           线水平异常情况暂不返回
**********************************************************************************************/
static void regression(int Middle_black[PROSH],int startline,float * AB)
{
  int i=0,SumX=0,SumY=0,SumXX=0,SumXY=0; 
  float SumUp=0,SumDown=0;
  int Length=PROSH-startline;
//不包含结束行
  for(i=startline;i<PROSH;i++)     
  { 
    SumX+=i;       
    SumY+=Middle_black[i];    //这里Middle_black为存放中线的数组
   SumXX+=i*i;
   SumXY+=i*Middle_black[i]; 
  }       
   SumUp=SumXY-(float)(SumX*SumY)/(float)Length;//求B等式上部分
   SumDown=SumXX-(float)(SumX*SumX)/(float)Length;//求B等式下部分
 
  if(SumDown==0) 
   ;//斜率过大即为行，先保留返回
  else 
  {
    *(AB+1)=SumUp/SumDown;       
    *AB=(float)(SumY-*(AB+1)*SumX)/(float)Length;  //截距
  }

}


/*********************************************************************************************
*函数名称：TFTSPI_Show_rgb565
*功    能：在TFT1.8上显示彩色图像
*参数说明：xs:起始X，ys:起始Y，w:图片宽度，h:图片高度 ，ppic[IMAGEH][IMAGEW][2]:  图片缓存区    
**********************************************************************************************/	
static void TFTSPI_Show_rgb565(uint8_t xs,uint8_t ys,uint8_t w,uint8_t h,uint8_t ppic[PROSH][PROSW][2]) 
{
    unsigned int i,j;	
    TFTSPI_Set_Pos(xs,ys,xs+w-1,ys+h);
    for(i = 0; i < h; i++)  	
      for(j = 0; j < w; j++) 
        TFTSPI_Write_Word((ppic[i][j][0]<<8)+ppic[i][j][1]);//高位在前，且两个数据组合成一个16位数据表示像素值
 }

/********************************************************************************************
函数名称：myfollow
* 功能：简单循迹代码，对其提中线并拟合
* 输入：Img[PROSH][PROSW]待处理的灰度图像，返回AB[0]是截距，AB[1]是斜率，也是输入
* 输出：peak行为蓝色，左线红色，右线绿色，中线置白
********************************************************************************************/
void myfollow(uint8_t Img[PROSH][PROSW],float* AB)
{
  uint32_t sum_t=0;//总像素和
  uint8_t threshold=0;//阈值
  int i=0,j=0;//循环变量
  uint8_t pixel[PROSH][PROSW]={0};
  int  width = PROSW,height = PROSH;//看着顺眼
  
  int peak=0;//最终L和R中大的那个
  int Middle[PROSH]={0};//中线坐标
  int derivative_2[PROSH]={0};//中线列坐标的绝对二阶导
  
   int shizi_tp[4][4]={0};//十字四个拐点turning point，依次存储上到下，左到右
  //存储三个关键阶跃点，依次是非赛道点，十字上阶跃点，下阶跃点
   
    typedef struct 
  {
    int coordinate[PROSH];//数组下标为行坐标，存储宽坐标
    int fg_suces[PROSH];//该行边线是否提取成功，寻完边线存在fg_suces为0应该是整行无边界
    int peak;//图像到哪行为止
    uint8_t none;//边线在第一列或最后列的累积次数
    uint8_t none_fg;//配合none做赛道在边界是否通的标志位
    //一阶差分补十字，左右边界次数判左右还是十字，matlab看看十字是否圆滑
  }EDGE;
  EDGE edge_L={0,0,0,0,0};//左右边缘
  EDGE edge_R={0,0,0,0,0};
  
  //uint8_t show_D2L[PROSH-1][PROSW]={0};//显示绝对一阶导到TFT
  uint8_t show_D2[PROSH][PROSW]={0};//显示绝对二阶导到TFT
  uint8_t show_t[PROSH][PROSW][2]={0};//显示左中右线的数组,show_t[PROSH][PROSW][0]放rgb565的高位
  
  
  
  //求和  
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
     sum_t+=Img[i][j];
  threshold=sum_t/(width*height);
 threshold=84;//假装求阈值hh
 
  //二值化
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
      if(Img[i][j]>=threshold)   
        pixel[i][j]=T_White;
      else
        pixel[i][j]=F_Black;
      
      mykey();
     TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,pixel[0]);//  传二值化图像 、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、  
      
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
    
/*    for (j=0;j<width;++j)//纵向滤波
      for (i=2;i<height-3;++i)
      {
        if(pixel[i-2][j]==T_White&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=T_White;pixel[i+1][j]=T_White;}
        else if (pixel[i-2][j]==F_Black&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=F_Black;pixel[i+1][j]=F_Black;}
      } 
*///暂时不要了，对后面分析有影响   
    mykey();
    TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,pixel[0]);//  传滤波二值化图像、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、    
    
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
       for (j=mid+1;j<width;++j)
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
       for (j=mid-1;j>=0;--j)
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

//Step 3 :提中线 

 int k=0; 
  for(k=height-1;k>=0;k--)
  {
    Middle[k]=(edge_R.coordinate[k]+edge_L.coordinate[k])/2;
    if(edge_L.fg_suces[k]==0) {edge_L.peak=k;break;}//初始的peak值
    if(edge_R.fg_suces[k]==0) {edge_R.peak=k;break;}
  }
  if(edge_L.peak==0)//选两者更大的，除去没赋的
    peak=edge_R.peak;
  else if(edge_R.peak==0)
    peak=edge_L.peak;
  else if(edge_L.peak>edge_R.peak)
    peak=edge_L.peak;
  else 
    peak=edge_R.peak;

  for(k=height-1;k>=peak+2;k--)//求绝对二阶导，输出导数信息、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、  
  {
    derivative_2[k]=abs(Middle[k]-2*Middle[k-1]+Middle[k-2]);
    show_D2[k][derivative_2[k]]=T_White;    
  }
  mykey();
  TFTSPI_CLS(u16WHITE);
  TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,show_D2[0]);

//step 4:对不同赛道分别处理
  for(k=peak;k<height;k++)
  {
    if(edge_L.coordinate[k]==0) edge_L.none++;
    else edge_L.none=0;
    if(edge_L.none>TSD_kind)
    {
      edge_L.none_fg=1;//左边通
      break;
    }
  }
    for(k=peak;k<height;k++)
  {
    if(edge_R.coordinate[k]==width-1) edge_R.none++;
    else edge_R.none=0;
    if(edge_R.none>TSD_kind)
    {
      edge_R.none_fg=1;//右边通
      break;
    }
  }
  /////////十字赛道处理///////////,总体是因为下面受畸变拐点比较光滑，上面则正常陡峭
  if(edge_L.none_fg==1 && edge_R.none_fg==1)//十字,思路：下拐点：如果后一个比前一个列坐标大则为后一个值，且后面递减至边界，左右上拐点分别见下面注释
  {
    TFTSPI_P16x16Str(8,7,"十字",u16BLACK,u16WHITE);//输出“十字”
    //找左下拐点
    for(k=height-1;k>=peak+1;k--)
    {
      if(edge_L.coordinate[k]-edge_L.coordinate[k-1]>0)//找到左下拐点可疑点
      {
        int m=k-1,fg_t_t=0;
        while(edge_L.coordinate[m]>0)//当判断列坐标为0时退出，符合条件
        {
          if(edge_L.coordinate[m]-edge_L.coordinate[m-1]<0)
          {
            fg_t_t=1;//不符合拐点后一直递减
            break;//while
          }
          m--;
        }
        if(fg_t_t==0)//符合条件
        {
          shizi_tp[2][0]=k;
          shizi_tp[2][1]=edge_L.coordinate[k];
          break;//for
        }      
      }    
    }//for
    //找右下拐点
    for(k=height-1;k>=peak+1;k--)
    {
      if(edge_R.coordinate[k]-edge_R.coordinate[k-1]<0)//找到右下拐点可疑点
      {
        int m=k-1,fg_t_t=0;
        while(edge_R.coordinate[m]<width-1)//当判断列坐标为width时退出，符合条件
        {
          if(edge_R.coordinate[m]-edge_R.coordinate[m-1]>0)
          {
            fg_t_t=1;//不符合拐点后一直递减
            break;//while
          }
          m--;
        }
        if(fg_t_t==0)//符合条件
        {
          shizi_tp[3][0]=k;
          shizi_tp[3][1]=edge_R.coordinate[k];
          break;//for
        }      
      }    
    }
    //找左上拐点
    for(k=shizi_tp[2][0]-1;k>=peak+2;k--)//找左上拐点为该点上一点，该点比上一点小，上一点比上上一点小或等于,且该点二阶导<-TSD_d2
    {
      if(edge_L.coordinate[k]<edge_L.coordinate[k-1] && edge_L.coordinate[k-1]<=edge_L.coordinate[k-2] &&
         edge_L.coordinate[k]-2*edge_L.coordinate[k-1]+edge_L.coordinate[k-2] < -TSD_d2)
       {
          shizi_tp[0][0]=k-1;
          shizi_tp[0][1]=edge_L.coordinate[k-1];
          break;
       }      
    }
    //找右上拐点（“或等于”漏了就很尴尬）
    for(k=shizi_tp[3][0]-1;k>=peak+2;k--)//找右上拐点为该点上一点，该点比上一点大，上一点比上上一点大或等于,且该点二阶导>TSD_d2
    {
       if(edge_R.coordinate[k]>edge_R.coordinate[k-1] && edge_R.coordinate[k-1]>=edge_R.coordinate[k-2] &&
          edge_R.coordinate[k]-2*edge_R.coordinate[k-1]+edge_R.coordinate[k-2] > TSD_d2)
       {
          shizi_tp[1][0]=k-1;
          shizi_tp[1][1]=edge_R.coordinate[k-1];
          break;
       }      
    }
    //补线//y=(y2-y1)*(x-x1)/(x2-x1)+y1
    for(k=shizi_tp[0][0];k<=shizi_tp[2][0];k++)//y=(y2-y1)*(x-x1)/(x2-x1)+y1
      edge_L.coordinate[k]=(k-shizi_tp[0][0])*(shizi_tp[2][1]-shizi_tp[0][1])/(shizi_tp[2][0]-shizi_tp[0][0])+shizi_tp[0][1];
    for(k=shizi_tp[1][0];k<=shizi_tp[3][0];k++)
      edge_R.coordinate[k]=(k-shizi_tp[1][0])*(shizi_tp[3][1]-shizi_tp[1][1])/(shizi_tp[3][0]-shizi_tp[1][0])+shizi_tp[1][1];
   //补线后重新再求中线
    int a_t,b_t;//确定上下界
    a_t=shizi_tp[0][0]<shizi_tp[1][0]?shizi_tp[0][0]:shizi_tp[1][0];
    b_t=shizi_tp[2][0]<shizi_tp[3][0]?shizi_tp[3][0]:shizi_tp[2][0];
    for(k=a_t;k<=b_t;k++)
      Middle[k]=(edge_R.coordinate[k]+edge_L.coordinate[k])/2;
    
    for(k=height-1;k>=peak+2;k--)//求绝对二阶导，输出导数信息、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、  
  {
    derivative_2[k]=abs(Middle[k]-2*Middle[k-1]+Middle[k-2]);
    //show_D2[k][derivative_2[k]]=T_White;可以再输出二阶绝对偏差来看看效果
    if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }
  }
  
}///////////////////十字处理完/////////////////
  /////////弯道赛道处理///////////
else if(edge_L.none_fg==1)//左弯道
{
  TFTSPI_P16x16Str(8,7,"左弯",u16BLACK,u16WHITE);//输出“左弯”
   for(k=height-1;k>=peak+2;k--)
     if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }  
}
else if(edge_R.none_fg==1)//右弯道
{
  TFTSPI_P16x16Str(8,7,"右弯",u16BLACK,u16WHITE);//输出“右弯”
   for(k=height-1;k>=peak+2;k--)
     if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }  
}
/////////////////弯道处理完//////////////////

//step 5:对中线进行拟合并输出各种信息
  regression(Middle,peak,AB);//二乘法拟合

/////////输出信息到TFT屏、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、、  
     for(k=0;k<width;k++)
     {
       show_t[peak][k][0]=B_H;//输出peak行为蓝色，即中线的最高行
       show_t[peak][k][1]=B_L;
     }
////////     
     for (k=0;k<height;k++)//显示左线
     {
       if(k>=shizi_tp[0][0] && k<=shizi_tp[2][0])
       {
         show_t[k][edge_L.coordinate[k]][0]=Y_H;//被补上的左线黄色
         show_t[k][edge_L.coordinate[k]][1]=Y_L;
       }
       else
       {
         show_t[k][edge_L.coordinate[k]][0]=R_H;//左线红色
         show_t[k][edge_L.coordinate[k]][1]=R_L;
       }
     }
////////      
     for (k=0;k<height;k++)//显示右线
     {
       if(k>=shizi_tp[1][0] && k<=shizi_tp[3][0])
       {
         show_t[k][edge_R.coordinate[k]][0]=P_H;//被补上的右线紫色
         show_t[k][edge_R.coordinate[k]][1]=P_L;
       }
       else
       {
         show_t[k][edge_R.coordinate[k]][0]=G_H;//右线绿色
         show_t[k][edge_R.coordinate[k]][1]=G_L;
       }
     }
////////    
      for (k=peak;k<height;k++)
     {
       show_t[k][Middle[k]][0]=T_White;//中线置白
       show_t[k][Middle[k]][1]=T_White;
     }
//////// 输出图像
     mykey();
     TFTSPI_Show_rgb565(0,0,PROSW,PROSH,show_t);
////////    显示截距和斜率  
     char  txt[10];
  for (int i=0;i<2;i++)
  {
   sprintf((char*)txt,"%5.4f",*(AB+i));
    TFTSPI_P6X8Str(i*12,15,(uint8_t*)txt,u16BLACK,u16WHITE);
  }
    sprintf((char*)txt,"%d",threshold);//显示阈值
    TFTSPI_P6X8Str(0,14,(uint8_t*)txt,u16BLACK,u16WHITE);
   //UARTSendPicture_s(); 传到上位机，但没有匹配的大小不匹配呀
}

void mykey()
{
  while(KEY_Read(1)!=1);
}

void myfollow_Test(void)//测试循迹代码
{ 
  TFTSPI_Init(0);        //LCD初始化  0:横屏  1：竖屏
  TFTSPI_CLS(u16WHITE);   //白色屏幕	
  LED_Init();
  KEY_Init();
  UART_Init(UART4, 115200);
  
      TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,wandao[0]);//初始图片显示
      float* ab=(float *)malloc(sizeof(float)*2);  //AB[0]是截距，AB[1]是斜率
    myfollow(wandao,ab);//处理
      free(ab);
//    //测试串口输出OK
//    float A=*ab;
//    float B=*(ab+1);
//     char  txt[20];
//   sprintf((char*)txt,"截距为%5.4f\n",A);
//     UART_PutStr(UART4,txt);  
//   sprintf((char*)txt,"斜率为%5.4f\n",B);
//     UART_PutStr(UART4,txt);  
    while(1)
    {
    
     }
}