/*---------------------------------------------------------------
【平    台】龙邱K60核心板-智能车板
【编    写】LQ-005
【E-mail  】chiusir@163.com
【软件版本】V1.0，龙邱开源代码，仅供参考，后果自负
【最后更新】2019年04月02日
【网    站】http://www.lqist.cn
【淘宝店铺】http://shop36265907.taobao.com
【编译平台】IAR 8.2
【功    能】OV7725测试
【注意事项】
----------------------------------------------------------------*/
#include "include.h"
#include "LQ_OV7725.h"

uint8_t Image_Data[IMAGEH][IMAGEW];      //图像原始数据存放
uint8_t Image_Use[LCDH][LCDW];           //压缩后之后用于存放屏幕显示数据
uint8_t Image_filter[LCDH][LCDW];        //储存滤波后的数据
double Gaussian_Temp[SIZE][SIZE] = {0};       //高斯模板值

uint8_t Pixle[LCDH][LCDW];               //二值化后用于OLED显示的数据
uint8_t  Line_Cont=0;                    //行计数
uint8_t  Field_Over_Flag=0;              //场标识


/*---------------------------------------------------------------
【函    数】PORTC_Interrupt
【功    能】PORTC端口的中断服务函数 用于摄像头行场中断
【参    数】无
【返 回 值】无
【注意事项】注意进入后要清除中断标志位
----------------------------------------------------------------*/
void PORTD_IRQHandler(void)
{     
  //行中断PTD14
  int n;    
    n=14;   //行中断
  if((PORTD_ISFR & (1<<n) ))//行中断 (1<<14)
  {    
    PORTD_ISFR |= (1<<n);   //清除中断标识
    // 用户程序            
    DMATransDataStart(DMA_CH4,(uint32_t)(&Image_Data[Line_Cont][0]));   //开启DMA传输 
    if(Line_Cont > IMAGEH)  //采集结束
    { 
      Line_Cont=0; 
      return ;
    } 
    ++Line_Cont;            //行计数
    return ; 
  }
  //场中断PTD15
  n=15;  //场中断
  if((PORTD_ISFR & (1<<n)))//(1<<15)
  {
    PORTD_ISFR |= (1<<n);  //清除中断标识
    // 用户程序 
    Line_Cont = 0;         //行计数清零
    Field_Over_Flag=1;     //场结束标识
  } 
}

/*---------------------------------------------------------------
【函    数】Test_OV7725
【功    能】测试OV7725 在OLED上显示图像
【参    数】无
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
void Test_OV7725(void)
{  
    LED_Init();
    TFTSPI_Init(0);                //TFT1.8初始化  0：横屏显示  1：竖屏显示  
    TFTSPI_CLS(u16BLUE);           //清屏
    
    /*  摄像头初始化 */
    OV7725_Init(50);               //摄像头初始化
    
    //摄像头有关接口初始化
    GPIO_ExtiInit(PTD14,rising_down);   //行中断
    GPIO_ExtiInit(PTD15,falling_up);    //场中断  
    
    /* 优先级配置 抢占优先级0  子优先级2   越小优先级越高  抢占优先级可打断别的中断 */
    NVIC_SetPriority(PORTD_IRQn,NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,2));
    NVIC_EnableIRQ(PORTD_IRQn);			         //使能PORTD_IRQn的中断 
    
    //八位数据输入口
    GPIO_PinInit(PTD0,GPI,0);                
    GPIO_PinInit(PTD1,GPI,0);
    GPIO_PinInit(PTD2,GPI,0);
    GPIO_PinInit(PTD3,GPI,0);
    GPIO_PinInit(PTD4,GPI,0);
    GPIO_PinInit(PTD5,GPI,0);
    GPIO_PinInit(PTD6,GPI,0);
    GPIO_PinInit(PTD7,GPI,0);     
    
    //初始化DMA采集       通道4        PTD0-PTD7           目的地址      触发信号 每次一个BYTE  长度    下拉上升沿触发
    DMA_PORTx2BUFF_Init (DMA_CH4, (void *)&PTD_BYTE0_IN,(void*)Image_Data, PTD13, DMA_BYTE1, (IMAGEW ), DMA_rising_down); 
    /* 摄像头初始化结束 */
    
     Gaussian_Init();
    while(1)
    { 
        LED_Reverse(1);           //LED指示程序运行状态
        if(Field_Over_Flag)       //完成一场图像采集
        { 
            Get_Use_Image();      //从采集图像数据中取出自己想要使用的大小
           Gaussian_filtering(LCDH,LCDW,Image_Use);
            // Median_filtering(LCDH,LCDW,Image_Use,3);
            TFTSPI_Show_Cmera(0, 0, 160, 120, Image_filter);      //二值化图像数据

            Field_Over_Flag= 0;       
        }    
    }
}


/*---------------------------------------------------------------
【函    数】Get_Use_Image
【功    能】获取需要使用的图像大小
【参    数】无
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
__ramfunc void Get_Use_Image(void)
{
  int i = 0,j = 0,row = 0,line = 0;
  int div_h = IMAGEH/LCDH;
  int div_w = IMAGEW/LCDW;
  for(i = 0; i  < IMAGEH; i+=div_h)  //240行，每4行采集一行，
  {
    for(j = 0;j < IMAGEW; j+=div_w)  //320 / 4  = 80，
    {        
      Image_Use[row][line] = Image_Data[i][j];         
      line++;        
    }      
    line = 0;
    row++;      
  }  
}

/*---------------------------------------------------------------
【函    数】Get_01_Value
【功    能】二值化
【参    数】mode  ：  0：使用大津法阈值    1：使用平均阈值
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
void Get_01_Value(uint8_t mode,uint8_t Image[LCDH][LCDW])
{
  int i = 0,j = 0;
  uint8_t Threshold;
  uint32_t  tv=0;
  char txt[16];
  if(mode)
  {
      //累加
      for(i = 0; i <LCDH; i++)
      {    
          for(j = 0; j <LCDW; j++)
          {                            
              tv+=Image[i][j];   //累加  
          } 
      }
      Threshold=tv/LCDH/LCDW;        //求平均值,光线越暗越小，全黑约35，对着屏幕约160，一般情况下大约100
      Threshold=Threshold*7/10+10;   //此处阈值设置，根据环境的光线来设定 
  }
  else
  {
      Threshold = GetOSTU(Image);//大津法阈值
      Threshold = (uint8_t)(Threshold * 0.5) + 70;
  } 
  sprintf(txt,"%03d",Threshold);//前者为大津法求得的阈值，后者为平均值 
#ifdef LQ_OLED 
  OLED_P6x8Str(80,1,(u8*)txt);
#else
  TFTSPI_P8X8Str(0,10,(u8*)txt,u16RED,u16BLUE);
#endif
  for(i = 0; i < LCDH; i++)
  {
    for(j = 0; j < LCDW; j++)
    {                                
      if(Image[i][j] >Threshold) //数值越大，显示的内容越多，较浅的图像也能显示出来    
        Pixle[i][j] =1; //1是白色，0是黑色，根据下面TFTSPI_Show_Cmera 中来   
      else                                        
        Pixle[i][j] =0;
    }    
  }
}

/*---------------------------------------------------------------
【函    数】TFTSPI_Show_Cmera
【功    能】在TFT1.8上画出摄像头的图像
【参    数】xs:  起始X   
【参    数】ys:  起始Y  
【参    数】w:   图片宽度 
【参    数】h:   图片高度  
【参    数】ppic[IMAGEH][IMAGEW]:  图片缓存区   
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/	
void TFTSPI_Show_Cmera(uint8_t xs,uint8_t ys,uint8_t w,uint8_t h,uint8_t ppic[LCDH][LCDW]) 
{
#if 1    //灰度显示
    unsigned int i,j;	
    uint16_t color;
    TFTSPI_Set_Pos(xs,ys,xs+w-1,ys+h);
    for(i = 0; i < h; i += 1)     
    { 	
        for(j = 0; j < w; j += 1) 
        {
           
            color = 0;
            color=((ppic[i][j]>>3))<<11;
            color=color|(((ppic[i][j]>>2))<<5);
            color=color|(((ppic[i][j])>>3));
            TFTSPI_Write_Word(color);
        }
        
    }
#else    //二值化显示
    unsigned int i,j;
    Median_filtering(LCDH,LCDW,Image_Use,3);
    Get_01_Value(0,Image_filter);
   //sauvola(Image_Use,0.5,6);
    
    TFTSPI_Set_Pos(xs,ys,xs+w-1,ys+h);
    for(i = 0; i < h; i += 1)     
    { 	
        for(j = 0; j < w; j += 1) 
        {
           
            if(Pixle[i][j])
            {
                TFTSPI_Write_Word(0xFFFF);
            }
            else
            {
                TFTSPI_Write_Word(0);
            }      
        }
        
    }
#endif
 }
/*---------------------------------------------------------------
【函    数】Draw_Road
【功    能】在OLED上画出摄像头的图像
【参    数】无
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
void Draw_Road(void)
{ 	 
  u8 i = 0, j = 0,temp=0;
  
  //发送帧头标志
  for(i=8;i<56;i+=8)//6*8=48行 
  {
    OLED_Set_Pos(24,i/8+1);//起始位置
    for(j=0;j<LCDW;j++)  //列数
    { 
      temp=0; 
      if(Pixle[0+i][j]) temp|=1;
      if(Pixle[1+i][j]) temp|=2;
      if(Pixle[2+i][j]) temp|=4;
      if(Pixle[3+i][j]) temp|=8;
      if(Pixle[4+i][j]) temp|=0x10;
      if(Pixle[5+i][j]) temp|=0x20;
      if(Pixle[6+i][j]) temp|=0x40;
      if(Pixle[7+i][j]) temp|=0x80;
      OLED_WrDat(temp); 	  	  	  	  
    }
  }  
}


/*---------------------------------------------------------------
【函    数】Pixle_Filter
【功    能】过滤噪点
【参    数】无
【返 回 值】无
【注意事项】
----------------------------------------------------------------*/
void Pixle_Filter(void)
{  
  int nr; //行
  int nc; //列
  
  for(nr=1; nr<LCDH-1; nr++)
  {  	    
    for(nc=1; nc<LCDW-1; nc=nc+1)
    {
      if((Pixle[nr][nc]==0)&&(Pixle[nr-1][nc]+Pixle[nr+1][nc]+Pixle[nr][nc+1]+Pixle[nr][nc-1]>2))         
      {
        Pixle[nr][nc]=1;
      }	
      else if((Pixle[nr][nc]==1)&&(Pixle[nr-1][nc]+Pixle[nr+1][nc]+Pixle[nr][nc+1]+Pixle[nr][nc-1]<2))         
      {
        Pixle[nr][nc]=0;
      }	
    }	  
  }  
}



/***************************************************************
* 
* 函数名称：SendPicture 
* 功能说明：发送图像到上位机 ，不同的上位机注意修改对应的数据接收协议
* 参数说明： 
* 函数返回：void 
* 修改时间：2018年3月27日 
* 备 注： 
***************************************************************/ 
void UARTSendPicture(uint8_t  tmImage[IMAGEH][IMAGEW]) 
{ 
  int i = 0, j = 0; 
  UART_PutChar(UART4,0x01); //发送帧头标志 WindowsFormsApplication1.exe
  UART_PutChar(UART4,0xfe); //发送帧头标志 WindowsFormsApplication1.exe
  for(i=0;i < IMAGEH; i++) 
  { 
    for(j=0;j <IMAGEW ;j++) 
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

/*************************************************************** 
* 
* 函数名称：uint8_t GetOSTU(uint8_t tmImage[IMAGEH][IMAGEW]) 
* 功能说明：求阈值大小 
* 参数说明： 
* 函数返回：阈值大小 
* 修改时间：2018年3月27日 
* 备 注： 
参考：https://blog.csdn.net/zyzhangyue/article/details/45841255
      https://www.cnblogs.com/moon1992/p/5092726.html
      https://www.cnblogs.com/zhonghuasong/p/7250540.html     
Ostu方法又名最大类间差方法，通过统计整个图像的直方图特性来实现全局阈值T的自动选取，其算法步骤为：
1) 先计算图像的直方图，即将图像所有的像素点按照0~255共256个bin，统计落在每个bin的像素点数量
2) 归一化直方图，也即将每个bin中像素点数量除以总的像素点
3) i表示分类的阈值，也即一个灰度级，从0开始迭代
4) 通过归一化的直方图，统计0~i 灰度级的像素(假设像素值在此范围的像素叫做前景像素) 所占整幅图像的比例w0，并统计前景像素的平均灰度u0；统计i~255灰度级的像素(假设像素值在此范围的像素叫做背景像素) 所占整幅图像的比例w1，并统计背景像素的平均灰度u1；
5) 计算前景像素和背景像素的方差 g = w0*w1*(u0-u1) (u0-u1)
6) i++；转到4)，直到i为256时结束迭代
7）将最大g相应的i值作为图像的全局阈值
缺陷:OSTU算法在处理光照不均匀的图像的时候，效果会明显不好，因为利用的是全局像素信息。
***************************************************************/ 
uint8_t GetOSTU(uint8_t tmImage[LCDH][LCDW]) //Image_Use为上面输入参数，数值大是白色，说明灰度值大是白色
{ 
  int16_t i,j; 
  uint32_t Amount = 0; 
  uint32_t PixelBack = 0; 
  uint32_t PixelIntegralBack = 0; 
  uint32_t PixelIntegral = 0; 
  int32_t PixelIntegralFore = 0; 
  int32_t PixelFore = 0; 
  double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // 类间方差; 
  int16_t MinValue, MaxValue; 
  uint8_t Threshold = 0;
  uint8_t HistoGram[256];              //  

  for (j = 0; j < 256; j++)  HistoGram[j] = 0; //初始化灰度直方图 
  
  for (j = 0; j < LCDH; j++) 
  { 
    for (i = 0; i < LCDW; i++) 
    { 
      HistoGram[tmImage[j][i]]++; //统计灰度级中每个像素在整幅图像中的个数
    } 
  } 
  
  for (MinValue = 0; MinValue < 256 && HistoGram[MinValue] == 0; MinValue++) ;        //获取最小灰度的值
  for (MaxValue = 255; MaxValue > MinValue && HistoGram[MaxValue] == 0; MaxValue--) ; //获取最大灰度的值      //改了
      
  if (MaxValue == MinValue)     return MaxValue;         // 图像中只有一个颜色    
  if (MinValue + 1 == MaxValue)  return MinValue;        // 图像中只有二个颜色
    
  for (j = MinValue; j <= MaxValue; j++)    Amount += HistoGram[j];        //  像素总数
    
  PixelIntegral = 0;
  for (j = MinValue; j <= MaxValue; j++)
  {
    PixelIntegral += HistoGram[j] * j;//灰度值总数
  }
  SigmaB = -1;
  for (j = MinValue; j < MaxValue; j++)
  {
    PixelBack = PixelBack + HistoGram[j];   //前景像素点数       //注释前、背景是对的
    PixelFore = Amount - PixelBack;         //背景像素点数
    OmegaBack = (double)PixelBack / Amount;//前景像素百分比
    OmegaFore = (double)PixelFore / Amount;//背景像素百分比
    PixelIntegralBack += HistoGram[j] * j;  //前景灰度值
    PixelIntegralFore = PixelIntegral - PixelIntegralBack;//背景灰度值
    MicroBack = (double)PixelIntegralBack / PixelBack;   //前景灰度百分比
    MicroFore = (double)PixelIntegralFore / PixelFore;   //背景灰度百分比
    Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);//计算类间方差
    if (Sigma > SigmaB)                    //遍历最大的类间方差g //找出最大类间方差以及对应的阈值
    {
      SigmaB = Sigma;
      Threshold = j;
    }
  }
  return Threshold;                        //返回最佳阈值;
} 


/***************************************************************
* 
* 函数名称：sauvola
* 功能说明：用sauvola二值化算法对输入图像二值化
* 参数说明：grayImage：输入需二值化的二维数组，k：修正参数，windowSize：窗口大小
* 函数返回：void 
* 修改时间：2020年2月3日
* 备 注： sauvola二值化算法
***************************************************************/ 
void sauvola(uint8_t grayImage[LCDH][LCDW],float k,int windowSize)
{	
	int whalf = windowSize >> 1;//窗口的一半
	
	int i,j;

	// create the integral image，创建积分图，用的不是增量式
	//unsigned long ** integralImg = (unsigned long**)malloc(LCDH*sizeof(unsigned long*));
	//unsigned long ** integralImgSqrt = (unsigned long**)malloc(LCDH*sizeof(unsigned long*));
        //if (NULL==integralImg||NULL==integralImgSqrt) return;
        
        /*for (int t=0;t<LCDH;t++)
        {
        integralImg[t]=(unsigned long*)malloc(LCDW*sizeof(unsigned long));
        integralImgSqrt[t]=(unsigned long*)malloc(LCDW*sizeof(unsigned long));
        
        //if (NULL==integralImg[t]||NULL==integralImgSqrt[t]) return;
        }
        */
        unsigned long integralImg[LCDH][LCDW];
        unsigned long integralImgSqrt[LCDH][LCDW];
	int sum = 0;
	int sqrtsum = 0;
	
	for (i=0; i<LCDH; i++)
	{
		// reset this column sum，重置此列的总和
		sum = 0;
		sqrtsum = 0;
 
		for (j=0; j<LCDW; j++)
		{
			
			sum += Image_Use[i][j];//该行灰度值之和
			sqrtsum +=Image_Use[i][j] * Image_Use[i][j];//integralImgSqrt数组：存放矩形内所有像素点的灰度值平方之和
 
			if (i==0)//第0行不需要加上前几行
			{
				integralImg[i][j] = sum;
				integralImgSqrt[i][j] = sqrtsum;
			}
			else
			{
				integralImgSqrt[i][j] = integralImgSqrt[i-1][j] + sqrtsum;
				integralImg[i][j] = integralImg[i-1][j] + sum;//之前几行的加上该行
			}
		}
	}
	
	//Calculate the mean and standard deviation using the integral image,使用积分图像计算平均值和标准偏差
	int xmin,ymin,xmax,ymax;
	double mean,std,threshold;
	double diagsum,idiagsum,diff,sqdiagsum,sqidiagsum,sqdiff,area;
 
	for (i=0; i<LCDH; i++){
		for (j=0; j<LCDW; j++){
			xmin = MAX(0,i - whalf);//存在最少为半个，中心点在边界的窗口
			ymin = MAX(0,j - whalf);
			xmax = MIN(LCDW-1,i+whalf);
			ymax = MIN(LCDH-1,j+whalf);
			
			area = (xmax - xmin + 1) * (ymax - ymin + 1);//对的，要加1
			if(area <= 0)
			{
				Pixle[i][j]=1;//讲道理此处的面积不能为0
				continue;
			}
			
			if(xmin == 0 && ymin == 0){//分类讨论
				diff = integralImg[ymax][xmax];
				sqdiff = integralImgSqrt[ymax][xmax];
			}else if(xmin > 0 && ymin == 0){
				diff = integralImg[ymax][xmax] - integralImg[ymax][xmin - 1];
				sqdiff = integralImgSqrt[ymax][xmax] - integralImgSqrt[ymax][xmin - 1];	
			}else if(xmin == 0 && ymin > 0){
				diff = integralImg[ymax][xmax] - integralImg[ymin - 1][xmax];
				sqdiff = integralImgSqrt[ymax][xmax] - integralImgSqrt[ymin - 1][xmax];;
			}else{
				diagsum = integralImg[ymax][xmax] + integralImg[ymin - 1][ xmin - 1];
				idiagsum = integralImg[ymin - 1][xmax] + integralImg[ymax][xmin - 1];
				diff = diagsum - idiagsum;//积分表求和法
 
				sqdiagsum = integralImgSqrt[ymax][xmax] + integralImgSqrt[ymin - 1][xmin - 1];
				sqidiagsum = integralImgSqrt[ymin - 1][xmax] + integralImgSqrt[ymax][xmin - 1];
				sqdiff = sqdiagsum - sqidiagsum;
			}
 
			mean = diff/area;//灰度之和/像素总数           //下面的：推导出来(area-1)应该没有-1，程序来源处者也不懂
			std  = sqrt((sqdiff - diff*diff/area)/(area-1));//sqdiff:矩形内每个灰度值平方之和，其实该公式经过推导化简详见笔记
			threshold = mean*(1+k*((std/128)-1));//mean:窗口内的灰度均值，k修正参数一般0<k<1
                        //根据Sauvola计算公式和以(i,j)为中心点的w邻域内的灰度均值与标准方差来计算当前点(i,j)的二值化阈值
			if(Image_Use[i][j] < threshold)
				Pixle[i][j]=0;
			else
				Pixle[i][j]=1;	
		}
	}
	/*for (int t=0;t<LCDH;t++)
        {
	free(integralImg[t]);
	free(integralImgSqrt[t]);
        }
        free(integralImg);
	free(integralImgSqrt);*/
}


/***************************************************************
* 
* 函数名称：Median_filtering
* 功能说明：中值滤波算法
* 参数说明：height：输入处理的图像高度，width：输入处理的图像宽度，(*data)[LCDW]：待处理的二维数组，windowsize：窗口大小
* 函数返回：void 
* 修改时间：2020年2月3日
* 备 注： Image_filter为滤波后数组,边界未处理
***************************************************************/ 
void Median_filtering(int height,int width,uint8_t (*data)[LCDW],int windowsize)//windowsize是奇数
{
     int whalf = windowsize >> 1;//窗口的一半
     unsigned char * pixel= (unsigned char*)malloc(windowsize*windowsize*sizeof(unsigned char));//滑动窗口的像素值，初始为0
     
     unsigned char mid;//中值
     unsigned char temp;//中间变量
     int flag;
     int m,i,j,x,y,k;
     for(j=0;j<height;j++)
     {
       for(i=0;i<width;i++)
       {
         if(j<whalf||i<whalf||j>=height-whalf||i>=width-whalf)//边界未作处理////////////////////////////////////////////////有bug左边界黑了几列
         {
           Image_filter[j][i]=data[j][i];
         }
         else
         {
            //将windowsize×windowsize滑动窗口中的所有像素值放入pixel[m]
            m=0;
            for(y=j-whalf;y<=j+whalf;y++)
              for(x=i-whalf;x<=i+whalf;x++)//逐行放
                pixel[m++]=data[y][x];
            k=0;
            //让一位数组pixel[windowsize×windowsize]进行降序排列
            do 
            {
               k++;
              flag=0;//循环结束的标志
              for(m=0;m<windowsize*windowsize-k;m++)
              { 
                if(pixel[m]<pixel[m+1])
                {
                temp=pixel[m];
                pixel[m]=pixel[m+1];
                pixel[m+1]=temp;
                flag=1;
                }
              }//for
            } while (flag==1);
            mid=pixel[(windowsize*windowsize-1)/2];
           Image_filter[j][i]=mid;
         }
       }
     }
     free(pixel);
}

void Gaussian_filtering(int height,int width,uint8_t (*data)[LCDW])
{
  
  int whalf=SIZE>>1;//窗口的一半
    int i,j,x,y,y1,x1;
    double sum=0;
     for(j=0;j<height;j++)
     {
       for(i=0;i<width;i++)
       {
         if(j<whalf||i<whalf||j>=height-whalf||i>=width-whalf)//边界未作处理，////////////////////////////////////////////////有bug左边界黑了几列
         {
           Image_filter[j][i]=data[j][i];
         }
         else
         {
           
         sum=0;
         for(y=j-whalf,y1=0;y<=j+whalf;y++,y1++)
         { 
           for(x=i-whalf,x1=0;x<=i+whalf;x++,x1++)                                        
            {/*                                    ////////////////////////////////////////////////////////////////////////////有bug一半黑了

              if(y<0 && x<0)                 sum+=Gaussian_Temp[y1][x1]*data[0][0];              //边界处理， 赋边界值，想象图像是无限制长，但是默认赋值的不是0而是对应边界点的值
              else if(y<0 && x>=width)       sum+=Gaussian_Temp[y1][x1]*data[0][width-1];
              else if(y>=height && x<0)      sum+=Gaussian_Temp[y1][x1]*data[height-1][0];
              else if(y>=height && x>=width) sum+=Gaussian_Temp[y1][x1]*data[height-1][width-1];
               else if(y<0)                  sum+=Gaussian_Temp[y1][x1]*data[0][x];
               else if(x<0)                  sum+=Gaussian_Temp[y1][x1]*data[y][0];
               else if(y>=height)            sum+=Gaussian_Temp[y1][x1]*data[height-1][x];
               else if(x>=height)            sum+=Gaussian_Temp[y1][x1]*data[y][width-1];
              else*/
               sum+=Gaussian_Temp[y1][x1]*data[y][x];//边界之内
            }
         }
         Image_filter[j][i]=(uint8_t)fabs(sum);
         
         }
       }
     }
       
}

void Gaussian_Init(void)
{
        //double weight;
	double sum = 0;
	double Sigmma = 1;
	int i,j;

	//weight = 2*M_PI*Sigmma*Sigmma;在归一化后能消掉，不用算了
	for(i =0;i <SIZE;i++)
	{
		for(j = 0;j < SIZE;j++)
		{
			Gaussian_Temp[i][j] =exp(-((i-SIZE/2)*(i-SIZE/2)+(j-SIZE/2)*(j-SIZE/2))/(2.0*Sigmma*Sigmma));
			sum += Gaussian_Temp[i][j];
		}
	}

	for(i = 0; i < SIZE;i++)
	   for(j = 0;j < SIZE;j++)
	      Gaussian_Temp[i][j] = Gaussian_Temp[i][j]/sum;//归一化处理
			
}


