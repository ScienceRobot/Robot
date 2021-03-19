//win_Motors.h
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
#ifndef _WIN_MOTORS_H
#define _WIN_MOTORS_H

int winMotors_AddFTWindow(void);
int winMotors_AddFTControls(FTWindow *twin);
int Populate_EthMotors_Dropdown(FTControl *tc);
void winMotors_key(FTWindow *twin,KeySym key);
int ddMotorPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol);
//void btnConnectToMotor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnwinMotorsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnSendInstructionToMotor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int SendUserSettingsInstruction(void);
#if USE_RT_PREEMPT
int Thread_TurnMotor(void);
#endif 
//int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Direction,int Speed,int Duration);
int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Speed,int Duration,int Queue);
int SendUserInstructionToMotor(unsigned char *Inst,int numbyte,unsigned int flags);
int SetEthMotorsDutyCycle(MAC_Connection *lmac,unsigned int DutyCycleClock);
int SendInstructionToMotor(MAC_Connection *lmac,unsigned char *Inst, int numbyte, unsigned int flags);
int SendInstructionToEthMotors(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags);
int StopAllMotors(void);
void btnMotorLeftArrow_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnMotorStop_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnMotorRightArrow_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int chkComplement_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkCompOpp_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkPair_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkPairOpp_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkDisableMotors_OnChange(FTWindow *twin,FTControl *tcontrol);
int winMotors_OnOpen(FTWindow *twin);
int winMotors_OnClose(FTWindow *twin);
#endif //_WIN_MOTORS_H
