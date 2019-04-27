
#include "LSM303.h"
#include "usart.h"
#include "I2C.h"
#include  <math.h>    //Keil library  

//************��ʼ��L3G4200D�ļ��ٶ�*********************************
void LSM303A_Init( void )
{
    
  I2C_WriteOneByte(I2C1,LSM303A_I2C_ADDR,LSM303A_CTRL_REG1, 0x37);
  I2C_WriteOneByte(I2C1,LSM303A_I2C_ADDR,LSM303A_CTRL_REG4, 0x00);
}
//************��ʼ��L3G4200D�ĴŸ�Ӧ*********************************

void LSM303M_Init( void )
{

	I2C_WriteOneByte(I2C1,LSM303M_I2C_ADDR,LSM303M_CRB_REG, 0xa0);
	
	I2C_WriteOneByte(I2C1,LSM303M_I2C_ADDR,LSM303M_MR_REG, 0x00);
}

//********��ȡ�Ÿ�Ӧ����************************************

void LAM303M_Raed(float *Data)
{
  static unsigned char buffer[6]={0};   
	unsigned char f;
	unsigned int Magn_Sensitivity_XY = 0, Magn_Sensitivity_Z = 0;
	uint16_t cen=0;
	
	I2C_Read(I2C1,LSM303M_I2C_ADDR,LSM303M_OUT_X_H,buffer,6);//|0x80
	I2C_Read(I2C1,LSM303M_I2C_ADDR,LSM303M_CRB_REG,&f,1);
  switch(f & 0xE0)
  {
  case LSM303DLHC_FS_1_3_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_1_3Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_1_3Ga;
    break;
  case LSM303DLHC_FS_1_9_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_1_9Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_1_9Ga;
    break;
  case LSM303DLHC_FS_2_5_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_2_5Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_2_5Ga;
    break;
  case LSM303DLHC_FS_4_0_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_4Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_4Ga;
    break;
  case LSM303DLHC_FS_4_7_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_4_7Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_4_7Ga;
    break;
  case LSM303DLHC_FS_5_6_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_5_6Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_5_6Ga;
    break;
  case LSM303DLHC_FS_8_1_GA:
    Magn_Sensitivity_XY = LSM303DLHC_M_SENSITIVITY_XY_8_1Ga;
    Magn_Sensitivity_Z = LSM303DLHC_M_SENSITIVITY_Z_8_1Ga;
    break;
  }
 /* 
  for(i=0; i<2; i++)
  {
		cen=(int16_t)(((int16_t)buffer[2*i] << 8) + buffer[2*i+1]) ;
    Data[i]=(float)(cen)*1000/Magn_Sensitivity_XY;
  
  }
	*/
	cen=(int16_t)(((uint16_t)buffer[0] << 8) + buffer[1]);
  Data[0]=(float)((int16_t)cen)*1000/Magn_Sensitivity_XY;
	
  cen=(int16_t)(((uint16_t)buffer[2] << 8) + buffer[3]);
  Data[2]=(float)((int16_t)cen)*1000/Magn_Sensitivity_Z;

	cen=(int16_t)(((uint16_t)buffer[4] << 8) + buffer[5]);
  Data[1]=(float)((int16_t)cen)*1000/Magn_Sensitivity_XY;
  /*
	cen=Data[1];
	Data[1]=Data[2];
	Data[2]=cen;
	*/
}
//********��ȡ���ٶ�����************************************
void LAM303A_Raed(float *Data)
{
  static unsigned char buffer[6]={0}, ctrlx[2],i; 
	unsigned char cDivider;
  signed short  int pnRawData[3];
	float LSM_Acc_Sensitivity=LSM_Acc_Sensitivity_2g;
	int16_t cen;
	I2C_Read(I2C1,LSM303A_I2C_ADDR,LSM303A_OUT_X_L|0x80,buffer,6);
	I2C_Read(I2C1,LSM303A_I2C_ADDR,LSM303A_CTRL_REG4|0x80,ctrlx,2);
   
  if(ctrlx[1]&0x40)
    cDivider=64;
  else
    cDivider=16;

  /* check in the control register4 the data alignment*/
  if(!(ctrlx[0] & 0x40) || (ctrlx[1] & 0x40)) /* Little Endian Mode or FIFO mode */
  {
    for(i=0; i<3; i++)
    {
			cen=(int16_t)((uint16_t)buffer[2*i+1] << 8) + buffer[2*i];
      pnRawData[i]=(int16_t)(cen)/cDivider;
     
    }
  }
  else /* Big Endian Mode */
  {
    for(i=0; i<3; i++)
      pnRawData[i]=((int16_t)((uint16_t)buffer[2*i] << 8) + buffer[2*i+1])/cDivider;
  }
  /* Read the register content */
  I2C_Read(I2C1,LSM303A_I2C_ADDR,LSM303A_CTRL_REG4,ctrlx,2);


  if(ctrlx[1]&0x40)
  {
    /* FIFO mode */
    LSM_Acc_Sensitivity = 0.25;
  }
  else
  {
    /* normal mode */
    /* switch the sensitivity value set in the CRTL4*/
    switch(ctrlx[0] & 0x30)
    {
    case LSM303DLHC_FULLSCALE_2G:
      LSM_Acc_Sensitivity = LSM_Acc_Sensitivity_2g;
      break;
    case LSM303DLHC_FULLSCALE_4G:
      LSM_Acc_Sensitivity = LSM_Acc_Sensitivity_4g;
      break;
    case LSM303DLHC_FULLSCALE_8G:
      LSM_Acc_Sensitivity = LSM_Acc_Sensitivity_8g;
      break;
    case LSM303DLHC_FULLSCALE_16G:
      LSM_Acc_Sensitivity = LSM_Acc_Sensitivity_16g;
      break;
    }
  }

  /* Obtain the mg value for the three axis */
  for(i=0; i<3; i++)
  {
    Data[i]=(float)pnRawData[i]*LSM_Acc_Sensitivity;
  }
}


/********************************************/
float Data_conversion(float *AccBuffer,float *MagBuffer)
{
  unsigned char i;
	float HeadingValue = 0.0f;
	float fNormAcc,fSinRoll,fCosRoll,fSinPitch,fCosPitch = 0.0f;
  float fTiltedX,fTiltedY = 0.0f;
	float fcosf=0;
	      for(i=0;i<3;i++)
        AccBuffer[i] /= 100.0f;
      
      fNormAcc = sqrt((AccBuffer[0]*AccBuffer[0])+(AccBuffer[1]*AccBuffer[1])+(AccBuffer[2]*AccBuffer[2]));
      
      fSinRoll = AccBuffer[1]/fNormAcc;
      fCosRoll = sqrt(1.0-(fSinRoll * fSinRoll));
      fSinPitch = AccBuffer[0]/fNormAcc;
      fCosPitch = sqrt(1.0-(fSinPitch * fSinPitch));

      
      fTiltedX = MagBuffer[0]*fCosPitch + MagBuffer[2]*fSinPitch;
      fTiltedY = MagBuffer[0]*fSinRoll*fSinPitch + MagBuffer[1]*fCosRoll - MagBuffer[2]*fSinRoll*fCosPitch;
			
      fcosf=fTiltedX /sqrt(fTiltedX*fTiltedX+fTiltedY*fTiltedY);
	
			if(fTiltedY>0)
			  HeadingValue = (float)(acos(fcosf)*180/PI);
			else
				HeadingValue =360-(float)(acos(fcosf)*180/PI);
			
      //HeadingValue = (float) ((atan2f((float)fTiltedY,(float)fTiltedX))*180)/PI;
      HeadingValue+=40;//�شŵı����͵���ı������11�����ҡ�
			if(HeadingValue>360)
				HeadingValue=HeadingValue-360;
   
	    return HeadingValue ;
	
	
}






