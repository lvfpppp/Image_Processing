#ifndef __VL53L0X_H__
#define __VL53L0X_H__


#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xc0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xc2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define VL53_REG_DIS                                0x1E
#define VL53ADDR                                    0x52    //0x52

/**
  * @brief    ����VL53
  *
  * @param    ��
  *
  * @return   ��
  *
  * @note     ��
  *
  * @example  
  *
  * @date     2019/4/17 ������
  */
void Test_Vl53(void);

/**
  * @brief    VL53 дn���Ĵ���
  *
  * @param    dev��    ��ַ
  * @param    reg��    �Ĵ���
  * @param    length;  ����
  * @param    data��   ָ��д������
  *
  * @return   
  *
  * @note     
  *
  * @example  
  *
  * @date     2019/4/29 ����һ
  */
void VL53_Write_nByte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data);



/**
  * @brief    VL53 ��n���Ĵ���
  *
  * @param    dev��    ��ַ
  * @param    reg��    �Ĵ���
  * @param    length;  ����
  * @param    data��   ָ���Ŷ�����
  *
  * @return   
  *
  * @note     
  *
  * @example  
  *
  * @date     2019/4/29 ����һ
  */
void VL53_Read_nByte(uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data);
#endif