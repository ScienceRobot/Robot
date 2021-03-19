//win_Accels.h
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
*/
#ifndef _WIN_ACCELS_H
#define _WIN_ACCELS_H

//structure used to track user settings for looking at data in the Accelerometer window
//#define ROBOT_WIN_ACCELS_DISPLAY_ACCEL_DATA  0x00000010  //if set, display accel data as it arrives
//typedef struct Robot_win_Accels_Info {
//uint32_t flags;
//} Robot_win_Accels_Info;


#define ACCEL_TIMER_INTERVAL_IN_MS 10 //50 //100//33 //33 ms - the interval between timer ISR calls. The Timer ISR processes sending samples (both one-time and polling), and accelerometer (PIC external) interrupts
#define ACCEL_SAMPLES_PER_SECOND 1000.0/ACCEL_TIMER_INTERVAL_IN_MS //100 samples/sec for 10ms samples


//Variables needed for Accel Window- currently only stores parameters for Calibrating Accels+Gyros in real-time
#define ACCEL_WINDOWS_STATUS_ALL_ETHACCELS		0x00000001 //calibrate the accels on all EthAccels PCBs (otherwise just one a single EthAccels PCB
typedef struct AccelWindowStatus
{
uint32_t flags;  //see above flags
//int NumPCBs;
int PCBNum;  //for calibrating a single EthAccels PCB
uint16_t AccelMask;
int AccelAndOrGyro;
int NumCalibSamples;
} AccelWindowStatus;


int winAccels_AddFTWindow(void);
int winAccels_AddFTControls(FTWindow *twin);
void winAccels_key(FTWindow *twin,KeySym key);
int FillAccelItemList(void);
int ddAccelPCBName_OnChange(FTWindow *twin,FTControl *tcontrol);
int btnSetAccelTimerInterval_Click(FTWindow *twin,FTControl *tcontrol,int x, int y);
int txtInterval_OnChange(FTWindow *twin,FTControl *tcontrol);
int SetUserEthAccelTimerInterval(void);
int SetEthAccelTimerInterval(MAC_Connection *lmac,uint16_t TimerInMS);
int chkAccelAll_OnChange(FTWindow *twin,FTControl *tcontrol);
int winAccels_AddFTControls(FTWindow *twin);
int btnAccelsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int GetAccelerometerSamples(Accels_PCB *leth,uint16_t AccelMask);
//int ClearAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask);
//int RequestAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask);  
//int WaitForAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask);
//int GetAccelerometerSample(int AccelNum,AccelSample *Sample);
int btnAccelsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAccelsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int	StartPollingAccelerometers(MAC_Connection *lmac,uint16_t mask);
int	StopPollingAccelerometers(MAC_Connection *lmac,uint16_t mask);
int btnAccelsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAccelsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnGetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnSetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
uint16_t GetAccelMask(void);
int btnAutoCalibrate_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnResetAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnGPSWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnwinAccelsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnSendInstructionToAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int SendUserInstructionToAccel(unsigned char *Inst, int numbyte, unsigned int flags);
int SendInstructionToAccel(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags);
int winAccels_OnOpen(FTWindow *twin);
int winAccels_OnClose(FTWindow *twin);
int chkAccel_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkAccel_UseCalibration_OnChange(FTWindow *twin, FTControl *tcontrol);
int chkAccel_Accelerometers_Only_OnChange(FTWindow *twin, FTControl *tcontrol);
//void btnCalibrateAccels_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
//void btnCalibrateGyros_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnCalibrateAllAccelGyro_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnCalibrateAccelGyro_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
//int CalibrateAccelGyro(int NumPCBs, int *PCBNum, uint16_t AccelMask, int AccelAndOrGyro, int NumCalibSamples);
int CreateAutoCalibrateThread(void);
int StopAutoCalibrateThread(void);
#if Linux
int Thread_AutoCalibrateAccelGyro(void);
#endif
#if WIN32
DWORD WINAPI Thread_AutoCalibrateAccelGyro(LPVOID lpParam);
#endif
#endif //_WIN_ACCELS_H
