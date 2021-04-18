//win_RobotModel.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if Linux
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#endif

#include "freethought.h"
#include "Robot.h"
#include "Free3D.h"
#include "win_RobotModel.h"
#include "win_Robot.h"
#include "win_Load_Robot_Log.h"
#include "robot_motor_mcu_instructions.h"

#include <errno.h>

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern F3D_Status F3DStatus; //Free3D global variables (icamera,imodel,ilight, etc.

RobotModelInfo RMInfo; //Robot Model Info 

#define ROBOTMODEL_RENDER_WIDTH 240
#define ROBOTMODEL_RENDER_HEIGHT 320

#if 0 
#define NUMACCELNAME 9
#define NUM_TOUCH_SENSORS 4
const char *AccelName[NUMACCELNAME] = { "Head","Torso","Hip","Left Upper Leg","Left Lower Leg","Left Foot","Right Upper Leg","Right Lower Leg","Right Foot" };
//w/o torso motor:
const int AccelYSpacing[NUMACCELNAME] = {  0,5,9,15,19,24,15,19,24 };

const int MotorNameMap[NUM_MOTOR_NAMES] = { MOTOR_NAME_HEAD,MOTOR_NAME_NECK,MOTOR_NAME_TORSO,MOTOR_NAME_LEFT_HIP,MOTOR_NAME_LEFT_SIDE,MOTOR_NAME_LEFT_LEG,MOTOR_NAME_LEFT_KNEE,
MOTOR_NAME_LEFT_ANKLE,MOTOR_NAME_LEFT_FOOT,MOTOR_NAME_RIGHT_HIP,MOTOR_NAME_RIGHT_SIDE,MOTOR_NAME_RIGHT_LEG,MOTOR_NAME_RIGHT_KNEE,
MOTOR_NAME_RIGHT_ANKLE,MOTOR_NAME_RIGHT_FOOT};

//with torso motor:
const int MotorYSpacing[NUM_MOTOR_NAMES] = {  3,4,8,12,13,14,18,22,23,12,13,14,18,22,23 }; 

//with no torso motor:
const int TouchYSpacing[NUM_TOUCH_SENSORS] = {  26,26,26,26}; 
#endif

int winRobotModel_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winRobotModel");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winRobotModel");
	strcpy(twin.title,"Robot Model");
	strcpy(twin.ititle,"Robot Model");
	if (PStatus.rw < 920 + 640) {
		twin.x = PStatus.rw - 640;
		if (twin.x < 0) {
			twin.x = 0;
		}
	}
	else {
		twin.x = 920;
	}
	twin.y=500;
	twin.w = ROBOTMODEL_RENDER_WIDTH*4+30; //*3
	twin.h = ROBOTMODEL_RENDER_HEIGHT+180;
	twin.Keypress=(FTControlfuncwk *)win_RobotModel_Keypress;
	twin.AddFTControls=(FTControlfuncw *)winRobotModel_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winRobotModel_OnOpen;
	twin.OnClose=(FTControlfuncw *)winRobotModel_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winRobotModel_AddFTWindow



int winRobotModel_AddFTControls(FTWindow *twin)
{
int fw,fh;
FTControl *tcontrol;
char *tstr[FTMedStr],*Name,*EthAccelsName,AnalogSensorName[1024];
int i,j,k,Row,Column,ColumnWidth,NumAnalogSensors,tlen,CurIndex,AnalogSensorNameNum,AnalogSensorNum;

#define DEFAULT_COLUMN_WIDTH 54

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winRobotModel'\n");

//get font from window

fw=twin->fontwidth;
fh=twin->fontheight;

tcontrol=(FTControl *)malloc(sizeof(FTControl));

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"imgModel");
tcontrol->type=CTImage;
tcontrol->x1=0;
tcontrol->y1=0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH;// 240;//256;//240;  
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT; // 320;


#if Linux
sprintf(tstr,"%s/images/black_240x320.bmp",PStatus.ProjectPath);
LoadImageFileToFTControl(tstr,tcontrol,FT_SCALE_IMAGE,0);
//LoadImageFileToFTControl("images/black_256x320.bmp",tcontrol,FT_SCALE_IMAGE,0);
#endif
#if WIN32
sprintf(tstr,"%s\\images\\black_240x320.bmp",PStatus.ProjectPath);
LoadImageFileToFTControl(tstr,tcontrol,FT_SCALE_IMAGE,0);
#endif
//tcontrol->flags|=CGrowX2|CGrowY2;
AddFTControl(twin,tcontrol);

//new line
//Time elapsed label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_TimeElapsed");
tcontrol->type = CTLabel;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 15;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "Time: 0 ms");
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Time elapsed in Stage label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_StageTimeElapsed");
tcontrol->type = CTLabel;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*16;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 32;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "Stage: 0 ms");
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);


//Play speed label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_PlayDelay");
tcontrol->type = CTLabel;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw *33;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 43;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "PlayDelay:");
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Play speed textbox
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtRobotModel_PlayDelay");
tcontrol->type = CTTextBox;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*44;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*53;
tcontrol->y2 = fh+fh/2;
//strcpy(tcontrol->text, "1.0");
//sprintf(tcontrol->text,"%03.1f",RMInfo.PlayDelay);
//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_LoadLogFile_Click;
tcontrol->OnChange=(FTControlfuncwc *)txtRobotModel_PlayDelay_OnChange;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Play speed label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_PlayDelayComment");
tcontrol->type = CTLabel;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw * 54;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 57;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "ms");
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);



//Render in real-time checkbox
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkRobotModel_RealTimeRender");
tcontrol->type = CTCheckBox;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw *57;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw *78;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "Render in real-time");
tcontrol->OnChange = (FTControlfuncwc *)chkRobotModel_RealTimeRender_OnChange;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);


//Stage label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_Go_To_Stage");
tcontrol->type = CTButton;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw * 81;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 87;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "Go to");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_Go_To_Stage_Click;
//tcontrol->textcolor[0]=0xffffff;  
//tcontrol->color[0]=0x005f00;  //green
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_Stage");
tcontrol->type = CTLabel;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw * 88;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 94;
tcontrol->y2 = fh+fh/2;
strcpy(tcontrol->text, "Stage:");
tcontrol->textcolor[0]=0xffffff;  
tcontrol->color[0]=0x005f00;  //green
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtRobotModel_Stage");
tcontrol->type = CTTextBox;
tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw * 94;
tcontrol->y1 = 0;
tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw * 105;
tcontrol->y2 = fh+fh/2;
//strcpy(tcontrol->text, "");
//tcontrol->textcolor[0]=0xffffff;  
//tcontrol->color[0]=0x005f00;  //green
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Go through each line in the Model.conf file, and add accel, motor, or touch control(s) in correct row and column
for(i=0;i<RStatus.ModelConf.NumEntries;i++) {
	//for each entry
	//determine if Accel, Motor, or Analog Sensor
	Name=RStatus.ModelConf.Entry[i].Name;
	Row=RStatus.ModelConf.Entry[i].Row;
	Column=RStatus.ModelConf.Entry[i].Column;
	ColumnWidth=RStatus.ModelConf.ColumnWidth;
	if (!strncmp(Name,"ACCEL",5)) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sAngle",Name);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + Column*ColumnWidth);
		tcontrol->y1 = fh*(2+Row);
		tcontrol->x2 = tcontrol->x1+ fw*(strlen(Name)+5);
		tcontrol->y2 = fh*(3+Row);
		sprintf(tcontrol->text, "%s Ang",GetAccelReadableName(Name));
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

		for(j=0;j<3;j++) {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sTargetAngle%i",Name,j);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(strlen(Name)+6 + j*5 + Column*ColumnWidth);
			tcontrol->y1 = fh*(2+Row);
			tcontrol->x2 = tcontrol->x1 + fw*5;
			tcontrol->y2 = fh*(3+Row);
			tcontrol->textcolor[0]=0x005f00;  //green
			//tcontrol->textcolor[0]=0xffffff;  //white	
			sprintf(tcontrol->text, "  *");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);
		} //for j

		//newline
		//(ex: Head) Accel A:
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sAngleA",Name);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);
			tcontrol->x2 = tcontrol->x1+fw*2;
			tcontrol->y2 = fh*(4+Row);
			strcpy(tcontrol->text, "A:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);

		//(ex: Head) Accel x,y,z
		for(j=0;j<3;j++) {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sAngle%d",Name,j);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((3+6*j) + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);//+fh/2;
			tcontrol->x2 = tcontrol->x1+fw*6;
			tcontrol->y2 = fh*(4+Row);//+fh/2;
			tcontrol->textcolor[0]=0x0000ff;
			strcpy(tcontrol->text, "0");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);
		} //for j

		//(ex: Head) Accel dA:
			memset(tcontrol, 0, sizeof(FTControl));
	//		sprintf(tcontrol->name, "lblRobotModel_%sAcceldA",AccelName[i]);
			sprintf(tcontrol->name, "lblRobotModel_%sAngledA",Name);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);
			tcontrol->x2 = tcontrol->x1 + fw*3;
			tcontrol->y2 = fh*(4+Row);
			strcpy(tcontrol->text, "dA:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);


		//(ex: Head) Accel dx,dy,dz
		for(j=0;j<3;j++) {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sAngled%d",Name,j);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((29 +j*6) + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);
			tcontrol->x2 = tcontrol->x1+fw*6;
			tcontrol->y2 = fh*(4+Row);//+fh/2;
			tcontrol->textcolor[0]=0x00afff;
			strcpy(tcontrol->text, "0");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);
		} //for j

		//newline
		//(ex: Head) Gyro G:
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sGyroG",Name);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + Column*ColumnWidth);
			tcontrol->y1 = fh*(4+Row);
			tcontrol->x2 = tcontrol->x1+fw*2;
			tcontrol->y2 = fh*(5+Row);
			strcpy(tcontrol->text, "G:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);

		//(ex: Head) Gyro x,y,z
		for(j=0;j<3;j++) {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sGyro%d",Name,j);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((3 +j*6) + Column*ColumnWidth);
			tcontrol->y1 = fh*(4+Row);
			tcontrol->x2 = tcontrol->x1+fw*5;
			tcontrol->y2 = fh*(5+Row);
			tcontrol->textcolor[0]=0xff0000;
			strcpy(tcontrol->text, "0");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);
		} //for j

		//(ex: Head) Gyro dG:
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sGyrodG",Name);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + Column*ColumnWidth);
			tcontrol->y1 = fh*(4+Row);
			tcontrol->x2 = tcontrol->x1+fw*4;
			tcontrol->y2 = fh*(5+Row);
			strcpy(tcontrol->text, "dG:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);

		//(ex: Head) Gyro dx,dy,dz
		for(j=0;j<3;j++) {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sGyrod%d",Name,j);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((29 +j*6) + Column*ColumnWidth);
			tcontrol->y1 = fh*(4+Row);
			tcontrol->x2 = tcontrol->x1+fw*5;
			tcontrol->y2 = fh*(5+Row);//fh/2;
			tcontrol->textcolor[0]=0xff7f00;
			strcpy(tcontrol->text, "0");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);
		} //for j

	} //if (!strncmp(RStatus.ModelConf.Entry[i].Name,"ACCEL",5)) {
	if (!strncmp(RStatus.ModelConf.Entry[i].Name,"MOTOR",5)) {
		//newline
		//Motor name
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%s",Name);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + Column*ColumnWidth); 
		tcontrol->y1 = fh*(2+Row);
		tcontrol->x2 = tcontrol->x1+fw*16;
		tcontrol->y2 = fh*(3+Row);
		sprintf(tcontrol->text, "%s",GetMotorReadableName(Name));
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

		//Motor Thrust
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sThrust",Name);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(18 + Column*ColumnWidth);
		tcontrol->y1 = fh*(2+Row);
		tcontrol->x2 = tcontrol->x1+fw*3;
		tcontrol->y2 = fh*(3+Row);
		tcontrol->textcolor[0]=0x3f3f3f;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

		//Accel Motor is currently using
		memset(tcontrol, 0, sizeof(FTControl)); 
		sprintf(tcontrol->name, "lblRobotModel_%sUseAccel",Name); //use readable name or motor name?
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(22 + Column*ColumnWidth);
		tcontrol->y1 = fh*(2+Row);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(ColumnWidth-1 + Column*ColumnWidth);
		tcontrol->y2 = fh*(3+Row);//+fh/2;
		tcontrol->textcolor[0]=0x3f3f3f;
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

	} //if (!strncmp(RStatus.ModelConf.Entry[i].Name,"MOTOR",5)) {
	if (!strncmp(RStatus.ModelConf.Entry[i].Name,"TOUCH",5)) {
		//Add Analog Sensors
		Name=RStatus.ModelConf.Entry[i].Name;
		//multiple touch sensors can be listed on one line (ex: TOUCH_SENSOR_FOOT_LEFT_FRONT|TOUCH_SENSOR_FOOT_LEFT_BACK)
		//so determine how many touch sensors are listed
		NumAnalogSensors=1;
		k=0;
		while(k<strlen(Name)) {
			if (Name[k]==124) {  //124=|
				NumAnalogSensors++;
			}
			k++;
		}	//while(k<strlen(Name))		

		//Need name of EthAccels PCB this touch sensor is on
		//Note that Name could be "TOUCH_SENSOR_FOOT_LEFT_FRONT|TOUCH_SENSOR_FOOT_LEFT_BACK"
		//but all Analog Sensors in a log entry are on the same EthAccels PCB so only need name of 1
		
		//Get EthAccels PCB Name
		tlen=GetWordLen(Name,124); //124=|
		if (tlen>1024) {
			tlen=1024;
		}
		strncpy(AnalogSensorName,Name,tlen);
		AnalogSensorName[tlen]=0;
		EthAccelsName=GetAnalogSensorPCBName(AnalogSensorName);
			

		if (EthAccelsName==0) {
			fprintf(stderr,"win_RobotModelc: GetAnalogSensorPCBName returned 0, Could not find PCB for touch sensor named %s\n",AnalogSensorName);
		} else {
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sTouchT",EthAccelsName);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);
			tcontrol->x2 = tcontrol->x1+fw*2;
			tcontrol->y2 = fh*(4+Row);
			strcpy(tcontrol->text, "T:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);

			//Touch sensor 0-3
			CurIndex=0;
			for(j=0;j<NumAnalogSensors;j++) {
				//Get AnalogSensor name
				tlen=GetWordLen(&Name[CurIndex],124);
				if (tlen>1024) {
					tlen=1024;
				}
				strncpy(AnalogSensorName,&Name[CurIndex],tlen);
				AnalogSensorName[tlen]=0;
				CurIndex=tlen+1; //go to next AnalogSensor
				//get AnalogSensor NameNum
				AnalogSensorNameNum=GetAnalogSensorNameNum(AnalogSensorName);
				if (AnalogSensorNameNum<0) {
					fprintf(stderr,"GetAnalogSensorNameNum returned -1 for Analog Sensor Name %s\n",AnalogSensorName);
				} else {
					//Get actual Analog Sensor number, for that need touch sensor name
					AnalogSensorNum=RStatus.AnalogSensorInfo[AnalogSensorNameNum].Num;
					memset(tcontrol, 0, sizeof(FTControl));
					//important to use the AnalogSensor number, not j
					sprintf(tcontrol->name, "lblRobotModel_%sTouch%d",EthAccelsName,AnalogSensorNum);
					tcontrol->type = CTLabel;
					tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(4 + j*5 + Column*ColumnWidth);
					tcontrol->y1 = fh*(3+Row);
					tcontrol->x2 = tcontrol->x1+fw*5;
					tcontrol->y2 = fh*(4+Row);
					tcontrol->textcolor[0]=0x9f009f;
					strcpy(tcontrol->text, "0");
					//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
					AddFTControl(twin, tcontrol);
				} //if (AnalogSensorNameNum<0) {
			} //for j


			//Analog Sensor dT change in touch:
			memset(tcontrol, 0, sizeof(FTControl));
			sprintf(tcontrol->name, "lblRobotModel_%sTouchdT",EthAccelsName);
			tcontrol->type = CTLabel;
			tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + Column*ColumnWidth);
			tcontrol->y1 = fh*(3+Row);
			tcontrol->x2 = tcontrol->x1+fw*4;
			tcontrol->y2 = fh*(4+Row);
			strcpy(tcontrol->text, "dT:");
			//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
			AddFTControl(twin, tcontrol);


			//Analog Sensor change in touch
			CurIndex=0;
			for(j=0;j<NumAnalogSensors;j++) {
				//Get AnalogSensor name
				tlen=GetWordLen(&Name[CurIndex],124); //124=|
				if (tlen>1024) {
					tlen=1024;
				}
				strncpy(AnalogSensorName,&Name[CurIndex],tlen);
				AnalogSensorName[tlen]=0;
				CurIndex=tlen+1; //go to next AnalogSensor
				//get AnalogSensor NameNum
				AnalogSensorNameNum=GetAnalogSensorNameNum(AnalogSensorName);
				if (AnalogSensorNameNum<0) {
					fprintf(stderr,"GetAnalogSensorNameNum returned -1 for Analog Sensor Name %s\n",AnalogSensorName);
				} else {
					AnalogSensorNum=RStatus.AnalogSensorInfo[AnalogSensorNameNum].Num;
					memset(tcontrol, 0, sizeof(FTControl));
					sprintf(tcontrol->name, "lblRobotModel_%sTouchd%d",EthAccelsName,AnalogSensorNum);
					tcontrol->type = CTLabel;
					tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(29 + j*5 + Column*ColumnWidth);
					tcontrol->y1 = fh*(3+Row);
					tcontrol->x2 = tcontrol->x1+fw*5;
					tcontrol->y2 = fh*(4+Row);
					tcontrol->textcolor[0]=0xcf00cf;
					strcpy(tcontrol->text, "0");
					//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
					AddFTControl(twin, tcontrol);
				} //if (AnalogSensorNameNum<0) {
			} //for j
		} //if (EthAccelsName==0) {
	} //if (!strncmp(RStatus.ModelConf.Entry[i].Name,"TOUCH",5)) {

} //for(i=0;i<RStatus.ModelConf.NumEntries;i++) {


#if 0 
//newline
//Add accels
#define LAST_SINGLE_COLUMN_ROW 5 //single column rows until legs, then double columns
//(ex: Head) Accel
for(i=0;i<NUMACCELNAME;i++) {
	memset(tcontrol, 0, sizeof(FTControl));
	//sprintf(tcontrol->name, "lblRobotModel_%sAccel",AccelName[i]);
	sprintf(tcontrol->name, "lblRobotModel_%sAngle",AccelName[i]);
	tcontrol->type = CTLabel;
	tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y1 = fh*(2+AccelYSpacing[i]);
	tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(strlen(AccelName[i])+6 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y2 = fh*(3+AccelYSpacing[i]);//+fh/2;
	//sprintf(tcontrol->text, "%s Accel",AccelName[i]);
	sprintf(tcontrol->text, "%s Ang",AccelName[i]);
	//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
	AddFTControl(twin, tcontrol);

	for(j=0;j<3;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sTargetAngle%i",AccelName[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(strlen(AccelName[i])+6 + j*5 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(2+AccelYSpacing[i]);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(strlen(AccelName[i])+11 + j*5 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(3+AccelYSpacing[i]);
		tcontrol->textcolor[0]=0x005f00;  //green
		//tcontrol->textcolor[0]=0xffffff;  //white	
		sprintf(tcontrol->text, "  *");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j


	//newline
	//(ex: Head) Accel A:
		memset(tcontrol, 0, sizeof(FTControl));
//		sprintf(tcontrol->name, "lblRobotModel_%sAccelA",AccelName[i]);
		sprintf(tcontrol->name, "lblRobotModel_%sAngleA",AccelName[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+AccelYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(3 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+AccelYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "A:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

	//(ex: Head) Accel x,y,z
	for(j=0;j<3;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
//		sprintf(tcontrol->name, "lblRobotModel_%sAccel%d",AccelName[i],j);
		sprintf(tcontrol->name, "lblRobotModel_%sAngle%d",AccelName[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((3+6*j) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+AccelYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((9+6*j) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+AccelYSpacing[i]);//+fh/2;
		tcontrol->textcolor[0]=0x0000ff;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j

	//(ex: Head) Accel dA:
		memset(tcontrol, 0, sizeof(FTControl));
//		sprintf(tcontrol->name, "lblRobotModel_%sAcceldA",AccelName[i]);
		sprintf(tcontrol->name, "lblRobotModel_%sAngledA",AccelName[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+AccelYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(27 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+AccelYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "dA:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);


	//(ex: Head) Accel dx,dy,dz
	for(j=0;j<3;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
//		sprintf(tcontrol->name, "lblRobotModel_%sAcceld%d",AccelName[i],j);
		sprintf(tcontrol->name, "lblRobotModel_%sAngled%d",AccelName[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((29 +j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+AccelYSpacing[i]);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((35+ j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+AccelYSpacing[i]);//+fh/2;
		tcontrol->textcolor[0]=0x00afff;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j

	//newline
	//(ex: Head) Gyro G:
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sGyroG",AccelName[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(4+AccelYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(3 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(5+AccelYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "G:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

	//(ex: Head) Gyro x,y,z
	for(j=0;j<3;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sGyro%d",AccelName[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((3 +j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(4+AccelYSpacing[i]);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((8 +j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(5+AccelYSpacing[i]);//+fh/2;
		tcontrol->textcolor[0]=0xff0000;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j

	//(ex: Head) Gyro dG:
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sGyrodG",AccelName[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(4+AccelYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(28 + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(5+AccelYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "dG:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

	//(ex: Head) Gyro dx,dy,dz
	for(j=0;j<3;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sGyrod%d",AccelName[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((29 +j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(4+AccelYSpacing[i]);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((34 +j*6) + (i>LAST_SINGLE_COLUMN_ROW)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(5+AccelYSpacing[i]);//fh/2;
		tcontrol->textcolor[0]=0xff7f00;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j
} //for i
#endif 

#if 0 
//Add Motors
for(i=0;i<NUM_MOTOR_NAMES;i++) {

	//newline
	//Motor name
	memset(tcontrol, 0, sizeof(FTControl));
	sprintf(tcontrol->name, "lblRobotModel_%s",ROBOT_MOTOR_READABLE_NAMES[MotorNameMap[i]]);
	tcontrol->type = CTLabel;
	tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + (i>8)*DEFAULT_COLUMN_WIDTH); //with Torso Motor, right side motors are i>8 //i>7
	tcontrol->y1 = fh*(2+MotorYSpacing[i]);
	tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(17 + (i>8)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y2 = fh*(3+MotorYSpacing[i]);//+fh/2;
	sprintf(tcontrol->text, "%s",ROBOT_MOTOR_READABLE_NAMES[MotorNameMap[i]]);
	//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
	AddFTControl(twin, tcontrol);

	//Motor Thrust
	memset(tcontrol, 0, sizeof(FTControl));
	sprintf(tcontrol->name, "lblRobotModel_%sThrust",ROBOT_MOTOR_READABLE_NAMES[MotorNameMap[i]]);
	tcontrol->type = CTLabel;
	tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(18 + (i>8)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y1 = fh*(2+MotorYSpacing[i]);
	tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(21 + (i>8)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y2 = fh*(3+MotorYSpacing[i]);//+fh/2;
	tcontrol->textcolor[0]=0x3f3f3f;
	strcpy(tcontrol->text, "0");
	//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
	AddFTControl(twin, tcontrol);

	//Accel Motor is currently using
	memset(tcontrol, 0, sizeof(FTControl));
	sprintf(tcontrol->name, "lblRobotModel_%sUseAccel",ROBOT_MOTOR_READABLE_NAMES[MotorNameMap[i]]);
	tcontrol->type = CTLabel;
	tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(22 + (i>8)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y1 = fh*(2+MotorYSpacing[i]);
	tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(DEFAULT_COLUMN_WIDTH-1 + (i>8)*DEFAULT_COLUMN_WIDTH);
	tcontrol->y2 = fh*(3+MotorYSpacing[i]);//+fh/2;
	tcontrol->textcolor[0]=0x3f3f3f;
	//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
	AddFTControl(twin, tcontrol);

} //for i
#endif

/*
//log file horizontal scroll bar
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_LogFileScroll");
tcontrol->type = CTHScroll;
tcontrol->x1 = 0;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT;
tcontrol->x2 = fw * 9;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * 3;
strcpy(tcontrol->text, "|<");
//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_RewindLogFile_Click;
tcontrol->flags |= CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);
*/

#if 0 
//Add Analog Sensors
//Analog Sensors  (currently only EthAccel in left and right legs)
for(i=0;i<2;i++) {

		memset(tcontrol, 0, sizeof(FTControl));
//		sprintf(tcontrol->name, "lblRobotModel_%sAccelA",AccelName[i]);
		sprintf(tcontrol->name, "lblRobotModel_%sTouchT",ROBOT_PCB_ETHACCEL_NAMES[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(1 + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+TouchYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(3 + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+TouchYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "T:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);

	//Touch sensor 0-3
	for(j=0;j<NUM_TOUCH_SENSORS;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sTouch%d",ROBOT_PCB_ETHACCEL_NAMES[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((4+5*j) + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+TouchYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((9+5*j) + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+TouchYSpacing[i]);//+fh/2;
		tcontrol->textcolor[0]=0x9f009f;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j


	//Analog Sensor dT change in touch:
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sTouchdT",ROBOT_PCB_ETHACCEL_NAMES[i]);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*(24 + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+TouchYSpacing[i]);//+fh/2;
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*(28+ (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+TouchYSpacing[i]);//+fh/2;
		strcpy(tcontrol->text, "dT:");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);


	//Analog Sensor change in touch
	for(j=0;j<NUM_TOUCH_SENSORS;j++) {
		memset(tcontrol, 0, sizeof(FTControl));
		sprintf(tcontrol->name, "lblRobotModel_%sTouchd%d",ROBOT_PCB_ETHACCEL_NAMES[i],j);
		tcontrol->type = CTLabel;
		tcontrol->x1 = ROBOTMODEL_RENDER_WIDTH + fw*((29 +j*5) + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y1 = fh*(3+TouchYSpacing[i]);
		tcontrol->x2 = ROBOTMODEL_RENDER_WIDTH + fw*((34+ j*5) + (i>0)*DEFAULT_COLUMN_WIDTH);
		tcontrol->y2 = fh*(4+TouchYSpacing[i]);//+fh/2;
		tcontrol->textcolor[0]=0xcf00cf;
		strcpy(tcontrol->text, "0");
		//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
		AddFTControl(twin, tcontrol);
	} //for j


} //for i

#endif


//Log Entry label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblRobotModel_LogEntry");
tcontrol->type = CTLabel;
tcontrol->x1 = fw;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*7.5;//3;//*2;
tcontrol->x2 = fw * 130;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh*9;//6;//*5;
strcpy(tcontrol->text, "Log Entry");
//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_RewindLogFile_Click;
//tcontrol->flags |=  CGrowY1 | CGrowX2 | CGrowY2;
tcontrol->flags |=  CGrowX2;
AddFTControl(twin, tcontrol);

/*
//Log Entry label
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblTimeStamp");
tcontrol->type = CTLabel;
tcontrol->x1 = 0;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT + fh*3;
tcontrol->x2 = fw * 90;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh *4;
strcpy(tcontrol->text, "Time:");
//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_RewindLogFile_Click;
tcontrol->flags |= CGrowX2;
AddFTControl(twin, tcontrol);
*/

//newline
//Load log file button
#define MODEL_BUTTONS_Y 9//6
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_LoadLogFile");
tcontrol->type = CTButton;
tcontrol->x1 = 5;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT +fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw*14;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh*(MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "Load Log");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_LoadLogFile_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1| CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);


//Rewind log file button
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_RewindLogFile");
tcontrol->type = CTButton;
tcontrol->x1 = fw*15;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT +fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw*24;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh*(MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "|<");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_RewindLogFile_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1| CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Play log file button
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_PlayLogFile");
tcontrol->type = CTButton;
tcontrol->x1 = fw*25;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw*34;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, ">");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_PlayLogFile_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1| CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Step forward one log entry at a time
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_StepLogFileForward");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 35;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw * 44;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "STEP FRWD");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_StepLogFileForward_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Skip forward to first non EthAccel event
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_SkipLogFileForward");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 45;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw * 54;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "SKIP FWD");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_SkipLogFileForward_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//Step backward one log entry at a time
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_StepLogFileBackward");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 55;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw * 64;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "STEP BKWD");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_StepLogFileBackward_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

/*
//Step one log entry at a time
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_StepTime");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 30;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*5;
tcontrol->x2 = fw * 39;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * 8;
strcpy(tcontrol->text, "STEP TIME");
//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_StepLogFileForward_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);
*/

//Stop log file button
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_StopLogFile");
tcontrol->type = CTButton;
tcontrol->x1 = fw*65;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT+fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw*74;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "STOP");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_StopLogFile_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1| CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);


//Close Robot Model window
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnRobotModel_Close");
tcontrol->type = CTButton;
tcontrol->x1 = fw*75;
tcontrol->y1 = ROBOTMODEL_RENDER_HEIGHT +fh*MODEL_BUTTONS_Y;
tcontrol->x2 = fw*84;
tcontrol->y2 = ROBOTMODEL_RENDER_HEIGHT + fh * (MODEL_BUTTONS_Y+3);
strcpy(tcontrol->text, "Close");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnRobotModel_Close_Click;
//tcontrol->flags |= CGrowX1 | CGrowY1| CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

if (tcontrol!=0) {
	free(tcontrol);
}


if (PStatus.flags&PInfo) {
  fprintf(stderr,"done with adding controls to winRobotModel.\n");
}

//#endif
return 1;
} //end winRobotModel_AddFTControls


void win_RobotModel_Keypress(FTWindow *twin,KeySym key)
{
	AXIS3D trot;
	CAMERA *lcamera;
	//POINT3D temp;
	MODEL *lmodel;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"win_RobotModel_Keypress\n");
	}

		//make sure no control has the focus
		if (twin->FocusFTControl==0 || twin->FocusFTControl->type==CTLabel) { 

		  switch (key)	{
			case XK_Shift_L://-30:
			case XK_Shift_R: //-31:  //ignore shift keys
		      break;
			case XK_Escape:	/*esc*/
	//	      CloseFTWindow(twin);
		      break;
			case XK_space: //space bar
				//stop playing log file
				btnRobotModel_StopLogFile_Click(twin,0,0,0);
			break;
			case XK_Left: //0xff51:  //left arrow key
	#if Linux
			case XK_KP_Left: //0xff96:  //key pad left arrow (no num lock)
	#endif
			case XK_KP_4:  //in Windows the VK_NUMPAD4 is the same no matter if the NumLock is on or not so check the PStatus.flags&PNumLock if needed
	#if Linux
		//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera left
					F3DStatus.curcamera->location.x-=F3DStatus.curcamera->trans_change;
				} else {
					F3DStatus.imodel->location.x-=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;
			case XK_Right: //0xff52:  //right arrow key
	#if Linux
			case XK_KP_Right: //0xff98:  //key pad right arrow (no num lock)
	#endif
			case XK_KP_6:
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera right
					F3DStatus.curcamera->location.x+=F3DStatus.curcamera->trans_change;			
				} else {
					F3DStatus.imodel->location.x+=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image
			break;
			case XK_Down: //0xff54:  //down arrow key
	#if Linux
			case XK_KP_Down: //0xff99:  //key pad down arrow (no num lock)
	#endif
			case XK_KP_2:
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera down
					F3DStatus.curcamera->location.y-=F3DStatus.curcamera->trans_change;			
				} else {
					F3DStatus.imodel->location.y-=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image
			break;
			case XK_Up: //up arrow key
	#if Linux
			case XK_KP_Up: //0xff99:  //key pad down arrow (no num lock)
	#endif
			case XK_KP_8:
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera up
					F3DStatus.curcamera->location.y+=F3DStatus.curcamera->trans_change;			
				} else {
					F3DStatus.imodel->location.y+=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image
			break;
			case XK_KP_5: //0xff99:  //key pad middle key (num lock)
	#if Linux
			case XK_KP_Begin: //0xff9d:  //key pad middle key (no num lock)
	#endif
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera forward
					F3DStatus.curcamera->location.z+=F3DStatus.curcamera->trans_change;			
				} else {
					F3DStatus.imodel->location.z+=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image
			break;
			case XK_KP_0: //0xff99:  //key pad 0 key (num lock)
	#if Linux
			case XK_KP_Insert: //0xff9d:  //key pad insert key (no num lock)
	#endif
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					//move camera backward
					F3DStatus.curcamera->location.z-=F3DStatus.curcamera->trans_change;			
				} else {
					F3DStatus.imodel->location.z-=F3DStatus.curcamera->trans_change; //make generic trans_change- not just for camera, or add one for model
					//F3DStatus.imodel->axis[0].loc.x-=F3DStatus.curcamera->trans_change;
				}
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;

			case XK_A://A key 
			case XK_a://a key 

	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux


				//rotate +y (yaw)
				//if (F3DStatus.flags&M3D_USE_CNC_ORIENTATION) { 
				//	MakeRotationMatrix_AroundZAxis(&trot,F3DStatus.curcamera->rot_change);
				//	F3DStatus.curcamera->axisangle.z+=F3DStatus.curcamera->rot_change;
				//} else {


				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					MakeRotationMatrix_AroundYAxis(&trot,F3DStatus.curcamera->rot_change);
					F3DStatus.curcamera->axisangle.y+=F3DStatus.curcamera->rot_change;
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);//RotateCurCamera(trot);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundYAxis(&trot,-F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image
				
			
			break; //A key

			case XK_B://B key 
	#if Linux
			case XK_b://b key - in Win32 case calue '100' already used VK_NUMPAD_5
	#endif
				//step frame backwards
				btnRobotModel_StepLogFileBackward_Click(twin,0,0,0);
			break; //B key
			case XK_C://C key 
	#if Linux
			case XK_c://c key - in Win32 case calue '100' already used VK_NUMPAD_5
	#endif
			//toggles move camera or model with awdx and arrow keys
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					RMInfo.flags&=~ROBOT_MODEL_INFO_CAMERA_SELECTED;
				} else {
					RMInfo.flags|=ROBOT_MODEL_INFO_CAMERA_SELECTED;
				} 
			break; //C key
			case XK_D://D key 
	#if Linux
			case XK_d://d key - in Win32 case calue '100' already used VK_NUMPAD_5
	#endif

	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				//rotate -y (yaw)
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

					//if (F3DStatus.flags&M3D_USE_CNC_ORIENTATION) { 
						//MakeRotationMatrix_AroundZAxis(&trot,-F3DStatus.curcamera->rot_change);
						//F3DStatus.curcamera->axisangle.z-=F3DStatus.curcamera->rot_change;
					//} else {
						MakeRotationMatrix_AroundYAxis(&trot,-F3DStatus.curcamera->rot_change);				
						F3DStatus.curcamera->axisangle.y-=F3DStatus.curcamera->rot_change;
					//}
					//MatrixMultiply3D(&F3DStatus.curcamera->axis,&F3DStatus.curcamera->axis,&trot);
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundYAxis(&trot,F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;
			case XK_F://F key 
	#if Linux
			case XK_f://f key
	#endif
				//step frame forwards
				btnRobotModel_StepLogFileForward_Click(twin,0,0,0);
			break; //S key


			case XK_G://G key 
	#if Linux
			case XK_g://g key
	#endif
				//Go or skip forward to non-EthAccel log entry
				btnRobotModel_SkipLogFileForward_Click(twin,0,0,0);
			break; //G key


			
			case XK_P://P key 
	#if Linux
			case XK_p://p key
	#endif
				//play
				btnRobotModel_PlayLogFile_Click(twin,0,0,0);
			break; //P key


			case XK_R://R key 
#if Linux
			case XK_r://r key
#endif
				//rewind
				RewindLogFile();
			break; //S key


			case XK_S://S key 
#if Linux
			case XK_s://s key - and number - skip to stage
#endif
				//skip to stage
				RMInfo.flags|=ROBOT_MODEL_INFO_GET_STAGE_NUMBER;
			break; //S key





			case XK_W://W key 
			case XK_w://w key 

	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				//rotate +x (pitch)
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
					MakeRotationMatrix_AroundXAxis(&trot,F3DStatus.curcamera->rot_change);
					F3DStatus.curcamera->axisangle.x+=F3DStatus.curcamera->rot_change;
					//MatrixMultiply3D(&F3DStatus.curcamera->axis,&F3DStatus.curcamera->axis,&trot);//RotateCurCamera(trot);
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundXAxis(&trot,-F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {
	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;
			case XK_X://X key 
			case XK_x://x key 
	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				//rotate -x (pitch)
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

					MakeRotationMatrix_AroundXAxis(&trot,-F3DStatus.curcamera->rot_change);
					F3DStatus.curcamera->axisangle.x-=F3DStatus.curcamera->rot_change;
					//MatrixMultiply3D(&F3DStatus.curcamera->axis,&F3DStatus.curcamera->axis,&trot);
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundXAxis(&trot,F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;
			case XK_Q://Q key 
			case XK_q://q key 

	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				//rotate +z (roll)
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

				//if (F3DStatus.flags&M3D_USE_CNC_ORIENTATION) { 
				//	MakeRotationMatrix_AroundYAxis(&trot,F3DStatus.curcamera->rot_change);
				//	F3DStatus.curcamera->axisangle.y+=F3DStatus.curcamera->rot_change;
				//} else {
					MakeRotationMatrix_AroundZAxis(&trot,F3DStatus.curcamera->rot_change);
					F3DStatus.curcamera->axisangle.z+=F3DStatus.curcamera->rot_change;
				//}
				//MatrixMultiply3D(&F3DStatus.curcamera->axis,&F3DStatus.curcamera->axis,&trot);//RotateCurCamera(trot);
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundZAxis(&trot,-F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image

			break;
			case XK_Z://Z key 
			case XK_z://z key 

	#if Linux
				//Lock RenderLock mutex
				pthread_mutex_lock(&F3DStatus.RenderLock);
	#endif //Linux

				//rotate -z (roll)
				if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

					//if (F3DStatus.flags&M3D_USE_CNC_ORIENTATION) { 
					//	MakeRotationMatrix_AroundYAxis(&trot,-F3DStatus.curcamera->rot_change);
					//	F3DStatus.curcamera->axisangle.y-=F3DStatus.curcamera->rot_change;
					//} else {
						MakeRotationMatrix_AroundZAxis(&trot,-F3DStatus.curcamera->rot_change);
						F3DStatus.curcamera->axisangle.z-=F3DStatus.curcamera->rot_change;
					//}
					//MatrixMultiply3D(&F3DStatus.curcamera->axis,&F3DStatus.curcamera->axis,&trot);
					MatrixMultiply3D(&F3DStatus.curcamera->axis,&trot,&F3DStatus.curcamera->axis);
				} else {
					lmodel=F3DStatus.imodel;
					if (lmodel!=0) {
						MakeRotationMatrix_AroundZAxis(&trot,F3DStatus.curcamera->rot_change); //todo make generic rot_change or add model_rot_change
						//lmodel->axis[0].axisangle.y+=F3DStatus.curcamera->rot_change;
						MatrixMultiply3D(&lmodel->axis[0].m,&trot,&lmodel->axis[0].m);//RotateCurCamera(trot);
					} //if (lmodel!=0) {
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_CAMERA_SELECTED) {

	#if Linux
				//Unlock RenderLock mutex
				pthread_mutex_unlock(&F3DStatus.RenderLock);
	#endif //Linux
				RenderModelMainImage();  //rerender the rendered image


			break;

			//		case XK_KP_1: //0xffb1:  //1 key
	//		case XK_KP_2: //0xffb2:  //1 key
	//		case XK_KP_3: //0xffb3:  //1 key
	//		case XK_KP_4: //0xffb4:  //1 key
	//		case XK_KP_5: //0xffb5:  //1 key
			case XK_0: //0x30:  //0 key
			case XK_1: //0x31:  //1 key
			case XK_2: //0x32:  //2 key
			case XK_3: //0x33:  //3 key
			case XK_4: //0x34:  //4 key
			case XK_5: //0x35:  //5 key
			case XK_6: //0x36:  //6 key
			case XK_7: //0x37:  //7 key
			case XK_8: //0x38:  //8 key
			case XK_9: //0x39:  //9 key

	//		case XK_KP_1: //0xffb1:  //1 key


				//change stage
				if (RMInfo.flags&ROBOT_MODEL_INFO_GET_STAGE_NUMBER) {
					RMInfo.flags&=~ROBOT_MODEL_INFO_GET_STAGE_NUMBER;  //clear flag
					RMInfo.flags|=ROBOT_MODEL_INFO_SKIP_PROCESS; //does not render each frame
					SkipToStage(key-0x30);
					RMInfo.flags&=~ROBOT_MODEL_INFO_SKIP_PROCESS; //reset flag

				} else { //if (RMInfo.flags&ROBOT_MODEL_INFO_GET_STAGE_NUMBER) {
					//change cameras
					lcamera=GetCameraByNum(key&0xf);
					if (lcamera!=0 && lcamera!=F3DStatus.curcamera) {
						//changing camera
						F3DStatus.curcamera=lcamera;
						RenderModelMainImage(); 
					} //if (lcamera!=0 && lcamera!=F3DStatus.curcamera) {
				} //else if (RMInfo.flags&ROBOT_MODEL_INFO_GET_STAGE_NUMBER) {
			break;
			case XK_period: //period - move 10 ms (1 sample) forward
				Skip10ms(1);
			break;
			case XK_comma:  //comma - move 10ms (1 sample) backward
				Skip10ms(0);
			break;
			case XK_slash: //forward slash - move to stage start or end
				SkipToStage(-1);
			break;
			default:
						//fprintf(stderr,"key=%x\n",key);
			break;
		    }  //switch (key)	{

	} //if (twin->FocusFTControl==0) {

} //void win_RobotModel_Keypress(FTWindow *twin,KeySym key)

//render the models in the rendered image and redraw it
int RenderModelMainImage(void)
{
	FTControl *tc;

	tc=GetFTControl("imgModel");
	if (tc!=0) {
		RenderSceneFromCamera(tc->image[0],tc->x2-tc->x1,tc->y2-tc->y1,F3DStatus.curcamera,0); 
		DrawFTControl(tc);
	}
	return(1);
} //int RenderModelMainImage(void)

int winRobotModel_OnOpen(FTWindow *twin)
{
	MODEL *lmodel;
	CAMERA *icamera,*lcamera;
	LIGHT *llight;
	char tstr[256],ModelFileName[1024];
	FTControl *tc;
//	POINT3D angle;


	//initialize RMInfo
	memset(&RMInfo,0,sizeof(RobotModelInfo));
	RMInfo.tc_time=GetFTControl("lblRobotModel_TimeElapsed");
	RMInfo.tc_stime=GetFTControl("lblRobotModel_StageTimeElapsed");
	RMInfo.tc_log=GetFTControl("lblRobotModel_LogEntry");



	//initialize F3DStatus structure if not done already
	if (!(RStatus.flags&ROBOT_STATUS_INIT_FREE3D)) {
		memset(&F3DStatus,0,sizeof(F3D_Status));
		RStatus.flags|=ROBOT_STATUS_INIT_FREE3D;
		F3DStatus.renderflags=F3D_RENDER_FLAT_SHADING|F3D_RENDER_TRANSFORM_POINTS_BY_MODEL_AXES|F3D_RENDER_PERSPECTIVE|F3D_RENDER_POLYGON_FACES|F3D_RENDER_POLYGON_CLIPPING|F3D_RENDER_USE_Z_BUFFER|F3D_RENDER_USE_LIGHTING;
//F3D_RENDER_USE_TEXTUREMAPS|F3D_RENDER_GOURAUD_SHADING   F3D_RENDER_CULL_BACKFACING_POLYGONS| |F3D_RENDER_LINES_ONLY  
	}

	//create camera if not already created
	if (F3DStatus.icamera==0) {
		F3DStatus.icamera=(CAMERA *)malloc(sizeof(CAMERA));
		icamera=F3DStatus.icamera;
		F3DStatus.curcamera=icamera;  //set the current camera to the first camera

		//camera 1 (front)
		memset(icamera,0,sizeof(CAMERA));
		icamera=(CAMERA *)F3DStatus.icamera;
		MakeIdentityMatrix(&icamera->axis);
		icamera->focus=300; //300 mm
		icamera->trans_change=10; //how much the camera position changes when an (arrow) key is pressed (in native units mm, or inches)
		icamera->rot_change=0.1; //how much (in radians) camera angle moves when a key is pressed
//for legs only		icamera->location.x=0.0; icamera->location.y=300.0; icamera->location.z=-1000.0;  //500mm away traditional axes
		icamera->location.x=0.0; icamera->location.y=570.0; icamera->location.z=-1500.0;

		F3DStatus.curcamera=icamera; //set this camera as the current camera


		//camera 2 (left or right)
		icamera->next=(CAMERA *)malloc(sizeof(CAMERA));
		lcamera=icamera->next;
		memset(lcamera,0,sizeof(CAMERA));
		//MakeIdentityMatrix(&lcamera->axis);
		//MakeRotationMatrix_AroundYAxis(&lcamera->axis,-PI/2.0); //turn to the left 90 degrees
		MakeRotationMatrix_AroundYAxis(&lcamera->axis,PI/2.0); //turn to the right 90 degrees
		lcamera->focus=300; //300 mm
		lcamera->trans_change=10; //how much the camera position changes when an (arrow) key is pressed (in native units mm, or inches)
		lcamera->rot_change=0.1; //how much (in radians) camera angle moves when a key is pressed
		//legs lcamera->location.x=1000.0; lcamera->location.y=300.0; lcamera->location.z=0.0;  //500mm away traditional axes
		//lcamera->location.x=1400.0; lcamera->location.y=570.0; lcamera->location.z=0.0; 
		lcamera->location.x=-1400.0; lcamera->location.y=570.0; lcamera->location.z=0.0; 

		//camera 3 (left or right diagonal)
		lcamera->next=(CAMERA *)malloc(sizeof(CAMERA));
		lcamera=lcamera->next;
		memset(lcamera,0,sizeof(CAMERA));
		//MakeIdentityMatrix(&lcamera->axis);
		//MakeRotationMatrix_AroundYAxis(&lcamera->axis,-PI/4.0); //turn camera to the left 45 degrees
		MakeRotationMatrix_AroundYAxis(&lcamera->axis,PI/4.0); //turn camera to the right 45 degrees
		lcamera->focus=300; //300 mm
		lcamera->trans_change=10; //how much the camera position changes when an (arrow) key is pressed (in native units mm, or inches)
		lcamera->rot_change=0.1; //how much (in radians) camera angle moves when a key is pressed
//		lcamera->location.x=700.0; lcamera->location.y=300.0; lcamera->location.z=-700.0;  //500mm away traditional axes
		//lcamera->location.x=1070.0; lcamera->location.y=570.0; lcamera->location.z=-1000.0;
		lcamera->location.x=-1070.0; lcamera->location.y=570.0; lcamera->location.z=-1000.0;

//		F3DStatus.curcamera=lcamera; //set this camera as the current camera


		//camera 4 (top)
		lcamera->next=(CAMERA *)malloc(sizeof(CAMERA));
		lcamera=lcamera->next;
		memset(lcamera,0,sizeof(CAMERA));
		//MakeIdentityMatrix(&lcamera->axis);
		MakeRotationMatrix_AroundXAxis(&lcamera->axis,PI/2); //turn to the left 45 degrees
		lcamera->focus=300; //300 mm
		lcamera->trans_change=10; //how much the camera position changes when an (arrow) key is pressed (in native units mm, or inches)
		lcamera->rot_change=0.1; //how much (in radians) camera angle moves when a key is pressed
		lcamera->location.x=0.0; lcamera->location.y=1000.0; lcamera->location.z=0.0;  //500mm away traditional axes

	}

	//create light if not already created
	if (F3DStatus.ilight==0) {  
		//ilight contains all lights 
		//that may be used in a scene 
		//but only AddLightToScene will add a light to a scene

		//ambient light
		F3DStatus.ilight = (LIGHT *)malloc(sizeof(LIGHT));
		llight = F3DStatus.ilight;
		memset(llight, 0, sizeof(LIGHT));
		strcpy(llight->name, "Ambient_Light");
		MakeIdentityMatrix(&llight->axis);
		llight->location.x = 0.0;// 1000.0;
		llight->location.y = 0.0;// 1000.0;
		llight->location.z = 0.0;// -1000.0;
		llight->deg = 0;// PI;
		llight->color = 0x333333;// 0xffffff;
		//now add the light to the scene
		AddLightToScene(llight);

		//directional light
		F3DStatus.ilight->next=(LIGHT *)malloc(sizeof(LIGHT));
		llight=F3DStatus.ilight->next;
		memset(llight,0,sizeof(LIGHT));
		strcpy(llight->name,"Directional_Light_01");
		MakeIdentityMatrix(&llight->axis);
		llight->location.x = 1000.0;// 1000.0;
		llight->location.y = 1000.0;// 1000.0;
		llight->location.z = -1000.0;// -1000.0;
		llight->deg = PI;// PI;
		llight->color=0xbbbbbb;				
		//now add the light to the scene
		AddLightToScene(llight); 



	} //if (F3DStatus.ilight==0) {  


	
	//todo: combine all the robot segments into one model
	//Load Robot model if not already loaded
	if (F3DStatus.imodel==0) {
		if (strlen(RStatus.CurrentRobotModelFile)==0) {
			sprintf(tstr,"No Robot Model file given");
			FTMessageBox(tstr,FTMB_OK,"Robot - Error",0);
			return(0);
		}
		//needs to have the correct path- loading a script can change the cd
#if Linux
		//sprintf(ModelFileName,"%s/models/RobotModel.mod",PStatus.ProjectPath);
		sprintf(ModelFileName,"%s/models/%s",RStatus.CurrentRobotFolder,RStatus.CurrentRobotModelFile);

#endif
#if WIN32
//		sprintf(ModelFileName,"%s\\models\\RobotModel.mod",PStatus.ProjectPath);
		sprintf(ModelFileName,"%s\\models\\%s",RStatus.CurrentRobotFolder,RStatus.CurrentRobotModelFile);
#endif
		lmodel=LoadModel(ModelFileName);  //if already loaded

		if (lmodel==0) {
			sprintf(tstr,"Failed to load model '%s'",ModelFileName);
			FTMessageBox(tstr,FTMB_OK,"Robot - Error",0);
			return(0);
		}

		//rotate robot
		//MakeRotationMatrix_AroundYAxis(&lmodel->axis[0],0.4); //turn to the left 45 degrees PI/4=0.785  PI/2=1.57


		//add this model to the current scene
		AddModelToScene(lmodel);

	} //	if (F3DStatus.imodel==0) {



	//and draw (render) the model in the main window image
	RenderModelMainImage();

	//set the flag that connects the accelerometer values to the robot model
	//RStatus.flags|=ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL;

	RMInfo.PlayDelay=0.0; //initialize log playback speed (0.0 = as fast as possible)
	tc=GetFTControl("txtRobotModel_PlayDelay");
	if (tc!=0) { //refresh control
		sprintf(tc->text,"%03.1f",RMInfo.PlayDelay);
		DrawFTControl(tc);
	}

	RMInfo.StageTimeElapsed=0;

	return(1);
} //int winRobotModel_OnOpen(FTWindow *twin)

int winRobotModel_OnClose(FTWindow *twin)
{
	//free Robot model?

	return(1);
} //int winRobotModel_OnClose(FTWindow *twin)


//Load a log file to replay Robot movements
int btnRobotModel_LoadLogFile_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{

	win_Load_Robot_Log_CreateFTWindow();

	twin->FocusFTControl=0; //make sure no control gets the focus, so keypresses will work


	return(1);
} //int btnRobotModel_LoadLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Play a log file (replays Robot movements)
int btnRobotModel_PlayLogFile_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	char tstr[512],tline[1024];
	//unsigned int *ifile;//pointer into log file (index into file),
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE), 
	//unsigned int *offset; //point to offset from start of buffer- when a log entry at the end of the buffer was split
	unsigned int *endbuf; //point to end of log file buffer
	char *buffer; //pointer into log file buffer
	int NumBytes,result;
	//unsigned int endbuf; 
//	int DelayMultiple; //how much to delay the movie frame rate (1 = fastest, >1=slower, ex: 2=2ms delay, 3=3ms, etc.)
	uint64_t filepos;


	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

#if Linux
	filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
	filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
	
	if (filepos >=RMInfo.LogFileSize) {  //at end of log file
		btnRobotModel_RewindLogFile_Click(twin,tcontrol,0,0);
	}


	//first see if a log file has been loaded
	if (RMInfo.fRobotLoadedLog == 0) {
		sprintf(tstr, "No log file loaded.");
		FTMessageBox(tstr, FTMB_OK, "Robot - Error", 0);
		return(0);
	}

	//LogFileDone = 0;
	RMInfo.flags|=ROBOT_MODEL_INFO_PLAYING_LOG;  //indicate that the log file is now playing, stopping will unset this flag

	//ifile = &RMInfo.LogFilePos; //quick index into log file, writes to RMInfo.LogFilePos so other functions can access it
	ibuf=&RMInfo.ibuf; //shortcut pointer to log file RAM buffer
	//offset=&RMInfo.offset; //shortcut pointer to log file offset index
	buffer=&RMInfo.buffer; //shortcut to point to log file buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to the end of the log file buffer


	//curtime = &RStatus.PlaybackTime; //quick index into current playback time, writes to RStatus.PlaybackTime so other functions can access it - probably move to RMInfo struct

//	offset = 0;
//	DelayMultiple = 100;  // ms of delay between each frame
//  if (RMInfo.PlayDelay==0.0) {
//		RMInfo.PlayDelay=1.0;
//	}  
//	DelayMultiple = (int) (33.0/RMInfo.PlayDelay);  // ms of delay between each frame 1.0=33mm (30fps)
	//fprintf(stderr,"DelayMultiple=%d",DelayMultiple);


	//todo: just use stepforward calls

	//for each line in the log file, run through and at the correct time apply the accelerometer motions
	//LineNum = 0;  
	while(RMInfo.flags&ROBOT_MODEL_INFO_PLAYING_LOG) {
		//while there is still data (and playing log file has not been stopped by user) keep reading from log file



		btnRobotModel_StepLogFileForward_Click(twin, tcontrol, 0, 0); //this function will clear the ROBOT_MODEL_INFO_PLAYING_LOG flag when done playing

		//each log entry does not represent a single frame so user can just add ms delay between log entries
#if Linux
		usleep(1000 * RMInfo.PlayDelay);//DelayMultiple);
#endif //Linux
#if WIN32
		Sleep(RMInfo.PlayDelay);//DelayMultiple);
#endif //Win32


		FT_GetInput(); //or else clicking stop button does not happen, stop button can clear ROBOT_MODEL_INFO_PLAYING_LOG flag 


	} //while(!LogFileDone) 


/*
	//for now go back to the beginning
#if Linux
	fseek(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
#if WIN32
	_fseeki64(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
*/

	return(1);
} //int btnRobotModel_PlayLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Skip forward to first non-EthAccel Log entry
int btnRobotModel_SkipLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int Done;
	//unsigned int *ifile;//pointer into log file (index into file),
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE),
	unsigned int *endbuf; //pointer to end of buffer
//	unsigned int *offset; //pointer to offset from start of buffer- when a log entry at the end of the buffer was split
	uint64_t filepos;

	//ifile = &RMInfo.LogFilePos; //quick index into log file, writes to RMInfo.LogFilePos so other functions can access it
	ibuf=&RMInfo.ibuf; //shortcut pointer to log file RAM buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to end of log file buffer
//	offset=&RMInfo.offset; //shortcut pointer to log file offset index


	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

	//todo: just change to look at RMInfo.Entry
	Done=0;
	RMInfo.flags|=ROBOT_MODEL_INFO_PLAYING_LOG;  //so user can stop
	while(!Done && (RMInfo.flags&ROBOT_MODEL_INFO_PLAYING_LOG)) {
#if Linux
		filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
		filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
		if (*ibuf == *endbuf && filepos >= RMInfo.LogFileSize) { //reach end of buffer and file
			Done = 1;

		} else {

			//skip a frame forward
			btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);

			//if (strncmp(&buffer[*ibuf+20],"ETHACC",6)) { - does not work when log entry extends over end of buffer
			//if (strncmp(RMInfo.tc_log->text+20, "ETHACC",6)) {
			//skip all accel/gyro scope, TurnMotor, and TargetAngle updated log entries 
			if (strncmp(RMInfo.tc_log->text+20, "ETHACC",6) && strncmp(RMInfo.tc_log->text+20,"MOTOR",5) && strncmp(RMInfo.tc_log->text+20,"TargetAngle",11)) { 

				Done = 1;
			} 

			FT_GetInput(); //or else clicking stop button does not happen, stop button can clearn ROBOT_MODEL_INFO_PLAYING_LOG flag 


		} //if (*ibuf == *endbuf && filepos >= RMInfo.LogFileSize) {

	} //while(!Done)

	return(1);
} //int btnRobotModel_SkipLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Step forward one log entry
int btnRobotModel_StepLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	char tstr[512],tline[1024];
	//unsigned int *ifile;//pointer into log file (index into file),
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE), 
	//unsigned int *offset; //pointer to offset from start of buffer- when a log entry at the end of the buffer was split
	unsigned int *endbuf; //pointer to end of buffer
	char *buffer; //pointer into log file buffer
	int NumBytes,result;
	unsigned int offset; //temp variable to store remaining characters when loading more log into the buffer
	uint64_t filepos;


	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

/*
	//first see if a log file has been loaded
	if (RMInfo.fRobotLoadedLog == 0) {
		sprintf(tstr, "No log file loaded.");
		FTMessageBox(tstr, FTMB_OK, "Robot - Error", 0);
		return(0);
	}
*/

	//ifile = &RMInfo.LogFilePos; //quick index into log file, writes to RMInfo.LogFilePos so other functions can access it
	ibuf = &RMInfo.ibuf; //shortcut pointer to log file buffer
	//offset = &RMInfo.offset; //shortcut pointer to log file buffer
	buffer = RMInfo.buffer; //shortcut to point to log file buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to end of log file buffer


#if Linux
	filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
	filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif

	if (*ibuf==*endbuf && filepos >= RMInfo.LogFileSize) {  //at end of log file
		if (RMInfo.flags&ROBOT_MODEL_INFO_PLAYING_LOG) { //end playing log if log is playing
			RMInfo.flags &= ~ROBOT_MODEL_INFO_PLAYING_LOG; //clear playing flag
		}
		return(1);
	}





//process a line in the log file

	//note that if the buffer has not been filled yet, ibuf=endbuf=0

	//Load a buffer full from the log file if necessary
	//if the current buffer pointer is >= the end of the buffer
	//if (*ibuf>=*endbuf) { //need to load more from log file

		//offset = *endbuf - *ibuf;
#if Linux
	filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
	filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif

	if (filepos==0 && *endbuf == 0) { //no log file has been loaded into the buffer yet
		//if (RMInfo.LogFileSize<LOGFILEBUFFERSIZE+*ifile) {  //remaining file data is smaller than LOGFILEBUFFERSIZE
		if (RMInfo.LogFileSize<LOGFILEBUFFERSIZE + filepos) {  //remaining file data is smaller than LOGFILEBUFFERSIZE
			//NumBytes = fread(buffer+offset, 1,RMInfo.LogFileSize-offset, RMInfo.fRobotLoadedLog);
			NumBytes = fread(buffer, 1, RMInfo.LogFileSize, RMInfo.fRobotLoadedLog);
		} else {
			//NumBytes = fread(buffer+offset, 1, LOGFILEBUFFERSIZE-offset, RMInfo.fRobotLoadedLog);
			NumBytes = fread(buffer, 1, LOGFILEBUFFERSIZE, RMInfo.fRobotLoadedLog);
		} 
		//*ifile+=NumBytes; //update global index into log data file
		//note that there may be an *offset (a few bytes) from the last load moved to the front of the buffer
		//*endbuf = *ibuf + offset+ NumBytes; //what ibuf will equal when done processing this buffer
		*endbuf = *ibuf + NumBytes; //what ibuf will equal when done processing this buffer
		*ibuf = 0; //reset current pointer into RAM buffer
	}

	//get a line from the log file buffer
	//fprintf(stderr,"buffer+*ibuf=%x,endbuf-*ibuf=%x ",buffer+*ibuf,endbuf-*ibuf);
	result = Robot_GetLineFromText((char *)tline, buffer+*ibuf,*endbuf-*ibuf); //+1 for chr(10)	
	if (tline[result-1] != 10) { //is not a complete line with linefeed
		//a full line was not read, 
		//could be end of file or only end of a buffer
#if Linux
		filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
		filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif

		//if (RMInfo.LogFileSize<LOGFILEBUFFERSIZE + *ifile) {  //remaining file data is smaller than LOGFILEBUFFERSIZE
		if (filepos==RMInfo.LogFileSize) { //last log entry
			//add line feed
			result++;
			tline[result-1] = 10;
			tline[result] = 0;
			NumBytes = 0;		
		} else {

			//need to read in more log file into buffer (replacing any existing data in the buffer except the current log entry)
			//fprintfd(stderr,"buffer break, %d remaining chars\n",result);
			memcpy(buffer, buffer + *ibuf, result); //copy remaining characters to the top of the buffer
													//read in more of the log file into the buffer
													//load more data

			if (RMInfo.LogFileSize < (LOGFILEBUFFERSIZE + filepos)) {  //remaining file data is smaller than LOGFILEBUFFERSIZE			
				NumBytes = fread(buffer + result, 1, RMInfo.LogFileSize - result, RMInfo.fRobotLoadedLog);
			}
			else {
				NumBytes = fread(buffer + result, 1, LOGFILEBUFFERSIZE - result, RMInfo.fRobotLoadedLog);
			}
			*ibuf = 0;
			*endbuf = result + NumBytes; //what ibuf will equal when done processing this buffer
		} //if (filepos==RMInfo.LogFileSize) { //last log entry
		//*ifile += NumBytes; //update global index into log data file

		//get line again
		result = Robot_GetLineFromText((char *)tline,buffer+*ibuf,*endbuf-*ibuf); //+1 for chr(10)	

	} //if (tline[result-1] != 10) { //is not a complete line with linefeed

	strcpy(RMInfo.LogLine,tline);

	if (!(RMInfo.flags&ROBOT_MODEL_INFO_SKIP_PROCESS) || 	!(RMInfo.flags&ROBOT_MODEL_INFO_GOT_FIRST_TIMESTAMP)) {  //do not process/interpret model log line if SKIP_PROCESS flag is set, unless have not processed at least one entry yet
		ProcessLogLine(tline); //set axes to accel		
	} 

	*ibuf += result;  //move log text buffer forward to the next line
	if (RMInfo.LastStepDir == -1) { 
		//last step was a backwards step so step forward one more time
		RMInfo.LastStepDir = 1;
		btnRobotModel_StepLogFileForward_Click(twin, tcontrol, x, y);
	} else {
		RMInfo.LastStepDir = 1;
	}

	return(1);
} //int btnRobotModel_StepLogFileForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Step backward one log entry
int btnRobotModel_StepLogFileBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	char tstr[512],tline[1024];
	//unsigned int *ifile;//pointer into log file (index into file),
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE), 
	//unsigned int *offset; //pointer to offset from start of buffer- when a log entry at the end of the buffer was split
	unsigned int *endbuf; //pointer to end of the buffer - used to store offset when log entry is split between two buffers
	char *buffer; //pointer into log file buffer
	int NumBytes,result;
	uint64_t filepos;

//todo fix: the first step backward stays at the same log entry-same for the firststep forward after stepping backward
//__solution use two indices- one for foward and one for backward- 
//__or store the last successful frame forward or backwards to know if getting an extra line is needed


//first see if a log file has been loaded
	if (RMInfo.fRobotLoadedLog == 0) {
		//sprintf(tstr, "No log file loaded.");
		//FTMessageBox(tstr, FTMB_OK, "Robot - Error", 0);
		return(0);
	}



	//ifile = &RMInfo.LogFilePos; //quick index into log file, writes to RMInfo.LogFilePos so other functions can access it
	buffer = RMInfo.buffer; //shortcut to point to log file buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to end of log file buffer
	ibuf = &RMInfo.ibuf; //shortcut pointer to log file buffer
	//offset=&RMInfo.offset; //shortcut pointer to log file offset index


#if Linux
	filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
	filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
	if (*ibuf==0 && ((filepos == 0 || filepos == LOGFILEBUFFERSIZE) || (RMInfo.LogFileSize<LOGFILEBUFFERSIZE && filepos == RMInfo.LogFileSize))) {  //at start of log file
		return(1);
	}



	//note that the log file must always be forward stepped before it can be backward stepped, and the first forward step (or play) loads the first buffer full of file data
	//so there will always be data in the buffer here

		//get a line from the log file buffer
		//fprintf(stderr,"buffer+*ibuf=%x,endbuf-*ibuf=%x ",buffer+*ibuf,endbuf-*ibuf);
		result = Robot_GetLineBackwardsFromText((char *)tline, buffer,*ibuf-1); //-1 to skip last chr(10)		

		//if result is negative, then the result in tline is:
    //1) an incomplete log entry OR
		//2) the first line in the buffer OR
		//3) the last log entry in the file

//		if (result<0 && RMInfo.LogFileSize==*ibuf) { 
			//the last entry in the log file causes a negative result
//			result=-result;
//		} 

		if (result<0) { //incomplete or first log entry in buffer
			//load another buffer backwards

		
/*
				if (*ibuf==-result && *ifile<=LOGFILEBUFFERSIZE) {
					//first log entry
					//no need to load anything more
					//at beginning of file
					//*ifile=0;
					*ibuf=0;
					ProcessLogLine(tline); //set axes to accel
					return(1);
				}
*/

				//if (*ifile>LOGFILEBUFFERSIZE) {
					//load the another buffer (previous LOGFILEBUFFERSIZE buffer block)
					//but only LOGFILEBUFFER minus the result size (note: result is negative so add)
#if Linux
					filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
					filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif

					//if (*ifile>=LOGFILEBUFFERSIZE*2+result) {
					if (filepos >= (LOGFILEBUFFERSIZE * 2 + result)) {
					//if (*ifile>LOGFILEBUFFERSIZE*2) {  //there is more than a full buffer in front of the current one (which was already read in)
						//seek file pointer back
#if Linux
						//go back past the current buffer, and then another buffer-result 
						fseek(RMInfo.fRobotLoadedLog,-(LOGFILEBUFFERSIZE*2+result), SEEK_CUR);
#endif
#if WIN32
						_fseeki64(RMInfo.fRobotLoadedLog,-(LOGFILEBUFFERSIZE*2+result), SEEK_CUR);
#endif						


						//*ifile-=(LOGFILEBUFFERSIZE*2+result); //update file pointer
						//NumBytes = fread(buffer, 1, LOGFILEBUFFERSIZE+result, RMInfo.fRobotLoadedLog);
						NumBytes = fread(buffer, 1, LOGFILEBUFFERSIZE, RMInfo.fRobotLoadedLog);
						//*ifile+=LOGFILEBUFFERSIZE; //update file pointer
						//*ifile-=(LOGFILEBUFFERSIZE+result); //update file pointer




						//copy the last fragment
						//memcpy(buffer+LOGFILEBUFFERSIZE+result,tline,-result); 
						//get the line again
						*ibuf=LOGFILEBUFFERSIZE;
						result = Robot_GetLineBackwardsFromText((char *)tline, buffer, *ibuf - 1); //-1 to skip last chr(10)		
					} else {
						//remaining file to read is less than a full buffersize
						//seek file pointer back to beginning of file


						
#if Linux
						filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
						filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
						if (-*ibuf == result && ((filepos == LOGFILEBUFFERSIZE) || (RMInfo.LogFileSize<LOGFILEBUFFERSIZE && filepos == RMInfo.LogFileSize))) {  //at start of log file
							//special case where this is the first log entry
							result = -result;
/*
							//in this special case we need to seek to the start of the file plus the result so that the next forward step will load a fresh buffer from the correct log file position
#if Linux
							//fseek(RMInfo.fRobotLoadedLog,LOGFILEBUFFERSIZE+result, SEEK_CUR);
							fseek(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
#if WIN32
							//_fseeki64(RMInfo.fRobotLoadedLog,LOGFILEBUFFERSIZE+result, SEEK_CUR);
							_fseeki64(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
*/

						} else {



#if Linux
							//fseek(RMInfo.fRobotLoadedLog,LOGFILEBUFFERSIZE+result, SEEK_CUR);
							fseek(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
#if WIN32
							//_fseeki64(RMInfo.fRobotLoadedLog,LOGFILEBUFFERSIZE+result, SEEK_CUR);
							_fseeki64(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
							//RMInfo.LogFilePos =0; //update LogFile Position

							//if (*ifile>=LOGFILEBUFFERSIZE) {
							//	*ifile-=LOGFILEBUFFERSIZE; //there was more than a buffersize remaining to the file begining
							//}
							//*ibuf=*ifile-result;  //note!: result is negative, 	


							//so read a full buffer load from the file beginning 
							//only read what is left
							//NumBytes = fread(buffer, 1, *ifile, RMInfo.fRobotLoadedLog);
/*
							if (RMInfo.LogFileSize < LOGFILEBUFFERSIZE) {  //if file is smaller than LOGFILEBUFFERSIZE
								//I think this code is never reached, because the first forward step would load the full file and there would never be a case where back stepping would have to load another buffer
#if Linux
								*ibuf = filepos - RMInfo.LogFileSize - result; //filepos before going to beginning of log file - current position( LOGFILEBUFFERSIZE) + characters from last read. Note!: result is negative,
#endif
#if WIN32
								*ibuf = filepos - RMInfo.LogFileSize - result;  //filepos before going to beginning of log file - current position( LOGFILEBUFFERSIZE) + characters from last read. Note!: result is negative,
#endif

								//NumBytes = fread(buffer+*offset, 1,RMInfo.LogFileSize, RMInfo.fRobotLoadedLog);
								NumBytes = fread(buffer, 1, RMInfo.LogFileSize, RMInfo.fRobotLoadedLog);
								//*ifile = RMInfo.LogFileSize;
								//RMInfo.LogFilePos += RMInfo.LogFileSize; //update LogFile Position
							}
							else {
*/
#if Linux
								*ibuf = filepos - LOGFILEBUFFERSIZE - result; //filepos before going to beginning of log file - current position( LOGFILEBUFFERSIZE) + characters from last read. Note!: result is negative,
#endif
#if WIN32
								*ibuf = filepos - LOGFILEBUFFERSIZE - result;  //filepos before going to beginning of log file - current position( LOGFILEBUFFERSIZE) + characters from last read. Note!: result is negative,
#endif

								NumBytes = fread(buffer, 1, LOGFILEBUFFERSIZE, RMInfo.fRobotLoadedLog);
								*endbuf = LOGFILEBUFFERSIZE;
								//*ifile = LOGFILEBUFFERSIZE;
								//RMInfo.LogFilePos += LOGFILEBUFFERSIZE; //update LogFile Position
//							}

							//get the line again
							//set the buffer pointer to re-read the log entry: the last file pointer+size of fragment from last buffer
							//no need to add the last fragment because it should be read in
							//memcpy(buffer+LOGFILEBUFFERSIZE+result-1,tline,-result);

	//						*ifile=LOGFILEBUFFERSIZE;  //update file pointer
							result = Robot_GetLineBackwardsFromText((char *)tline, buffer, *ibuf - 1); //-1 to skip last chr(10)		

						} //if (-*ibuf == result && (filepos == LOGFILEBUFFERSIZE) || (RMInfo.LogFileSize<LOGFILEBUFFERSIZE && filepos == RMInfo.LogFileSize)) {  //at start of log file
					} //if (filepos >= LOGFILEBUFFERSIZE * 2 + result) {
					
				//} 
/*
		if (tline[result-1] != 10) { //is not a complete line with linefeed
			//fprintfd(stderr,"buffer break, %d remaining chars\n",result);
			*offset = result; //hold on to this text, for the next time around through the while(!logFileDone) loop
			memcpy(buffer,buffer + *ibuf, *offset); //copy remaining characters to the top of the buffer
			*ibuf=0;
			RMInfo.flags&=~ROBOT_MODEL_INFO_LOADED_BUFFER;  //clear buffer loaded flag- need to load more data
			//load more data
			NumBytes = fread(buffer+*offset, 1, LOGFILEBUFFERSIZE-*offset, RMInfo.fRobotLoadedLog);
			RMInfo.flags|=ROBOT_MODEL_INFO_LOADED_BUFFER;  //set buffer loaded flag
			if (NumBytes < LOGFILEBUFFERSIZE-*offset) { //no more buffer to read
				//LogFileDone = 1;
				RMInfo.flags&=~ROBOT_MODEL_INFO_LOADED_BUFFER;  //clear buffer loaded flag
			}
			*ifile+=NumBytes; //update global index into log data file
			endbuf = *ibuf + *offset+ NumBytes; //what ibuf will equal when done processing this buffer
			*offset = 0; //reset offset

			//get line again
			result = Robot_GetLineFromText((char *)tline,buffer+*ibuf,endbuf-*ibuf); //+1 for chr(10)	
		} //if (tline[result-1] != 10) { //is not a complete line with linefeed
*/
			strcpy(RMInfo.LogLine,tline);  //copy line for skip to stage code

			if (!(RMInfo.flags&ROBOT_MODEL_INFO_SKIP_PROCESS)) {  //do not process/interpret model log line if SKIP_PROCESS flag is set
				ProcessLogLine(tline); //set axes to accel		
			} 
			if (result<0) { //can only mean the beginning of the file was reached
				*ibuf+=result;
			} else {
				*ibuf -= result;  //move log text buffer forward to the next line
			} 
		}  else { //if (result<0)


			if (!(RMInfo.flags&ROBOT_MODEL_INFO_SKIP_PROCESS)) {  //do not process/interpret model log line if SKIP_PROCESS flag is set
				ProcessLogLine(tline); //set axes to accel		
			} 
			*ibuf -= result;  //move log text buffer forward to the next line

			if (*ibuf <= 0) {
				*ibuf=0;
				//*ifile-=LOGFILEBUFFERSIZE;
#if Linux
				filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
				filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif

				//if (*ifile>0) {
				if (filepos>0) {
					//load the previous LOGFILEBUFFERSIZE
					NumBytes = fread(buffer-LOGFILEBUFFERSIZE, 1, LOGFILEBUFFERSIZE, RMInfo.fRobotLoadedLog);
				//NumBytes = fread(buffer+*offset, 1, LOGFILEBUFFERSIZE-*offset, RMInfo.fRobotLoadedLog);

				}
				//RMInfo.flags&=~ROBOT_MODEL_INFO_LOADED_BUFFER;  //clear buffer loaded flag - need to load more data
				//load buffer here
			}//if (*ibuf <= 0) {
		} //

		if (RMInfo.LastStepDir == 1) {
			//last step was a forewards step so step backward one more time
			RMInfo.LastStepDir = -1;
			btnRobotModel_StepLogFileBackward_Click(twin, tcontrol, x, y);
		} else {
			RMInfo.LastStepDir = -1;
		}

		
		return(1);
} //int btnRobotModel_StepLogFileBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Interpret a single line in the Model Log (move robot, fill fields)
int ProcessLogLine(char *tline) {
	char tline2[1024],tstr[512],tstr2[512];
	char *timestampstr,*pcbname,AccelOrTouch,*param;
	int AccelNum, AccelBase; //current accel number, current starting (lowest) Accel number for a PCB
	int GotTouchData, TouchNum;
	AnalogSensor ts;//[NUM_TOUCH_SENSORS];
	int DoneWithAccels,GotAccelData,i,j; 
	AccelSample fSample;
	POINT3D angle,gyro;
	MODEL *lmodel;
	FTControl*tc,*tc2,*tc3;
	float lval;
	int tlen,tnum;
	int PCBNum,MotorNum,MotorThrust,MotorDur,MotorNameNum;
	int AxisNum,AccelNameNum;
	char *MotorName,*AccelName,AccelName2[1024];

//	RMInfo.LineNum++; //increase current log line number

	//skip any empty newline or carriage return only line
	if (strlen(tline)<3) {
		return(1);
	} 

	//print log entry -currently log uses tab, should probably use comma
	strcpy(tline2,tline);
	ReplaceTabWithSpace(tline2);
	strcpy(RMInfo.tc_log->text, tline2);
	DrawFTControl(RMInfo.tc_log);

	//print formatted timestamp


	//every log entry must start with a time stamp
	//todo: error where tlin is only \n
	//parse the line into "timestamp pcbname accel# x y accel# x y ..."
	//get a word on this line
	timestampstr = strtok(tline, "\t"); //timestamp
	//timestampstr = strtok(tline2, " "); //timestamp

	//get the robot model
	lmodel = F3DStatus.imodel;


	//if this is the first timestamp then set the playback time to it- todo: we need to set the initial playback time at the time of loading
	if (!(RMInfo.flags&ROBOT_MODEL_INFO_GOT_FIRST_TIMESTAMP)) {
		//*curtime = ConvertTimeStampToMS(timestampstr);
		RMInfo.StartTime = ConvertTimeStampToMS(timestampstr);
		RMInfo.flags|=ROBOT_MODEL_INFO_GOT_FIRST_TIMESTAMP;
	}


	//convert timestamp into milliseconds
	RMInfo.EventTime = ConvertTimeStampToMS(timestampstr);
	RMInfo.TimeElapsed=RMInfo.EventTime-RMInfo.StartTime;
	//fprintf(stderr,"EventTime:%lli -StartTime:%lli = ElapsedTime:%lli",RMInfo.EventTime,RMInfo.StartTime,RMInfo.TimeElapsed);
	sprintf(RMInfo.tc_time->text,"Time: %llu ms",RMInfo.TimeElapsed);
	DrawFTControl(RMInfo.tc_time);
	//fprintf(stderr,"TimeElapsed=%lli",RMInfo.TimeElapsed);

	//only determine StageTimeElapsed if Current Stage has started 
	RMInfo.StageTimeElapsed=RMInfo.EventTime-RMInfo.StartStageTime;
	if (RMInfo.StageNumber==0 || RMInfo.StartStageTime>RMInfo.EventTime) {
		RMInfo.StageTimeElapsed=0;
	}
	sprintf(RMInfo.tc_stime->text,"Stage: %llu ms",RMInfo.StageTimeElapsed);
	DrawFTControl(RMInfo.tc_stime);


	pcbname = strtok(NULL, "\t\n"); //pcbname \n will catch any line without a tab like "Script Aborted"

	if (pcbname != 0) {
	
		//get PCB Num from pcbname
		PCBNum=GetPCBNameNum(pcbname);


		//START ETHACCEL
		if (!strncmp(pcbname, "ETHAC", 5)) {
			//this entry is for an EthAccels PCB
#if 0 
			if (!strcmp(pcbname, "ETHACCELS_LEFT_LEG")) {
				AccelBase = 1; //axis 1,2,3 are left foot, lower leg, upper leg 
			}
			if (!strcmp(pcbname, "ETHACCELS_RIGHT_LEG")) {
				AccelBase = 4; //axis 4,5,6 are right foot, lower leg, upper leg
			}
			if (!strcmp(pcbname, "ETHACCELS_TORSO")) {
				AccelBase = 7; //axis 7,8,9 are waist, torso and head
			}
			if (!strcmp(pcbname, "ETHACCELS_LEFT_ARM")) {
				AccelBase = 10; //axis 10,11,12 are left upper arm, lower arm, and hand
			}
			if (!strcmp(pcbname, "ETHACCELS_RIGHT_ARM")) {
				AccelBase = 13; //axis 13,14,15 are right upper arm, lower arm, and hand
			}
#endif
			memset(&angle, 0, sizeof(POINT3D)); //clear the angle



			//determine if Accel or Touch data
			param = strtok(NULL, "\t"); //A=Accel T=Touch
			//param = strtok(NULL, " "); //A=Accel T=Touch
			AccelOrTouch = 0;
			if (param[0] == 84) { //T
				AccelOrTouch = 1;
			}


			if (AccelOrTouch) {
				//T (Touch sensor samples)
				GotTouchData = 0;

				param = strtok(NULL, "\t");
				while (param != 0) { // && !DoneWithTouch) {
					//read in touch sensor number, percent pressed, percent change, voltage
					TouchNum = atoi(param);
					param = strtok(NULL, "\t");  //get percent touched
					ts.Percent = atof(param);
					param = strtok(NULL, "\t");  //get percent change
					ts.PercentChange = atof(param);
					param = strtok(NULL, "\t\n");
					ts.fSample = atof(param); //needs to be reversed for model
					GotTouchData = 1;
					param = strtok(NULL, "\t\n");


				//update text boxes
					sprintf(tstr, "lblRobotModel_%sTouch%d", pcbname, TouchNum);
					tc = GetFTControl(tstr);
					if (tc != 0) {
						//					lval=atof(tc->text);  //last value
						sprintf(tc->text, "%03.1f", ts.Percent);
						DrawFTControl(tc);
					}
					sprintf(tstr, "lblRobotModel_%sTouchd%d", pcbname, TouchNum);
					tc = GetFTControl(tstr);
					if (tc != 0) {
						sprintf(tc->text, "%03.1f", ts.PercentChange);  //difference of current-last value
						DrawFTControl(tc);
					}

				} //while (param != 0 ) {


			}
			else { //if (AccelOrTouch) {
				//A  (Accelerometer Samples)
				//for each accelerometer sample in the log apply the accelerometer data to the correct model axes

				GotAccelData = 0;
				param = strtok(NULL, "\t");
				while (param != 0) { //read in all Accel numbers and samples

					//read in accel num, and x,y,z (pitch,yaw,roll)
					AccelNum = atoi(param);
					param = strtok(NULL, "\t");  //get x (pitch)
					//fSample.x = atof(param);
					//currently need to convert from degrees to radians
					//angle.x = -atof(param) * 2 * PI / 360.0; //pitch needs to be reversed for the model to be rendered correctly
					angle.x = atof(param); //convert pitch to float
					param = strtok(NULL, "\t");  //get y (yaw)
					angle.y = atof(param);//0.0;//for now keep yaw at 0  atof(param) * 2 * PI / 360.0;
					param = strtok(NULL, "\t\n");  //get z (roll)
					//angle.z = -atof(param) * 2 * PI / 360.0;   //roll also need to be reversed to render correctly
					angle.z = atof(param);   //convert roll to float
					param = strtok(NULL, "\t\n"); //go to next sample


	//read in gyro data
					gyro.x = atof(param); //pitch 
					param = strtok(NULL, "\t");  //get y (yaw)
					gyro.y = atof(param);   //yaw
					param = strtok(NULL, "\t\n");  //get z (roll)
					gyro.z = atof(param);   //roll
					param = strtok(NULL, "\t\n"); //go to next sample



	//fill in accel labels
#if 0 
					AccelNameMap = -1;
					switch (AccelBase) {
					case 1: //Left Leg EthAccel
						switch (AccelNum) {
						case 0:
							AccelNameMap = 5; //foot
							break;
						case 1:
							AccelNameMap = 4; //lower leg		
							break;
						case 2:
							AccelNameMap = 3; //upper leg
							break;
						} //switch(AccelNum) {
						break;
					case 4: //Right Leg EthAccel
						switch (AccelNum) {
						case 0:
							AccelNameMap = 8; //foot
							break;
						case 1:
							AccelNameMap = 7; //lower leg		
							break;
						case 2:
							AccelNameMap = 6; //upper leg
							break;
						} //switch(AccelNum) {
						break;
					case 7: //torso EthAccel
						switch (AccelNum) {
						case 0:
							AccelNameMap = 2; //Waist
							break;
						case 1:
							AccelNameMap = 1; //Torso		
							break;
						case 2:
							AccelNameMap = 0; //Head		
							break;
						} //switch(AccelNum) {

						break;  //case 7 torso
					default:
						break;

					} //switch(AccelBase) {
#endif

					//get Accel NameNum using PCBNum and AccelNum
					AccelNameNum=GetAccelNameNum_By_PCB(PCBNum,AccelNum);
					//update text boxes
					if (AccelNameNum>-1) {
						//get Accel Name
						AccelName=RStatus.AccelInfo[AccelNameNum].Name;

						for (i = 0; i < 3; i++) {
							sprintf(tstr, "lblRobotModel_%sAngle%d", AccelName, i);
							tc = GetFTControl(tstr);
							if (tc != 0) {
								lval = atof(tc->text);  //last value
								sprintf(tc->text, "%03.1f", angle.i[i]);
								if (tc->color[0] == FT_RED || tc->color[0] == FT_ORANGE) {
									//if segment moving toward target color orange otherwise color red
									sprintf(tstr, "lblRobotModel_%sTargetAngle%d", AccelName, i);
									tc2 = GetFTControl(tstr); //get TargetAngle
									if (tc2 != 0) {
										//if Target<Segment and gyro<0 or Target>Segment and gyro>0 then orange (correct dir)
										if ((atof(tc2->text) < angle.i[i] && gyro.i[i] < 0.0) || (atof(tc2->text) > angle.i[i] && gyro.i[i] > 0.0)) {
											tc->color[0] = FT_ORANGE;
										}
										else {
											tc->color[0] = FT_RED;
										}
									} //if (tc2!=0)
								} // if (tc->color[0]==FT_RED
								DrawFTControl(tc);
							}
							//sprintf(tstr,"lblRobotModel_%sAcceld%d",AccelName[AccelNameMap],i);
							sprintf(tstr, "lblRobotModel_%sAngled%d", AccelName, i);
							tc = GetFTControl(tstr);
							if (tc != 0) {
								sprintf(tc->text, "%03.1f", angle.i[i] - lval);  //difference of current-last value
								DrawFTControl(tc);
							}
							sprintf(tstr, "lblRobotModel_%sGyro%d", AccelName, i);
							tc = GetFTControl(tstr);
							if (tc != 0) {
								lval = atof(tc->text); //last value
								sprintf(tc->text, "%03.1f", gyro.i[i]);
								DrawFTControl(tc);
							}
							sprintf(tstr, "lblRobotModel_%sGyrod%d", AccelName, i);
							tc = GetFTControl(tstr);
							if (tc != 0) {
								sprintf(tc->text, "%03.1f", gyro.i[i] - lval);  //difference of current-last value
								DrawFTControl(tc);
							}
						} //for i
					} //if (AccelNameNum>-1)

						//convert degrees to radians for 3D model
						//currently need to convert from degrees to radians
					angle.x = -angle.x * 2 * PI / 360.0; //pitch needs to be reversed for the model to be rendered correctly
					//angle.y = 0.0;//for now keep yaw at 0  atof(param) * 2 * PI / 360.0;
					angle.z = -angle.z * 2 * PI / 360.0;   //roll also need to be reversed to render correctly



					//the accelerometer x sample represents a rotation around the z axis
					//and the accelerometer y sample represents a rotation around the x axis
					//get the angle from the acceleration 
					//for pitch 0g=0 degrees, 1g=90degrees
					//for roll 0g=0 degrees, 1g=90degrees
					//todo: make a lookup table
					/*
					angle.z = asin(fSample.x); //roll (around z axis)
					angle.x = -asin(fSample.y); //pitch (around x axis)
					if (fSample.z<0) { //accel is upside down so make angles negative
						angle.z*=-1.0;
						angle.x*=-1.0;
					}
					*/


					//	fprintf(stderr,"%d: %0.3f %0.3f %0.3f\n",AccelNum,angle.x,angle.y,angle.z);												
						//because there are 2 angles, a rotation matrix is made with the first angle
						//and then that matrix is rotated by the second angle
						//todo: the rotation functions could perhaps be faster, in assembler, but also without matrices 
					//MakeRotationMatrix_AroundZAxis(&lmodel->axis[AccelBase + AccelNum].m, angle.z);
					AxisNum=RStatus.AccelInfo[AccelNameNum].ModelAxis;
					MakeRotationMatrix_AroundZAxis(&lmodel->axis[AxisNum].m, angle.z);
					//overwrite the model axis with the axis from the correct associated accelerometer
					angle.z = 0; //only two angles can be rotated
					//angle.x = 0; //only two angles can be rotated
					//to do make RotateAxis3D with 3 angles
					RotateAxis3D(&lmodel->axis[AxisNum].m, &angle);
					GotAccelData = 1;
				} //while (param != 0) //(!DoneWithAccels) {


				if (GotAccelData) {
					RenderModelMainImage();
				} //GotAccelData


			} //if (AccelOrTouch) {
		} //if (!strncmp(pcbname,"ETHAC",5)) {
		//END ETHACCEL



		//if (!strncmp(pcbname, "ETHMO", 5)) { //EthMotors
		if (!strncmp(pcbname, "MOTOR", 5)) { //log now contains the individual motor name
			//read in Motor Name (ex: MOTOR_LEFT_ANKLE)
			//param = strtok(NULL, "\t");  //MotorName
			//read in Motor num, strength (thrust), and duration
			//param = strtok(NULL, "\t");  
			//MotorNum = atoi(param);
			param = strtok(NULL, "\t");
			MotorThrust = atoi(param);
			param = strtok(NULL, "\t\n");
			MotorDur = atoi(param);

			MotorNameNum = GetMotorNameNum(pcbname);
			if (MotorNameNum<0) {
				fprintf(stderr,"GetMotorNameNum returned -1 for name %s\n",pcbname);
			} else {
				MotorName = RStatus.MotorInfo[MotorNameNum].Name;
				PCBNum = GetPCBNameNum(MotorName);
				MotorNum = RStatus.MotorInfo[MotorNameNum].Num;  //Get number of motor on PCB
				//MotorName = RStatus.MotorInfo[MotorNameNum].ReadableName;//GetMotorReadableName_By_PCB(PCBNum, MotorNum);  //12 motors per EthMotors PCB, and they are in order in the static text array
				
				//fill text boxes with Motor info

				sprintf(tstr, "lblRobotModel_%s",MotorName);
				tc = GetFTControl(tstr);
				if (tc != 0) {
					if (MotorThrust == 0) {
						//set label background color
						//tc->textcolor[1]=0xcfcfcf;
						tc->color[0] = 0xdfdfdf;
					}
					else {
						//tc->textcolor[1]=0xcf00cf;
						tc->color[0] = 0xcf00cf;
					}
					DrawFTControl(tc);
				} //if (tc!=0)

				sprintf(tstr, "lblRobotModel_%sThrust",MotorName);
				//fprintf(stderr,"%s\n",tstr);
				tc = GetFTControl(tstr);
				if (tc != 0) {
					if (MotorThrust == 0) {
						tc->textcolor[0] = 0x3f3f3f;
					}
					else {
						tc->textcolor[0] = 0xcf00cf;
					}
					sprintf(tc->text, "%0d", MotorThrust);
					DrawFTControl(tc);
				}
			} //if (MotorNameNum<0) {
		} //if (!strncmp(pcbname, "ETHMO", 5)) { //EthMotors


		if (!strncmp(pcbname, "Changing", 8)) { //ex: "Changing Left Ankle Motor from Accel[0] (Left Foot Z 0.0 -10 5.0) to Accel[1] (Left Lower Leg Z -7.0 -8.0 -6.0)"
			//note that pcbname contains the entire string
			//read up to motor name
			//read in Motor Readable Name  //ex:"Left Ankle Motor"
			//fprintf(stderr,"%s\n",pcbname);
			
			GetTextUpToString(&pcbname[9]," from",tline2);

			MotorName=GetMotorNameFromReadableName(tline2);
			//fprintf(stderr,"%s\n",tline2);
			//fill text boxes with Accel Motor is using info
			sprintf(tstr, "lblRobotModel_%sUseAccel",MotorName);
			tc = GetFTControl(tstr);
			if (tc != 0) {
				tlen=strlen(tline2);
				GetTextUpToString(&pcbname[14+tlen], " to Angle[",tline2);  //get up to UseAngle number
				strcpy(tstr,&pcbname[14+tlen+strlen(tline2)+10]);
				tstr[strlen(tstr)-1]=0; //remove last )

				//strncpy(tstr,&pcbname[14+tlen+strlen(tline2)+10],(tc->x2-tc->x1)/tc->fontwidth);
				if (tstr[0]==48) { //0
					tnum=0;
					tc->textcolor[0] = 0x009000;
				} else {
					tnum=1;
					tc->textcolor[0] = 0x009090;
				} 
				sprintf(tc->text, "%d:%s",tnum,&tstr[4]);
				//fprintf(stderr,"text: %s",tc->text);
				DrawFTControl(tc);

			} //if (tc!=0)

		} //if (!strncmp(pcbname, "Changing", 8)) {

		if (!strncmp(pcbname, "Initial", 7)) { //ex: "Initial: Left Ankle Motor using Accel[1] (Left Lower Leg Z -7.0 -8.0 -6.0)"
			//note that pcbname contains the entire string
			//read up to motor name
			//read in Motor Readable Name  //ex:"Left Ankle Motor"
			//fprintf(stderr,"%s\n",pcbname);
			
			GetTextUpToString(&pcbname[9]," using ",tline2);

			MotorName=GetMotorNameFromReadableName(tline2);
			//fprintf(stderr,"%s\n",tline2);
			//fill text boxes with Accel Motor is using info
			sprintf(tstr, "lblRobotModel_%sUseAccel",MotorName);
			tc = GetFTControl(tstr);
			if (tc != 0) {
				tlen=strlen(tline2);
				GetTextUpToString(&pcbname[16+tlen], "Angle[",tline2);  //get up to UseAngle number
	
				strcpy(tstr,&pcbname[22+tlen+strlen(tline2)]);
				tstr[strlen(tstr)-1]=0; //remove last )
				//strncpy(tstr,&pcbname[14+tlen+strlen(tline2)+6],(tc->x2-tc->x1)/tc->fontwidth);
				if (tstr[0]==48) { //0
					tnum=0;
					tc->textcolor[0] = 0x009000;
				} else {
					tnum=1;
					tc->textcolor[0] = 0x009090;
				} 
				sprintf(tc->text, "%d:%s",tnum,&tstr[4]);
				//fprintf(stderr,"text: %s",tc->text);
				DrawFTControl(tc);
			} //if (tc!=0)
		} //if (!strncmp(pcbname, "Initial", 7)) {


		if (!strncmp(pcbname, "TargetAngle",11)) { //ex: "TargetAngle updated: Left Ankle Motor Accel[0] (Left Lower Leg Z -7.0 -8.0 -6.0)"
			//note that pcbname contains the entire string
			//read up to motor name
			//read in Motor Readable Name  //ex:"Left Ankle Motor"
			//fprintf(stderr,"%s\n",pcbname);
			
			GetTextUpToString(&pcbname[21]," Accel[",tline2);

			MotorName=GetMotorNameFromReadableName(tline2);
			//fprintf(stderr,"%s\n",tline2);
			//fill text boxes with Accel Motor is using info
			sprintf(tstr, "lblRobotModel_%sUseAccel",MotorName);
			tc = GetFTControl(tstr);
			if (tc != 0) {
				tlen=strlen(tline2);
				GetTextUpToString(&pcbname[28+tlen], ")",tstr);  //get rest of string from "0] (Right Upper Leg...)"
	
				//fprintf(stderr,"%s\n",tstr);
				//strcpy(tstr,&pcbname[29+tlen+strlen(tline2)]); //get rest of string from "0] (Right Upper Leg...)"
				//tstr[strlen(tstr)-1]=0; //remove last )
				//strncpy(tstr,&pcbname[14+tlen+strlen(tline2)+6],(tc->x2-tc->x1)/tc->fontwidth);
				if (tstr[0]==48) { //0
					tnum=0;
					tc->textcolor[0] = 0x009000;
				} else {
					tnum=1;
					tc->textcolor[0] = 0x009090;
				} 
				sprintf(tc->text, "%d:%s",tnum,&tstr[4]);
				//fprintf(stderr,"text: %s",tc->text);
				DrawFTControl(tc);
			} //if (tc!=0)
		} //if (!strncmp(pcbname, "TargetAngle",12)) {

		//check for "Angle out of range" and "Angle range reached"
	//	if (!strncmp(pcbname, "Angle", 5)) {
		//if (!strncmp(pcbname, "Angle range", 11) || !strncmp(pcbname, "Angle out", 9)) {
		if (!strncmp(pcbname, "Angle in", 8) || !strncmp(pcbname, "Angle out", 9)) {
			//this entry is for an "Angle in range" or "Angle out of range" log entry
			//read next word
			param = strtok(pcbname, " ");
			param = strtok(NULL, " ");
			if (!strncmp(param, "out", 3)) {
				//"Angle out of range"
				param = strtok(NULL, "'"); //remove "of range"  
				param = strtok(NULL, "'"); //get past unique name (ex: 'ACCEL_TORSO_X')
				tlen = strlen(param);
				strncpy(tstr, param, tlen - 2); //-X, Y or Z
				if (param[tlen - 1] == 'X') {
					j = 0;
				}
				else {
					if (param[tlen - 1] == 'Y') {
						j = 1;
					}
					else {
						j = 2;
					}
				} //if (param[tlen-1]=='X') {

				tstr[tlen - 2] = 0;
				//from readable name get regular name
				//AccelName=GetAccelNameFromReadableName(tstr);
				memcpy(AccelName,tstr,tlen-2);
				if (!AccelName) {
					fprintf(stderr,"Could not get Accel name from readable name '%s'\n",tstr);
				} else { //if (!AccelName) {

					//update Angle color
					sprintf(tstr2, "lblRobotModel_%sAngle%d", AccelName, j);
					//fprintf(stderr,"%s\n",tstr2);
					tc = GetFTControl(tstr2);
					if (tc != 0) {
						//tc->color[0] = FT_RED;// 0x9f0000;  //red
						tc->textcolor[0] = 0xffffff;  //white
						//if segment moving toward target color orange otherwise color red
						sprintf(tstr2, "lblRobotModel_%sTargetAngle%d", AccelName, j);
						tc2 = GetFTControl(tstr2); //get TargetAngle
						if (tc2 != 0) {
							//if Target<Segment and gyro<0 or Target>Segment and gyro>0 then orange (correct dir)
							//angle and gyro need to be retrieved here, because this log entry does not contain accel/gyro info
							sprintf(tstr2, "lblRobotModel_%sAngle%d", AccelName, j);
							tc3 = GetFTControl(tstr2); //get TargetAngle
							if (tc3 != 0) {
								angle.i[j] = atof(tc3->text);
								sprintf(tstr2, "lblRobotModel_%sGyro%d", AccelName, j);
								tc3 = GetFTControl(tstr2); //get TargetAngle
								if (tc3 != 0) {
									gyro.i[j] = atof(tc3->text);

									if ((atof(tc2->text) < angle.i[j] && gyro.i[j] < 0.0) || (atof(tc2->text) > angle.i[j] && gyro.i[j] > 0.0)) {
										tc->color[0] = FT_ORANGE;
									}
									else {
										tc->color[0] = FT_RED;
									}
								} //if (tc3!=0)
							} //if (tc3!=0)
						} //if (tc2!=0)

						DrawFTControl(tc);
					} //if (tc!=0)

					//update Target Angle
					sprintf(tstr2, "lblRobotModel_%sTargetAngle%d", AccelName, j);
					//fprintf(stderr,"%s\n",tstr2);
					tc = GetFTControl(tstr2);
					if (tc != 0) {
						//angle is text between < <
						param = strtok(NULL, "<");
						param = strtok(NULL, "<");
						//sprintf(tc->text, "%s", param);
						sprintf(tc->text, "%03.1f", atof(param));
						DrawFTControl(tc);
					}
				} //else if (!AccelName) {

			}	else { //if (!strncmp(param, "out", 3)) {
				//if (!strncmp(param, "range", 5)) {
				if (!AccelName) {
					fprintf(stderr,"Could not get Accel name from readable name '%s'\n",tstr);
				} else { //if (!AccelName) {

					if (!strncmp(param, "in", 2)) {
						//"Angle range reached"
						param = strtok(NULL, "'"); //get up to first single quote
						param = strtok(NULL, "'"); //get accelerometer name
						//param now has something like 'Left Lower Leg X'
						tlen = strlen(param);
						strncpy(tstr, param, tlen - 2); //-X, Y or Z
						if (param[tlen - 1] == 'X') {
							j = 0;
						}
						else {
							if (param[tlen - 1] == 'Y') {
								j = 1;
							}
							else {
								j = 2;
							}
						} //if (param[tlen-1]=='X') {

						//param = strtok(NULL, "'"); //get to readable name 
						//param = strtok(NULL, "'"); //param now has readable name (ex: "Left Lower Leg X")

						tstr[tlen - 2] = 0;
						//AccelName=GetAccelNameFromReadableName(tstr);
						memcpy(AccelName,tstr,tlen-2);

						sprintf(tstr2, "lblRobotModel_%sAngle%d", AccelName, j);
						//fprintf(stderr,"%s\n",tstr2);
						tc = GetFTControl(tstr2);
						if (tc != 0) {
							tc->color[0] = 0x009f00;  //green
							tc->textcolor[0] = 0xffffff;  //white
							DrawFTControl(tc);
						}

						//update Target Angle
						sprintf(tstr2, "lblRobotModel_%sTargetAngle%d", AccelName, j);
						//fprintf(stderr,"%s\n",tstr2);
						tc = GetFTControl(tstr2);
						if (tc != 0) {
							//angle is text between < <
							param = strtok(NULL, "<");
							param = strtok(NULL, "<");
							sprintf(tc->text, "%03.1f", atof(param));
							DrawFTControl(tc);
						}
					} //if (!AccelName) {
				} //if (!strncmp(param, "in", 2)) {  //if (!strncmp(param, "range", 5)) {
			} //if (!strncmp(param, "out", 3)) {
		} //if (!strncmp(pcbname, "Angle in",11) || !strncmp(pcbname, "Angle out",9) ) { //if (!strncmp(pcbname, "Angle range",11) || !strncmp(pcbname, "Angle out",9) ) {
	//	} //if (!strncmp(pcbname, "Angle", 5)) {

		if (!strncmp(pcbname, "START STAGE", 11)) {
			//Found start of stage, update label
			sprintf(tstr, "txtRobotModel_Stage");
			tc = GetFTControl(tstr);
			if (tc != 0) {
				//Get Stage number
				strcpy(tstr2, &pcbname[12]);
				tstr2[strlen(pcbname) - 11] = 0;  //remove tab
				//sprintf(tc->text, "Stage: %s", tstr2);
				sprintf(tc->text, "%s", tstr2);
				DrawFTControl(tc);
			} //tc!=0

			//set StageNumber, StartStageTime and StageTimeElapsed
			RMInfo.StageNumber=atoi(tstr2);
			RMInfo.StartStageTime=RMInfo.EventTime;
			RMInfo.StageTimeElapsed=0;
			sprintf(RMInfo.tc_stime->text, "Stage: 0 ms");
			DrawFTControl(RMInfo.tc_stime);

			//clear background colors of angles
			//because thrust can remain, ClearAccelAndMotorBackgroundColors(); //clear all accel and motor background colors
		} //if (!strncmp(pcbname,"START STAGE",11)) {

		if (!strncmp(pcbname, "END STAGE", 9)) {
			//Found end of stage, update label - erase stage number (helps to know when script is done)
			sprintf(tstr, "txtRobotModel_Stage");
			tc = GetFTControl(tstr);
			if (tc != 0) {
				strcpy(tstr2, &pcbname[10]);
				tstr2[strlen(pcbname) - 9] = 0;  //remove tab
				//sprintf(tc->text, "Stage: %s Done", tstr2);
				sprintf(tc->text, "%s Done", tstr2);
				DrawFTControl(tc);
			} //tc!=0
		} //if (!strncmp(pcbname,"END STAGE",9)) {


		if (!strncmp(pcbname, "Changed Tar", 11)) {
			//ex: "Changed TargetAngle 'Left Lower Leg X' 'Left Foot Motor' 0.04 (-15.83<-14.83<-13.83) Offset=-1.67"

			//get Accel and xyz
			param = strtok(pcbname, " ");
			param = strtok(NULL, "'");
			param = strtok(NULL, "'");
			//param now has something like "Left Lower Leg X"
			tlen = strlen(param);
			strncpy(tstr, param, tlen - 2); //-X, Y or Z
			if (param[tlen - 1] == 'X') {
				j = 0;
			}
			else {
				if (param[tlen - 1] == 'Y') {
					j = 1;
				}
				else {
					j = 2;
				}
			} //if (param[tlen-1]=='X') {

			tstr[tlen - 2] = 0;
			//AccelName=GetAccelNameFromReadableName(tstr);
			memcpy(AccelName,tstr,tlen-2);

			if (!AccelName) {
				fprintf(stderr,"Could not get Accel name from readable name '%s'\n",tstr);
			} else { //if (!AccelName) {

				//update Target Angle
				sprintf(tstr2, "lblRobotModel_%sTargetAngle%d", AccelName, j);
				//fprintf(stderr,"%s\n",tstr2);
				tc = GetFTControl(tstr2);
				if (tc != 0) {
					//angle is text between < <
					param = strtok(NULL, "<");
					param = strtok(NULL, "<");
					sprintf(tc->text, "%03.1f", atof(param));
					DrawFTControl(tc);
				} //if (tc !=0 )
			} //if (!AccelName) {					
		} //if (!strncmp(pcbname,"Changed Tar",11)) {
	} //if (pcbname!=0) {
	return(1);
} //int ProcessLogLine(char *tline) {

int btnRobotModel_StopLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	if (!RMInfo.fRobotLoadedLog) {
//		return(0);
//	}

	RMInfo.flags&=~ROBOT_MODEL_INFO_PLAYING_LOG;
	return(1);
} //int btnRobotModel_StopLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int btnRobotModel_RewindLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{

	RewindLogFile();
	return(1);
} //int btnRobotModel_RewindLogFile_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

int RewindLogFile(void) 
{
MODEL *lmodel;
int i,j;
char tstr[512];
FTControl *tc;

	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

	//go back to the beginning of log file
#if Linux
	fseek(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
#if WIN32
	_fseeki64(RMInfo.fRobotLoadedLog, 0, SEEK_SET);
#endif
	

	//RMInfo.LogFilePos=0;
	RMInfo.LastStepDir = 0; //reset last step direction
	RMInfo.ibuf = 0; //set current position in buffer to 0
	RMInfo.endbuf = 0; //set end buffer to 0
//	RMInfo.LineNum=0;

	//reset Robot Model axes 
	lmodel=GetModel("RobotModel");
	if (lmodel) {
//		for(i=0;i<lmodel->numaxis;i++) {
		for(i=1;i<lmodel->numaxis;i++) {
			MakeIdentityMatrix(&lmodel->axis[i].m);
		} //for i
	}  //if (lmodel)

	//redraw Robot model
	RenderModelMainImage();

	RMInfo.TimeElapsed=0;  //log time elapsed in ms
	sprintf(RMInfo.tc_time->text,"Time: 0 ms");
	DrawFTControl(RMInfo.tc_time);

	RMInfo.StageTimeElapsed=0;  //stage time elapsed in ms
	sprintf(RMInfo.tc_stime->text,"Stage: 0 ms");
	DrawFTControl(RMInfo.tc_stime);


	RMInfo.tc_log->text[0]=0;
	DrawFTControl(RMInfo.tc_log);

	//reset Stage: textbox
	sprintf(tstr, "txtRobotModel_Stage");
	tc=GetFTControl(tstr);
	if (tc!=0) {
		tc->text[0]=0;//sprintf(tc->text,"Stage:");
		DrawFTControl(tc);
	} 

	ResetAccelAndMotorColors(); //reset all accel and motor values and colors

	return(1);
} //int RewindLogFile(void)

//Close Robot Model window
int btnRobotModel_Close_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{

	CloseFTWindow(twin);

	return(1);
} //int btnRobotModel_Close_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Convert a string timestamp into milliseconds - for now just the hour, minute and second, but eventually since 1/1/1900
//TimeStamp looks like YYYYMMDD_HHMMSS_MMM
uint64_t ConvertTimeStampToMS(char *ts)
{
	char tstr[5];
	int year, mon, day, hour, min, sec, ms;

	strncpy(tstr, ts, 4);
	tstr[4] = 0;
	year = atoi(tstr);
	strncpy(tstr, ts+4, 2);
	tstr[2] = 0;
	mon = atoi(tstr);
	strncpy(tstr, ts + 6, 2);
	tstr[2] = 0;
	day = atoi(tstr);

	strncpy(tstr, ts + 9, 2);
	tstr[2] = 0;
	hour = atoi(tstr);
	strncpy(tstr, ts + 11, 2);
	tstr[2] = 0;
	min = atoi(tstr);
	strncpy(tstr, ts + 13, 2);
	tstr[2] = 0;
	sec = atoi(tstr);

	strncpy(tstr, ts + 16, 3);
	tstr[3] = 0;
	ms = atoi(tstr);

	return(hour*3600000+min*60000+sec*1000+ms);
}


int txtRobotModel_PlayDelay_OnChange(FTWindow *twin,FTControl *tcontrol) 
{
	RMInfo.PlayDelay=atof(tcontrol->text);
	return(1);
} //int txtRobotModel_PlayDelay_OnChange(FTWindow *twin,FTControl *tcontrol)


int chkRobotModel_RealTimeRender_OnChange(FTWindow *twin,FTControl *tcontrol) 
{
	if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
		RMInfo.flags&=~ROBOT_MODEL_INFO_RENDER_IN_REALTIME;
	} else {
		RMInfo.flags|=ROBOT_MODEL_INFO_RENDER_IN_REALTIME;
	}
	return(1);
} //int chkRobotModel_RealTimeRender_OnChange(FTWindow *twin,FTControl *tcontrol)


int ReplaceTabWithSpace(char *tline) 
{
	int len,i;

	len=strlen(tline);
	for(i=0;i<len;i++) {
		if (tline[i]==9 ||tline[i]==10) {
			tline[i]=32;
		} 
	} //for i

	return(1);
} //int ReplaceTabWithSpace(char **tline) 


//Skip forward to stage start or end
int SkipToStage(int StageNum)
{
	int Done;
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE),
	unsigned int *endbuf; //pointer to end of buffer
	uint64_t filepos;
	char tstr[1024],NumStr[20];
	//char *lbuf;

	ibuf=&RMInfo.ibuf; //shortcut pointer to log file RAM buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to end of log file buffer
	//lbuf=(char *)&RMInfo.buffer; //shortcut to log file buffer

	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

	if (StageNum>-1) {
		//user gave specific stage to go to
		sprintf(NumStr,"%d",StageNum);  //used below in while
	
		//compare to the current stage and rewind if user is requesting an earlier stage than the current stage
		//for now just rewind- scripts currently do not run for a long time
		RewindLogFile();
	} //if (StageNum>-1) {
	

	//todo: just change to look at RMInfo.Entry
	Done=0;
	RMInfo.flags|=ROBOT_MODEL_INFO_PLAYING_LOG; //so user can stop
	while(!Done && (RMInfo.flags&ROBOT_MODEL_INFO_PLAYING_LOG)) {
#if Linux
		filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
		filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
		if (*ibuf == *endbuf && filepos >= RMInfo.LogFileSize) { //reach end of buffer and file
			Done = 1;

		} else {

			FT_GetInput(); //or else clicking stop button does not happen, stop button can clearn ROBOT_MODEL_INFO_PLAYING_LOG flag 

			//skip a frame forward
			btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);

			//+20 to skip over timestamp ex: 20170227_155026_638
			//use ibuf since when ProcessLogLine is not called, no text is copied to tc_log FTControl
//			if (!strncmp(RMInfo.tc_log->text+20, "START STAGE",11) || !strncmp(RMInfo.tc_log->text+20, "END STAGE",9)) { 
			if (!strncmp(RMInfo.LogLine+20, "START STAGE",11) || !strncmp(RMInfo.LogLine+20, "END STAGE",9) || !strncmp(RMInfo.LogLine+20, "Abort",5)) { 
				if (StageNum<0) {
					//skip a frame forward to go to the Stage entry and refresh the display
					RMInfo.flags&=~ROBOT_MODEL_INFO_SKIP_PROCESS; //clear flag
					//btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);
					Done = 1;  //No particvular stage requested by user
				} else {
					sprintf(tstr,"START STAGE %s",NumStr);
					//if (!strncmp(RMInfo.tc_log->text+20,tstr,12+strlen(NumStr))) {
					if (!strncmp(RMInfo.LogLine+20,tstr,12+strlen(NumStr))) {
						//skip a frame forward to go to the Stage entry and refresh the display
						RMInfo.flags&=~ROBOT_MODEL_INFO_SKIP_PROCESS; //clear flag
						//btnRobotModel_StepLogFileBackward_Click(0, 0, 0, 0);
						//btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);
						ProcessLogLine(RMInfo.LogLine);	//update display
						//found start of stage requested by user
						Done=1;
					} //if (!strcmp(RMInfo.tc_log->text+20,tstr))
				} //if (StageNum<0) {
			} //if (!strncmp(lbuf+*ibuf+20, "START STAGE",11) || !strncmp(lbuf+*ibuf+20, "END STAGE",9)) { 



		} //if (*ibuf == *endbuf && filepos >= RMInfo.LogFileSize) {

	} //while(!Done)

	return(1);
} //int SkipToStage(int StageNum)



//Skip forward one sample (10ms). dir=1 skip forward 10ms, dir=0 skip backward 10ms
int Skip10ms(int dir)
{
	int Done,NumDiff;
	unsigned int *ibuf; // pointer to index into log buffer (index into current read in buffer chunk of LOGBUFFERSIZE),
	unsigned int *endbuf; //pointer to end of buffer
	uint64_t filepos;
	char LastTimeStamp[20];
	uint64_t TS1,TS2;

	ibuf=&RMInfo.ibuf; //shortcut pointer to log file RAM buffer
	endbuf = &RMInfo.endbuf; //shortcut to point to end of log file buffer

	if (!RMInfo.fRobotLoadedLog) {
		return(0);
	}

	LastTimeStamp[0]=0;
	//todo: just change to look at RMInfo.Entry
	Done=0;
	while(!Done) {
#if Linux
		filepos = ftell(RMInfo.fRobotLoadedLog);
#endif
#if WIN32
		filepos = _ftelli64(RMInfo.fRobotLoadedLog);
#endif
		if ((dir && *ibuf == *endbuf && filepos >= RMInfo.LogFileSize) || 
					(!dir && *ibuf== 0))  { //reach end of buffer and file skipping forward, or at start and skipping backward
			Done = 1;

		} else {

//			FT_GetInput(); //or else clicking stop button does not happen, stop button can clearn ROBOT_MODEL_INFO_PLAYING_LOG flag 

			if (LastTimeStamp[0]==0) {
				strncpy(LastTimeStamp,RMInfo.tc_log->text,19); //get a timestamp
				TS1=ConvertTimeStampToMS(LastTimeStamp);
				if (dir) {
					btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);
				} else {
					//btnRobotModel_StepLogFileBackward_Click(0, 0, 0, 0);
				}
			} else {
				TS2=ConvertTimeStampToMS(RMInfo.tc_log->text);
				//if 10ms, skip over timestamp ex: 20170227_155026_638 to _648
				NumDiff=TS2-TS1;
				if ((dir && (NumDiff>=10)) || (!dir && ((NumDiff<=-10) || TS2==0))) {  //time stamp is greater than 10 ms  
					Done = 1;
				} else { //if ((dir && (NumDiff>=10)) || (!dir && (NumDif<=-10))) {
					if (dir) {
						//skip a frame forward
						btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);
					} else {
						//skip a frame backward
						btnRobotModel_StepLogFileBackward_Click(0, 0, 0, 0);
					}
				} //else if ((dir && (NumDiff>=10)) || (!dir && (NumDif<=-10))) {
			} //if (LastTimeStamp[0]==0) {


	
		} //if (*ibuf == *endbuf && filepos >= RMInfo.LogFileSize) {

	} //while(!Done)

	return(1);
} //int Skip10ms(int dir)


void ResetAccelAndMotorColors(void) {

int i,j,k,NumAnalogSensors,tlen,CurIndex,AnalogSensorNameNum,AnalogSensorNum;
FTControl *tc;
char tstr[512],*Name,*EthAccelsName,AnalogSensorName[1024];


	//reset all accel and motor values and colors

	//Go through all Model.conf entries
	for(i=0;i<RStatus.ModelConf.NumEntries;i++) {
		//for each entry
		//determine if Accel, Motor, or Analog Sensor
		Name=RStatus.ModelConf.Entry[i].Name;

		if (!strncmp(Name,"ACCEL",5)) {
			for(j=0;j<3;j++) {
				sprintf(tstr, "lblRobotModel_%sAngle%d",Name,j);
				tc=GetFTControl(tstr);
				if (!tc) {
					fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
				} else {
					sprintf(tc->text,"0");
					tc->color[0]=0xdfdfdf;  //gray any green or red color background
					tc->textcolor[0]=0x0000ff;  //blue
					DrawFTControl(tc);
				}

				sprintf(tstr, "lblRobotModel_%sTargetAngle%d",Name,j);
				tc=GetFTControl(tstr);
				if (!tc) {
					fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
				} else {
					sprintf(tc->text,"*");
					tc->textcolor[0]=0x005f00;  //green;
					DrawFTControl(tc);
				}

				sprintf(tstr, "lblRobotModel_%sAngled%d",Name,j);
				tc=GetFTControl(tstr);
				if (!tc) {
					fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
				} else {
					sprintf(tc->text,"0");
				}
				DrawFTControl(tc);

				sprintf(tstr, "lblRobotModel_%sGyro%d",Name,j);
				tc=GetFTControl(tstr);
				if (!tc) {
					fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
				} else {
					sprintf(tc->text,"0");
				}
				DrawFTControl(tc);

				sprintf(tstr, "lblRobotModel_%sGyrod%d",Name,j);
				tc=GetFTControl(tstr);
				if (!tc) {
					fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
				} else {
					sprintf(tc->text,"0");
					DrawFTControl(tc);
				}
			} //for j
		} //if (!strncmp(Name,"ACCEL",5)) {

		//reset all touch sensor values
		if (!strncmp(Name,"TOUCH",5)) {

			NumAnalogSensors=1;
			k=0;
			while(k<strlen(Name)) {
				if (Name[k]==124) {  //124=|
					NumAnalogSensors++;
				}
				k++;
			}	//while(k<strlen(Name))		

			//Get EthAccels PCB Name
			tlen=GetWordLen(Name,124); //124=|
			if (tlen>1024) {
				tlen=1024;
			}
			strncpy(AnalogSensorName,Name,tlen);
			AnalogSensorName[tlen]=0;
			EthAccelsName=GetAnalogSensorPCBName(AnalogSensorName);

			CurIndex=0;
			for(j=0;j<NumAnalogSensors;j++) {
				//Get AnalogSensor name
				tlen=GetWordLen(&Name[CurIndex],124);
				if (tlen>1024) {
					tlen=1024;
				}
				strncpy(AnalogSensorName,&Name[CurIndex],tlen);
				AnalogSensorName[tlen]=0;
				CurIndex=tlen+1; //go to next AnalogSensor
				//get AnalogSensor NameNum
				AnalogSensorNameNum=GetAnalogSensorNameNum(AnalogSensorName);
				if (AnalogSensorNameNum<0) {
					fprintf(stderr,"GetAnalogSensorNameNum returned -1 for Analog Sensor Name %s\n",AnalogSensorName);
				} else {
					//Get actual Analog Sensor number, for that need touch sensor name
					AnalogSensorNum=RStatus.AnalogSensorInfo[AnalogSensorNameNum].Num;


					sprintf(tstr, "lblRobotModel_%sTouch%d",EthAccelsName,AnalogSensorNum);
					tc=GetFTControl(tstr);
					if (!tc) {
						fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
					} else {
						sprintf(tc->text,"0");
						DrawFTControl(tc);
					}

					sprintf(tstr, "lblRobotModel_%sTouchd%d",EthAccelsName,AnalogSensorNum);
					tc=GetFTControl(tstr);
					if (!tc) {
						fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
					} else {
						sprintf(tc->text,"0");
					}
					DrawFTControl(tc);
				} //if (AnalogSensorNameNum<0) {
			} //for j
		} //if (!strncmp(Name,"TOUCH",5)) {

		//reset thrust and color for Motors
		if (!strncmp(Name,"MOTOR",5)) {
			//sprintf(tstr,"lblRobotModel_%s",GetMotorReadableName(Name));
			sprintf(tstr,"lblRobotModel_%s",Name);
			tc=GetFTControl(tstr);
			if (!tc) {
				fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
			} else {
				tc->color[0]=0xdfdfdf;
				DrawFTControl(tc);
			} 

			sprintf(tstr,"lblRobotModel_%sThrust",Name);
			tc=GetFTControl(tstr);
			if (!tc) {
				fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
			} else {
				sprintf(tc->text,"0");
				tc->textcolor[0]=0x3f3f3f;
				DrawFTControl(tc);
			}

			sprintf(tstr,"lblRobotModel_%sUseAccel",Name);
			tc=GetFTControl(tstr);
			if (!tc) {
				fprintf(stderr,"GetFTControl failed on control name %s\n",tstr);
			} else {
				tc->text[0]=0;
				DrawFTControl(tc);
			}

		} //if (!strncmp(Name,"MOTOR",5)) {

	} //for i	

} //void ResetAccelAndMotorColors(void) {

void ClearAccelAndMotorBackgroundColors(void)
{
int i,j;
FTControl *tc;
char tstr[512],*Name;

	//reset all accel and motor values and colors

	//Go through all Model.conf entries
	for(i=0;i<RStatus.ModelConf.NumEntries;i++) {
		//for each entry
		//determine if Accel, Motor, or Analog Sensor
		Name=RStatus.ModelConf.Entry[i].Name;

		if (!strncmp(Name,"ACCEL",5)) {
			for(j=0;j<3;j++) {
				sprintf(tstr, "lblRobotModel_%sAngle%d",Name,j);
				tc=GetFTControl(tstr);
				tc->color[0]=0xdfdfdf;  //gray any green or red color background
				tc->textcolor[0]=0x0000ff;  //blue
				DrawFTControl(tc);

				sprintf(tstr, "lblRobotModel_%sTargetAngle%d",Name,j);
				tc=GetFTControl(tstr);
				tc->textcolor[0]=0x005f00;  //green;
				DrawFTControl(tc);
			} //for j
		} //if (!strncmp(Name,"ACCEL",5)) {


	//reset thrust and color for Motors
		if (!strncmp(Name,"MOTOR",5)) {
			sprintf(tstr,"lblRobotModel_%s",Name);
			tc=GetFTControl(tstr);
			tc->color[0]=0xdfdfdf;
			DrawFTControl(tc);

			sprintf(tstr,"lblRobotModel_%sThrust",Name);
			tc=GetFTControl(tstr);
			tc->textcolor[0]=0x3f3f3f;
			DrawFTControl(tc);
		} //if (!strncmp(Name,"MOTOR",5)) {
	} //for i

} //ClearAccelAndMotorBackgroundColors(coid)


int GetTextUpToString(char *src,char *match,char *dest) 
{
	int srclen,matchlen,cur,FoundMatch;

	srclen=strlen(src);
	matchlen=strlen(match);
	cur=0;
	FoundMatch=0;

	while(!FoundMatch && (cur+matchlen<srclen+1)) {
		if (!strncmp(&src[cur],match,matchlen)) {
			//found match
			dest[cur]=0; //terminal dest string	
			FoundMatch=1;  //end while
			//fprintf(stderr,"Found match: %s\n",dest);
		} else { //if (!strncmp
			dest[cur]=src[cur]; //copy character to dest
			cur++;
		} //if (!strn
	} //while
	
	return(0); //no match

} //int GetTextUpToString(char *src,char *match,*dest) {


//Go to Stage button - quickly skip forward to a stage
int btnRobotModel_Go_To_Stage_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTControl *tc;

	tc=GetFTControl("txtRobotModel_Stage");
	if (tc!=0) {

		RMInfo.flags|=ROBOT_MODEL_INFO_SKIP_PROCESS; //set flag
		SkipToStage(atoi(tc->text));
		//skip a frame forward to refresh the display
//		btnRobotModel_StepLogFileForward_Click(0, 0, 0, 0);
		RMInfo.flags&=~ROBOT_MODEL_INFO_SKIP_PROCESS; //clear flag
		twin->FocusFTControl=0; //so keys can move robot model and frames forward

		return(1);
	} //if (tc!=0)
 
return(0);
} //int btnRobotModel_Go_To_Stage_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
