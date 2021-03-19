//win_Load_Robot.c
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
#include "freethought.h"
#include "Robot.h"
#include "win_Load_Robot.h"
#include "Free3D.h" //for GetFileNameFromPath
#if Linux
#include <strings.h>
#endif


extern ProgramStatus PStatus; //for info messages
extern RobotStatus RStatus; //for robot settings

int winLoadRobot_AddFTWindow(void)
{
FTWindow twin,*twin2;
int newx,newy;

twin2=GetFTWindow("winLoadRobot");
if (twin2!=0) {
	newx=twin2->x+300;
	newy=twin2->y+20;
} else {
	newx=300;
	newy=100;
}

memset(&twin,0,sizeof(FTWindow));
strcpy(twin.name,"winLoadRobot");
strcpy(twin.title,"Load Robot");
strcpy(twin.ititle,"Load Robot");
twin.x=newx;
twin.y=newy;
twin.w=500;
twin.h=370;
twin.OnOpen=(FTControlfuncw *)winLoadRobot_OnOpen;
twin.AddFTControls=(FTControlfuncw *)winLoadRobot_AddFTControls;


//change dir to Robots folder
chdir(RStatus.RobotsFolder);

//fprintf(stderr,"create window main_CreateFTWindow\n");
CreateFTWindow(&twin);
twin2=GetFTWindow(twin.name);
DrawFTWindow(twin2);

//fprintf(stderr,"end main_CreateFTWindow\n");
return 1;
} //end winLoadRobot_AddFTWindow


int winLoadRobot_AddFTControls(FTWindow *twin)
{
FTControl *tcontrol,*tcontrol2;
//FTItem titem;
int fh,fw;


if (PStatus.flags&PInfo) {
	if (!twin) {
	  fprintf(stderr,"Adding controls for window '%s'\n",twin->name);
	} else {
	  fprintf(stderr,"Error: twin==0 in winLoadRobot_AddFTControls()\n");
		return(0);
	}
}

fw=twin->fontwidth;
fh=twin->fontheight;

tcontrol=(FTControl *)malloc(sizeof(FTControl));

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadRobotPath");
tcontrol->type=CTTextBox;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->x2=fw*60;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowX2;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblLoadRobotName");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*10;
tcontrol->flags=CGrowY1|CGrowY2;
strcpy(tcontrol->text,"filename:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadRobotName");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*11;
tcontrol->y1=fh*21;
tcontrol->x2=fw*60;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowY1|CGrowX2|CGrowY2;
tcontrol->flags=CGrowX2|CGrowY1|CGrowY2;
AddFTControl(twin,tcontrol);




memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"foLoadRobot");
tcontrol->type=CTFileOpen;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
tcontrol->x2=fw*60;
tcontrol->y2=fh*20;
tcontrol->flags=CGrowX2|CGrowY2|CVScroll;//|CHScroll;
tcontrol2=GetFTControl("txtLoadRobotPath");
tcontrol->TextBoxPath=tcontrol2; //so the textbox control with the path will automatically get updated when the file open control path changes
tcontrol2=GetFTControl("txtLoadRobotName");
tcontrol->TextBoxFile=tcontrol2; //so this control will automatically get updated with the currently selected filename in the file open control
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadRobot_OK");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"OK");
tcontrol->x1=fw*5;
tcontrol->y1=fh*23;
tcontrol->x2=fw*13;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_LoadRobot_OK_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadRobot_Cancel");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"Cancel");
tcontrol->x1=fw*14;
tcontrol->y1=fh*23;
tcontrol->x2=fw*22;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_LoadRobot_Cancel_Click;
AddFTControl(twin,tcontrol);



if (tcontrol!=0) {
	free(tcontrol);
}

if (PStatus.flags&PInfo) {
  fprintf(stderr,"Done adding FTControls for window %s\n",twin->name);
}

//#endif
return(1);
} //end winLoadRobot_AddFTControls


int winLoadRobot_OnOpen(FTWindow *twin)
{
	FTControl *tc;

	//update the list of loaded scripts
//	tc = GetFTControl("ddScripts");
//	UpdateScriptList(tc);

	//set the window focus to file open control for mousewheel to work scrolling
	tc = GetFTControl("foLoadRobot");
	if (tc != 0) {
		FT_SetFocus(twin, tc);
		return(1);
	}
	return(0);
}


void btn_LoadRobot_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
CloseFTWindow(twin);
}

unsigned int btn_LoadRobot_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
	char *tstr,tstr2[FTMedStr],tmsg[FTMedStr];
	FTControl *tcontrol,*tcontrol2;//,*tc;
	int FoundError;


	//Load script(s) from file

	//get path from control
	tcontrol=GetFTControl("foLoadRobot");
	tcontrol2=GetFTControl("txtLoadRobotName");


	//Unload any currently loaded robot
	if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {
		//fprintf(stderr,"Unloading Robot %s\n",RStatus.CurrentRobotName);
		UnloadRobot();
	}
	GetFileNameWithoutExtension(tcontrol2->text,tstr2);
	if (LoadRobot(tcontrol->FileOpenPath,tstr2)) {
		fprintf(stderr,"Loaded Robot %s\n",RStatus.CurrentRobotName);
		sprintf(tmsg,"Robot %s loaded",tstr2);
		FTMessageBox(tmsg,FTMB_NONE,"Robot - Info",500);

	} else {  //LoadRobot
		fprintf(stderr,"Failed to load Robot %s\n",tstr2);
		FoundError=1;
		sprintf(tmsg,"Error loading Robot: %s",tstr2);
		FTMessageBox(tmsg,FTMB_NONE,"Robot - Info",500);	
	}//LoadRobot


	if (!FoundError) {
		CloseFTWindow(twin);
		return(1);
	} else {
		CloseFTWindow(twin);
		return(0);
	}

} //void btn_LoadRobot_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)


