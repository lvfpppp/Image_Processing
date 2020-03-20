#ifndef __LQ_UART_H
#define __LQ_UART_H

#define K_SUM 3
/*------------------------------------------------------------------------------------------------------
【函    数】Test_Uart
【功    能】测试UART 
【参    数】无
【返 回 值】无
【实    例】Test_PIT(); //测试PIT定时器
【注意事项】
--------------------------------------------------------------------------------------------------------*/
void Test_Uart(void);
void UART_PutImage(UART_Type * uratn, uint8_t *str);
void UART_PutImageChar(UART_Type * uratn, uint8_t ch);
#endif