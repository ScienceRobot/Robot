//win_Load_Robot_Log.c
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
*/#include "freethought.h"
#include "Robot.h"
#include "win_RobotModel.h"  //for RMInfo.iLogFile
#include "win_Load_Robot_Log.h"

extern ProgramStatus PStatus; //for info messages
extern RobotStatus RStatus;
extern RobotModelInfo RMInfo; //Robot Model Info 


int win_Load_Robot_Log_CreateFTWindow(void)
{
FTWindow twin,*twin2;
int newx,newy;

twin2=GetFTWindow("win_Load_Robot_Log");
if (twin2!=0) {
	newx=twin2->x+300;
	newy=twin2->y+20;
} else {
	newx=300;
	newy=100;
}

memset(&twin,0,sizeof(FTWindow));
strcpy(twin.name,"win_Load_Robot_Log");
strcpy(twin.title,"Load Robot Log File");
strcpy(twin.ititle,"Load Robot Log File");
twin.x=newx;
twin.y=newy;
twin.w=550;
twin.h=370;
//twin.Keypress=(FTControlfuncwk *)main_Keypress;
//twin.OnOpen=(FTControlfuncw *)main_OnOpen;
//twin.GotFocus=(FTControlfuncw *)main_GotFocus;
//twin.ButtonClick[0]=(FTControlfuncwcxy *)main_ButtonDownUpOnFTControl;
twin.AddFTControls=(FTControlfuncw *)win_Load_Robot_Log_AddFTControls;

//change dir to logs folder
#if Linux
//chdir(RStatus.ModelLogFolder);
chdir(RStatus.CurrentRobotModelLogFolder);
#endif
#if WIN32
_chdir(RStatus.CurrentRobotModelLogFolder);
#endif

//fprintf(stderr,"create window main_CreateFTWindow\n");
CreateFTWindow(&twin);
twin2=GetFTWindow(twin.name);
DrawFTWindow(twin2);

//fprintf(stderr,"end main_CreateFTWindow\n");
return 1;
} //end win_Load_Robot_Log_CreateFTWindow



int win_Load_Robot_Log_AddFTControls(FTWindow *twin)
{
FTControl *tcontrol,*tcontrol2;
//FTItem titem;
int fh,fw;

if (PStatus.flags&PInfo) {
	fprintf(stderr, "Adding FTControls for window %s\n", twin->name);
}

fw=twin->fontwidth;
fh=twin->fontheight;

tcontrol=(FTControl *)malloc(sizeof(FTControl));

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadLogPath");
tcontrol->type=CTTextBox;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->x2=fw*73;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowX2;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblLoadLogName");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*10;
tcontrol->flags=CGrowY1|CGrowY2;
strcpy(tcontrol->text,"filename:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadLogName");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*11;
tcontrol->y1=fh*21;
tcontrol->x2=fw*73;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowY1|CGrowX2|CGrowY2;
tcontrol->flags=CGrowX2|CGrowY1|CGrowY2;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"foLoadLog");
tcontrol->type=CTFileOpen;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
tcontrol->x2=fw*73;
tcontrol->y2=fh*20;
tcontrol->flags=CGrowX2|CGrowY2|CVScroll;//|CHScroll;
tcontrol2=GetFTControl("txtLoadLogPath");
tcontrol->TextBoxPath=tcontrol2; //so the textbox control with the path will automatically get updated when the file open control path changes
tcontrol2=GetFTControl("txtLoadLogName");
tcontrol->TextBoxFile=tcontrol2; //so this control will automatically get updated with the currently selected filename in the file open control
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadLog_OK");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"OK");
tcontrol->x1=fw*5;
tcontrol->y1=fh*23;
tcontrol->x2=fw*13;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_Load_Robot_Log_OK_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadLog_Cancel");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"Cancel");
tcontrol->x1=fw*14;
tcontrol->y1=fh*23;
tcontrol->x2=fw*22;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_Load_Robot_Log_Cancel_Click;
AddFTControl(twin,tcontrol);


if (tcontrol!=0) {
	free(tcontrol);
}

if (PStatus.flags&PInfo) {
  fprintf(stderr,"Done adding FTControls for window %s\n",twin->name);
}

//#endif
return(1);
} //end win_Load_Robot_Log_AddFTControls


void btn_Load_Robot_Log_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
CloseFTWindow(twin);
}

//load the log and store the pointer in the global Robot structure
unsigned int btn_Load_Robot_Log_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
	char *tstr,tstr2[FTMedStr],tstr3[FTMedStr];
	FTControl *tcontrol,*tcontrol2,*tc;
	int ExitLoop,MultipleFiles,cur,cur2,FoundError;
	char *log;

	//Load log from file

	//get path from control
	tcontrol=GetFTControl("foLoadLog");
	tcontrol2=GetFTControl("txtLoadLogName");
	
	//go through list of file names, but only load the first selected file
	ExitLoop=0;
	FoundError=0;
	tstr=tcontrol2->text;
	cur=0;
	cur2=0;
	MultipleFiles=0;
	while(!ExitLoop) {
		while (tstr[cur]!=44 && tstr[cur]!=0) { // comma (,)
			tstr2[cur2]=tstr[cur];
			cur++;
			cur2++;
			ExitLoop=1; //we only want the first file
		} //while (tstr3[cur]!=44 && tstr3[cur]!=0) { // comma (,)
		tstr2[cur2]=0;
		cur2=0;
		if (tstr[cur]==0) {
			ExitLoop=1;
		} else {
			cur++;
			MultipleFiles=1;
		}

		//note that no file extension is added
#if Linux
		sprintf(tstr3,"%s/%s",tcontrol->FileOpenPath,tstr2);
#endif
#if WIN32
		sprintf(tstr3,"%s\\%s",tcontrol->FileOpenPath,tstr2);
#endif

		//try to open the (first selected) log file
		RMInfo.fRobotLoadedLog = fopen(tstr3, "rb");
		if (RMInfo.fRobotLoadedLog == 0) {
			sprintf(tstr, "Could not load log file: %s",tstr3);
			FTMessageBox(tstr, FTMB_OK, "Robot - Error",0);
			return(0);
		}

#if Linux
		fseeko(RMInfo.fRobotLoadedLog,0,SEEK_END);
		RMInfo.LogFileSize=ftello(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
		_fseeki64(RMInfo.fRobotLoadedLog,0,SEEK_END);
		RMInfo.LogFileSize=_ftelli64(RMInfo.fRobotLoadedLog);
#endif

		rewind(RMInfo.fRobotLoadedLog);
		RMInfo.flags&=~ROBOT_MODEL_INFO_GOT_FIRST_TIMESTAMP;
		RewindLogFile(); //rewind any existing log file settings


	} //while(!ExitLoop) {


	//RMInfo.LogFilePos = 0; //set index into file initially to 0

	RMInfo.ibuf = 0; //reset pointer in local buffer (could be reload log)
	RMInfo.LastStepDir = 0; //reset last step direction
	//RMInfo.offset=0; //reset offset in local buffer (could be reload)
	//sprintf(tstr, "Log file %s loaded", tstr3);
	//FTMessageBox(tstr, FTMB_NONE, "Robot - Info",500);


	//close this window
	CloseFTWindow(twin);

	return(1);
} //void btn_Load_Robot_Log_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
