/*Version： 1.0*/

#ifndef _DFI2C_H
#define _DFI2C_H

#include <Wire.h>

#define DFI2C_PID_HAND    0x7e
#define DFI2C_PID_ERROR   0x7f
#define DFI2C_SYS_MASTER  127

class DFI2C
{
  private:
    unsigned char *BufStartAddr;
    unsigned char BufSize;
    unsigned char Front;
    unsigned char Rear; 
    unsigned char GeneratePID(unsigned char PID);
    unsigned char CheckPID(unsigned char PID);
    
  public:
    DFI2C(unsigned char *BufAddr,unsigned char Num);
    unsigned char Write(unsigned char SSA,unsigned char RegAddr,unsigned char Num);
    unsigned char Read(unsigned char SSA,unsigned char RegAddr,unsigned char Num);
    unsigned char DetectError(void);
};

#endif
