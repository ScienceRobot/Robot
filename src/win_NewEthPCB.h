//win_NewEthPCB.h
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
#ifndef _WIN_NEWETHPCB_H
#define _WIN_NEWETHPCB_H

int win_NewEthPCB_CreateFTWindow(void);
int win_NewEthPCB_AddFTControls(FTWindow *twin);
void btn_NewEthPCB_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
unsigned int btn_NewEthPCB_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
int ddNewEthPCB_Location_OnChange(FTWindow *twin, FTControl *tFTControl);
#endif /* _WIN_NEWETHPCB_H */
