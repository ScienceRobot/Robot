//win_RobotModel.h
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
#ifndef _WIN_ROBOTMODEL_H
#define _WIN_ROBOTMODEL_H

#define LOGFILEBUFFERSIZE 5000  //how many bytes are read in from a log file at once


//structure used to track user settings for looking at 3D model of Robot in Robot Model window
#define ROBOT_MODEL_INFO_PLAYING_LOG  0x00000001  //Log file is currently playing
#define ROBOT_MODEL_INFO_GOT_FIRST_TIMESTAMP  0x00000002  //got logs first time stamp- used to track elapsed time
//#define ROBOT_MODEL_INFO_LOADED_BUFFER  0x00000004  //loaded log file data into buffer
#define ROBOT_MODEL_INFO_CAMERA_SELECTED  0x00000004  //if set user keys move camera, othewise they move robot model
#define ROBOT_MODEL_INFO_RENDER_IN_REALTIME  0x00000008  //if set, render robot body segment accel angles in real-time (alt: LIVE_RENDER)
#define ROBOT_MODEL_INFO_SKIP_PROCESS 			0x000000010  //skip Process Model Log line, used to quickly skip to a specific stage
#define ROBOT_MODEL_INFO_GET_STAGE_NUMBER		0x000000020  //after comma keypress, expecting keypress number. After number skip to that stage
//#define ROBOT_MODEL_REACHED_END_OF_LOGFILE  0x00000008  //reached end of log file
typedef struct RobotModelInfo {
uint32_t flags;
FILE *fRobotLoadedLog; //pointer to loaded Robot log file (used to play back earlier robot movements on the animated 3D model)
//unsigned int LogFilePos; //current index in bytes into loaded log file from beginning of file
//unsigned int BufStartFilePos; //File position of buffer start- needed to know when there are no more entries to step back to
unsigned int LogFileSize; //Size of log file
char buffer[LOGFILEBUFFERSIZE];//buffer of LOGFILEBUFFERSIZE bytes to hold file in RAM
char LogLine[2048]; //temporarily holds Log line
unsigned int ibuf;  //current index into buffer - maybe should be 64-bit but I doubt there would ever be alog file that large
//unsigned int bufsize; //current buffer size- how much of the log file is loaded in RAM
unsigned int endbuf; //index of end of buffer
int LastStepDir; //direction of last step 0=none, 1=forward,-1=backward. Needed to step an extra log entry when changing directions, otherwise the same log entry will be counted as a step.
//unsigned int offset;   //offset into RAM buffer, needed because a log entry may be split between two buffers, so part is copied to the top (for forward step), or bottom (for reverse step), and the rest read in from the file
//unsigned int LineNum; //number of current line in log
float	PlayDelay; //speed to playback a log
uint64_t StartTime; //start of log time in ms
unsigned int StageNumber; //number of stage
uint64_t StartStageTime; //start of stage time in ms
uint64_t EventTime; //time in ms of current log event
uint64_t TimeElapsed; //time elapsed when playing log (in terms of log time) in ms
uint64_t StageTimeElapsed; //stage time elapsed in ms
FTControl *tc_time,*tc_stime,*tc_log;  //links to the time and stage time labels and log entry controls
} RobotModelInfo;

int winRobotModel_AddFTWindow(void);
int winRobotModel_AddFTControls(FTWindow *twin);
void win_RobotModel_Keypress(FTWindow *twin,KeySym key);
int RenderModelMainImage(void);
int winRobotModel_OnOpen(FTWindow *twin);
int winRobotModel_OnClose(FTWindow *twin);
int btnRobotModel_LoadLogFile_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnRobotModel_PlayLogFile_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnRobotModel_SkipLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRobotModel_StepLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRobotModel_StepLogFileBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int ProcessLogLine(char *tline);
int btnRobotModel_StopLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnRobotModel_RewindLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int RewindLogFile(void);
int btnRobotModel_Close_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
uint64_t ConvertTimeStampToMS(char *ts);
int txtRobotModel_PlayDelay_OnChange(FTWindow *twin,FTControl *tcontrol);
int chkRobotModel_RealTimeRender_OnChange(FTWindow *twin,FTControl *tcontrol);
int ReplaceTabWithSpace(char *tline);
int SkipToStage(int StageNum);
int Skip10ms(int dir);
void ResetAccelAndMotorColors(void);
void ClearAccelAndMotorBackgroundColors(void);
int GetTextUpToString(char *src,char *match,char *dest);
int btnRobotModel_Go_To_Stage_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
#endif /* _WIN_ROBOTMODEL_H */
