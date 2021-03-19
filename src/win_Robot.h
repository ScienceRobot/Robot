//win_Robot.h
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
#ifndef _WIN_ROBOT_H
#define _WIN_ROBOT_H

int winRobot_AddFTWindow(void);
int winRobot_AddFTControls(FTWindow *twin);
int GetLocalNetworkInterfaces(void);
int btnRobotModel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int chkUseWirelessInterface_OnChange(FTWindow *twin,FTControl *tcontrol);
int btnRobotModelLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRobotNetworkLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int chkLogInData_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkLogOutData_OnChange(FTWindow *twin,FTControl *tcontrol);
int btnMotors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAccels_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnCameras_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnLoadRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnTasks_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
//int btnConnectToIPs_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnIdentifyEthPCBs_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
//int DisconnectGenericIPs(void);
int btnListenToOutside_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnRobotExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnSendInstructionToRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
MAC_Connection *ListenToOutside(void);
int SendInstructionToRobot(unsigned char *Inst, int numbyte, unsigned int flags);
int winRobot_OnOpen(FTWindow *twin);
int winRobot_OnClose(FTWindow *twin);
void test_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
#endif //_WIN_ROBOT_H
