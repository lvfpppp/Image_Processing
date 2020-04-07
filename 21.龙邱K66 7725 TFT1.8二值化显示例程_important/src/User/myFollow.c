#include "include.h"

//extern uint8_t saidao_1[51][70];
extern uint8_t wandao[PROSH][PROSW] ;
extern void mykey();

/*********************************************************************************************
* 
* �������ƣ�SendPicture_s 
* ����˵��������ͼ����λ�� ����ͬ����λ��ע���޸Ķ�Ӧ�����ݽ���Э��
* ע  ��  ���޸���ԭSendPicture�Ĵ���ͼ���С
**********************************************************************************************/ 
static void UARTSendPicture_s(uint8_t  tmImage[PROSH][PROSW]) 
{ 
  int i = 0, j = 0; 
  UART_PutChar(UART4,0x01); //����֡ͷ��־ WindowsFormsApplication1.exe
  UART_PutChar(UART4,0xfe); //����֡ͷ��־ WindowsFormsApplication1.exe
  for(i=0;i < PROSH; i++) 
  { 
    for(j=0;j <PROSW ;j++) 
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


/************************************��С���˷���������б��***********************************
* �������ƣ�regression, y = A+Bx
* ����˵��������Middle_black�������꣬startline����Middle_black��ʼ��
* �� �� ֵ��AB[0]�ǽؾ࣬AB[1]��б�ʣ�Ҳ������
*  ע  �� ������б��Ӧ����0�������ؾ�Ӧ����LCDW/2����
*           ��ˮƽ�쳣����ݲ�����
**********************************************************************************************/
static void regression(int Middle_black[PROSH],int startline,float * AB)
{
  int i=0,SumX=0,SumY=0,SumXX=0,SumXY=0; 
  float SumUp=0,SumDown=0;
  int Length=PROSH-startline;
//������������
  for(i=startline;i<PROSH;i++)     
  { 
    SumX+=i;       
    SumY+=Middle_black[i];    //����Middle_blackΪ������ߵ�����
   SumXX+=i*i;
   SumXY+=i*Middle_black[i]; 
  }       
   SumUp=SumXY-(float)(SumX*SumY)/(float)Length;//��B��ʽ�ϲ���
   SumDown=SumXX-(float)(SumX*SumX)/(float)Length;//��B��ʽ�²���
 
  if(SumDown==0) 
   ;//б�ʹ���Ϊ�У��ȱ�������
  else 
  {
    *(AB+1)=SumUp/SumDown;       
    *AB=(float)(SumY-*(AB+1)*SumX)/(float)Length;  //�ؾ�
  }

}


/*********************************************************************************************
*�������ƣ�TFTSPI_Show_rgb565
*��    �ܣ���TFT1.8����ʾ��ɫͼ��
*����˵����xs:��ʼX��ys:��ʼY��w:ͼƬ��ȣ�h:ͼƬ�߶� ��ppic[IMAGEH][IMAGEW][2]:  ͼƬ������    
**********************************************************************************************/	
static void TFTSPI_Show_rgb565(uint8_t xs,uint8_t ys,uint8_t w,uint8_t h,uint8_t ppic[PROSH][PROSW][2]) 
{
    unsigned int i,j;	
    TFTSPI_Set_Pos(xs,ys,xs+w-1,ys+h);
    for(i = 0; i < h; i++)  	
      for(j = 0; j < w; j++) 
        TFTSPI_Write_Word((ppic[i][j][0]<<8)+ppic[i][j][1]);//��λ��ǰ��������������ϳ�һ��16λ���ݱ�ʾ����ֵ
 }

/********************************************************************************************
�������ƣ�myfollow
* ���ܣ���ѭ�����룬���������߲����
* ���룺Img[PROSH][PROSW]������ĻҶ�ͼ�񣬷���AB[0]�ǽؾ࣬AB[1]��б�ʣ�Ҳ������
* �����peak��Ϊ��ɫ�����ߺ�ɫ��������ɫ�������ð�
********************************************************************************************/
void myfollow(uint8_t Img[PROSH][PROSW],float* AB)
{
  uint32_t sum_t=0;//�����غ�
  uint8_t threshold=0;//��ֵ
  int i=0,j=0;//ѭ������
  uint8_t pixel[PROSH][PROSW]={0};
  int  width = PROSW,height = PROSH;//����˳��
  
  int peak=0;//����L��R�д���Ǹ�
  int Middle[PROSH]={0};//��������
  int derivative_2[PROSH]={0};//����������ľ��Զ��׵�
  
   int shizi_tp[4][4]={0};//ʮ���ĸ��յ�turning point�����δ洢�ϵ��£�����
  //�洢�����ؼ���Ծ�㣬�����Ƿ������㣬ʮ���Ͻ�Ծ�㣬�½�Ծ��
   
    typedef struct 
  {
    int coordinate[PROSH];//�����±�Ϊ�����꣬�洢������
    int fg_suces[PROSH];//���б����Ƿ���ȡ�ɹ���Ѱ����ߴ���fg_sucesΪ0Ӧ���������ޱ߽�
    int peak;//ͼ������Ϊֹ
    uint8_t none;//�����ڵ�һ�л�����е��ۻ�����
    uint8_t none_fg;//���none�������ڱ߽��Ƿ�ͨ�ı�־λ
    //һ�ײ�ֲ�ʮ�֣����ұ߽���������һ���ʮ�֣�matlab����ʮ���Ƿ�Բ��
  }EDGE;
  EDGE edge_L={0,0,0,0,0};//���ұ�Ե
  EDGE edge_R={0,0,0,0,0};
  
  //uint8_t show_D2L[PROSH-1][PROSW]={0};//��ʾ����һ�׵���TFT
  uint8_t show_D2[PROSH][PROSW]={0};//��ʾ���Զ��׵���TFT
  uint8_t show_t[PROSH][PROSW][2]={0};//��ʾ�������ߵ�����,show_t[PROSH][PROSW][0]��rgb565�ĸ�λ
  
  
  
  //���  
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
     sum_t+=Img[i][j];
  threshold=sum_t/(width*height);
 threshold=84;//��װ����ֵhh
 
  //��ֵ��
  for ( i=0;i<height;++i)
    for ( j=0;j<width;++j)
      if(Img[i][j]>=threshold)   
        pixel[i][j]=T_White;
      else
        pixel[i][j]=F_Black;
      
      mykey();
     TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,pixel[0]);//  ����ֵ��ͼ�� ��������������������������������������������������������������������������������������������������������������  
      
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
    
/*    for (j=0;j<width;++j)//�����˲�
      for (i=2;i<height-3;++i)
      {
        if(pixel[i-2][j]==T_White&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=T_White;pixel[i+1][j]=T_White;}
        else if (pixel[i-2][j]==F_Black&&pixel[i-2][j]==pixel[i-1][j]&&pixel[i+2][j]==pixel[i+3][j]&&pixel[i-2][j]==pixel[i+2][j])
        {pixel[i][j]=F_Black;pixel[i+1][j]=F_Black;}
      } 
*///��ʱ��Ҫ�ˣ��Ժ��������Ӱ��   
    mykey();
    TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,pixel[0]);//  ���˲���ֵ��ͼ�񡢡�����������������������������������������������������������������������������������������������������������������    
    
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
       for (j=mid+1;j<width;++j)
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

//Step 3 :������ 

 int k=0; 
  for(k=height-1;k>=0;k--)
  {
    Middle[k]=(edge_R.coordinate[k]+edge_L.coordinate[k])/2;
    if(edge_L.fg_suces[k]==0) {edge_L.peak=k;break;}//��ʼ��peakֵ
    if(edge_R.fg_suces[k]==0) {edge_R.peak=k;break;}
  }
  if(edge_L.peak==0)//ѡ���߸���ģ���ȥû����
    peak=edge_R.peak;
  else if(edge_R.peak==0)
    peak=edge_L.peak;
  else if(edge_L.peak>edge_R.peak)
    peak=edge_L.peak;
  else 
    peak=edge_R.peak;

  for(k=height-1;k>=peak+2;k--)//����Զ��׵������������Ϣ��������������������������������������������������������������������������������������������������������������������  
  {
    derivative_2[k]=abs(Middle[k]-2*Middle[k-1]+Middle[k-2]);
    show_D2[k][derivative_2[k]]=T_White;    
  }
  mykey();
  TFTSPI_CLS(u16WHITE);
  TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,show_D2[0]);

//step 4:�Բ�ͬ�����ֱ���
  for(k=peak;k<height;k++)
  {
    if(edge_L.coordinate[k]==0) edge_L.none++;
    else edge_L.none=0;
    if(edge_L.none>TSD_kind)
    {
      edge_L.none_fg=1;//���ͨ
      break;
    }
  }
    for(k=peak;k<height;k++)
  {
    if(edge_R.coordinate[k]==width-1) edge_R.none++;
    else edge_R.none=0;
    if(edge_R.none>TSD_kind)
    {
      edge_R.none_fg=1;//�ұ�ͨ
      break;
    }
  }
  /////////ʮ����������///////////,��������Ϊ�����ܻ���յ�ȽϹ⻬����������������
  if(edge_L.none_fg==1 && edge_R.none_fg==1)//ʮ��,˼·���¹յ㣺�����һ����ǰһ�����������Ϊ��һ��ֵ���Һ���ݼ����߽磬�����Ϲյ�ֱ������ע��
  {
    TFTSPI_P16x16Str(8,7,"ʮ��",u16BLACK,u16WHITE);//�����ʮ�֡�
    //�����¹յ�
    for(k=height-1;k>=peak+1;k--)
    {
      if(edge_L.coordinate[k]-edge_L.coordinate[k-1]>0)//�ҵ����¹յ���ɵ�
      {
        int m=k-1,fg_t_t=0;
        while(edge_L.coordinate[m]>0)//���ж�������Ϊ0ʱ�˳�����������
        {
          if(edge_L.coordinate[m]-edge_L.coordinate[m-1]<0)
          {
            fg_t_t=1;//�����Ϲյ��һֱ�ݼ�
            break;//while
          }
          m--;
        }
        if(fg_t_t==0)//��������
        {
          shizi_tp[2][0]=k;
          shizi_tp[2][1]=edge_L.coordinate[k];
          break;//for
        }      
      }    
    }//for
    //�����¹յ�
    for(k=height-1;k>=peak+1;k--)
    {
      if(edge_R.coordinate[k]-edge_R.coordinate[k-1]<0)//�ҵ����¹յ���ɵ�
      {
        int m=k-1,fg_t_t=0;
        while(edge_R.coordinate[m]<width-1)//���ж�������Ϊwidthʱ�˳�����������
        {
          if(edge_R.coordinate[m]-edge_R.coordinate[m-1]>0)
          {
            fg_t_t=1;//�����Ϲյ��һֱ�ݼ�
            break;//while
          }
          m--;
        }
        if(fg_t_t==0)//��������
        {
          shizi_tp[3][0]=k;
          shizi_tp[3][1]=edge_R.coordinate[k];
          break;//for
        }      
      }    
    }
    //�����Ϲյ�
    for(k=shizi_tp[2][0]-1;k>=peak+2;k--)//�����Ϲյ�Ϊ�õ���һ�㣬�õ����һ��С����һ�������һ��С�����,�Ҹõ���׵�<-TSD_d2
    {
      if(edge_L.coordinate[k]<edge_L.coordinate[k-1] && edge_L.coordinate[k-1]<=edge_L.coordinate[k-2] &&
         edge_L.coordinate[k]-2*edge_L.coordinate[k-1]+edge_L.coordinate[k-2] < -TSD_d2)
       {
          shizi_tp[0][0]=k-1;
          shizi_tp[0][1]=edge_L.coordinate[k-1];
          break;
       }      
    }
    //�����Ϲյ㣨������ڡ�©�˾ͺ����Σ�
    for(k=shizi_tp[3][0]-1;k>=peak+2;k--)//�����Ϲյ�Ϊ�õ���һ�㣬�õ����һ�����һ�������һ�������,�Ҹõ���׵�>TSD_d2
    {
       if(edge_R.coordinate[k]>edge_R.coordinate[k-1] && edge_R.coordinate[k-1]>=edge_R.coordinate[k-2] &&
          edge_R.coordinate[k]-2*edge_R.coordinate[k-1]+edge_R.coordinate[k-2] > TSD_d2)
       {
          shizi_tp[1][0]=k-1;
          shizi_tp[1][1]=edge_R.coordinate[k-1];
          break;
       }      
    }
    //����//y=(y2-y1)*(x-x1)/(x2-x1)+y1
    for(k=shizi_tp[0][0];k<=shizi_tp[2][0];k++)//y=(y2-y1)*(x-x1)/(x2-x1)+y1
      edge_L.coordinate[k]=(k-shizi_tp[0][0])*(shizi_tp[2][1]-shizi_tp[0][1])/(shizi_tp[2][0]-shizi_tp[0][0])+shizi_tp[0][1];
    for(k=shizi_tp[1][0];k<=shizi_tp[3][0];k++)
      edge_R.coordinate[k]=(k-shizi_tp[1][0])*(shizi_tp[3][1]-shizi_tp[1][1])/(shizi_tp[3][0]-shizi_tp[1][0])+shizi_tp[1][1];
   //���ߺ�������������
    int a_t,b_t;//ȷ�����½�
    a_t=shizi_tp[0][0]<shizi_tp[1][0]?shizi_tp[0][0]:shizi_tp[1][0];
    b_t=shizi_tp[2][0]<shizi_tp[3][0]?shizi_tp[3][0]:shizi_tp[2][0];
    for(k=a_t;k<=b_t;k++)
      Middle[k]=(edge_R.coordinate[k]+edge_L.coordinate[k])/2;
    
    for(k=height-1;k>=peak+2;k--)//����Զ��׵������������Ϣ��������������������������������������������������������������������������������������������������������������������  
  {
    derivative_2[k]=abs(Middle[k]-2*Middle[k-1]+Middle[k-2]);
    //show_D2[k][derivative_2[k]]=T_White;������������׾���ƫ��������Ч��
    if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }
  }
  
}///////////////////ʮ�ִ�����/////////////////
  /////////�����������///////////
else if(edge_L.none_fg==1)//�����
{
  TFTSPI_P16x16Str(8,7,"����",u16BLACK,u16WHITE);//��������䡱
   for(k=height-1;k>=peak+2;k--)
     if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }  
}
else if(edge_R.none_fg==1)//�����
{
  TFTSPI_P16x16Str(8,7,"����",u16BLACK,u16WHITE);//��������䡱
   for(k=height-1;k>=peak+2;k--)
     if(derivative_2[k]>TSD_d2)
    {
      peak=k-1;
      break;
    }  
}
/////////////////���������//////////////////

//step 5:�����߽�����ϲ����������Ϣ
  regression(Middle,peak,AB);//���˷����

/////////�����Ϣ��TFT����������������������������������������������������������������������������������������������������������������������  
     for(k=0;k<width;k++)
     {
       show_t[peak][k][0]=B_H;//���peak��Ϊ��ɫ�������ߵ������
       show_t[peak][k][1]=B_L;
     }
////////     
     for (k=0;k<height;k++)//��ʾ����
     {
       if(k>=shizi_tp[0][0] && k<=shizi_tp[2][0])
       {
         show_t[k][edge_L.coordinate[k]][0]=Y_H;//�����ϵ����߻�ɫ
         show_t[k][edge_L.coordinate[k]][1]=Y_L;
       }
       else
       {
         show_t[k][edge_L.coordinate[k]][0]=R_H;//���ߺ�ɫ
         show_t[k][edge_L.coordinate[k]][1]=R_L;
       }
     }
////////      
     for (k=0;k<height;k++)//��ʾ����
     {
       if(k>=shizi_tp[1][0] && k<=shizi_tp[3][0])
       {
         show_t[k][edge_R.coordinate[k]][0]=P_H;//�����ϵ�������ɫ
         show_t[k][edge_R.coordinate[k]][1]=P_L;
       }
       else
       {
         show_t[k][edge_R.coordinate[k]][0]=G_H;//������ɫ
         show_t[k][edge_R.coordinate[k]][1]=G_L;
       }
     }
////////    
      for (k=peak;k<height;k++)
     {
       show_t[k][Middle[k]][0]=T_White;//�����ð�
       show_t[k][Middle[k]][1]=T_White;
     }
//////// ���ͼ��
     mykey();
     TFTSPI_Show_rgb565(0,0,PROSW,PROSH,show_t);
////////    ��ʾ�ؾ��б��  
     char  txt[10];
  for (int i=0;i<2;i++)
  {
   sprintf((char*)txt,"%5.4f",*(AB+i));
    TFTSPI_P6X8Str(i*12,15,(uint8_t*)txt,u16BLACK,u16WHITE);
  }
    sprintf((char*)txt,"%d",threshold);//��ʾ��ֵ
    TFTSPI_P6X8Str(0,14,(uint8_t*)txt,u16BLACK,u16WHITE);
   //UARTSendPicture_s(); ������λ������û��ƥ��Ĵ�С��ƥ��ѽ
}

void mykey()
{
  while(KEY_Read(1)!=1);
}

void myfollow_Test(void)//����ѭ������
{ 
  TFTSPI_Init(0);        //LCD��ʼ��  0:����  1������
  TFTSPI_CLS(u16WHITE);   //��ɫ��Ļ	
  LED_Init();
  KEY_Init();
  UART_Init(UART4, 115200);
  
      TFTSPI_Show_Pic_huidu(0,0,PROSW,PROSH,wandao[0]);//��ʼͼƬ��ʾ
      float* ab=(float *)malloc(sizeof(float)*2);  //AB[0]�ǽؾ࣬AB[1]��б��
    myfollow(wandao,ab);//����
      free(ab);
//    //���Դ������OK
//    float A=*ab;
//    float B=*(ab+1);
//     char  txt[20];
//   sprintf((char*)txt,"�ؾ�Ϊ%5.4f\n",A);
//     UART_PutStr(UART4,txt);  
//   sprintf((char*)txt,"б��Ϊ%5.4f\n",B);
//     UART_PutStr(UART4,txt);  
    while(1)
    {
    
     }
}