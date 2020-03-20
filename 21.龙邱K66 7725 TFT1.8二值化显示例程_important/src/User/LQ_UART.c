/*-----------------------------------------------------------------------------------------------------
【平    台】龙邱K60核心板-智能车板
【编    写】LQ-005
【E-mail  】chiusir@163.com
【软件版本】V1.0，龙邱开源代码，仅供参考，后果自负
【最后更新】2019年04月02日
【网    站】http://www.lqist.cn
【淘宝店铺】http://shop36265907.taobao.com
【编译平台】IAR 8.2
【功    能】UART例子
【注意事项】
-------------------------------------------------------------------------------------------------------*/
#include "include.h"
#include "LQ_UART.h"

extern char u4_data[40];
extern int success_fg;

float K_pid[K_SUM]={0};

//修改一下UART_PutStr，str输入类型char改为uint8_t
void UART_PutImage(UART_Type * uratn, uint8_t *str)
{
  while(*str)
  {  
    //等待发送缓冲区空
    while(!(UART_S1_REG(uratn) & UART_S1_TDRE_MASK));
    //发送数据
    UART_D_REG(uratn) = *str;
    str++;
  }
}
//一个字节一个字节发送
void UART_PutImageChar(UART_Type * uratn, uint8_t ch)
{
    //等待发送缓冲区空
    while(!(UART_S1_REG(uratn) & UART_S1_TDRE_MASK));
    //发送数据
    UART_D_REG(uratn) = ch;
}
    
    
//通过配合串口4的输入达到修改参数的目的
//主要把值放入K_pid中，并放到flash中特定位置
void Parameter_Init(void)
{
   int t_n;
Label_restart:
  success_fg=0;//置0重新输入，
  t_n=0;
  UART_PutStr(UART4,"input ");
  UART_PutChar(UART4,(char)(K_SUM+48));
  UART_PutStr(UART4," parameters :\n");
  UART_PutStr(UART4,"input K...A...E strings!\n");
  while(1)
    {if(success_fg==1) break;
    LPTMR_delayus(10);}//等数据传完

  for (int i=0;i<40;i++)
  {
    if(u4_data[i]!=0) //找到开头位置
    {t_n=i;
    break;}
  }
  int fg_kind=0;//整数，小数标志位
  if(u4_data[t_n]!='K' && u4_data[t_n]!='k' )//开头不对，输入错误
  {
    UART_PutStr(UART4,"输入错误，请重新输入\n");
    for(int i=0;i<40; i++)
      u4_data[i]=0;
    goto Label_restart;
  }
  else//提取参数k_pid
  {t_n++;//先进一位到数字（本来在字母上）
   
    for(int k=0;k<K_SUM;k++)//执行K_SUM次
    {
      int x_tp=-1;//处理小数用的（only）
      fg_kind=0;
      K_pid[k]=u4_data[t_n]-48;
      do
      {
          if(u4_data[t_n+1]=='.')
          {
          t_n++;
          fg_kind=1;
          }
        t_n++;
        if(fg_kind==0)
        {
          K_pid[k]=u4_data[t_n]-48+K_pid[k]*10;
        } 
        else
        {
          K_pid[k]+=(u4_data[t_n]-48)*pow(10,x_tp--);
        }
        
      }while(t_n<40 && u4_data[t_n+1]!='A' && u4_data[t_n+1]!='a' && u4_data[t_n+1]!='E' && u4_data[t_n+1]!='e' );
        
      if(t_n>=40)
      {
        UART_PutStr(UART4,"提取参数错误，重新提取\n");
        for(int i=0;i<40; i++)
          u4_data[i]=0;
        goto Label_restart;
      }
      else if(u4_data[t_n+1]=='E' || u4_data[t_n+1]=='e' )
      {
      UART_PutStr(UART4,"提取参数成功\n");  
      break;
      }
    t_n+=2;
    }
  }
  
  FLASH_Init(); //Flash初始化
  FLASH_EraseSector(2);
  FLASH_WriteBuf(2,(uint8_t *)K_pid, sizeof(K_pid), 0);//写入扇区，sizeof(）加的很妙，应该是float储存内容不变，但函数内部每次按8b来放
  float k_pid_temp[K_SUM]={0};
  /* 从倒数第1个扇区 0偏移位置开始 读出数据 */
	FLASH_ReadBuff(2, 0, sizeof(K_pid), (char *)k_pid_temp);
    
  char  txt[20]={' '};
  for (int i=0;i<K_SUM;i++)
  {
   sprintf((char*)txt,"k%d:%6.3f\n",i+1,k_pid_temp[i]);
    UART_PutStr(UART4,txt);  //显示写入后再读出的数据
  }
 
}
/*------------------------------------------------------------------------------------------------------
【函    数】Test_Uart
【功    能】测试UART 
【参    数】无
【返 回 值】无
【实    例】Test_Uart(); //测试串口
【注意事项】
--------------------------------------------------------------------------------------------------------*/
void Test_Uart(void)
{
  //LED_Init();
  
  UART_Init(UART4, 115200);
  
  /* 优先级配置 抢占优先级1  子优先级2   越小优先级越高  抢占优先级可打断别的中断 */
  NVIC_SetPriority(UART4_RX_TX_IRQn,NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1,2));
  
  NVIC_EnableIRQ(UART4_RX_TX_IRQn);			          //使能UART4_RX_TX_IRQn的中断


  Parameter_Init();

       
}

