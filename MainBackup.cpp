 /***************************************************************
Copyright(c) 2014  AITON. All rights reserved.
Author:     AITON
FileName:   MainBackup.cpp
Date:       2013-4-13
Description:
Version:    V1.0
History:    2014.05.29 
********************************************************************************************/
#include "MainBackup.h"
#include "TscMsgQueue.h"
#include "ace/Date_Time.h"
#include "MainBoardLed.h"
#include "ManaKernel.h"
#include "GbtMsgQueue.h"
#include "ace/Vector_T.h"

CManaKernel * pManaKernel = CManaKernel::CreateInstance();
SThreadMsg sTscMsg;
SThreadMsg sTscMsgSts;	

template<typename T>
static T bytes2T(unsigned char *bytes)
{
    T res = 0;
    int n = sizeof(T);
    memcpy(&res, bytes, n);
    return res;
}

template<typename T>
static unsigned char * T2bytes(T u)
{
    int n = sizeof(T);
    unsigned char* b = new unsigned char[n];
    memcpy(b, &u, n);
    return b;
}


MainBackup::MainBackup() 
{
	OpenDev();
	
	m_ucLastManualSts = MAINBACKUP_MANUAL_SELF;
#ifdef TSC_DEBUG
	ACE_DEBUG((LM_DEBUG,"create MainBackup\n"));
#endif
}

MainBackup* MainBackup::CreateInstance()
{
	static MainBackup cMainBackup;
	return &cMainBackup;
}

MainBackup::~MainBackup() 
{
#ifdef TSC_DEBUG
	ACE_DEBUG((LM_DEBUG,"destroy MainBackup\n"));
#endif
}


void MainBackup::OpenDev()
{
	if(m_iSerial3fd <=0)
	{
		m_iSerial3fd = CSerialCtrl::CreateInstance()->GetSerialFd3();
		ACE_DEBUG((LM_DEBUG,"%s:%d MainBackup open dev m_iSerial3fd %d\n",__FILE__,__LINE__,m_iSerial3fd));
	}
	
}

bool MainBackup::SendBackup(Byte *pByte ,int iSize)
{
	ACE_DEBUG((LM_DEBUG,"%s:%d WriteComPort ############################### %d\n",__FILE__,__LINE__));
	int len_tty = -1;
	len_tty = CSerialCtrl::CreateInstance()->WriteComPort(pByte, iSize);
	if (len_tty < 0) {		
		ACE_DEBUG((LM_DEBUG,"%s:%d Error: WriteComPort Error %d\n",__FILE__,__LINE__));
		
	}
	return true;
}

bool MainBackup::RecevieBackup(Byte *pByte ,int iSize)
{
	
	return true;
}

/**************************************************************
Function:        MainBackup::OperateManual
Description:    执行手控的相关操作
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::OperateManual(Ushort mbs)
{
	
	CGbtMsgQueue *pGbtMsgQueue = CGbtMsgQueue::CreateInstance();
	ACE_OS::memset( &sTscMsg    , 0 , sizeof(SThreadMsg));
	ACE_OS::memset( &sTscMsgSts , 0 , sizeof(SThreadMsg));
	switch(mbs)
	{
		case MAINBACKUP_MANUAL_SELF:
			if (m_ucLastManualSts == MAINBACKUP_MANUAL_SELF)
				return;
			pGbtMsgQueue->SendTscCommand(OBJECT_SWITCH_MANUALCONTROL,0);
			ACE_DEBUG((LM_DEBUG,"%s:%d ************** MAINBACKUP_MANUAL_SELF TscMsg! \n",__FILE__,__LINE__));
			m_ucLastManualSts = MAINBACKUP_MANUAL_SELF;
			break;
		case MAINBACKUP_MANUAL_MANUAL:
			if (m_ucLastManualSts == MAINBACKUP_MANUAL_MANUAL)
				return;
			pGbtMsgQueue->SendTscCommand(OBJECT_CURTSC_CTRL,4);
			ACE_DEBUG((LM_DEBUG,"%s:%d First Send  Manual TscMsg! \n",__FILE__,__LINE__));
			m_ucLastManualSts = MAINBACKUP_MANUAL_MANUAL;
			break;
		case MAINBACKUP_MANUAL_NEXT_STEP:
			
			pGbtMsgQueue->SendTscCommand(OBJECT_GOSTEP,0);
			ACE_DEBUG((LM_DEBUG,"%s:%d Send Next Step TscMsg ! \n",__FILE__,__LINE__));
			pManaKernel->SndMsgLog(LOG_TYPE_MANUAL,6,0,0,0);
			m_ucLastManualSts = MAINBACKUP_MANUAL_NEXT_STEP;
			break;
		case MAINBACKUP_MANUAL_YELLOW_FLASH:
			if (m_ucLastManualSts == MAINBACKUP_MANUAL_YELLOW_FLASH)
			{
				ACE_DEBUG((LM_DEBUG,"%s:%d the old ctrl equals the new ctrl ! \n",__FILE__,__LINE__));
				return ;
			}
			pGbtMsgQueue->SendTscCommand(OBJECT_SWITCH_MANUALCONTROL,254);
			ACE_DEBUG((LM_DEBUG,"%s:%d Send CTRL_PANEL FLASH! TscMsg!\n",__FILE__,__LINE__));
			m_ucLastManualSts = MAINBACKUP_MANUAL_YELLOW_FLASH;
			break;
		case MAINBACKUP_MANUAL_ALL_RED:
			if (m_ucLastManualSts == MAINBACKUP_MANUAL_ALL_RED)
			{
				ACE_DEBUG((LM_DEBUG,"%s:%d the old ctrl equals the new ctrl ! \n",__FILE__,__LINE__));
				return ;
			}
			pGbtMsgQueue->SendTscCommand(OBJECT_SWITCH_MANUALCONTROL,253);
			ACE_DEBUG((LM_DEBUG,"%s:%d Send CTRL_PANEL ALLRED TscMsg!\n",__FILE__,__LINE__));
			m_ucLastManualSts = MAINBACKUP_MANUAL_ALL_RED;
			break;
		case MAINBACKUP_MANUAL_NEXT_PHASE:
				//因为存在过度步，如果在过度步用户按下按钮。那么就不让其作用。
			
			pGbtMsgQueue->SendTscCommand(OBJECT_SWITCH_STAGE,0);
			ACE_DEBUG((LM_DEBUG,"%s:%d Send MAC_CTRL_NEXT_PHASE TscMsg !\n",__FILE__,__LINE__));
			m_ucLastManualSts = MAINBACKUP_MANUAL_NEXT_PHASE;
			break;
		case MAINBACKUP_MANUAL_NEXT_DIREC:
				
				break;
		case MAINBACKUP_MANUAL_NORTH:

				break;
		case MAINBACKUP_MANUAL_EAST:

				break;
		case MAINBACKUP_MANUAL_SOUTH:

				break;
		case MAINBACKUP_MANUAL_WEST:

				break;
	}
}

/**************************************************************
Function:        MainBackup::DoReadId
Description:    读取信号机ID操作			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoReadId()
{
	Byte readID[7] = {0xaa,0x55,0x04,MAINBACKUP_READ_ID,0xff,0xff,0xff};
	Byte chksum = ~(MAINBACKUP_READ_ID+0xff+0xff);
	readID[6] = chksum;
	SendBackup(readID,7);
}

/**************************************************************
Function:        MainBackup::DoManual
Description:    形成核心板信号机程序发送到备份单片机,进行手控按钮查询			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoWriteId()
{
	char CCdkey[8] = {0};
	Cdkey::CreateInstance()->GetCdkey(CCdkey);
	Byte writeID[14] = {0xaa,0x55,0x0b,MAINBACKUP_WRITE_ID,0xff,CCdkey[0],CCdkey[1],CCdkey[2],CCdkey[3],CCdkey[4],CCdkey[5],CCdkey[6],CCdkey[7],0xff};
	Byte chksum = ~(MAINBACKUP_WRITE_ID+0xff+CCdkey[0]+CCdkey[1]+CCdkey[2]+CCdkey[3]+CCdkey[4]+CCdkey[5]+CCdkey[6]+CCdkey[7]);
	writeID[13] = chksum;
	SendBackup(writeID,14);
}

/**************************************************************
Function:        MainBackup::DoReadLED
Description:    发送读取LED状态指令，再由接收线程接收处理			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoReadLED()
{
	
	Byte readLED[7] = {0xaa,0x55,0x04,MAINBACKUP_READ_LED,0xff,0xff,0xff};
	Byte chksum = ~(MAINBACKUP_READ_LED+0xff+0xff);
	readLED[6] = chksum;
	SendBackup(readLED,7);
}

/**************************************************************
Function:        MainBackup::DoWriteLED
Description:    发送写入LED状态指令，再由接收线程接收处理			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoWriteLED()
{
	Byte workmode = pManaKernel->m_pRunData->ucWorkMode;
	Uint ctrl = pManaKernel->m_pRunData->uiCtrl;
	bool degrad = pManaKernel->bDegrade;
	Byte data = 0x00;
	if(MODE_TSC == workmode)
	{
		//这里表示信号机正在tsc模式下运行
		data = data | 0x00;
	}
	else if(MODE_PSC1 == workmode || MODE_PSC2 == workmode)
	{
		//这里表示psc运行模式
		data = data |0x08;
	}
	else if(MODE_OTHER == workmode)
	{
		//这里表示其它未知
	}


	if(CTRL_MANUAL == ctrl)
	{
		data = data | 0x04;
	}
	else
	{
		//data = data & 0x00;
	}
	//是否降级，在LED上显示出来
	if(degrad)
	{
		data = data | 0x01;
	}
	else
	{
		data = data | 0x00;
	}
	Byte writeLED[7] = {0xaa,0x55,0x04,MAINBACKUP_WRITE_LED,0xff,data,0xff};
	Byte chksum = ~(MAINBACKUP_WRITE_LED+0xff+data);
	writeLED[6] = chksum;
	SendBackup(writeLED,7);
}
/**************************************************************
Function:        MainBackup::DoSendStep
Description:    将信号机的步伐数据发送到备份单片机		
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoSendStep(SStepInfo stepInfos[],Byte stepNum)
{
	int i,j;
	for(i=0;i<stepNum;i++)
	{
		SStepInfo stepInfo = stepInfos[i];
		Byte lampOn[MAX_LAMP] = {0};
		ACE_OS::memcpy(lampOn,stepInfo.ucLampOn,MAX_LAMP);
		Byte lampFlash[MAX_LAMP] = {0};
		ACE_OS::memcpy(lampFlash,stepInfo.ucLampFlash,MAX_LAMP);
		Byte time = stepInfo.ucStepLen;
		//ACE_DEBUG((LM_DEBUG,"%s:%d================= Send Step: time %d!================\n",__FILE__,__LINE__,time));
		Byte sendBit[13] = {0};
		sendBit[0] = time;
		for(j=0;j<MAX_LAMP;j++)
		{
			
			//ACE_DEBUG((LM_DEBUG,"%s:%d<<<<< Send Step: lampOn %d!>>>>>>\n",__FILE__,__LINE__,lampOn[j]));
			if(lampFlash[j] == 1 && lampOn[j] == 1)	//灯闪
			{
				switch(j)
				{
					case 0:
						sendBit[12] |= LAMP_RED_FLASH;
						break;
					case 1:
						sendBit[12] |= LAMP_YELLOW_FLASH;
						break;
					case 2:
						sendBit[12] |= LAMP_GREEN_FLASH;
						break;
					case 3:
						sendBit[12] |=  0x20;
						break;
					case 4:
						sendBit[12] |=  0x28;
						break;
					case 5:
						sendBit[12] |= 0x30;
						break;
					case 6:
						sendBit[12] |= 0x00;    //第一级灯的前两个字节
						sendBit[11] |= 0x01;	//第一块第三个灯组 的第一个字节
						break;
					case 7:
						sendBit[12] |= 0x01;
						sendBit[11] |= 0x01;
						break;
					case 8:
						sendBit[12] |= 0x02;
						sendBit[11] |= 0x01;
						break;
					case 9:
						sendBit[11] |= 0x08;
						break;
					case 10:
						sendBit[11] |= 0x0a;
						break;
					case 11:
						sendBit[11] |= 0x0c;
						break;
					case 12:
						sendBit[11] |= 0x40;
						break;
					case 13:
						sendBit[11] |= 0x50;
						break;
					case 14:
						sendBit[11] |= 0x60;
						break;
					case 15:
						sendBit[11] |= 0x00;
						sendBit[10] |= 0x02;
						break;
					case 16:
						sendBit[11] |= 0x01;
						sendBit[10] |= 0x02;
						break;
					case 17:
						sendBit[11] |= 0x00;
						sendBit[10] |= 0x03;
						break;
					case 18:
						sendBit[10] |= 0x10;
						break;
					case 19:
						sendBit[10] |= 0x14;
						break;
					case 20:
						sendBit[10] |= 0x18;
						break;
					case 21:
						sendBit[10] |= 0x80;
						break;
					case 22:
						sendBit[10] |= 0xa0;
						break;
					case 23:
						sendBit[10] |= 0xc0;
						break;
					case 24:
						sendBit[9] |= 0x04;
						break;
					case 25:
						sendBit[9] |= 0x05;
						break;
					case 26:
						sendBit[9] |= 0x06;
						break;
					case 27:
						sendBit[9] |= 0x20;
						break;
					case 28:
						sendBit[9] |= 0x28;
						break;
					case 29:
						sendBit[9] |= 0x30;
						break;
					case 30:
						sendBit[9] |= 0x00;
						sendBit[8] |= 0x01;
						break;
					case 31:
						sendBit[9] |= 0x01;
						sendBit[8] |= 0x01;
						break;
					case 32:
						sendBit[9] |= 0x02;
						sendBit[8] |= 0x01;
						break;
					case 33:
						sendBit[8] |= 0x08;
						break;
					case 34:
						sendBit[8] |= 0x0a;
						break;
					case 35:
						sendBit[8] |= 0x0c;
						break;
					case 36:
						sendBit[8] |= 0x40;
						break;
					case 37:
						sendBit[8] |= 0x50;
						break;
					case 38:
						sendBit[8] |= 0x60;
						break;
					case 39:
						sendBit[8] |= 0x00;
						sendBit[7] |= 0x02;
						break;
					case 40:
						sendBit[8] |= 0x01;
						sendBit[7] |= 0x02;
						break;
					case 41:
						sendBit[8] |= 0x00;
						sendBit[7] |= 0x03;
						break;
					case 42:
						sendBit[7] |= 0x10;
						break;
					case 43:
						sendBit[7] |= 0x14;
						break;
					case 44:
						sendBit[7] |= 0x18;
						break;
					case 45:
						sendBit[7] |= 0x80;
						break;
					case 46:
						sendBit[7] |= 0xa0;
						break;
					case 47:
						sendBit[7] |= 0xc0;
						break;
					case 48:
						sendBit[6] |= 0x04;
						break;
					case 49:
						sendBit[6] |= 0x05;
						break;
					case 50:
						sendBit[6] |= 0x06;
						break;
					case 51:
						sendBit[6] |= 0x20;
						break;
					case 52:
						sendBit[6] |= 0x28;
						break;
					case 53:
						sendBit[6] |= 0x30;
						break;
					case 54:
						sendBit[6] |= 0x00;
						sendBit[5] |= 0x01;
						break;
					case 55:
						sendBit[6] |= 0x01;
						sendBit[5] |= 0x01;
						break;
					case 56:
						sendBit[6] |= 0x02;
						sendBit[5] |= 0x01;
						break;
					case 57:
						sendBit[5] |= 0x08;
						break;
					case 58:
						sendBit[5] |= 0x0a;
						break;
					case 59:
						sendBit[5] |= 0x0c;
						break;
					case 60:
						sendBit[5] |= 0x40;
						break;
					case 61:
						sendBit[5] |= 0x50;
						break;
					case 62:
						sendBit[5] |= 0x60;
						break;
					case 63:
						sendBit[5] |= 0x00;
						sendBit[4] |= 0x02;
						break;
					case 64:
						sendBit[5] |= 0x01;
						sendBit[4] |= 0x02;
						break;
					case 65:
						sendBit[5] |= 0x00;
						sendBit[4] |= 0x03;
						break;
					case 66:
						sendBit[4] |= 0x10;
						break;
					case 67:
						sendBit[4] |= 0x14;
						break;
					case 68:
						sendBit[4] |= 0x18;
						break;
					case 69:
						sendBit[4] |= 0x80;
						break;
					case 70:
						sendBit[4] |= 0xa0;
						break;
					case 71:
						sendBit[4] |= 0xc0;
						break;
					case 72:
						sendBit[3] |= 0x04;
						break;
					case 73:
						sendBit[3] |= 0x05;
						break;
					case 74:
						sendBit[3] |= 0x06;
						break;
					case 75:
						sendBit[3] |= 0x20;
						break;
					case 76:
						sendBit[3] |= 0x28;
						break;
					case 77:
						sendBit[3] |= 0x30;
						break;
					case 78:
						sendBit[3] |= 0x00;
						sendBit[2] |= 0x01;
						break;
					case 79:
						sendBit[3] |= 0x01;
						sendBit[2] |= 0x01;
						break;
					case 80:
						sendBit[3] |= 0x02;
						sendBit[2] |= 0x01;
						break;
					case 81:
						sendBit[2] |= 0x08;
						break;
					case 82:
						sendBit[2] |= 0x0a;
						break;
					case 83:
						sendBit[2] |= 0x0c;
						break;
					case 84:
						sendBit[2] |= 0x40;
						break;
					case 85:
						sendBit[2] |= 0x50;
						break;
					case 86:
						sendBit[2] |= 0x60;
						break;
					case 87:
						sendBit[2] |= 0x00;
						sendBit[1] |= 0x02;
						break;
					case 88:
						sendBit[2] |= 0x01;
						sendBit[1] |= 0x02;
						break;
					case 89:
						sendBit[2] |= 0x00;
						sendBit[1] |= 0x03;
						break;
					case 90:
						sendBit[1] |= 0x10;
						break;
					case 91:
						sendBit[1] |= 0x14;
						break;
					case 92:
						sendBit[1] |= 0x18;
						break;
					case 93:
						sendBit[1] |= 0x80;
						break;
					case 94:
						sendBit[1] |= 0xa0;
						break;
					case 95:
						sendBit[1] |= 0xc0;
						break;
					
					
					default:
						break;
				}
			}
			else if (lampOn[j] ==1 && lampFlash[j] == 0)
			{
				switch(j)
				{
					case 0:
						sendBit[12] |= LAMP_RED;
						break;
					case 1:
						sendBit[12] |= LAMP_YELLOW;
						break;
					case 2:
						sendBit[12] |= LAMP_GREEN;
						break;
					case 3:
						sendBit[12] |= 0x08;
						break;
					case 4:
						sendBit[12] |= 0x10;
						break;
					case 5:
						sendBit[12] |= 0x18;
						break;
					case 6:
						sendBit[12] |= 0x40;    //第一级灯的前两个字节
						sendBit[11] |= 0x00;	//第一块第三个灯组 的第一个字节
						break;
					case 7:
						sendBit[12] |= 0x80;
						sendBit[11] |= 0x00;
						break;
					case 8:
						sendBit[12] |= 0xc0;
						sendBit[11] |= 0x00;
						break;
					case 9:
						sendBit[11] |= 0x02;
						break;
					case 10:
						sendBit[11] |= 0x04;
						break;
					case 11:
						sendBit[11] |= 0x06;
						break;
					case 12:
						sendBit[11] |= 0x10;
						break;
					case 13:
						sendBit[11] |= 0x20;
						break;
					case 14:
						sendBit[11] |= 0x30;
						break;
					case 15:
						sendBit[11] |= 0x80;
						sendBit[10] |= 0x00;
						break;
					case 16:
						sendBit[11] |= 0x00;
						sendBit[10] |= 0x01;
						break;
					case 17:
						sendBit[11] |= 0x80;
						sendBit[10] |= 0x01;
						break;
					case 18:
						sendBit[10] |= 0x04;
						break;
					case 19:
						sendBit[10] |= 0x08;
						break;
					case 20:
						sendBit[10] |= 0x0c;
						break;
					case 21:
						sendBit[10] |= 0x20;
						break;
					case 22:
						sendBit[10] |= 0x40;
						break;
					case 23:
						sendBit[10] |= 0x60;
						break;
					case 24:
						sendBit[9] |= 0x01;
						break;
					case 25:
						sendBit[9] |= 0x02;
						break;
					case 26:
						sendBit[9] |= 0x03;
						break;
					case 27:
						sendBit[9] |= 0x08;
						break;
					case 28:
						sendBit[9] |= 0x10;
						break;
					case 29:
						sendBit[9] |= 0x18;
						break;
					case 30:
						sendBit[9] |= 0x40;
						sendBit[8] |= 0x00;
						break;
					case 31:
						sendBit[9] |= 0x80;
						sendBit[8] |= 0x00;
						break;
					case 32:
						sendBit[9] |= 0xc0;
						sendBit[8] |= 0x00;
						break;
					case 33:
						sendBit[8] |= 0x02;
						break;
					case 34:
						sendBit[8] |= 0x04;
						break;
					case 35:
						sendBit[8] |= 0x06;
						break;
					case 36:
						sendBit[8] |= 0x10;
						break;
					case 37:
						sendBit[8] |= 0x20;
						break;
					case 38:
						sendBit[8] |= 0x30;
						break;
					case 39:
						sendBit[8] |= 0x80;
						sendBit[7] |= 0x00;
						break;
					case 40:
						sendBit[8] |= 0x00;
						sendBit[7] |= 0x01;
						break;
					case 41:
						sendBit[8] |= 0x80;
						sendBit[7] |= 0x01;
						break;
					case 42:
						sendBit[7] |= 0x04;
						break;
					case 43:
						sendBit[7] |= 0x08;
						break;
					case 44:
						sendBit[7] |= 0x0c;
						break;
					case 45:
						sendBit[7] |= 0x20;
						break;
					case 46:
						sendBit[7] |= 0x40;
						break;
					case 47:
						sendBit[7] |= 0x60;
						break;
					case 48:
						sendBit[6] |= 0x01;
						break;
					case 49:
						sendBit[6] |= 0x02;
						break;
					case 50:
						sendBit[6] |= 0x03;
						break;
					case 51:
						sendBit[6] |= 0x08;
						break;
					case 52:
						sendBit[6] |= 0x10;
						break;
					case 53:
						sendBit[6] |= 0x18;
						break;
					case 54:
						sendBit[6] |= 0x40;
						sendBit[5] |= 0x00;
						break;
					case 55:
						sendBit[6] |= 0x80;
						sendBit[5] |= 0x00;
						break;
					case 56:
						sendBit[6] |= 0xc0;
						sendBit[5] |= 0x00;
						break;
					case 57:
						sendBit[5] |= 0x02;
						break;
					case 58:
						sendBit[5] |= 0x04;
						break;
					case 59:
						sendBit[5] |= 0x06;
						break;
					case 60:
						sendBit[5] |= 0x10;
						break;
					case 61:
						sendBit[5] |= 0x20;
						break;
					case 62:
						sendBit[5] |= 0x30;
						break;
					case 63:
						sendBit[5] |= 0x80;
						sendBit[4] |= 0x00;
						break;
					case 64:
						sendBit[5] |= 0x00;
						sendBit[4] |= 0x01;
						break;
					case 65:
						sendBit[5] |= 0x80;
						sendBit[4] |= 0x01;
						break;
					case 66:
						sendBit[4] |= 0x04;
						break;
					case 67:
						sendBit[4] |= 0x08;
						break;
					case 68:
						sendBit[4] |= 0x0c;
						break;
					case 69:
						sendBit[4] |= 0x20;
						break;
					case 70:
						sendBit[4] |= 0x40;
						break;
					case 71:
						sendBit[4] |= 0x60;
						break;
					case 72:
						sendBit[3] |= 0x01;
						break;
					case 73:
						sendBit[3] |= 0x02;
						break;
					case 74:
						sendBit[3] |= 0x03;
						break;
					case 75:
						sendBit[3] |= 0x08;
						break;
					case 76:
						sendBit[3] |= 0x10;
						break;
					case 77:
						sendBit[3] |= 0x18;
						break;
					case 78:
						sendBit[3] |= 0x40;
						sendBit[2] |= 0x00;
						break;
					case 79:
						sendBit[3] |= 0x80;
						sendBit[2] |= 0x00;
						break;
					case 80:
						sendBit[3] |= 0xc0;
						sendBit[2] |= 0x00;
						break;
					case 81:
						sendBit[2] |= 0x02;
						break;
					case 82:
						sendBit[2] |= 0x04;
						break;
					case 83:
						sendBit[2] |= 0x06;
						break;
					case 84:
						sendBit[2] |= 0x10;
						break;
					case 85:
						sendBit[2] |= 0x20;
						break;
					case 86:
						sendBit[2] |= 0x30;
						break;
					case 87:
						sendBit[2] |= 0x80;
						sendBit[1] |= 0x00;
						break;
					case 88:
						sendBit[2] |= 0x00;
						sendBit[1] |= 0x01;
						break;
					case 89:
						sendBit[2] |= 0x80;
						sendBit[1] |= 0x01;
						break;
					case 90:
						sendBit[1] |= 0x04;
						break;
					case 91:
						sendBit[1] |= 0x08;
						break;
					case 92:
						sendBit[1] |= 0x0c;
						break;
					case 93:
						sendBit[1] |= 0x20;
						break;
					case 94:
						sendBit[1] |= 0x40;
						break;
					case 95:
						sendBit[1] |= 0x60;
						break;
					
					
					default:
						break;
				}
			}
			else if(lampOn[j] == 0 && lampFlash[j] == 0)
			{
				switch(j)
				{
					case 0:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 1:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 2:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 3:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 4:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 5:
						sendBit[12] |= LAMP_OFF_ALL;
						break;
					case 6:
						sendBit[12] |= LAMP_OFF_ALL;    //第一级灯的前两个字节
						sendBit[11] |= LAMP_OFF_ALL;	//第一块第三个灯组 的第一个字节
						break;
					case 7:
						sendBit[12] |= LAMP_OFF_ALL;
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 8:
						sendBit[12] |= LAMP_OFF_ALL;
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 9:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 10:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 11:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 12:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 13:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 14:
						sendBit[11] |= LAMP_OFF_ALL;
						break;
					case 15:
						sendBit[11] |= LAMP_OFF_ALL;
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 16:
						sendBit[11] |= LAMP_OFF_ALL;
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 17:
						sendBit[11] |= LAMP_OFF_ALL;
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 18:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 19:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 20:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 21:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 22:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 23:
						sendBit[10] |= LAMP_OFF_ALL;
						break;
					case 24:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 25:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 26:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 27:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 28:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 29:
						sendBit[9] |= LAMP_OFF_ALL;
						break;
					case 30:
						sendBit[9] |= LAMP_OFF_ALL;
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 31:
						sendBit[9] |= LAMP_OFF_ALL;
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 32:
						sendBit[9] |= LAMP_OFF_ALL;
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 33:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 34:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 35:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 36:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 37:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 38:
						sendBit[8] |= LAMP_OFF_ALL;
						break;
					case 39:
						sendBit[8] |= LAMP_OFF_ALL;
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 40:
						sendBit[8] |= LAMP_OFF_ALL;
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 41:
						sendBit[8] |= LAMP_OFF_ALL;
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 42:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 43:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 44:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 45:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 46:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 47:
						sendBit[7] |= LAMP_OFF_ALL;
						break;
					case 48:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 49:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 50:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 51:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 52:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 53:
						sendBit[6] |= LAMP_OFF_ALL;
						break;
					case 54:
						sendBit[6] |= LAMP_OFF_ALL;
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 55:
						sendBit[6] |= LAMP_OFF_ALL;
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 56:
						sendBit[6] |= LAMP_OFF_ALL;
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 57:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 58:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 59:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 60:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 61:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 62:
						sendBit[5] |= LAMP_OFF_ALL;
						break;
					case 63:
						sendBit[5] |= LAMP_OFF_ALL;
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 64:
						sendBit[5] |= LAMP_OFF_ALL;
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 65:
						sendBit[5] |= LAMP_OFF_ALL;
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 66:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 67:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 68:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 69:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 70:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 71:
						sendBit[4] |= LAMP_OFF_ALL;
						break;
					case 72:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 73:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 74:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 75:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 76:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 77:
						sendBit[3] |= LAMP_OFF_ALL;
						break;
					case 78:
						sendBit[3] |= LAMP_OFF_ALL;
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 79:
						sendBit[3] |= LAMP_OFF_ALL;
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 80:
						sendBit[3] |= LAMP_OFF_ALL;
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 81:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 82:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 83:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 84:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 85:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 86:
						sendBit[2] |= LAMP_OFF_ALL;
						break;
					case 87:
						sendBit[2] |= LAMP_OFF_ALL;
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 88:
						sendBit[2] |= LAMP_OFF_ALL;
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 89:
						sendBit[2] |= LAMP_OFF_ALL;
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 90:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 91:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 92:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 93:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 94:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					case 95:
						sendBit[1] |= LAMP_OFF_ALL;
						break;
					
					
					default:
						break;
				}
			}
			//ACE_DEBUG((LM_DEBUG,"%s:%d<<<<< Send Step: lampFlash %d!>>>>>>\n",__FILE__,__LINE__,lampFlash[j]));
	
		}
		Byte sendStep[19] = {0xaa,0x55,0x10,MAINBACKUP_LAMP,i,sendBit[0],sendBit[1],sendBit[2],sendBit[3],sendBit[4],sendBit[5],sendBit[6],sendBit[7],sendBit[8],sendBit[9],sendBit[10],sendBit[11],sendBit[12],0xff};
		Byte chksum = ~(MAINBACKUP_LAMP+i+sendBit[0]+sendBit[1]+sendBit[2]+sendBit[3]+sendBit[4]+sendBit[5]+sendBit[6]+sendBit[7]+sendBit[8]+sendBit[9]+sendBit[10]+sendBit[11]+sendBit[12]);
		sendStep[18] = chksum;
		ACE_DEBUG((LM_DEBUG,"%s:%d<<<<< Send Step: sendStep[0]=%x sendStep[1]=%x sendStep[2]=%x sendStep[3]=%x sendStep[4]=%x sendStep[5]=%x sendStep[6]=%x sendStep[7]=%x sendStep[8]=%x sendStep[9]=%x sendStep[10]=%x sendStep[11]=%x sendStep[12]=%x sendStep[13]=%x sendStep[14]=%x sendStep[15]=%x sendStep[16]=%x sendStep[17]=%x sendStep18]=%x >>>>>>\n",__FILE__,__LINE__,sendStep[0],sendStep[1],sendStep[2],sendStep[3],sendStep[4],sendStep[5],sendStep[6],sendStep[7],sendStep[8],sendStep[9],sendStep[10],sendStep[11],sendStep[12],sendStep[13],sendStep[14],sendStep[15],sendStep[16],sendStep[17],sendStep[18]));
		SendBackup(sendStep,19);
	}
}

/**************************************************************
Function:        MainBackup::DoManual
Description:    形成核心板信号机程序发送到备份单片机,进行手控按钮查询			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::DoManual()
{
	Byte readManual[7] = {0xaa,0x55,0x04,MAINBACKUP_READ_MANUAL,0xff,0xff,0xff};
	Byte chksum = ~(MAINBACKUP_READ_MANUAL+0xff+0xff);
	readManual[6] = chksum;

	Byte reByte[8] = {0};
	SendBackup(readManual,7);
}
/**************************************************************
Function:        MainBackup::HeartBeat
Description:    心跳			
Input:          无
Output:         无
Return:         无
***************************************************************/
void MainBackup::HeartBeat()
{
	Byte stepNo = pManaKernel->m_pRunData->ucStepNo;
	//组合心跳指令
	Byte heart[7] = {0xaa,0x55,0x04,MAINBACKUP_HEART,0xff,stepNo,0xff};
	Byte chksum = ~(MAINBACKUP_HEART+0xff+stepNo);
	heart[6] = chksum;
	// 500ms 发送心跳数据，无返回数据
	//SendBackup(heart,sizeof(heart)/sizeof(heart[0]));
	Byte reByte[8] = {0};
	SendBackup(heart,7);
	
}
/**************************************************************
Function:        MainBackup::OperateBackup
Description:     公共方法，包括备份单片机中手动，指示灯，备份功能共用			
Input:          pbytes 字节数组，是发送到pic的指令
				rsLen  pic单片机返回给核心板的数据长度
Output:         无
Return:         无
***************************************************************/
void MainBackup::OperateBackup(Byte *pbytes, Uint pLen,Byte *rsBytes, Uint rsLen)
{

		ACE_Guard<ACE_Thread_Mutex> guard(m_sMutex);
		//ACE_DEBUG((LM_DEBUG,"%s:%d sizeof(pbytes)/sizeof(pbytes[0]): %d  \n",__FILE__,__LINE__,sizeof(pbytes)/sizeof(pbytes[0])));
		//SendBackup(pbytes,pLen);
		//RecevieBackup(rsBytes, rsLen);
		//ACE_DEBUG((LM_DEBUG,"%s:%d pbytes[0]: %d  -- pbytes[1]: %d -- pbytes[2]: %d -- pbytes[3]: %d -- pbytes[4]: %d -- pbytes[5]: %d -- pbytes[6]: %d -- pbytes[7]: %d\n",__FILE__,__LINE__,pbytes[0],pbytes[1],pbytes[2],pbytes[3],pbytes[4],pbytes[5],pbytes[6],pbytes[7]));
		//这里是查询按钮状态，返回数据0xaa 55 05 03 FF [00  00] chk

	
}
/**************************************************************
Function:        MainBackup::ReadLED
Description:     核心板发送读取指示灯指令后，备份单片机返回的LED字节			
Input:          status 字节数，LED的1个字节
Output:         无
Return:         无
***************************************************************/
void MainBackup::ReadLED(Byte &status)
{
	 //一个字节中，低两位表示信号机运行状态，反应到主板的指示灯上。
	 //00 表示工作正常，绿色；01 表示降级 黄色；10 表示 异常红色
	Byte runLed = status & 0x03; 
	 //bit2 表示 运行手控还是自动运行。0表示自动运行灯亮起，1表示手动运行灯灭
	Byte selfAndManual = (status >> 2) & 0x01;	
	 //bit3表示 tsc 与 psc 模式。0表示 tsc 模式灯亮起；1表示psc模式灯灭
	Byte tscAndpsc = (status >>1 ) & 0x01;
	ACE_DEBUG((LM_DEBUG,"%s:%d MSG: LED status %d\n",__FILE__,__LINE__,status));
	
	switch(runLed)
	{
		case MAINBACKUP_LED_MODE_GREEN:

			break;
		case MAINBACKUP_LED_MODE_YELLOW:

			break;
		case MAINBACKUP_LED_MODE_RED:

			break;
		default:
			break;

	}

	switch(selfAndManual)
	{
		case MAINBACKUP_AUTO_SELF:

			break;
		case MAINBACKUP_AUTO_MANUAL:

			break;
		default:
			break;
	}
	switch(tscAndpsc)
	{
		case MAINBACKUP_TSCP_TSC:

			break;
		case MAINBACKUP_TSCP_PSC:

			break;
		default:
			break;
	}
}

/**************************************************************
Function:        MainBackup::ReadID
Description:     核心板发送读取指令后，备份单片机返回的id字节数组			
Input:          pbyte 字节数组，ID的8个字节
Output:         无
Return:         无
***************************************************************/
void MainBackup::ReadID(Byte *pByte)
{
	ACE_DEBUG((LM_DEBUG,"%s:%d MSG: ReadID array[0] %d -array[1] %d-array[2] %d-array[3] %d-array[4] %d-array[5] %d-array[6] %d-array[7] %d\n",__FILE__,__LINE__,pByte[0],pByte[1],pByte[2],pByte[3],pByte[4],pByte[5],pByte[6],pByte[7]));
	return ;
}


/**************************************************************
Function:        MainBackup::WriteIDTF
Description:     核心板发送保存指令后，备份单片机返回的id字节数组			
Input:          pbyte 字节数组，ID的8个字节
Output:         无
Return:         无
***************************************************************/
void MainBackup::WriteIDTF(Byte *pByte)
{
	ACE_DEBUG((LM_DEBUG,"%s:%d MSG: ReadID array[0] %d -array[1] %d-array[2] %d-array[3] %d-array[4] %d-array[5] %d-array[6] %d-array[7] %d\n",__FILE__,__LINE__,pByte[0],pByte[1],pByte[2],pByte[3],pByte[4],pByte[5],pByte[6],pByte[7]));
	return ;

}

void MainBackup::WriteLEDTF(Byte &writeLED)
{
	ACE_DEBUG((LM_DEBUG,"%s:%d MSG: writeLED  %d\n",__FILE__,__LINE__,writeLED));
	return ;

}

void MainBackup::CurrentSetp(Byte &setp)
{
	ACE_DEBUG((LM_DEBUG,"%s:%d MSG: setp  %d\n",__FILE__,__LINE__,setp));
	return ;
}


void MainBackup::SetpTable(Byte *setpTable)
{

	return ;
}

/**************************************************************
Function:        MainBackup::OperateBackup
Description:     公共方法，包括备份单片机中手动，指示灯，备份功能共用			
Input:          pbytes 字节数组，是发送到pic的指令
				rsLen  pic单片机返回给核心板的数据长度
Output:         无
Return:         无
***************************************************************/
void* MainBackup::Recevie(void* arg)
{
	//OpenDev();
		//ACE_Guard<ACE_Thread_Mutex> guard(m_sMutex);
		//ACE_DEBUG((LM_DEBUG,"%s:%d sizeof(pbytes)/sizeof(pbytes[0]): %d  \n",__FILE__,__LINE__,sizeof(pbytes)/sizeof(pbytes[0])));
		//SendBackup(pbytes,pLen);
		ACE_DEBUG((LM_DEBUG,"%s:%d ####################OperateManual #############! \n",__FILE__,__LINE__));
		Byte pByte[255] = {0};
	MainBackup *pMainBackup = MainBackup::CreateInstance();
	CSerialCtrl::CreateInstance()->OpenComPort(3, 57600, 8, "1", 'N');
		//pMainBackup->RecevieBackup(rsBytes, 8);
		while(1){
			bzero(pByte, 255);
			int len_tty = -1;
			len_tty = CSerialCtrl::CreateInstance()->ReadComPort(pByte, 255);
		
			//ACE_DEBUG((LM_DEBUG,"%s:%d Recv:m_iSerial3fd %d   iSize  %d  \n" ,__FILE__,__LINE__,m_iSerial3fd,iSize));
			if (len_tty < 0) {		
				ACE_DEBUG((LM_DEBUG,"%s:%d Error: ReadComPort Error %d\n",__FILE__,__LINE__));
				return NULL;
			}
			ACE_DEBUG((LM_DEBUG,"%s:%d Recv: %d bytes, [%X %X %X %X %X %X %X %X ]\n",__FILE__,__LINE__,len_tty,pByte[0],pByte[1],pByte[2],pByte[3],pByte[4],pByte[5],pByte[6],pByte[7]));
			
			//len_tty = CSerialCtrl::CreateInstance()->WriteComPort(pByte, len_tty);	///这里是将外部设备输入的同时，输出给外部设备。调试使用
			//CSerialCtrl::CreateInstance()->WriteComPort(" recved:", sizeof(" recved:"));	   //字节类型，所以字符串不能写
			
			//>>这里主要是将得到的字节数组255个中的有用的部分取出，放到resultBytes.
			Uint bLen= pByte[2] + 2;
			Byte *resultBytes = new Byte[bLen];
			ACE_OS::memcpy(resultBytes,pByte,bLen);
			
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		
		//ACE_DEBUG((LM_DEBUG,"%s:%d pbytes[0]: %d  -- pbytes[1]: %d -- pbytes[2]: %d -- pbytes[3]: %d -- pbytes[4]: %d -- pbytes[5]: %d -- pbytes[6]: %d -- pbytes[7]: %d\n",__FILE__,__LINE__,pbytes[0],pbytes[1],pbytes[2],pbytes[3],pbytes[4],pbytes[5],pbytes[6],pbytes[7]));
		//这里是查询按钮状态，返回数据0xaa 55 05 03 FF [00  00] chk
			Byte ManualBytes[2] = {0};
			Ushort ManualButtonSts;
				
			Byte LEDStatus = 0x00;
			//ID的字节长度为8
			Byte readId[8] = {0};
			
			Byte writeId[8] = {0};

			Byte writeLED = 0x00;

			Byte setp = 0x00;

			Byte setpTable[13] = {0};
//		ACE_DEBUG((LM_DEBUG,"%s:%d Error: ManualButtonSts \n",__FILE__,__LINE__,ManualButtonSts));
		if (resultBytes[0] == 0xaa && resultBytes[1] == 0x55)
		{
			//ACE_DEBUG((LM_DEBUG,"%s:%d aa 55\n",__FILE__,__LINE__));
			switch(resultBytes[3])
			{	
				//ACE_DEBUG((LM_DEBUG,"%s:%d aa 55 5\n",__FILE__,__LINE__));
				case MAINBACKUP_RECEVIE_READ_MANUAL:
					ManualBytes = {resultBytes[5],resultBytes[6]};
					ManualButtonSts = bytes2T<Ushort>(ManualBytes);
					ACE_DEBUG((LM_DEBUG,"%s:%d ManualButtonSts: %d\n",__FILE__,__LINE__,ManualButtonSts));
					pMainBackup->OperateManual(ManualButtonSts);
					//delete ManualBytes
					break;
				case MAINBACKUP_RECEVIE_ID:
					readId = {resultBytes[5],resultBytes[6],resultBytes[7],resultBytes[8],resultBytes[9],resultBytes[10],resultBytes[11],resultBytes[12]};
					pMainBackup->ReadID(readId);
					break;
				case MAINBACKUP_RECEVIE_WRITE_ID_TF:		///送发数据到PIC
					writeId = {resultBytes[5],resultBytes[6],resultBytes[7],resultBytes[8],resultBytes[9],resultBytes[10],resultBytes[11],resultBytes[12]};
					pMainBackup->WriteIDTF(writeId);
					break;
				case MAINBACKUP_RECEVIE_READ_LED:
					LEDStatus = resultBytes[5];
					pMainBackup->ReadLED(LEDStatus);
					
					break;
				case MAINBACKUP_RECEVIE_WRITE_LED_TF:  ///送发数据到PIC
					writeLED =  resultBytes[5];
					pMainBackup->WriteLEDTF(writeLED);
					break;
					
				case MAINBACKUP_RECEVIE_SETP:
					setp = resultBytes[5];
					pMainBackup->CurrentSetp(setp);
					break;
				case MAINBACKUP_RECEVIE_SETP_TABLE:
					setpTable = {};
					pMainBackup->SetpTable(setpTable);
					break;
				case MAINBACKUP_RECEVIE_NONE:

					break;
				default:

					break;
			}
		}	
		delete [] resultBytes;
		resultBytes = NULL;
	}
}
