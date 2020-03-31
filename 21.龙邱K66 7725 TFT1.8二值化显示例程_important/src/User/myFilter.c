#include "include.h"

double Gaussian_Temp[SIZE][SIZE] = {0};       //��˹ģ��ֵ��һ��ֱ��ȡģ��
uint8_t Image_filter[LCDH][LCDW];        //�����˲��������

/*---------------------------------------------------------------
����    ����Pixle_Filter
����    �ܡ��������
����    ������
���� �� ֵ����
��ע����������˲����Զ�ֵ��ͼ���˲�
----------------------------------------------------------------*/
void Pixle_Filter(uint8_t Pixle[LCDH][LCDW])
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

//ûɶ�ã�һ��ֱ����ģ�����
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
