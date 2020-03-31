#include "include.h"

double Gaussian_Temp[SIZE][SIZE] = {0};       //高斯模板值，一般直接取模板
uint8_t Image_filter[LCDH][LCDW];        //储存滤波后的数据

/*---------------------------------------------------------------
【函    数】Pixle_Filter
【功    能】过滤噪点
【参    数】无
【返 回 值】无
【注意事项】三点滤波，对二值化图像滤波
----------------------------------------------------------------*/
void Pixle_Filter(uint8_t Pixle[LCDH][LCDW])
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

//没啥用，一般直接拿模板就行
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
