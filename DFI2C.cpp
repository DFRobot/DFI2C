/*
DFI2C_V1.0 Arduino Lib说明
使用该库，只要在实例化对象时，指定数据缓冲区的首地址和大小，注意缓冲区的大小应比实际寄存器数多1。
1、写寄存器过程
只要将待发送的数据写入用户定义的数据缓冲区中，然后调用Write方法，指定系统从设备地址、用户寄存器起始地址和连续操作的寄存器数，即可。
若调用该方法返回0，说明写操作成功，反之，操作失败。
2、读寄存器过程
调用Read方法，指定系统从设备地址、用户寄存器起始地址和连续操作的寄存器数，即可。
若调用该方法返回0，说明读操作成功，反之，操作失败。当操作成功时，用户即可从自己定义的缓冲区中，从首地址开始读取相应字节数的数据。

DFRobot
2013-5-31 Version: 1.0


*/

#include "DFI2C.h"
#include <Wire.h>

unsigned char Error[5];

/*DFI2C系统主设备接收到系统从设备发来的异常包*/
void DFI2C_ReceiveEvent(int Num)
{
  unsigned char i;
  for(i=0;i<Num;i++)
  {
    if((unsigned char)Num<=sizeof(Error))
      Error[i]=Wire.read();
  }
}

/*构造函数*/
DFI2C::DFI2C(unsigned char *BufAddr,unsigned char Num)
{
  Wire.begin(DFI2C_SYS_MASTER); 
  Wire.onReceive(DFI2C_ReceiveEvent);
  this->BufStartAddr=BufAddr;
  this->BufSize=Num;
}

/*生成PID*/
unsigned char DFI2C::GeneratePID(unsigned char PID)
{
  unsigned char count=0,temp=PID;
  while(temp)
  {
    temp&=(temp-1);
    count++;
  }
  if(count%2)
    return (PID<<1);
  else
    return ((PID<<1)+1);
}

/*校验PID*/
unsigned char DFI2C::CheckPID(unsigned char PID)
{
  unsigned char count=0,temp=PID;
  while(temp)
  {
    temp&=(temp-1);
    count++;
  }
  if(count%2)
    return PID;
  else 
    return 0xff;
}




/*写寄存器操作，SSA-模块地址，RegAddr-寄存器起始地址，Num-连续操作的字节数*/
unsigned char DFI2C::Write(unsigned char SSA,unsigned char RegAddr,unsigned char Num)
{
  unsigned char i,temp,check=0;
  
  //写寄存器PID+Data+Check
  Wire.beginTransmission(SSA);
  Wire.write(this->GeneratePID(RegAddr));
  for(i=0;i<Num;i++)
  {
    temp=*(this->BufStartAddr+i);
    Wire.write(temp);
    check+=temp;
  }
  Wire.write(check);
  Wire.endTransmission();
  
  //写握手PID   
  Wire.beginTransmission(SSA);
  Wire.write(this->GeneratePID(DFI2C_PID_HAND));
  Wire.endTransmission();
  
  //读校验字节
  Wire.requestFrom((int)SSA,1,true);
  if(Wire.read()==check)
    return 0;
  else
    return 0xff;
}

/*读寄存器操作，SSA-模块地址，RegAddr-寄存器起始地址，Num-连续操作的字节数*/
unsigned char DFI2C::Read(unsigned char SSA,unsigned char RegAddr,unsigned char Num)
{
  unsigned char temp,check=0;
  
  //写寄存器PID
  Wire.beginTransmission(SSA);
  Wire.write(this->GeneratePID(RegAddr));
  Wire.endTransmission();
  
  //读寄存器Data
  Wire.requestFrom((int)SSA,(int)Num,true);
  this->Front=0;
  this->Rear=0;
  while(Wire.available())
  { 
    temp=Wire.read();
    check+=temp;
    //允许覆盖
    *(this->BufStartAddr+this->Rear)=temp;
    this->Rear=(this->Rear+1)%this->BufSize;
  }
  
  //写握手PID   
  Wire.beginTransmission(SSA);
  Wire.write(this->GeneratePID(DFI2C_PID_HAND));
  Wire.endTransmission();
  
  //读校验字节
  Wire.requestFrom((int)SSA,1,true);
  if(Wire.read()==check)
  {
    if((this->Rear+this->BufSize-this->Front)%this->BufSize==Num)
      return 0;
    else 
      return 0xff;
  }
  return 0xff;
}

/*异常检测，若收到正确的异常包，则返回0，否则返回0xff*/
unsigned char DFI2C::DetectError(void)
{
  if(Error[0]==this->GeneratePID(DFI2C_PID_ERROR))
  {
    if(Error[1]!=0)
    {
      if(Error[4]==Error[1]+Error[2]+Error[3])
        return 0;
    }
  }
  return 0xff;
}

