/*---------------------------------------------------------------
��ƽ    ̨������K60���İ�-���ܳ���
����    д��LQ-005
��E-mail  ��chiusir@163.com
������汾��V1.0������Դ���룬�����ο�������Ը�
�������¡�2019��04��02��
����    վ��http://www.lqist.cn
���Ա����̡�http://shop36265907.taobao.com
������ƽ̨��IAR 8.2
����    �ܡ�OV7725����
��ע�����
----------------------------------------------------------------*/
#include "include.h"
#include "LQ_OV7725.h"

uint8_t Image_Data[IMAGEH][IMAGEW];      //ͼ��ԭʼ���ݴ��
uint8_t Image_Use[LCDH][LCDW];           //ѹ����֮�����ڴ����Ļ��ʾ����
uint8_t Image_filter[LCDH][LCDW];        //�����˲��������
double Gaussian_Temp[SIZE][SIZE] = {0};       //��˹ģ��ֵ

uint8_t Pixle[LCDH][LCDW];               //��ֵ��������OLED��ʾ������
uint8_t  Line_Cont=0;                    //�м���
uint8_t  Field_Over_Flag=0;              //����ʶ


/*---------------------------------------------------------------
����    ����PORTC_Interrupt
����    �ܡ�PORTC�˿ڵ��жϷ����� ��������ͷ�г��ж�
����    ������
���� �� ֵ����
��ע�����ע������Ҫ����жϱ�־λ
----------------------------------------------------------------*/
void PORTD_IRQHandler(void)
{     
  //���ж�PTD14
  int n;    
    n=14;   //���ж�
  if((PORTD_ISFR & (1<<n) ))//���ж� (1<<14)
  {    
    PORTD_ISFR |= (1<<n);   //����жϱ�ʶ
    // �û�����            
    DMATransDataStart(DMA_CH4,(uint32_t)(&Image_Data[Line_Cont][0]));   //����DMA���� 
    if(Line_Cont > IMAGEH)  //�ɼ�����
    { 
      Line_Cont=0; 
      return ;
    } 
    ++Line_Cont;            //�м���
    return ; 
  }
  //���ж�PTD15
  n=15;  //���ж�
  if((PORTD_ISFR & (1<<n)))//(1<<15)
  {
    PORTD_ISFR |= (1<<n);  //����жϱ�ʶ
    // �û����� 
    Line_Cont = 0;         //�м�������
    Field_Over_Flag=1;     //��������ʶ
  } 
}

/*---------------------------------------------------------------
����    ����Test_OV7725
����    �ܡ�����OV7725 ��OLED����ʾͼ��
����    ������
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
void Test_OV7725(void)
{  
    LED_Init();
    TFTSPI_Init(0);                //TFT1.8��ʼ��  0��������ʾ  1��������ʾ  
    TFTSPI_CLS(u16BLUE);           //����
    
    /*  ����ͷ��ʼ�� */
    OV7725_Init(50);               //����ͷ��ʼ��
    
    //����ͷ�йؽӿڳ�ʼ��
    GPIO_ExtiInit(PTD14,rising_down);   //���ж�
    GPIO_ExtiInit(PTD15,falling_up);    //���ж�  
    
    /* ���ȼ����� ��ռ���ȼ�0  �����ȼ�2   ԽС���ȼ�Խ��  ��ռ���ȼ��ɴ�ϱ���ж� */
    NVIC_SetPriority(PORTD_IRQn,NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,2));
    NVIC_EnableIRQ(PORTD_IRQn);			         //ʹ��PORTD_IRQn���ж� 
    
    //��λ���������
    GPIO_PinInit(PTD0,GPI,0);                
    GPIO_PinInit(PTD1,GPI,0);
    GPIO_PinInit(PTD2,GPI,0);
    GPIO_PinInit(PTD3,GPI,0);
    GPIO_PinInit(PTD4,GPI,0);
    GPIO_PinInit(PTD5,GPI,0);
    GPIO_PinInit(PTD6,GPI,0);
    GPIO_PinInit(PTD7,GPI,0);     
    
    //��ʼ��DMA�ɼ�       ͨ��4        PTD0-PTD7           Ŀ�ĵ�ַ      �����ź� ÿ��һ��BYTE  ����    ���������ش���
    DMA_PORTx2BUFF_Init (DMA_CH4, (void *)&PTD_BYTE0_IN,(void*)Image_Data, PTD13, DMA_BYTE1, (IMAGEW ), DMA_rising_down); 
    /* ����ͷ��ʼ������ */
    
     Gaussian_Init();
    while(1)
    { 
        LED_Reverse(1);           //LEDָʾ��������״̬
        if(Field_Over_Flag)       //���һ��ͼ��ɼ�
        { 
            Get_Use_Image();      //�Ӳɼ�ͼ��������ȡ���Լ���Ҫʹ�õĴ�С
           Gaussian_filtering(LCDH,LCDW,Image_Use);
            // Median_filtering(LCDH,LCDW,Image_Use,3);
            TFTSPI_Show_Cmera(0, 0, 160, 120, Image_filter);      //��ֵ��ͼ������

            Field_Over_Flag= 0;       
        }    
    }
}


/*---------------------------------------------------------------
����    ����Get_Use_Image
����    �ܡ���ȡ��Ҫʹ�õ�ͼ���С
����    ������
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
__ramfunc void Get_Use_Image(void)
{
  int i = 0,j = 0,row = 0,line = 0;
  int div_h = IMAGEH/LCDH;
  int div_w = IMAGEW/LCDW;
  for(i = 0; i  < IMAGEH; i+=div_h)  //240�У�ÿ4�вɼ�һ�У�
  {
    for(j = 0;j < IMAGEW; j+=div_w)  //320 / 4  = 80��
    {        
      Image_Use[row][line] = Image_Data[i][j];         
      line++;        
    }      
    line = 0;
    row++;      
  }  
}

/*---------------------------------------------------------------
����    ����Get_01_Value
����    �ܡ���ֵ��
����    ����mode  ��  0��ʹ�ô����ֵ    1��ʹ��ƽ����ֵ
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
void Get_01_Value(uint8_t mode,uint8_t Image[LCDH][LCDW])
{
  int i = 0,j = 0;
  uint8_t Threshold;
  uint32_t  tv=0;
  char txt[16];
  if(mode)
  {
      //�ۼ�
      for(i = 0; i <LCDH; i++)
      {    
          for(j = 0; j <LCDW; j++)
          {                            
              tv+=Image[i][j];   //�ۼ�  
          } 
      }
      Threshold=tv/LCDH/LCDW;        //��ƽ��ֵ,����Խ��ԽС��ȫ��Լ35��������ĻԼ160��һ������´�Լ100
      Threshold=Threshold*7/10+10;   //�˴���ֵ���ã����ݻ����Ĺ������趨 
  }
  else
  {
      Threshold = GetOSTU(Image);//�����ֵ
      Threshold = (uint8_t)(Threshold * 0.5) + 70;
  } 
  sprintf(txt,"%03d",Threshold);//ǰ��Ϊ�����õ���ֵ������Ϊƽ��ֵ 
#ifdef LQ_OLED 
  OLED_P6x8Str(80,1,(u8*)txt);
#else
  TFTSPI_P8X8Str(0,10,(u8*)txt,u16RED,u16BLUE);
#endif
  for(i = 0; i < LCDH; i++)
  {
    for(j = 0; j < LCDW; j++)
    {                                
      if(Image[i][j] >Threshold) //��ֵԽ����ʾ������Խ�࣬��ǳ��ͼ��Ҳ����ʾ����    
        Pixle[i][j] =1; //1�ǰ�ɫ��0�Ǻ�ɫ����������TFTSPI_Show_Cmera ����   
      else                                        
        Pixle[i][j] =0;
    }    
  }
}

/*---------------------------------------------------------------
����    ����TFTSPI_Show_Cmera
����    �ܡ���TFT1.8�ϻ�������ͷ��ͼ��
����    ����xs:  ��ʼX   
����    ����ys:  ��ʼY  
����    ����w:   ͼƬ��� 
����    ����h:   ͼƬ�߶�  
����    ����ppic[IMAGEH][IMAGEW]:  ͼƬ������   
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/	
void TFTSPI_Show_Cmera(uint8_t xs,uint8_t ys,uint8_t w,uint8_t h,uint8_t ppic[LCDH][LCDW]) 
{
#if 1    //�Ҷ���ʾ
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
#else    //��ֵ����ʾ
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
����    ����Draw_Road
����    �ܡ���OLED�ϻ�������ͷ��ͼ��
����    ������
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
void Draw_Road(void)
{ 	 
  u8 i = 0, j = 0,temp=0;
  
  //����֡ͷ��־
  for(i=8;i<56;i+=8)//6*8=48�� 
  {
    OLED_Set_Pos(24,i/8+1);//��ʼλ��
    for(j=0;j<LCDW;j++)  //����
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
����    ����Pixle_Filter
����    �ܡ��������
����    ������
���� �� ֵ����
��ע�����
----------------------------------------------------------------*/
void Pixle_Filter(void)
{  
  int nr; //��
  int nc; //��
  
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
* �������ƣ�SendPicture 
* ����˵��������ͼ����λ�� ����ͬ����λ��ע���޸Ķ�Ӧ�����ݽ���Э��
* ����˵���� 
* �������أ�void 
* �޸�ʱ�䣺2018��3��27�� 
* �� ע�� 
***************************************************************/ 
void UARTSendPicture(uint8_t  tmImage[IMAGEH][IMAGEW]) 
{ 
  int i = 0, j = 0; 
  UART_PutChar(UART4,0x01); //����֡ͷ��־ WindowsFormsApplication1.exe
  UART_PutChar(UART4,0xfe); //����֡ͷ��־ WindowsFormsApplication1.exe
  for(i=0;i < IMAGEH; i++) 
  { 
    for(j=0;j <IMAGEW ;j++) 
    { 
      if(tmImage[i][j]==0xfe) 
      { 
        tmImage[i][j]=0xff; //��ֹ���ͱ�־λ 
      } 
      UART_PutChar(UART4,tmImage[i][j]); 
    } 
  }
  UART_PutChar(UART4,0xfe); //����֡β��־ 
  UART_PutChar(UART4,0x01); //����֡β��־ 
} 

/*************************************************************** 
* 
* �������ƣ�uint8_t GetOSTU(uint8_t tmImage[IMAGEH][IMAGEW]) 
* ����˵��������ֵ��С 
* ����˵���� 
* �������أ���ֵ��С 
* �޸�ʱ�䣺2018��3��27�� 
* �� ע�� 
�ο���https://blog.csdn.net/zyzhangyue/article/details/45841255
      https://www.cnblogs.com/moon1992/p/5092726.html
      https://www.cnblogs.com/zhonghuasong/p/7250540.html     
Ostu������������������ͨ��ͳ������ͼ���ֱ��ͼ������ʵ��ȫ����ֵT���Զ�ѡȡ�����㷨����Ϊ��
1) �ȼ���ͼ���ֱ��ͼ������ͼ�����е����ص㰴��0~255��256��bin��ͳ������ÿ��bin�����ص�����
2) ��һ��ֱ��ͼ��Ҳ����ÿ��bin�����ص����������ܵ����ص�
3) i��ʾ�������ֵ��Ҳ��һ���Ҷȼ�����0��ʼ����
4) ͨ����һ����ֱ��ͼ��ͳ��0~i �Ҷȼ�������(��������ֵ�ڴ˷�Χ�����ؽ���ǰ������) ��ռ����ͼ��ı���w0����ͳ��ǰ�����ص�ƽ���Ҷ�u0��ͳ��i~255�Ҷȼ�������(��������ֵ�ڴ˷�Χ�����ؽ�����������) ��ռ����ͼ��ı���w1����ͳ�Ʊ������ص�ƽ���Ҷ�u1��
5) ����ǰ�����غͱ������صķ��� g = w0*w1*(u0-u1) (u0-u1)
6) i++��ת��4)��ֱ��iΪ256ʱ��������
7�������g��Ӧ��iֵ��Ϊͼ���ȫ����ֵ
ȱ��:OSTU�㷨�ڴ�����ղ����ȵ�ͼ���ʱ��Ч�������Բ��ã���Ϊ���õ���ȫ��������Ϣ��
***************************************************************/ 
uint8_t GetOSTU(uint8_t tmImage[LCDH][LCDW]) //Image_UseΪ���������������ֵ���ǰ�ɫ��˵���Ҷ�ֵ���ǰ�ɫ
{ 
  int16_t i,j; 
  uint32_t Amount = 0; 
  uint32_t PixelBack = 0; 
  uint32_t PixelIntegralBack = 0; 
  uint32_t PixelIntegral = 0; 
  int32_t PixelIntegralFore = 0; 
  int32_t PixelFore = 0; 
  double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // ��䷽��; 
  int16_t MinValue, MaxValue; 
  uint8_t Threshold = 0;
  uint8_t HistoGram[256];              //  

  for (j = 0; j < 256; j++)  HistoGram[j] = 0; //��ʼ���Ҷ�ֱ��ͼ 
  
  for (j = 0; j < LCDH; j++) 
  { 
    for (i = 0; i < LCDW; i++) 
    { 
      HistoGram[tmImage[j][i]]++; //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    } 
  } 
  
  for (MinValue = 0; MinValue < 256 && HistoGram[MinValue] == 0; MinValue++) ;        //��ȡ��С�Ҷȵ�ֵ
  for (MaxValue = 255; MaxValue > MinValue && HistoGram[MaxValue] == 0; MaxValue--) ; //��ȡ���Ҷȵ�ֵ      //����
      
  if (MaxValue == MinValue)     return MaxValue;         // ͼ����ֻ��һ����ɫ    
  if (MinValue + 1 == MaxValue)  return MinValue;        // ͼ����ֻ�ж�����ɫ
    
  for (j = MinValue; j <= MaxValue; j++)    Amount += HistoGram[j];        //  ��������
    
  PixelIntegral = 0;
  for (j = MinValue; j <= MaxValue; j++)
  {
    PixelIntegral += HistoGram[j] * j;//�Ҷ�ֵ����
  }
  SigmaB = -1;
  for (j = MinValue; j < MaxValue; j++)
  {
    PixelBack = PixelBack + HistoGram[j];   //ǰ�����ص���       //ע��ǰ�������ǶԵ�
    PixelFore = Amount - PixelBack;         //�������ص���
    OmegaBack = (double)PixelBack / Amount;//ǰ�����ذٷֱ�
    OmegaFore = (double)PixelFore / Amount;//�������ذٷֱ�
    PixelIntegralBack += HistoGram[j] * j;  //ǰ���Ҷ�ֵ
    PixelIntegralFore = PixelIntegral - PixelIntegralBack;//�����Ҷ�ֵ
    MicroBack = (double)PixelIntegralBack / PixelBack;   //ǰ���ҶȰٷֱ�
    MicroFore = (double)PixelIntegralFore / PixelFore;   //�����ҶȰٷֱ�
    Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);//������䷽��
    if (Sigma > SigmaB)                    //����������䷽��g //�ҳ������䷽���Լ���Ӧ����ֵ
    {
      SigmaB = Sigma;
      Threshold = j;
    }
  }
  return Threshold;                        //���������ֵ;
} 


/***************************************************************
* 
* �������ƣ�sauvola
* ����˵������sauvola��ֵ���㷨������ͼ���ֵ��
* ����˵����grayImage���������ֵ���Ķ�ά���飬k������������windowSize�����ڴ�С
* �������أ�void 
* �޸�ʱ�䣺2020��2��3��
* �� ע�� sauvola��ֵ���㷨
***************************************************************/ 
void sauvola(uint8_t grayImage[LCDH][LCDW],float k,int windowSize)
{	
	int whalf = windowSize >> 1;//���ڵ�һ��
	
	int i,j;

	// create the integral image����������ͼ���õĲ�������ʽ
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
		// reset this column sum�����ô��е��ܺ�
		sum = 0;
		sqrtsum = 0;
 
		for (j=0; j<LCDW; j++)
		{
			
			sum += Image_Use[i][j];//���лҶ�ֵ֮��
			sqrtsum +=Image_Use[i][j] * Image_Use[i][j];//integralImgSqrt���飺��ž������������ص�ĻҶ�ֵƽ��֮��
 
			if (i==0)//��0�в���Ҫ����ǰ����
			{
				integralImg[i][j] = sum;
				integralImgSqrt[i][j] = sqrtsum;
			}
			else
			{
				integralImgSqrt[i][j] = integralImgSqrt[i-1][j] + sqrtsum;
				integralImg[i][j] = integralImg[i-1][j] + sum;//֮ǰ���еļ��ϸ���
			}
		}
	}
	
	//Calculate the mean and standard deviation using the integral image,ʹ�û���ͼ�����ƽ��ֵ�ͱ�׼ƫ��
	int xmin,ymin,xmax,ymax;
	double mean,std,threshold;
	double diagsum,idiagsum,diff,sqdiagsum,sqidiagsum,sqdiff,area;
 
	for (i=0; i<LCDH; i++){
		for (j=0; j<LCDW; j++){
			xmin = MAX(0,i - whalf);//��������Ϊ��������ĵ��ڱ߽�Ĵ���
			ymin = MAX(0,j - whalf);
			xmax = MIN(LCDW-1,i+whalf);
			ymax = MIN(LCDH-1,j+whalf);
			
			area = (xmax - xmin + 1) * (ymax - ymin + 1);//�Եģ�Ҫ��1
			if(area <= 0)
			{
				Pixle[i][j]=1;//������˴����������Ϊ0
				continue;
			}
			
			if(xmin == 0 && ymin == 0){//��������
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
				diff = diagsum - idiagsum;//���ֱ���ͷ�
 
				sqdiagsum = integralImgSqrt[ymax][xmax] + integralImgSqrt[ymin - 1][xmin - 1];
				sqidiagsum = integralImgSqrt[ymin - 1][xmax] + integralImgSqrt[ymax][xmin - 1];
				sqdiff = sqdiagsum - sqidiagsum;
			}
 
			mean = diff/area;//�Ҷ�֮��/��������           //����ģ��Ƶ�����(area-1)Ӧ��û��-1��������Դ����Ҳ����
			std  = sqrt((sqdiff - diff*diff/area)/(area-1));//sqdiff:������ÿ���Ҷ�ֵƽ��֮�ͣ���ʵ�ù�ʽ�����Ƶ���������ʼ�
			threshold = mean*(1+k*((std/128)-1));//mean:�����ڵĻҶȾ�ֵ��k��������һ��0<k<1
                        //����Sauvola���㹫ʽ����(i,j)Ϊ���ĵ��w�����ڵĻҶȾ�ֵ���׼���������㵱ǰ��(i,j)�Ķ�ֵ����ֵ
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
* �������ƣ�Median_filtering
* ����˵������ֵ�˲��㷨
* ����˵����height�����봦���ͼ��߶ȣ�width�����봦���ͼ���ȣ�(*data)[LCDW]��������Ķ�ά���飬windowsize�����ڴ�С
* �������أ�void 
* �޸�ʱ�䣺2020��2��3��
* �� ע�� Image_filterΪ�˲�������,�߽�δ����
***************************************************************/ 
void Median_filtering(int height,int width,uint8_t (*data)[LCDW],int windowsize)//windowsize������
{
     int whalf = windowsize >> 1;//���ڵ�һ��
     unsigned char * pixel= (unsigned char*)malloc(windowsize*windowsize*sizeof(unsigned char));//�������ڵ�����ֵ����ʼΪ0
     
     unsigned char mid;//��ֵ
     unsigned char temp;//�м����
     int flag;
     int m,i,j,x,y,k;
     for(j=0;j<height;j++)
     {
       for(i=0;i<width;i++)
       {
         if(j<whalf||i<whalf||j>=height-whalf||i>=width-whalf)//�߽�δ������////////////////////////////////////////////////��bug��߽���˼���
         {
           Image_filter[j][i]=data[j][i];
         }
         else
         {
            //��windowsize��windowsize���������е���������ֵ����pixel[m]
            m=0;
            for(y=j-whalf;y<=j+whalf;y++)
              for(x=i-whalf;x<=i+whalf;x++)//���з�
                pixel[m++]=data[y][x];
            k=0;
            //��һλ����pixel[windowsize��windowsize]���н�������
            do 
            {
               k++;
              flag=0;//ѭ�������ı�־
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
  
  int whalf=SIZE>>1;//���ڵ�һ��
    int i,j,x,y,y1,x1;
    double sum=0;
     for(j=0;j<height;j++)
     {
       for(i=0;i<width;i++)
       {
         if(j<whalf||i<whalf||j>=height-whalf||i>=width-whalf)//�߽�δ������////////////////////////////////////////////////��bug��߽���˼���
         {
           Image_filter[j][i]=data[j][i];
         }
         else
         {
           
         sum=0;
         for(y=j-whalf,y1=0;y<=j+whalf;y++,y1++)
         { 
           for(x=i-whalf,x1=0;x<=i+whalf;x++,x1++)                                        
            {/*                                    ////////////////////////////////////////////////////////////////////////////��bugһ�����

              if(y<0 && x<0)                 sum+=Gaussian_Temp[y1][x1]*data[0][0];              //�߽紦�� ���߽�ֵ������ͼ���������Ƴ�������Ĭ�ϸ�ֵ�Ĳ���0���Ƕ�Ӧ�߽���ֵ
              else if(y<0 && x>=width)       sum+=Gaussian_Temp[y1][x1]*data[0][width-1];
              else if(y>=height && x<0)      sum+=Gaussian_Temp[y1][x1]*data[height-1][0];
              else if(y>=height && x>=width) sum+=Gaussian_Temp[y1][x1]*data[height-1][width-1];
               else if(y<0)                  sum+=Gaussian_Temp[y1][x1]*data[0][x];
               else if(x<0)                  sum+=Gaussian_Temp[y1][x1]*data[y][0];
               else if(y>=height)            sum+=Gaussian_Temp[y1][x1]*data[height-1][x];
               else if(x>=height)            sum+=Gaussian_Temp[y1][x1]*data[y][width-1];
              else*/
               sum+=Gaussian_Temp[y1][x1]*data[y][x];//�߽�֮��
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

	//weight = 2*M_PI*Sigmma*Sigmma;�ڹ�һ��������������������
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
	      Gaussian_Temp[i][j] = Gaussian_Temp[i][j]/sum;//��һ������
			
}


