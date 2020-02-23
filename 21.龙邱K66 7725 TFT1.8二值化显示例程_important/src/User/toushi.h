
#ifndef TOUSHI_H
#define TOUSHI_H

#define TS_W 78  //透视变换后图像长宽对应原 W ，H
#define TS_H 141

#define IN_W 160
#define IN_H 127
//#define CPS 2     //压缩倍数


/* Function Declarations */
extern void TouShi(  uint8 grayImage[IN_H][IN_W]);
#endif

