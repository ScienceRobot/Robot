//win_Motors.h

int winPower_AddFTWindow(void);
int winPower_AddFTControls(FTWindow *twin);
int Populate_EthPower_Dropdown(FTControl *tc);
int ddPowerPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol);
void btnwinPowerExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnSendInstructionToPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int SendUserInstructionToPower(unsigned char *Inst,int numbyte,unsigned int flags);
int SendInstructionToPower(MAC_Connection *lmac,unsigned char *Inst, int numbyte, unsigned int flags);
int winPower_OnOpen(FTWindow *twin);
int winPower_OnClose(FTWindow *twin);
int TurnOff3V(void);
int TurnOff36V(void);
void btnToggle3V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnToggle36V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
void btnUpdatePower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
