#ifndef __LQ_UART_H
#define __LQ_UART_H

#define K_SUM 3
/*------------------------------------------------------------------------------------------------------
����    ����Test_Uart
����    �ܡ�����UART 
����    ������
���� �� ֵ����
��ʵ    ����Test_PIT(); //����PIT��ʱ��
��ע�����
--------------------------------------------------------------------------------------------------------*/
void Test_Uart(void);
void UART_PutImage(UART_Type * uratn, uint8_t *str);
void UART_PutImageChar(UART_Type * uratn, uint8_t ch);
#endif