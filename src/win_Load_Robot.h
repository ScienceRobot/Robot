//win_Load_Robot.h

#ifndef _WIN_LOAD_ROBOT_H
#define _WIN_LOAD_ROBOT_H

int winLoadRobot_AddFTWindow(void);
int winLoadRobot_AddFTControls(FTWindow *twin);
int winLoadRobot_OnOpen(FTWindow *twin);
void btn_LoadRobot_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
unsigned int btn_LoadRobot_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
#endif /* _WIN_LOAD_ROBOT_H */
