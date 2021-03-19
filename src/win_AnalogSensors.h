//win_AnalogSensors.h
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
#ifndef _WIN_ANALOGSENSORS_H
#define _WIN_ANALOGSENSORS_H

int winAnalogSensors_AddFTWindow(void);
void winAnalogSensors_key(FTWindow *twin,KeySym key);
int chkAnalogSensorAll_OnChange(FTWindow *twin,FTControl *tcontrol);
int winAnalogSensors_AddFTControls(FTWindow *twin);
int btnAnalogSensorsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsGetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnAnalogSensorsSetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int SetAnalogSensorsMinAndMax(MAC_Connection *lmac,unsigned int Mask,float MinV,float MaxV);
void btnwinAnalogSensorsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnSendInstructionToAnalogSensor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int winAnalogSensors_OnOpen(FTWindow *twin);
int winAnalogSensors_OnClose(FTWindow *twin);
int chkAnalogSensor_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol);

#endif //_WIN_ANALOGSENSORS_H
