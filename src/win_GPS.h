//win_GPS.h

int winGPS_AddFTWindow(void);
int winGPS_AddFTControls(FTWindow *twin);
int btnGPSGetData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnGPSStopData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnGPSSendData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnGPSStaticNav_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int chkSendAllGPSData_OnChange(FTWindow *twin,FTControl *tcontrol);
void btnwinGPSExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
//int winGPS_OnOpen(FTWindow *twin);
//int winGPS_OnClose(FTWindow *twin);
