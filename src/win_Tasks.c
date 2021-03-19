//win_Tasks.c
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

#include "freethought.h"
#include "Robot.h"
#include "win_Tasks.h"
#include "win_Motors.h"
#include "win_Accels.h"
#include "win_Load_Script.h"
#include "win_RobotModel.h"
#include "robot_accelmagtouchgps_pic_instructions.h"
#include "robot_motor_pic_instructions.h"

#include <errno.h>

#if USE_RTAI
#include <rtai_lxrt.h>
#endif //USE_RTAI

#if USE_RT_PREEMPT
#define MOTOR_QUEUE 1  //using TurnMotor thread and queue
#else
#define MOTOR_QUEUE 0  //just sending TurnMotor immediately
#endif //USE_RT_PREEMPT


extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern RobotModelInfo RMInfo; //Robot Model Info 

RobotTaskEngine iTaskEngine; //task thread variables

const char *XYZ[3] = { "X","Y","Z"}; //used for logging


int winTasks_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winTasks");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winTasks");
	strcpy(twin.title,"Tasks");
	strcpy(twin.ititle,"Tasks");
	twin.x=640;
	twin.y=560;
	twin.w=580;
	twin.h=300;
	twin.Keypress=(FTControlfuncwk *)winTasks_key;
	twin.AddFTControls=(FTControlfuncw *)winTasks_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winTasks_OnOpen;
//	twin.OnClose=(FTControlfuncw *)winTasks_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winTasks_AddFTWindow



int winTasks_AddFTControls(FTWindow *twin)
{
	int fw,fh;
	FTControl *tcontrol;
	int OptGroupNum;

#if WIN32
	DWORD dwWaitResult;
#endif

	if (PStatus.flags&PInfo) {
		if (!twin) {
			fprintf(stderr,"Adding controls for window '%s'\n",twin->name);
		} else {
			fprintf(stderr,"Error: twin==0 in winTasks_AddFTControls()\n");
			return(0);
		}
	}

#if WIN32
	//lock the GetInput thread while creating the window controls
	//not really needed, except when called from a thread- and AddFTControl functions shouldn't be called from a thread
	 dwWaitResult = WaitForSingleObject( 
		          PStatus.GetInputMutex,    // handle to mutex
		          INFINITE);  // no time-out interval
	 
	 if (dwWaitResult!=WAIT_OBJECT_0) {
		 fprintf(stderr,"Could not lock GetInputMutex");
		 //return(0);
	 }
#endif
	//get font from window

	fw=twin->fontwidth;
	fh=twin->fontheight;

	tcontrol=(FTControl *)malloc(sizeof(FTControl));

	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLoadScript");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh;
	tcontrol->x2=fw*10;
	tcontrol->y2=fh*5;
	strcpy(tcontrol->text,"Load Script");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLoadScript_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnUnloadScript");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*11;
	tcontrol->y1=fh;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*5;
	strcpy(tcontrol->text,"Unload Script");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnUnloadScript_Click;
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRunScript");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh;
	tcontrol->x2=fw*30;
	tcontrol->y2=fh*5;
	strcpy(tcontrol->text,"Run Script");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRunScript_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnReloadScript");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*31;
	tcontrol->y1=fh;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*5;
	strcpy(tcontrol->text,"Reload Script");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnReloadScript_Click;
	AddFTControl(twin,tcontrol);


	//todo: UnloadScript

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblScript");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh;
	tcontrol->y2=fh*2+fh/2;
	strcpy(tcontrol->text,"scripts:");
	AddFTControl(twin,tcontrol);

	//ddScripts is underneath lblScript
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"ddScripts");
	tcontrol->type=CTDropdown;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh*3;
	tcontrol->x2=fw*80;
	tcontrol->y2=fh*4+fh/2;
	tcontrol->flags|=CGrowX2;
//	tcontrol->OnChange=(FTControlfuncwc *)ddScripts_OnChange;
	AddFTControl(twin,tcontrol);


#if 0 
	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnBalance");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*10;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Start Balancing");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnBalance_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeftStep");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*11;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Left Step");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeftStep_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRightStep");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*30;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Right Step");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRightStep_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeanLeft");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*31;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Lean Left");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeanLeft_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeanRight");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*50;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Lean Right");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeanRight_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeanForward");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*51;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*60;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Lean Forward");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeanForward_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeanBackward");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*61;
	tcontrol->y1=fh*6;
	tcontrol->x2=fw*70;
	tcontrol->y2=fh*10;
	strcpy(tcontrol->text,"Lean Backward");
	//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeanBackward_Click;
	AddFTControl(twin,tcontrol);
#endif 
/*
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLeftKick");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*31;
	tcontrol->y1=fh*27;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*31;
	strcpy(tcontrol->text,"Left Kick");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLeftKick_Click;
	AddFTControl(twin,tcontrol);
*/


	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"optAccelAcqInt");
	tcontrol->type=CTOption;
	tcontrol->OptionGroup=GetFreeOptionGroupNumber(twin); 
	OptGroupNum=tcontrol->OptionGroup;
	tcontrol->x1=fw;
	tcontrol->y1=fh*11;
	tcontrol->x2=fw*8;
	strcpy(tcontrol->text,"Intr");
//	tcontrol->OnChange=(FTControlfuncwc *)OptAccelAcqInt_OnChange;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"optAccelAcqPoll");
	tcontrol->type=CTOption;
	tcontrol->OptionGroup=OptGroupNum;
	tcontrol->x1=fw;
	tcontrol->y1=fh*12+fh/2;
	tcontrol->x2=fw*8;
	strcpy(tcontrol->text,"Poll");
	tcontrol->value=1; //set by default
//	tcontrol->OnChange=(FTControlfuncwc *)OptAccelAcqPoll_OnChange;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"optAccelAcqSingle");
	tcontrol->type=CTOption;
	tcontrol->OptionGroup=OptGroupNum;
	tcontrol->x1=fw;
	tcontrol->y1=fh*14;
	tcontrol->x2=fw*8;
	strcpy(tcontrol->text,"Single");
//	tcontrol->OnChange=(FTControlfuncwc *)OptAccelAcqSingle_OnChange;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnStartAllAccelAndTouch");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*11;
	tcontrol->y1=fh*11;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*15;
	strcpy(tcontrol->text,"Start All Accel and Touch");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnStartAllAccelAndTouch_Click;
	AddFTControl(twin,tcontrol);



	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnStartAllAccel");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*11;
	tcontrol->x2=fw*30;
	tcontrol->y2=fh*15;
	strcpy(tcontrol->text,"Start All Accel");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnStartAllAccel_Click;
	AddFTControl(twin,tcontrol);




	//newline
	/*
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnStand");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*16;
	tcontrol->x2=fw*10;
	tcontrol->y2=fh*20;
	strcpy(tcontrol->text,"Stand");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnStand_Click;
	AddFTControl(twin,tcontrol);
	*/

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnwinTasksClose");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*48;
	tcontrol->y1=fh*16;
	tcontrol->x2=fw*68;
	tcontrol->y2=fh*20;
	strcpy(tcontrol->text,"Close");  
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinTasksClose_Click;
	AddFTControl(twin,tcontrol);

	if (tcontrol!=0) {
		free(tcontrol);
	}


#if WIN32
//release ownership of the GetInput Mutex
 if (! ReleaseMutex(PStatus.GetInputMutex)) {
     // Handle error.
	fprintf(stderr,"Could not release GetInputMutex");
  } 

#endif


if (PStatus.flags&PInfo)
  fprintf(stderr,"done with adding controls to outfile.\n");

//#endif
return 1;
} //end Robot_AddFTControls


void winTasks_key(FTWindow *twin,KeySym key)
{

#if 0 
    switch (key)	{
			case XK_Shift_L://-30:
			case XK_Shift_R: //-31:  //ignore shift keys
        break;
      case XK_Escape:	/*esc*/
				//stop balancing
				if (twin->FocusFTControl==0) {  //esc key and no control has focus
					//PStatus.flags|=PExit;
					StopBalancing();
				}	     
        break;

    default:
#endif 
//			fprintf(stderr,"key=%x\n",key);
  		//fprintf(stderr,"key: %c\n",(char)key);
			//set Task Engine flag for thread 
			iTaskEngine.flags|=ROBOT_TASK_ENGINE_KEYPRESS;
			iTaskEngine.key=key;

    //break;
    //}
} //void winTasks_key(FTWindow *twin,KeySym key)

int winTasks_OnOpen(FTWindow *twin)
{
	FTControl *tcontrol;


	//note the MAC label and IP controls must be created by the time this functions is called in order to fill them
	tcontrol=GetFTControl("ddScripts");
	if (tcontrol!=0) {
		UpdateScriptList(tcontrol);  //add loaded script names to dropdown control
	}

	//initialize the Task Engine structure
	memset(&iTaskEngine,0,sizeof(RobotTaskEngine));


	return(1);
} //int winTasks_OnOpen(FTWindow *twin)


void btnwinTasksClose_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinTasksClose_Click

//Move into a standing position- presumes that the robot is being held above the ground
int btnStand_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0 //for now
	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;

	lscript=0;

	strcpy(ScriptName,"Stand01");

	//see if the script is already loaded
	lscript=GetRobotScript(ScriptName);
	if (!lscript) {
		//we need to load the script	
		ScriptData=malloc(4096);

		sprintf(ScriptData,"0,StartAccelerometer(ETHACCELS_LEFT_LEG,7,POLLING)\n");
		sprintf(ScriptData,"%s0,StartAccelerometer(ETHACCELS_RIGHT_LEG,7,POLLING)\n",ScriptData);
		sprintf(ScriptData,"%s0,StartAccelerometer(ETHACCELS_TORSO,1,POLLING)\n",ScriptData);
		sprintf(ScriptData,"%s0,StartAnalogSensor(ETHACCELS_LEFT_LEG,0x6,POLLING)\n",ScriptData);
		sprintf(ScriptData,"%s0,StartAnalogSensor(ETHACCELS_RIGHT_LEG,0x6,POLLING)\n",ScriptData);
//		sprintf(ScriptData,"%s0,StartAnalogSensor(ETHACCELS_LEFT_LEG,0x3,POLLING)\n",ScriptData);
//		sprintf(ScriptData,"%s0,StartAnalogSensor(ETHACCELS_RIGHT_LEG,0x3,POLLING)\n",ScriptData);
//foot
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,0,1,2000,ETHACCELS_LEFT_LEG,0,0,BETWEEN,0,-1,1,WAIT_STAGE)\n",ScriptData);
//ankle
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,1,1,2000,ETHACCELS_LEFT_LEG,0,2,BETWEEN,0,-1,1,WAIT_STAGE)\n",ScriptData);
//lower leg
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,2,1,2000,ETHACCELS_LEFT_LEG,1,0,BETWEEN,3.5,-0.5,0.5,WAIT_STAGE)\n",ScriptData);
//upper leg
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,3,1,2000,ETHACCELS_LEFT_LEG,2,0,BETWEEN,-5.5,0.5,0.5,WAIT_STAGE)\n",ScriptData);

		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,6,1,2000,ETHACCELS_RIGHT_LEG,0,0,BETWEEN,0,-1,1,WAIT_STAGE)\n",ScriptData);
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,7,1,2000,ETHACCELS_RIGHT_LEG,0,2,BETWEEN,0,-1,1,WAIT_STAGE)\n",ScriptData);
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,1,2000,ETHACCELS_RIGHT_LEG,1,0,BETWEEN,3.5,-0.5,0.5,WAIT_STAGE)\n",ScriptData);
		sprintf(ScriptData,"%s200,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,1,2000,ETHACCELS_RIGHT_LEG,2,0,BETWEEN,-5.5,-0.5,0.5,WAIT_STAGE)\n",ScriptData);

		ScriptLen=strlen(ScriptData);
		lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
		if (!lscript) {
			fprintf(stderr,"Error loading script %s\n",ScriptName);
		} 
		free(ScriptData);
	} //if (!lscript) {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
	} //if (!lscript) {
#endif 

	return(1);
} //int btnStand_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Lean Left- move right and left lower ankle motors (1 and 7)
//OLD CODE- DO NOT USE
int btnLeanLeft_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0 //for now

	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;

	lscript=0;

	strcpy(ScriptName,"LeanLeft01");

	//see if the script is already loaded
	lscript=GetRobotScript(ScriptName);
	if (!lscript) {
		//we need to load the script	
		ScriptData=malloc(4096);

		sprintf(ScriptData,"200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,2,100)\n");
		sprintf(ScriptData,"%s200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,2,100)\n",ScriptData);

		ScriptLen=strlen(ScriptData);
		lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
		if (!lscript) {
			fprintf(stderr,"Error loading script %s\n",ScriptName);
		} 
		free(ScriptData);
	} //if (!lscript) {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
	} //if (!lscript) {
#endif 

	return(1);
} //int btnLeanLeft_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Lean Right- move right and left lower ankle motors (1 and 7)
int btnLeanRight_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0   //for now
	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;

	lscript=0;

	strcpy(ScriptName,"LeanRight01");

	//see if the script is already loaded
	lscript=GetRobotScript(ScriptName);
	if (!lscript) {
		//we need to load the script	
		ScriptData=malloc(4096);

		sprintf(ScriptData,"200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,-2,100)\n");
		sprintf(ScriptData,"%s200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,-2,100)\n",ScriptData);

		ScriptLen=strlen(ScriptData);
		lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
		if (!lscript) {
			fprintf(stderr,"Error loading script %s\n",ScriptName);
		} 
		free(ScriptData);
	} //if (!lscript) {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
	} //if (!lscript) {
#endif 
	return(1);
} //int btnLeanRight_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Lean Forward- move right and left lower feet motors (0 and 6)
int btnLeanForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0 //for now

	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;


	lscript=0;
	strcpy(ScriptName,"LeanForward01");

	//see if the script is already loaded
	lscript=GetRobotScript(ScriptName);
	if (!lscript) {
		//we need to load the script	
		ScriptData=malloc(4096);

		sprintf(ScriptData,"200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,1,100)\n");
		sprintf(ScriptData,"%s200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,-1,100)\n",ScriptData);

		ScriptLen=strlen(ScriptData);
		lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
		if (!lscript) {
			fprintf(stderr,"Error loading script %s\n",ScriptName);
		} 
		free(ScriptData);
	} //if (!lscript) {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
	} //if (!lscript) {
#endif 

	return(1);
} //int btnLeanForward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Lean Backward- move right and left lower feet motors (0 and 6)
int btnLeanBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0 //for now
	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;

	lscript=0;

	strcpy(ScriptName,"LeanBackward01");

	//see if the script is already loaded
	lscript=GetRobotScript(ScriptName);
	if (!lscript) {
		//we need to load the script	
		ScriptData=malloc(4096);

		sprintf(ScriptData,"200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,-5,100)\n");
		sprintf(ScriptData,"%s200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,5,100)\n",ScriptData);

		ScriptLen=strlen(ScriptData);
		lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
		if (!lscript) {
			fprintf(stderr,"Error loading script %s\n",ScriptName);
		} 
		free(ScriptData);
	} //if (!lscript) {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
	} //if (!lscript) {
#endif 

	return(1);
} //int btnLeanBackward_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Start/Stop all accelerometers and touch sensors
int btnStartAllAccelAndTouch_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	
	if (!strcmp(tcontrol->text,"Start All Accel and Touch")) {
		if (StartAllAccelsAndAnalogSensors(1)) { //1=include touch sensors
				sprintf(tcontrol->text,"Stop All Accel and Touch");
				DrawFTControl(tcontrol);
		} 
	} else {  //	if (!strcmp(tcontrol->text,"Start All Accel and Touch")) {
		if (StopAllAccelsAndAnalogSensors(1)) {  //1=include touch sensors
				sprintf(tcontrol->text,"Start All Accel and Touch");
				DrawFTControl(tcontrol);
		} 
	} //ifelse (!strcmp(tcontrol->text,"Start All Accel and Touch")) {

	return(1);
} //int btnStartAllAccelAndTouch_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

int btnStartAllAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	if (!strcmp(tcontrol->text,"Start All Accel")) {
		if (StartAllAccelsAndAnalogSensors(0)) { //0=exclude touch sensors
				sprintf(tcontrol->text,"Stop All Accel");
				DrawFTControl(tcontrol);
		} 
	} else {  //	if (!strcmp(tcontrol->text,"Start All Accel and Touch")) {
		if (StopAllAccelsAndAnalogSensors(0)) {  //0=exclude touch sensors
				sprintf(tcontrol->text,"Start All Accel");
				DrawFTControl(tcontrol);
		} 
	} //ifelse (!strcmp(tcontrol->text,"Start All Accel and Touch")) {

	return(1);
} //int btnStartAllAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int StartAllAccelsAndAnalogSensors(int IncludeAnalog) {
	RobotScript *lscript;
	FTControl *tc;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;
	MAC_Connection *lmac;
	Robot_PCB *pcb;
	unsigned int mask;

	lscript=0;

	//determine which option is selected: interrupt, polling, single 
	tc=GetFTControl("optAccelAcqInt");
	if (tc!=0 && tc->value)  {

		if (IncludeAnalog) {
			strcpy(ScriptName,"StartAccelTouchRelInterrupt");
		} else {
			strcpy(ScriptName,"StartAccelRelInterrupt");
		}

		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAngles)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StartAccelerometer(%s,%d,RELATIVE_INTERRUPT)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) {

					if (IncludeAnalog) {
						mask=pcb->AccelsPCB.AnalogSensorMask;//mask=(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StartAnalogSensor(%s,%d,RELATIVE_INTERRUPT)\n",ScriptData,pcb->Name,mask);
						}
					} 

				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)

			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			}
			free(ScriptData);

/*				tc=GetFTControl("ddScripts");
			if (tc!=0) {
				UpdateScriptList(tc);
			}
*/

		} //if (!lscript) {

	} //if (tc!=0 && tc->value)  {



	tc=GetFTControl("optAccelAcqPoll");
	if (tc!=0 && tc->value)  {
		if (IncludeAnalog) {
			strcpy(ScriptName,"StartAccelTouchPolling");
		} else {
			strcpy(ScriptName,"StartAccelPolling");
		}

		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAngles)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StartAccelerometer(%s,%d,POLLING)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) 

					if (IncludeAnalog) {
						//not that touch sensors need to be correct or will crash EthAccels currently
						mask=pcb->AccelsPCB.AnalogSensorMask;//0x6;//(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StartAnalogSensor(%s,%d,POLLING)\n",ScriptData,pcb->Name,mask);
						}
					} 

				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)

			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			} 
			free(ScriptData);
		} //if (!lscript) {

	} //if (tc!=0 && tc->value)  {


	tc=GetFTControl("optAccelAcqSingle");
	if (tc!=0 && tc->value)  {

		if (IncludeAnalog) {
			strcpy(ScriptName,"StartAccelTouchSingle");
		} else {
			strcpy(ScriptName,"StartAccelSingle");
		}

		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAccels)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StartAccelerometer(%s,%d,SINGLE_SAMPLE)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) {

					if (IncludeAnalog) {
						mask=pcb->AccelsPCB.AnalogSensorMask;//mask=(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StartAnalogSensor(%s,%d,SINGLE_SAMPLE)\n",ScriptData,pcb->Name,mask);
						}
					} 
				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)

			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			} 
			free(ScriptData);
		} //if (!lscript) {
	} //if (tc!=0 && tc->value)  {

	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
		return(1);
	} //if (!lscript) {


	return(0);
} //int StartAllAccelsAndAnalogSensors(int IncludeAnalog) {


//need more direct function to stop all accels and touchsensors when closed while they are getting samples
int StopAllAccelsAndAnalogSensors(int IncludeAnalog) 
{
	FTControl *tc;
	RobotScript *lscript;
	char ScriptName[255],*ScriptData;
	int ScriptLen;
	RobotTask *ltask;
	MAC_Connection *lmac;
	Robot_PCB *pcb;
	unsigned int mask;


	if (PStatus.flags&PInfo) {
		fprintf(stderr,"StopAllAccelsAndAnalogSensors()\n");
	}

	tc=GetFTControl("optAccelAcqInt");
	if (tc!=0 && tc->value)  {

		if (IncludeAnalog) {
			strcpy(ScriptName,"StopAccelTouchRelInterrupt");
		} else {
			strcpy(ScriptName,"StopAccelRelInterrupt");
		}
		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAccels)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StopAccelerometer(%s,%d,RELATIVE_INTERRUPT)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) {

					if (IncludeAnalog) {
						mask=pcb->AccelsPCB.AnalogSensorMask;//mask=(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StopAnalogSensor(%s,%d,RELATIVE_INTERRUPT)\n",ScriptData,pcb->Name,mask);
						}
					} 
				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)
			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			} 
			free(ScriptData);
		} //if (!lscript) {

		//run the script
		if (lscript) {
			ltask=malloc(sizeof(RobotTask));
			memset(ltask,0,sizeof(RobotTask));
			ltask->kind=ROBOT_TASK_KIND_SCRIPT;
			ltask->iScript=lscript; 
			AddRobotTask(ltask);
		} //if (!lscript) {

	} //	if (tc!=0 && tc->value)  {


	tc=GetFTControl("optAccelAcqPoll");
	if (tc!=0 && tc->value)  {
		if (IncludeAnalog) {
			strcpy(ScriptName,"StopAccelTouchPolling");
		} else {
			strcpy(ScriptName,"StopAccelPolling");
		} 

		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAccels)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StopAccelerometer(%s,%d,POLLING)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) {

					if (IncludeAnalog) {
						mask=pcb->AccelsPCB.AnalogSensorMask;//mask=(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StopAnalogSensor(%s,%d,POLLING)\n",ScriptData,pcb->Name,mask);
						}
					} 
				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)
			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			} 
			free(ScriptData);
		} //if (!lscript) {
	} //if (tc!=0 && tc->value)  {




	tc=GetFTControl("optAccelAcqSingle");
	if (tc!=0 && tc->value)  {
		if (IncludeAnalog) {
			strcpy(ScriptName,"StartAccelTouchSingle");
		} else {
			strcpy(ScriptName,"StartAccelSingle");
		}

		//see if the script is already loaded
		lscript=GetRobotScript(ScriptName);
		if (!lscript) {
			//we need to load the script	
			ScriptData=malloc(4096);
			memset(ScriptData,0,4096);
			//go through all MAC_Connections and and send print StartAccelerometer instruction
		 	lmac=RStatus.iMAC_Connection;
			while(lmac!=0) {
				if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) {
					//this is an EthAccels PCB
					pcb=&lmac->pcb;
					mask=pcb->AccelsPCB.AccelMask;//mask=(1<<pcb->AccelsPCB.NumAccels)-1;
					if (mask>0) {
						sprintf(ScriptData,"%s0,StopAccelerometer(%s,%d,SINGLE_SAMPLE)\n",ScriptData,pcb->Name,mask);
					} //if (mask>0) {

					if (IncludeAnalog) {
						mask=pcb->AccelsPCB.AnalogSensorMask;//mask=(1<<pcb->AccelsPCB.NumAnalogSensors)-1;
						if (mask>0) {
							sprintf(ScriptData,"%s0,StopAnalogSensor(%s,%d,SINGLE_SAMPLE)\n",ScriptData,pcb->Name,mask);
						}
					} 
				} //if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->Name!=0 && !strncmp(lmac->Name,"Accel",5)) 
				lmac=lmac->next;
			} //while (lmac!=0)
			ScriptLen=strlen(ScriptData);
			lscript=LoadRobotScript(ScriptName,ScriptData,ScriptLen);
			if (!lscript) {
				fprintf(stderr,"Error loading script %s\n",ScriptName);
			} 
			free(ScriptData);
		} //if (!lscript) {
	} //if (tc!=0 && tc->value)  {



	//run the script
	if (lscript) {
		ltask=malloc(sizeof(RobotTask));
		memset(ltask,0,sizeof(RobotTask));
		ltask->kind=ROBOT_TASK_KIND_SCRIPT;
		ltask->iScript=lscript; 
		AddRobotTask(ltask);
		return(1);
	} //if (!lscript) {


	return(0);
} //int StopAllAccelsAndAnalogSensors(int IncludeAnalog) 

//Left Step
int btnLeftStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	//for now StepLeft();

	return(1);
} //int btnLeftStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Right Step
int btnRightStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	return(1);
} //int btnRightStep_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Load Script
int btnLoadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;

	twin2=GetFTWindow("winLoadScript");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winLoadScript_AddFTWindow();
	}	

	return(1);
} //int btnLoadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Reload Script
int btnReloadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTControl *tc;
	RobotScript *lscript;
	char tstr[512];

	//get the scripts dropdown control to get the script name
	tc=GetFTControl("ddScripts");
	if (tc!=0) {
		lscript=GetRobotScript(tc->text);
		if (lscript) {
			strcpy(tstr,lscript->filename); //need to copy to temp storage because lscript gets deleted
			lscript=LoadRobotScriptFile(tstr);  //note that LoadRobotScriptFile unloads and frees loaded script, and mallocs space for the script being loaded again, so get new lscript pointer to reloaded script. 
			if (lscript!=0) {
					sprintf(tstr,"Script %s reloaded",lscript->name);
					FTMessageBox(tstr,FTMB_NONE,"Robot - Info",500);
			} else { //if (!LoadRobotScriptFile(tstr)) {
				fprintf(stderr,"Error: LoadRobotScriptFile(%s) returned 0\n",tc->text);
				//because script with error is unloaded, refresh textbox with filename
				UpdateScriptList(tc);  //update loaded script names in dropdown control
				return(0);
			}  //if (!LoadRobotScriptFile(tstr)) {

		} else {
			fprintf(stderr,"Error: Could not find script named %s\n",tc->text);
			return(0);
		} 
	} //if (tc!=0) 

	return(1);
} //int btnReloadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)



//unload a Robot Script
int btnUnloadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTControl *tc;
	RobotScript *lscript;

	tc=GetFTControl("ddScripts");
	lscript=GetRobotScript(tc->text);
	if (lscript!=0) {
		UnloadRobotScript(lscript); //updates dropdown too
	}

	return(1);
} //int btnLoadScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)



//Run Script
int btnRunScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTControl *tc;
	RobotTask *ltask;
	RobotScript *lscript;
	int i;

	tc=GetFTControl("ddScripts");
	if (tc!=0) {
		if (strlen(tc->text)!=0) {
			lscript=GetRobotScript(tc->text);
			if (lscript!=0) {
				ltask=malloc(sizeof(RobotTask));
				memset(ltask,0,sizeof(RobotTask));
				ltask->kind=ROBOT_TASK_KIND_SCRIPT;
				ltask->iScript=lscript; 
				//before adding script to task list, clear MAINTAIN_THRUST bit, so no MAINTAIN_THRUST flag is set when script starts 
				//for(i=0;i<NUM_MOTOR_NAMES;i++) {
				for(i=0;i<RStatus.NumMotorsExpected;i++) {
					RStatus.MotorInfo[i].flags&=~MOTOR_INFO_TABLE_MAINTAIN_THRUST;
				} //for i
				AddRobotTask(ltask);
			} //if (lscript!=0) {
		}//if (strlen(tc->text)!=0) { 
	} //if (tc!=0) {
	return(1);
} //int btnRunScript_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Balance the robot
int btnBalance_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
#if 0 //for now
	FTControl *tc;

	if (strcmp(tcontrol->text,"Start Balancing")) {
		//stop balancing
		strcpy(tcontrol->text,"Start Balancing");
		if (!StopBalancing()) {
			fprintf(stderr,"StopBalancing() failed.\n");
		} //
	} else {
		//make sure Accelerometers and Touch sensors are on
		tc=GetFTControl("btnStartAllAccelAndTouch");
		if (tc!=0) {
			if (!strncmp(tc->text,"Start",5)) {
				FTMessageBox("For balancing to start, all accelerometers and touch sensors need to be on",FTMB_OK,"Info",0);
			} else {
				StartBalancing();
			} //elseif (!strncmp(tc->text,"Start",5)) {
		} //if (tc!=0) {

//		lmac->flags|=ROBOT_STATUS_BALANCE_THREAD_CREATED;
	} //if (strcmp(tcontrol->text,"Start Balancing") {
#endif 
	return(1);
} //int btnBalance_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Thread that does balancing for the robot. 
//This thread currently is designed with two main sections:
//Section 1: Is the robot falling?
//Section 2: How to respond if the robot is falling, and implement the response
//This thread presumes that Accel data is coming in
#if Linux
int Thread_BalanceRobot(void) 
#endif
#if WIN32
DWORD WINAPI Thread_BalanceRobot(LPVOID lpParam) 
#endif
{
	return(1);
} //int Thread_BalanceRobot(void) 


int StartBalancing(void)
{
	return(1);
} //int StartBalancing(void)


int StopBalancing(void)
{
	return(1);
} //int StopBalancing(void)


int StepLeft(void) 
{

	return(1);
} //int StepLeft(void)



//add (an already allocated) task to the main Robot list of loaded tasks
int AddRobotTask(RobotTask *ltask)
{
	RobotTask *itask;

	if (ltask==0) {
		return(0);
	}

	if (RStatus.iTask==0) { 
		//no tasks yet
		RStatus.iTask=ltask;
		return(1);
	}
	
	itask=RStatus.iTask;

	//go to end of list
	while(itask->next!=0) {
		itask=itask->next;
	} //while

	itask->next=ltask;
	return(1);

} //int AddRobotTask(RobotTask *ltask)

//Remove a task from the main Robot list of tasks
//doesn't free the task- FreeTask() does that
//perhaps should delete all instructions in task here
int RemoveRobotTask(RobotTask *ltask)
{
	RobotTask *ltask2,*last;


	if (!ltask) {
		return(1);
	}
	ltask2=RStatus.iTask;
	last=0;
	while(ltask2!=0) {
		if (ltask2==ltask) {
			if (last==0) {
				RStatus.iTask=ltask->next;
				//fprintf(stderr,"RStatus.iTask=%p\n",RStatus.iTask);
				return(1);
			} else {
				last->next=ltask->next;
				return(1);
			}//last==0
		} //if (ltask2==ltask) {
		last=ltask2;
		ltask2=ltask2->next;
	} //while
	return(1);
} //int RemoveRobotTask(RobotTask *ltask)


//free any malloc'd memory for a single task (doesn't free any script in the task)
int FreeRobotTask(RobotTask *ltask)
{
	if (ltask != 0) {
		free(ltask); //free the memory allocated for this task
	} //if (ltask!=0) {

	return(1);
} //int FreeRobotTask(RobotTask *ltask)

//free all robot tasks
int FreeAllRobotTasks(void)
{
	RobotTask *ltask;


	if (PStatus.flags&PInfo) {
		fprintf(stderr,"FreeAllRobotTasks()\n");
	}


	while(RStatus.iTask!=0) {
			ltask=RStatus.iTask;
			//probably there should be a check to stop all motor movements when removing a Robot task
			RemoveRobotTask(ltask); //this sets RStatus.iTask=TStatus.iTask->next;
			FreeRobotTask(ltask);
	} //	while(RStatus.iTask!=0) {

	return(1);
} //int FreeAllRobotTasks(void)

//******************

//add the pointer to an instruction to the main Robot "condition list": the list of RobotInstructions waiting for a condition to be met
//presumes inst has already been malloc'd.
//Perhaps should be AddConditionalInstruction since these can only be conditional instructions, but they are RobotInstructions, so perhaps keep as is.
//Perhaps AddCondInstructionPointer AddPointerToRobotInstruction AddPointerToInsttoInstList
int AddCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst)
{
	RobotInstructionList *tinstlist;

	if (linstlist==0) { 
		fprintf(stderr,"AddCondInstruction: linstlist==0\n");
		return(0);
	}

	if (linst==0) {
		fprintf(stderr,"AddCondInstruction: linst==0\n");
		return(0);
	}

	if (*linstlist==0) { 
		//fprintf(stderr,"no iCond inst yet\n");
		//no instructions yet, allocate and add first instruction
		*linstlist=(RobotInstructionList *)malloc(sizeof(RobotInstructionList));
		(*linstlist)->Inst=linst;
		(*linstlist)->next=0;
		return(1);
	}
	
	
	tinstlist=*linstlist;
	//otherwise go to end of list
	while(tinstlist->next!=0) {
		tinstlist=tinstlist->next;
	} //while

	//allocate and fill another instructionlist record
//	fprintf(stderr,"at least 1 iCond inst\n");
	tinstlist->next=(RobotInstructionList *)malloc(sizeof(RobotInstructionList));
	tinstlist=tinstlist->next;
	tinstlist->Inst=linst;
	tinstlist->next=0;

	return(1);

} //int AddCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst)

//Delete an already allocated instruction from an instruction list and free the itemlist record
int DeleteCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst)
{
	RobotInstructionList *tinstlist,*last;
	//EthAccelsInstruction *leai;
	EthMotorsInstruction *lemi;

	if (linstlist==0) { 
		fprintf(stderr,"DeleteCondInstruction: linstlist==0\n");
		return(0);
	}

	if (linst==0) {
		fprintf(stderr,"DeleteCondInstruction: linst==0\n");
		return(0);
	}

	if (*linstlist==0) { 
		//no instructions in list
		fprintf(stderr,"DeleteCondInstruction: *listnlist==0 - no instructions in instruction list\n");
		return(0);
	}

	//otherwise go through inst list and find and delete instlist with inst
	tinstlist=*linstlist;
	last=tinstlist;
	while(tinstlist!=0) {
		if (tinstlist->Inst==linst) {
			lemi=(EthMotorsInstruction *)&linst->EMInst;
			//leai=(EthAccelsInstruction *)&linst->EAInst;

			if (linst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR) { //AnalogSensor inst has no EthMotors
				//fprintf(stderr,"DeleteCondInstruction ST=%d %s+%s\n",linst->OrigStartTime,lemi->MotorReadableName, leai->SensorName);
				//if Thrust is !=0 send Thrust=0 to stop motor, unless instruction has MAINTAIN_THRUST flag
				if ((lemi->LastThrust!=0 || lemi->Thrust!=0) && !(linst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST)) {
					//fprintf(stderr,"Setting Motor Thrust=0 on %s\n",lemi->MotorReadableName);

					TurnMotor(lemi->mac[0],lemi->MotorNum[0],0,10,MOTOR_QUEUE); //stop motor
					if (linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
						TurnMotor(lemi->mac[1],lemi->MotorNum[1],0,10,MOTOR_QUEUE); //stop motor
					}
					lemi->Thrust=0; 
					lemi->LastThrust=0;
				} //if (lemi->LastThrust!=0 || lemi->Thrust!=0) {
			} //if (linst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR) {	 

			if (linst->MotionLogfptr!=0) {  //close Motion log file if open
				fclose(linst->MotionLogfptr);
				linst->MotionLogfptr=0;
			}

			if (linst->FutureLogfptr!=0) {  //close Future Motion log file if open
				fclose(linst->FutureLogfptr);
				linst->FutureLogfptr=0;
			}


			//while here- clear any flags in the instruction (the instruction is not deleted here- but is no longer being processed, so a conditional instruction can be run again) - note that conditional instructions are no longer re-run (for example the angle fell out of the TurnMotorToAngle range)- now instructions just end when the angle range is met the first time.
			linst->EAInst.flags=linst->EAInst.origflags;  //reset inst flags
			linst->flags=linst->origflags;  //reset inst flags
			linst->EAInst.LastUseAngle=0;  //reset LastUseAngle			
			//linst->flagSetCurrentAngle=0;  //clear flagSetCurrentAngle
 			linst->flagAngleInRange=0;  //clear flagAngleInRange

			linst->StartTime=linst->OrigStartTime; //in case script is run a second time, the inst StartTime will be the original start time

			//match - now delete and free the InstList record
			if (tinstlist==*linstlist) { //first record in list			
				//assign the first record to the first record's next record
				//note that this will set RobotTask->iCond=0 when there are no more tasks		
				*linstlist=(RobotInstructionList *)tinstlist->next;  
				free(tinstlist);


				return(1);
			} //
			//is not first record
			last->next=tinstlist->next;
			free(tinstlist);
			return(1);
		} //
		last=tinstlist;
		tinstlist=tinstlist->next;
	} //while



	fprintf(stderr,"DeleteCondInstruction: no match found\n");

	return(0);
} //int DeleteCondInstruction(RobotInstructionList **linstlist,RobotInstruction *linst)


int DeleteAllCondInstructions(RobotInstructionList **linstlist)
{
	RobotInstructionList *tinstlist,*nlist;

	if (linstlist==0) { 
		fprintf(stderr,"DeleteAllCondInstructions: linstlist==0\n");
		return(0);
	}

	if (*linstlist==0) { 
		//no instructions in list
		fprintf(stderr,"DeleteAllCondInstructions: *listnlist==0 - no instructions in instruction list\n");
		return(0);
	}

	//otherwise go through inst list and delete all instructions	
	tinstlist=*linstlist;
	while(tinstlist!=0) {
		nlist=tinstlist->next;
		DeleteCondInstruction(linstlist,tinstlist->Inst); //frees instlist record
		tinstlist=nlist;
	} //while

	return(1);

} //int DeleteAllCondInstructions(RobotInstructionList **linstlist)



//Create the RobotTasks thread- the thread that process all Robot tasks (currently: 1:run scripts, 2:balance)
int CreateRobotTasksThread(void)
{
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
	int HighestPriority;
#endif


		//create RobotTasks thread 
#if Linux
		//get highest priority possible
		HighestPriority=sched_get_priority_max(SCHED_FIFO);
		//fprintf(stderr,"Highest Priority=%d\n",HighestPriority); //=99 in Linux
		//set thread priority
		pthread_attr_init(&tattr);
		pthread_attr_getschedparam(&tattr,&param);
		param.sched_priority=HighestPriority-1;//98;
		pthread_attr_setschedparam(&tattr,&param);
		pthread_create(&RStatus.RobotTasksThread,&tattr,(void *)Thread_RobotTasks,NULL);
#endif
#if WIN32
		RStatus.RobotTasksThread=CreateThread(0,0,Thread_RobotTasks,0,0,0);
#endif

		RStatus.flags|=ROBOT_STATUS_TASK_THREAD_RUNNING;

	return(1);
} //int CreateRobotTasksThread(void)



int StopRobotTasksThread(void)
{
	int Failed,iResult;

	Failed=0;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"StopRobotTasksThread\n");
	}
/*
	tc=GetFTControl("btnTasks");
	if (tc!=0) {
		strcpy(tc->text,"Start Tasks");
		DrawFTControl(tc);
	}
*/


	RStatus.flags&=~ROBOT_STATUS_TASK_THREAD_RUNNING;  //exits while in thread
#if Linux
	usleep(100000); //0.1s for thread to exit while fully
	iResult=pthread_cancel(RStatus.RobotTasksThread);
	if (iResult<0) {
		Failed=1;
	}
		//wait for thread to terminal cleanly
		if (iResult < 0) {
	    //handle_error_en(iResult,"pthread_cancel");
			fprintf(stderr,"pthread_cancel failed on RobotTasksThread\n");
		} 
		/* Join with thread to see what its exit status was */
		iResult = pthread_join(RStatus.RobotTasksThread, 0);
		if (iResult != 0) {
	    //handle_error_en(iResults, "pthread_join");
			fprintf(stderr,"pthread_join failed on RobotTasksThread\n");
		} 
#endif
#if WIN32
	Sleep(100); //wait 100ms for thread while to exit cleanly
	if (TerminateThread(RStatus.RobotTasksThread,0)<0) {
		Failed=1;
	}
	Sleep(100); //for now wait 100ms todo: wait until thread is terminated
#endif


	return(!Failed);
	//return(1);
} //int StopRobotTasksThread(void)


#if USE_RT_PREEMPT
//Create the TurnMotor thread- the thread that runs once every 100us, and sends a single TurnMotor command if one is in the queue. Because two TurnMotor instrucitons need 100us between them for the EthMotors PCB, a TurnMotor thread is used to not delay the TurnMotor call by (usleep) 100us in the tasks thread.
int CreateTurnMotorThread(void)
{
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
	int HighestPriority;
#endif


		//create RobotTasks thread 
#if Linux
		//get highest priority possible
		HighestPriority=sched_get_priority_max(SCHED_FIFO);
		//fprintf(stderr,"Highest Priority=%d\n",HighestPriority); //=99 in Linux
		//set thread priority
		pthread_attr_init(&tattr);
		pthread_attr_getschedparam(&tattr,&param);
		param.sched_priority=HighestPriority-2;//is -1 for Task thread 98;
		pthread_attr_setschedparam(&tattr,&param);
		pthread_create(&RStatus.TurnMotorThread,&tattr,(void *)Thread_TurnMotor,NULL);
#endif
#if WIN32
//not yet implemented		RStatus.RobotTasksThread=CreateThread(0,0,Thread_RobotTasks,0,0,0);
#endif

		RStatus.flags|=ROBOT_STATUS_TURN_MOTOR_THREAD_RUNNING;

	return(1);
} //int CreateTurnMotorThread(void)

int StopTurnMotorThread(void)
{
	int Failed,iResult;

	Failed=0;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"StopTurnMotorThread\n");
	}

	RStatus.flags&=~ROBOT_STATUS_TURN_MOTOR_THREAD_RUNNING;  //exits while in thread
#if Linux
	usleep(100000); //0.1s for thread to exit while fully
	iResult=pthread_cancel(RStatus.TurnMotorThread);
	if (iResult<0) {
		Failed=1;
	}
		//wait for thread to terminal cleanly
		if (iResult < 0) {
	    //handle_error_en(iResult,"pthread_cancel");
			fprintf(stderr,"pthread_cancel failed on TurnMotorThread\n");
		} 
		/* Join with thread to see what its exit status was */
		iResult = pthread_join(RStatus.TurnMotorThread, 0);
		if (iResult != 0) {
	    //handle_error_en(iResults, "pthread_join");
			fprintf(stderr,"pthread_join failed on TurnMotorThread\n");
		} 
#endif
#if WIN32
//	Sleep(100); //wait 100ms for thread while to exit cleanly
//	if (TerminateThread(RStatus.TurnMotorThread,0)<0) {
//		Failed=1;
//	}
//	Sleep(100); //for now wait 100ms todo: wait until thread is terminated
#endif

	return(!Failed);
} //int StopTurnMotorThread(void)
#endif //USE_RT_PREEMPT


int AbortTask(RobotTask **pltask) 
{
	RobotInstructionList *tlist;
	EthMotorsInstruction *lemi;
	RobotInstruction *tinst;
	RobotTask *ltask;


	if (iTaskEngine.flags&ROBOT_TASK_ENGINE_ABORTING_TASK) {
		return(0); //already aborting task
	} 

	iTaskEngine.flags|=ROBOT_TASK_ENGINE_ABORTING_TASK;

	fprintf(stderr,"AbortTask\n");

	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		LogRobotModelData("Abort Task");
	}

	

	//First stop all motors - for some reason on abort some motors would not stop instantly- possibly EthMotors firmware error, but may have been solved (but not verified) by now.
	StopAllMotors(); 

	fprintf(stderr,"Stopped all motors\n");

	ltask=*pltask;  //get address of the task
	if (ltask) {
		if (ltask->iCond!=0) {
			//go through all instructions in the condition list one more time
			tlist=ltask->iCond; //go to start of list of instructions waiting for a condition
			while(tlist!=0) {							
				tinst=tlist->Inst; //shorthand for instruction  
				//stop the motor
				lemi=(EthMotorsInstruction *)&tinst->EMInst;
				//leai=(EthAccelsInstruction *)&tinst->EAInst;
				//if (lemi->Thrust!=0 || lemi->LastThrust!=0) {
				//motor is still turning
				if (lemi->mac[0]!=0) {
					TurnMotor(lemi->mac[0],lemi->MotorNum[0],0,99,MOTOR_QUEUE); //stop motor
					if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
						TurnMotor(lemi->mac[1],lemi->MotorNum[1],0,99,MOTOR_QUEUE); //stop motor
					} 
				} 
				lemi->LastThrust=0;
				//} //if (lemi->Thrust!=0) {
				//Delete each instruction in the stage
				//fprintf(stderr,"Delete inst ost=%d num=%d\n",tinst->OrigStartTime,tinst->InstNum);
				DeleteCondInstruction(&ltask->iCond,tinst); 
				tlist=tlist->next; //go to next instruction with a condition
			} //while(tlist!=0) {
		} //if ltask->iCond!=0

		//just to be sure, stop all motors here
		//for each EthMotors PCB
		//for each motor

		fprintf(stderr,"Wait for 0.5s\n");


#if Linux
		usleep(500000); //wait 0.5s for thread to finish processing TurnMotor calls- otherwise Motors can continue to turn
#endif
#if WIN32
		Sleep(500); //wait 500ms for thread to finish processing TurnMotor calls - otherwise motors can continue to turn
#endif

		fprintf(stderr,"Deleting task\n");


		//note that we only need to delete the task, since a script is attached to only one task. The other instructions do not need to be deleted, because they stay in the memory allocated when the script was loaded.
		RemoveRobotTask(ltask);
		fprintf(stderr,"FreeRobotTask\n");
		FreeRobotTask(ltask);
		ltask=0;
		//close LogFile if still open (if a script aborts because no accel data for >1second, or cannot get PCB)
		if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			//fprintf(stderr,"1 second since sample CloseRobotLogFile()\n");
			fprintf(stderr,"CloseRobotModelLogFile\n");
			CloseRobotModelLogFile();
		} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

		*pltask=0; //set address of task to 0
	} //if (ltask)

	iTaskEngine.flags&=~ROBOT_TASK_ENGINE_ABORTING_TASK; //clear ABORTING_TASK flag
	return(1);
} //int AbortTask(RobotTask **ltask);



//IncreaseThrust: means increasing the force of the LastThrust depending on the thrust relative to the direction of the target angle. Ex: a negative thrust towards the target, will increase by becoming more negative. If a positive thrust is needed to get to the target but the LastThrust was negative, IncreaseThrust will make the Thrust more positive (LastThrust=-1 new Thrust=0). For OPPOSE_MOTION, where there is no TargetAngle, AngNeedsToInc is calculated before hand.
void IncreaseThrust(EthMotorsInstruction *lemi,int AngNeedsToInc,int IncAngleDir,int UseAngle) {

	//note MaxThrust is always positive
	if (AngNeedsToInc) { //SegmentAngle needs to increase to reach TargetAngle
		if (IncAngleDir>0) { //Motor turning CW (positive) increases associated angle
			if (lemi->LastThrust<lemi->MaxThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust+1;
			}
		} else { //if (IncAngleDir)
			//Motor turning CCW (negative) increases associated angle
			if (lemi->LastThrust>-lemi->MaxThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust-1;
			}
		} //if (IncAngleDir>0)
	} else { //if (AngNeedsToInc) {
		//SegmentAngle needs to decrease to reach TargetAngle
		if (IncAngleDir>0) { //Motor turning CW (positive) increases associated angle
			if (lemi->LastThrust>-lemi->MaxThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust-1;
			}
		} else { //if (IncAngleDir>0)
			//Motor turning CCW (negative) increases associated angle
			if (lemi->LastThrust<lemi->MaxThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust+1;
			}
		} //if (IncAngleDir>0)
	} //if (AngNeedsToInc) {

} //void IncreaseThrust(EthMotorsInstruction *lemi)


//DecreaseThrust: means decreasing the force of the LastThrust (if + make more negative, if - make more positive). Note because decreasing thrust can send Segment in the wrong direction we check against MaxReverseThrust as the decrease limit (which depends on if the Segment is moving with or against gravity). Note that both MaxThrust and MaxReverseThrust are always positive. Amount is always positive.
void DecreaseThrust(EthMotorsInstruction *lemi,int AngNeedsToInc,int IncAngleDir, int Amount,int UseAngle) {

	if (AngNeedsToInc) { //SegmentAngle needs to increase to reach TargetAngle
		if (IncAngleDir>0) { //Motor turning CW (positive) increases associated angle
			if ((lemi->LastThrust-Amount)>=-lemi->MaxReverseThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust-Amount;
			}
		} else { //if (IncAngleDir>0)
			//Motor turning CCW (negative) increases associated angle
			if ((lemi->LastThrust+Amount)<=lemi->MaxReverseThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust+Amount;
			}
		} //if (IncAngleDir>0)
	} else { //if (AngNeedsToInc) {
		//SegmentAngle needs to decrease to reach TargetAngle
		if (IncAngleDir>0) { //Motor turning CW (positive) increases associated angle
			//if Thrust<0 T=Y (moving in correct direction) (ex: -14 +1=-13 <MaxReverse=6)
			//if Thrust>0 T=N (braking) (ex: 1 + 1 = 2 <MaxReverse=6)
			if ((lemi->LastThrust+Amount)<=lemi->MaxReverseThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust+Amount;
			}
		} else { //if (IncAngleDir>0)
			//Motor turning CCW (negative) increases associated angle
			if ((lemi->LastThrust-Amount)>=-lemi->MaxReverseThrust[UseAngle]) {
				lemi->Thrust=lemi->LastThrust-Amount;
			}
		} //if (IncAngleDir>0)
	} //if (AngNeedsToInc) {
} //void DecreaseThrust(EthMotorsInstruction *lemi,int AngNeedsToInc,int IncAngleDir, int Amount) {





//note that the below thread does not have any pause/sleep
#if Linux
int Thread_RobotTasks(void) 
#endif
#if WIN32
DWORD WINAPI Thread_RobotTasks(LPVOID lpParam) 
#endif
{
#define PI 3.14159265359
#define NUM_FUTURE_SAMPLES 100//50//30 //number of samples to look into the future to predict future motion
	RobotTask *ltask;
	RobotInstruction *MainInst,*LogInst,*tempinst;
	RobotScript *lscript;
	uint32_t RobotCurrentTime;
	MAC_Connection *lmac;
	Accels_PCB *leth[MAX_ANGLES_PER_INSTRUCTION];
	EthAccelsInstruction *leai;		
	EthMotorsInstruction *lemi;
	EthAnalogSensorInstruction *leti;
	Accelerometer *laccel;
	int InstLen,i,j,skip,MotorNameNum;
	unsigned char Inst[512];
	char LogStr[2048],tstr[2048];
	uint32_t CurrentTimeInMS;
	char timestamp[128];
	int DisabledTimeConsumingProcesses,DisabledRealTimeRender,DisabledShowAccelData,DisabledShowTouchData;
	FTControl *tc;
	FILE *fptr;
	struct stat st;
	int result;

#if USE_RT_PREEMPT
//#define RT_PRIORITY (49) //PREEMPT_RT uses 50 for kernel tasklets and interrupt handlers by default so use 49
//#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
//#define TasksTimerInterval 10000000 //10ms
	struct timespec t;
	struct sched_param param;
	int keep_on_running = 1;
#endif
#if USE_RTAI
	static RT_TASK *RT_Instructions_Task;
	static RTIME TasksTimerInterval;
	int keep_on_running = 1;
#endif

	fprintf(stderr,"Starting RobotTasks thread\n");

	DisabledTimeConsumingProcesses=0;
	DisabledRealTimeRender=0;
	DisabledShowAccelData=0;
	DisabledShowTouchData=0;
//	UseAngle=0;

	while(!(RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING));  //wait for flag to be set

	//MaxThrust=ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE-5; //current I presume MaxThrust does not change (20-5=16) and is the same for all EthMotors
	//in my experience if a motor cannot move an accelerometer with 10/20 (3/7) then more thrust will not matter.


#if USE_RT_PREEMPT
	param.sched_priority = RT_PRIORITY;
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		fprintf(stderr, "Error: In Thread_RobotTasks() sched_setscheduler failed.\n");
	}

	//Lock memory
	/*
	if (mlockall(MCL_CURRENT_MCL_FUTURE) == -1) {
	fprintf(stderr, "Error: In Thread_RobotTasks() mlockall failed.\n");
	}*/

	//pre-fault our stack
	//stack_prefault();

	clock_gettime(CLOCK_MONOTONIC, &t);

	//start after one second
	t.tv_sec++;

	while (keep_on_running) {
		//wait until next timer interval
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		//calculate next time to run
		t.tv_nsec += TasksTimerInterval;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}

		//set keep_on_running to 0 if you want to exit
		if (!(RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING)) {
			keep_on_running = 0;
		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
#endif //USE_RT_PREEMPT



#if USE_RTAI
  if (!(RT_Instructions_Task = rt_task_init_schmod(nam2num("RTAI_Instructions"), 0, 0, 0, SCHED_FIFO, 0xff))) {  
	//RT_TASK*rt_task_init_schmod(unsigned   long   name,   int   priority,   int   stack_size,   int
	//max_msg_size, int policy, int cpus_allowed)
                printf("Error: RTAI: Cannot init periodic RT_Instructions_Task\n");
  }
	TasksTimerInterval = nano2count(10000000); //every 10ms (10000000ns)
	rt_task_make_periodic(RT_Instructions_Task, rt_get_time(), TasksTimerInterval);
	rt_make_hard_real_time();
	while (keep_on_running) {
	//insert your main periodic loop here
    rt_task_wait_period();
		//set keep_on_running to 0 if you want to exit
		if (!(RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING)) {
			keep_on_running=0;
		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
#endif //USE_RTAI


#if !(USE_RTAI | USE_RT_PREEMPT)
//	while(1) {
	while (RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING) {
#endif //!(USE_RTAI | USE_RT_PREEMPT)

		//check for any active tasks
		if (RStatus.iTask!=0) {
			//there are active tasks

//fprintf(stderr,"1 ");

			//disable anything that might cause problems with running scripts/motions
			if (DisabledTimeConsumingProcesses<20) {  //waits 200ms to let time-consuming processes end

				//disable "Render in real-time"
				if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
					DisabledRealTimeRender=1;
					RMInfo.flags&=~ROBOT_MODEL_INFO_RENDER_IN_REALTIME;
					tc=GetFTControl("chkRobotModel_RealTimeRender");
					if (tc!=0) {
						tc->value=0;
						DrawFTControl(tc);
					} //if (tc!=0)
				} //if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {

				//disable "Show Data" for accelerometers
				if (RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) {
					DisabledShowAccelData=1;
					RStatus.flags&=~ROBOT_STATUS_SHOW_ACCEL_DATA;
					tc=GetFTControl("chkAccel_ShowData");
					if (tc!=0) {
						tc->value=0;
						DrawFTControl(tc);
					} //if (tc!=0)
				} //if (RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) {

				//disable "Show Data" for touch sensors
				if (RStatus.flags&ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA) {
					DisabledShowTouchData=1;
					RStatus.flags&=~ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA;
					tc=GetFTControl("chkAnalogSensor_ShowData");
					if (tc!=0) {
						tc->value=0;
						DrawFTControl(tc);
					} //if (tc!=0)
				} //if (RStatus.flags&ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA) {


				DisabledTimeConsumingProcesses++;

				//IMPORTANT: this needs time to take effect or else samples are not processed quickly enough
			} else { //if (DisabledTimeConsumingProcesses<10)
			//todo: fix indentation

			//go through all tasks
			ltask=RStatus.iTask;

			//tasks are in a linked list
			//the below loop will process each task, one at a time
			//going from ltask to ltask->next
			//until ltask==0;
			//if one task has a linked list of instructions
			//each instruction will be processed once
			//similarly moving from one instruction to the next using the ->next pointer
			//until the last instruction MainInst==0;

#if (USE_RTAI | USE_RT_PREEMPT)
			//using real-time timer
			//run through all tasks (and their associated instructions) only once every 10ms
			if (ltask!=0) {
#endif //USE_RTAI
#if !(USE_RTAI | USE_RT_PREEMPT)
			//non-real time just continuously loop through tasks
			while(ltask!=0) {
#endif //!(USE_RTAI | USE_RT_PREEMPT)

//fprintf(stderr,"2 ");

				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					//LogRobotModelData("Start Tasks");
				}

				ltask->flags&=~ROBOT_TASK_ABORT_TASK;	//clear abort flag if rerunning		

				RobotCurrentTime=GetTimeInMS();

				//if this task (script) is just starting:
				//set the ROBOT_TASK_RUNNING flag, and set the TaskStartTime, and (for scripts) set the CurrentInstruction
				if (!(ltask->flags&ROBOT_TASK_RUNNING)) {
					//Task is starting now

					ltask->flags|=ROBOT_TASK_RUNNING;
					ltask->StartTime=RobotCurrentTime;

					//if ((ltask->kind&ROBOT_TASK_KIND_SCRIPT) && ltask->CurrentRobotInst!=0) {
					//need ltask->iScript because of mysterious error when identifying Eth PCBs crashes here- memory corruption?
					if (ltask->kind&ROBOT_TASK_KIND_SCRIPT && ltask->iScript!=0) {						
						ltask->CurrentRobotInst=ltask->iScript->iRobotInst;
						//go through all instructions and set the StartTime to the OrigStartime in case this script is being re-run
						tempinst=ltask->CurrentRobotInst;
						while(tempinst!=0) {
							tempinst->StartTime=tempinst->OrigStartTime;
							tempinst=tempinst->next;
						} //while
					} //if (ltask->kind&ROBOT_TASK_KIND_SCRIPT) {
				} //if (!(ltask->flags&ROBOT_TASK_RUNNING) {


//START CHECK FOR KEYPORESS TO STOP LOOPING
				//check for a keypress while a task is running.
				//a keypress can be used to stop a loop or end a script
				if (iTaskEngine.flags&ROBOT_TASK_ENGINE_KEYPRESS) {
					iTaskEngine.flags&=~ROBOT_TASK_ENGINE_KEYPRESS; //clear keypress flag

					fprintf(stderr,"key=%x (%c)\n",(unsigned int)iTaskEngine.key,(char)iTaskEngine.key);
					//fprintf(stderr,"key: %c\n",(char)iTaskEngine.key);

					//key==0xff1b (esc), 0x31=1, 0x20=space
					//add keypress to log
					if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						sprintf(LogStr,"key=%x (%c)",(unsigned int)iTaskEngine.key,(char)iTaskEngine.key);
						LogRobotModelData(LogStr);
					}


					//if looping
					if (ltask->flags&ROBOT_TASK_LOOPING) {
 						//if KeyCode[0] is pressed, set the LoopCount to the EndLoopCount
						if (ltask->KeyCode[0]==0 || iTaskEngine.key==ltask->KeyCode[0]) {
							ltask->LoopCount=ltask->LoopEndCount;  //set the current loop to be the last
							ltask->flags|=ROBOT_TASK_KEY_ENDING_LOOP;
							fprintf(stderr,"Keypress ending loop\n");
						}  
 						//if KeyCode[1] is pressed, abort the script
						if (ltask->KeyCode[1]==0 || iTaskEngine.key==ltask->KeyCode[1]) {
							ltask->LoopCount=ltask->LoopEndCount;  //set the current loop to be the last
							ltask->flags|=ROBOT_TASK_KEY_ENDING_LOOP;
							ltask->flags|=ROBOT_TASK_ABORT_TASK; //AbortScript=1;
							fprintf(stderr,"Keypress ending loop and aborting script\n");
						}  
					} //if (ltask->flags&ROBOT_TASK_LOOPING) {
				} //if (iTaskEngine.flags&ROBOT_TASK_ENGINE_KEYPRESS) {

//END CHECK FOR KEYPORESS TO STOP LOOPING

				//fprintf(stderr,"3 ");
				

				if (ltask->kind&ROBOT_TASK_KIND_SCRIPT && ltask->iScript!=0) {
					//Task is a RobotScript
					//script format is: 
					//# are comments and can be on the same line after an instruction, or on a line by themselves				
					//StartTimeInMilliseconds,EthMotorsPCBName(ex:MOTOR_UPPER_BODY),Motor#(0-11),Strength(+=clockwise,-=counter-clockwise),DurationInMilliseconds
					//...
					//Note that insts must be in chronological order, but StartTimes can be the same for sequential instruction lines (ex: 2 or 3 motors all start turning at 1500milliseconds)

					//at some point there may only be tasks that are scripts- since StartBalanceMode() will be just another instruction

					//I think initially, there was a thought that there may be different kinds of tasks
					//perhaps balancing being a task
					//but currently there are only scripts and balancing is done in a separate thread.

					lscript=ltask->iScript; //shorthand


					//get current script instruction
					MainInst=ltask->CurrentRobotInst;
					//MainInst is used to store the current instruction before determining if the inst has a condition on it or not
					//tinst (below) is used to hold the instruction while going through the conditional instruction list
					//so it is important to distinguish between the two.
					//if TaskStartTime>=TaskStartTime+script->StartTime, 
					//the time for this instruction is here or has past,
					//send the Robot instruction and advance the CurrentInstruction
					//note that MainInst (CurrentRobotInst) can = 0, where there are only conditional instructions remaining
					//if (MainInst!=0 && RobotCurrentTime>=ltask->StartTime+MainInst->StartTime) {


					if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						//LogRobotModelData("Go Thru Insts");
						//sprintf(LogStr,"flag=%d LineNum=%d TaskST=%u InstOST=%u RobotTime=%u TaskST=%u InstST=%u Task+InstST=%u",((ltask->flags&ROBOT_TASK_WAIT_STAGE)!=0),MainInst->LineNum,ltask->StageNumber,MainInst->OrigStartTime,RobotCurrentTime,ltask->StartTime,MainInst->StartTime,(ltask->StartTime+MainInst->StartTime));
						//LogRobotModelData(LogStr);
					}



					//*****START WHILE GO THROUGH INSTRUCTIONS HAPPENING NOW******

					//fprintf(stderr,"4 ");
					//START go through all script instructions and start any whose start time has passed.
					//execute the next instruction if:
					//there is an instruction
					//this task is not currently waiting for a WAIT_STAGE instruction(s) to end, or this instruction has the same original starttime as the WAIT_STAGE instruction(s) (is in the same stage currently being processed in the task) AND is not an EndLoop instructions in the same stage
					//and this instruction starttime has passed.
					//note that insts of the same stage can run at different times if they are separated by an EndTask inst, because an EndStage inst will wait for the loop to complete before going to the next sequential instruction in the script
//note that below while will end if current instruction is in the future
					while (MainInst!=0 && ltask!=0 && (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) || ((ltask->StageNumber==MainInst->OrigStartTime) && (MainInst->InstNum!=ROBOT_INSTRUCTION_END_LOOP))) && RobotCurrentTime>=(ltask->StartTime+MainInst->StartTime)) {

//					if (MainInst!=0 && (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) || ltask->StageNumber==MainInst->OrigStartTime) && RobotCurrentTime>=ltask->StartTime+MainInst->StartTime) {
						//Send the instruction to the robot

						//if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						//	LogRobotModelData("Start Inst");
						//}




						//because an instruction can have the WAIT_STAGE flag set, some instructions may start later than their starttime so set the instruction starttime with the RobotCurrentTime - the ltask->StartTime
						MainInst->StartTime=RobotCurrentTime-ltask->StartTime;


						//(is done below) //reset any EthAccels instruction flags- because a script may be run multiple times
						//MainInst->EAInst.flags&=~(ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE|ETHACCELS_INSTRUCTION_PCB_NOT_FOUND|ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING|ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST); 
						MainInst->EAInst.WaitAndSeeTime=0; //reset WaitAndSee time


						//fprintf(stderr,"Start inst with OrigStartTime=%u StartTime=%u\n",MainInst->OrigStartTime,MainInst->StartTime);

						//if the task WAIT_STAGE flag is not set, and this instruction has the WAIT_STAGE flag set, this marks the start of a script stage
						if (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) && (MainInst->flags&ROBOT_INSTRUCTION_WAIT_STAGE)) {
							//ltask->StageStartTime=MainInst->StartTime; //save the stage start time
							//fprintf(stderr,"START STAGE %d\n",MainInst->OrigStartTime);
							//If logging, record that the first script stage has started
							if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
								sprintf(LogStr,"START STAGE %d",MainInst->OrigStartTime);
								LogRobotModelData(LogStr);
							}
						} //if (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) && (MainInst->flags&ROBOT_INSTRUCTION_WAIT_STAGE)) {
						
#if 0 
						//Print instruction to Model Log
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							sprintf(LogStr,"Inst:%s",MainInst->InstText);
							LogRobotModelData(LogStr);
						}
#endif

						//Now branch on the instruction kind and execute the instruction
						switch(MainInst->InstNum) {
							case ROBOT_INSTRUCTION_START_LOGGING:
								GetTimeStamp(timestamp); //get timestamp
								strncpy(tstr,lscript->name,strlen(lscript->name)-3);  //get filename minus file extension
								tstr[strlen(lscript->name)-3]=0;
#if Linux
								sprintf(RStatus.RobotModelLogFileName,"%s/model/Model_%s_%s.log",RStatus.CurrentRobotLogFolder,timestamp,tstr);
#endif
#if WIN32
								sprintf(RStatus.RobotModelLogFileName,"%s\\model\\Model_%s_%s.log",RStatus.CurrentRobotLogFolder,timestamp,tstr);
#endif

								OpenRobotModelLogFile();
								
								//Save script in ModelLog folder
#if Linux
								sprintf(LogStr,"%s/model/Model_%s_%s_script.log",RStatus.CurrentRobotLogFolder,timestamp,tstr);
#endif
#if WIN32
								sprintf(LogStr,"%s\\model\\Model_%s_%s_script.log",RStatus.CurrentRobotLogFolder,timestamp,tstr);
#endif
								fptr=fopen(LogStr,"wb");
								if (fptr!=0) {
									//print each instruction into the script log file
									LogInst=lscript->iRobotInst;
									while (LogInst!=0) {
										fprintf(fptr,"%s\n",LogInst->InstText);
										LogInst=LogInst->next;
									} //whjile (LogInst!=0)
									fclose(fptr);
								} else { //if (fptr!=0) {
									fprintf(stderr,"Could not open Script log file '%s'\n",LogStr);
								}	//if (fptr!=0) {



							break;
							case ROBOT_INSTRUCTION_LOG_SCRIPT_FILENAME:
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									//If logging, record that the script has started
									sprintf(LogStr,"Script filename: '%s'",lscript->filename);
									LogRobotModelData(LogStr);
								} 
							break;
							case ROBOT_INSTRUCTION_STOP_LOGGING:
								//fprintf(stderr,"Actual CloseRobotLogFile()\n");
								CloseRobotModelLogFile();
							break;
							//todo: change to ROBOT_INSTRUCTION_START_ACCELEROMETERS(ALL,Interrupt,Threshold,Relative)
							case ROBOT_INSTRUCTION_START_ACCELEROMETER:
							case ROBOT_INSTRUCTION_STOP_ACCELEROMETER:
								InstLen=3;  //inst+16-bit mask, (int=4 for abs/rel)

								//switch on Acquisition kind
								switch(MainInst->EAInst.AcqKind) {
									case ETHACCELS_INSTRUCTION_SINGLE_SAMPLE:
										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ACCELEROMETER) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES; 
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES; //just so the instruction will work
											fprintf(stderr,"Error in script: you do not need to stop a single Accelerometer sample\n");
										} 
									break;
									case ETHACCELS_INSTRUCTION_ABSOLUTE_INTERRUPT:
									case ETHACCELS_INSTRUCTION_RELATIVE_INTERRUPT:

										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ACCELEROMETER) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_START_ACCELEROMETER_INTERRUPT;
											InstLen=4;  //absolute/relative adds an extra byte for interrupt, polling=3 bytes
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_ACCELEROMETER_INTERRUPT;
										} 
										
										if (MainInst->EAInst.AcqKind==ETHACCELS_INSTRUCTION_RELATIVE_INTERRUPT) {
											Inst[3]=1; //relative	
										} else {
											Inst[3]=0;
										} 

									break;
									case ETHACCELS_INSTRUCTION_POLLING:
										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ACCELEROMETER) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_START_POLLING_ACCELEROMETER; 
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_POLLING_ACCELEROMETER; 
										} 
									break;
								}; //switch(MainInst->AcqKind) {


								if (MainInst->EAInst.PCBNum[0]==ALL_ETHACCELS) { //ROBOT_PCB_ETHACCELS_ALL) {
									//all EthAccels
									//Go through all the MAC_Connections and start all accelerometers 
									lmac=RStatus.iMAC_Connection;
									while(lmac!=0) {
										if (!strncmp(lmac->pcb.Name,"ETHACCEL",8)) {
											memcpy(&Inst[1],(uint16_t *)&MainInst->EAInst.AccelMask,2);
											SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
										} //if (!strncmp(lmac->pcb.Name,"ETHACCEL",8)) {
										lmac=lmac->next;
									} //while

/*
									//Go through all the EthAccel PCBs and start the selected accelerometers
									//memcpy(&Inst[3],&Threshold,2);
									//for(i=0;i<RStatus.NumEthAccelsPCBs;i++) {
									for(i=0;i<RStatus.NumEthAccelsPCBsExpected;i++) {
										//get shorthand to MAC_Connection of EthAccel
										lmac=RStatus.EthAccelsPCB[i].mac;
										memcpy(&Inst[1],(uint16_t *)&MainInst->EAInst.AccelMask,2);
										SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
									} //for i
*/
								} else { //if (MainInst->EAInst.PCBNum[0]==ALL_ETHACCELS) { //ROBOT_PCB_ETHACCELS_ALL) {
									//only 1 EthAccel
									//get mac for EthAccel
									lmac=Get_MAC_Connection_By_PCBNum(MainInst->EAInst.PCBNum[0]);
									if (!lmac) {
										//fprintf(stderr,"No MAC_Connection found for %s\n",ROBOT_PCB_NAMES[MainInst->EAInst.PCBNum[0]]); 
										fprintf(stderr,"No MAC_Connection found for PCB Named %s\n",RStatus.EthPCBInfo[MainInst->EAInst.PCBNum[0]].Name); 
									} else { //if (!lmac) {
										//uses a 16-bit AccelNum mask - still 3 bytes in instruction
										memcpy(&Inst[1],(uint16_t *)&MainInst->EAInst.AccelMask,2);
										SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
									} //ifelse (!lmac) {
								} //ifelse (MainInst->EAInst.PCBNum[0]==ROBOT_PCB_ETHACCELS_ALL) {

							break; //Start or Stop accelerometer (single, interrupt, or polling)



							case ROBOT_INSTRUCTION_START_ANALOG_SENSOR:
							case ROBOT_INSTRUCTION_STOP_ANALOG_SENSOR:
								//switch on Acquisition kind
								switch(MainInst->EAInst.AcqKind) {
									case ETHACCELS_INSTRUCTION_SINGLE_SAMPLE:
										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ANALOG_SENSOR) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_VALUES; 
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_VALUES; //just so the instruction will work
											fprintf(stderr,"Error in script: you do not need to stop a single Touch Sensor sample\n");
										} 
									break;
									case ETHACCELS_INSTRUCTION_ABSOLUTE_INTERRUPT:
									case ETHACCELS_INSTRUCTION_RELATIVE_INTERRUPT:

										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ANALOG_SENSOR) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_START_ANALOG_SENSORS_INTERRUPT;
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_ANALOG_SENSORS_INTERRUPT;
										} 

										//note that currently for touch sensors there only is a relative interrupt (not absolute)

									break;
									break;
									case ETHACCELS_INSTRUCTION_POLLING:
										if (MainInst->InstNum==ROBOT_INSTRUCTION_START_ANALOG_SENSOR) {
											Inst[0]=ROBOT_ACCELMAGTOUCH_START_POLLING_ANALOG_SENSORS; 
										} else {
											Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_POLLING_ANALOG_SENSORS; 
										} 
									break;
								}; //switch(MainInst->AcqKind) {

								InstLen=5;
								if (MainInst->EAInst.PCBNum[0]==ALL_ETHACCELS) {
									//all EthAccels
									//Go through all the EthAccel PCBs and start the selected touch sensors
									//memcpy(&Inst[3],&Threshold,2);

									//Go through all the MAC_Connections and start all accelerometers 
									lmac=RStatus.iMAC_Connection;
									while(lmac!=0) {
										if (!strncmp(lmac->pcb.Name,"ETHACCEL",8)) {
											memcpy(&Inst[1],&MainInst->EAInst.AccelMask,4);
											SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
										} //if (!strncmp(lmac->pcb.Name,"ETHACCEL",8)) {
										lmac=lmac->next;
									} //while

/*
									for(i=0;i<RStatus.NumEthAccelsPCBs;i++) {
										//get shorthand to MAC_Connection of EthAccel
										lmac=RStatus.EthAccelsPCB[i].mac;
										memcpy(&Inst[1],&MainInst->EAInst.AccelMask,4);
										SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
									} //for i
*/
								} else { //if (MainInst->EAInst.PCBNum[0]==ALL_ETHACCELS) {
									//only 1 EthAccel
									//get mac for EthAccel
									lmac=Get_MAC_Connection_By_PCBNum(MainInst->EAInst.PCBNum[0]);
									if (!lmac) {
//										fprintf(stderr,"No MAC_Connection found for %s\n",ROBOT_PCB_NAMES[MainInst->EAInst.PCBNum[0]]);
										fprintf(stderr,"No MAC_Connection found for PCB named %s\n",RStatus.EthPCBInfo[MainInst->EAInst.PCBNum[0]].Name); 
									} else { //if (!lmac) {
										memcpy(&Inst[1],&MainInst->EAInst.AccelMask,4);
										SendInstructionToAccel(lmac,Inst,InstLen,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
									} //ifelse (!lmac) {
								} //ifelse (MainInst->EAInst.PCBNum[0]==ROBOT_PCB_ETHACCELS_ALL) {
							break;
							case ROBOT_INSTRUCTION_TURN_MOTOR:								
								//get the MAC_Connection for the PCB, if not found, print error
								//lmac=Get_MAC_Connection_By_PCBName(MainInst->EMInst.EthMotorsPCB);
								//lmac=Get_MAC_Connection_By_PCBNum(MainInst->EMInst.PCBNum);
								//if (lmac==0) {
								//		fprintf(stderr,"No MAC_Connection found for %s\n",ROBOT_PCB_NAMES[MainInst->EMInst.PCBNum]); 
								//} else {
									//fprintf(stderr,"CurrentTime=%u\n",RobotCurrentTime);
								//} //if (lmac==0) {

								lemi=(EthMotorsInstruction *)&MainInst->EMInst;

								//fprintf(stderr,"TurnMotor PCB=%s Num=%d Speed=%d Dur=%d\n",lmac->pcb.PCBName,lemi->MotorNum,lemi->Thrust,lemi->Duration);
								//fprintf(stderr,"TurnMotor PCB=%s Num=%d Thrust=%d Dur=%d\n",lemi->mac[0]->pcb.Name,lemi->MotorNum[0],lemi->Thrust,lemi->Duration);
								TurnMotor(lemi->mac[0],lemi->MotorNum[0],lemi->Thrust,lemi->Duration,MOTOR_QUEUE);

							break; //ROBOT_INSTRUCTION_TURN_MOTOR:
							//case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE:
							//case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_RELATIVE_ANGLE:
							case ROBOT_INSTRUCTION_MOTOR_ANGLE:
							//case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE_WAIT:									

								//currently the initial TurnMotor is left to the below analysis of the condition
								//if (MainInst->InstNum==ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE) {
									//process the condition

								//if this inst has a WAIT_STAGE flag set, set the task WAIT_STAGE flag and record the inst (stage) starttime.
								if (MainInst->flags&ROBOT_INSTRUCTION_WAIT_STAGE) {
									ltask->flags|=ROBOT_TASK_WAIT_STAGE;
									//fprintf(stderr,"Set ROBOT_TASK_WAIT_STAGE for inst with OrigStartTime=%i\n",MainInst->OrigStartTime);
									ltask->StageNumber=MainInst->OrigStartTime;  //must be OrigStartTime so insts with same original time can be grouped together- Inst StartTime changes depending on when the inst is actually started

									//if this instruction has the WAIT_STAGE flag set but no REQUIRED (is opposite of what was HOLD_ANGLE) flag set, set the task flag to
									//indicate that there is at least 1 instruction in this stage that can end the stage before the timeout,
									//so all HoldAngle instructions will be ended when all TurnMotorUntil instructions in the stage end.
									//if (!(MainInst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
									//if (!(MainInst->flags&ROBOT_INSTRUCTION_REQUIRED)) {
									if (MainInst->flags&ROBOT_INSTRUCTION_REQUIRED) {
										ltask->flags|=ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST;
										//fprintf(stderr,"Set ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST for inst with OrigStartTime=%i\n",MainInst->OrigStartTime);
									} 
								} //if (MainInst->flags&ROBOT_INSTRUCTION_WAIT_STAGE)

								//get and store address of Ethernet PCB(s) here to save time below
								skip=0;
								leai=(EthAccelsInstruction *)&MainInst->EAInst;
								for(j=0;j<MainInst->NumAngles;j++) {
									leai->PCB[j]=Get_AccelsPCB_By_PCBNum(leai->PCBNum[j]);
									if (leai->PCB[j]==0) {
										//fprintf(stderr,"No EthAccels PCB found for %s\n",ROBOT_PCB_NAMES[leai->PCBNum[j]]);
										fprintf(stderr,"No EthAccels PCB found for PCB with name %s\n",RStatus.EthPCBInfo[leai->PCBNum[j]].Name);
										skip=1;
									}  
									//warning about relative angle missing second accel- should never happen, because such a script would not load
									if ((MainInst->flagRelativeAngle&(1<<j)) && (MainInst->NumAngles==j+1) && j<3) {
										fprintf(stderr,"Accel %d uses a relative angle but no Accel %d given in instruction on line %d flagRelativeAngle=%x\n",j+1,j+2,MainInst->LineNum,MainInst->flagRelativeAngle);
											skip=1;
									}

								} //for j


								//update the MAC_Connection to the EthMotors to make sure it is the latest
								MainInst->EMInst.mac[0]=Get_MAC_Connection_By_PCBNum(MainInst->EMInst.PCBNum[0]);
								if (MainInst->EMInst.mac[0]==0) {
									fprintf(stderr,"No MAC_Connection found for EthMotors PCB %s\n",RStatus.EthPCBInfo[MainInst->EMInst.PCBNum[0]].Name);
									skip=1;
								}  

								if (MainInst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
									//inst uses 2 motors
									MainInst->EMInst.mac[1]=Get_MAC_Connection_By_PCBNum(MainInst->EMInst.PCBNum[1]);
									if (MainInst->EMInst.mac[1]==0) {
										fprintf(stderr,"No MAC_Connection found for EthMotors PCB %s\n",RStatus.EthPCBInfo[MainInst->EMInst.PCBNum[1]].Name);
										skip=1;
									}  
								} //if (MainInst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {

								if (!skip) {
									if (MainInst->flags&ROBOT_INSTRUCTION_MOTION_LOG) {
										//make motion log file name
										GetTimeStamp(timestamp); //get timestamp
										sprintf(tstr,"%s_%s.csv",MainInst->MotionLogFileName,timestamp);
										//open motion log file
										MainInst->MotionLogfptr=fopen(tstr,"wb");
										if (MainInst->MotionLogfptr==0) {
											fprintf(stderr,"Error opening Motion Log File %s\n",tstr);
										} else {
											//output header
											//fprintf(MainInst->MotionLogfptr,"%s\t%s\t%04.2f\n",MainInst->EAInst.MotorReadableName,MainInst->EAInst.SensorName,MainInst->EAInst.Angle[0]);
											//fprintf(MainInst->MotionLogfptr,"Time\tThrust\tThrustNeeded\tWeight\tAng\tVelocity\tAccel\tExpAccel\tA/V\n");
											//fprintf(MainInst->MotionLogfptr, "Time\tThrust\tThrustNeeded\tWeight\tAng\tVelocity\tAccel\tJerk\tExpAccel\tA/V\n");
											fprintf(MainInst->MotionLogfptr, "Time\tThrust\tThrustNeeded\tWeight\tAngle\tVelocity\tAccel\tAccel3\tJerk\tExpAccel\tA/V\n");
										} //if (MainInst->MotionLogfptr==0) {
									} //if (MainInst->flags&ROBOT_INSTRUCTION_MOTION_LOG) {

									if (MainInst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										//make future motion log file name
										GetTimeStamp(timestamp); //get timestamp
#if Linux
										sprintf(tstr,"%s/future/Future_%s_%s.csv",RStatus.CurrentRobotLogFolder,timestamp,MainInst->FutureLogFileName);
#endif
#if WIN32
										sprintf(tstr,"%s\\future\\Future_%s_%s.csv",RStatus.CurrentRobotLogFolder,timestamp,MainInst->FutureLogFileName);
#endif
										result = stat(RStatus.CurrentRobotLogFolder, &st);
										if (result!=0) {
											//folder does not exist - so create it
											result=mkdir(RStatus.CurrentRobotLogFolder,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
											if (result!=0) {
												fprintf(stderr,"could not create robot log folder '%s'\n",RStatus.CurrentRobotLogFolder);
											}
										} //

										//see if robot log/future folder exists
										result = stat(RStatus.CurrentRobotFutureLogFolder, &st);
										if (result!=0) {
											//folder does not exist - so create it
											result=mkdir(RStatus.CurrentRobotFutureLogFolder,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
											if (result!=0) {
												fprintf(stderr,"could not create robot log future folder '%s'\n",RStatus.CurrentRobotFutureLogFolder);
											}
										} //

										//open future motion log file
										MainInst->FutureLogfptr=fopen(tstr,"wb");
										if (MainInst->FutureLogfptr==0) {
											fprintf(stderr,"Error opening Future Log File %s\n",tstr);
										} else {
											//output header
											//fprintf(MainInst->MotionLogfptr,"%s\t%s\t%04.2f\n",MainInst->EAInst.MotorReadableName,MainInst->EAInst.SensorName,MainInst->EAInst.Angle[0]);
											//fprintf(MainInst->MotionLogfptr,"Time\tThrust\tThrustNeeded\tWeight\tAngle\tVelocity\tAccel\tExpAccel\tA/V\n");
											//fprintf(MainInst->MotionLogfptr, "Time\tThrust\tThrustNeeded\tWeight\tAngle\tVelocity\tAccel\tJerk\tExpAccel\tA/V\n");
											sprintf(LogStr,"Time\tInst Time\tQuantity\tTrgt/ZrVl/Thr (WG)\tInfo/CrctDir\tTargetAngle\tCurrentAngle\tVelocity\tAccel");
											tstr[0]=0;
											for(i=0;i<NUM_FUTURE_SAMPLES+1;i++) {
												sprintf(tstr,"%s\t%d",tstr,i*10);
											}
											fprintf(MainInst->FutureLogfptr, "%s%s\n",LogStr,tstr);
										} //if (MainInst->FutureLogfptr==0) {
									} //if (MainInst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {


									//if the last instruction that used this motor had the MAINTAIN_THRUST flag set
									//set both lemi->Thrust and lemi->LastThrust to MotorInfo[ThisMotor].LastThrust here
									lemi=&MainInst->EMInst;
									//fprintf(stderr,"G0\n");
									MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[0],lemi->MotorNum[0]);									
									if (RStatus.MotorInfo[MotorNameNum].flags&MOTOR_INFO_TABLE_MAINTAIN_THRUST) {
										//fprintf(stderr,"Thrust maintained\n");
										//This motor has the MAINTAIN_THRUST flag set, which means that an instruction with the MAINTAIN_THRUST flag was last used on this motor 
										//set LastThrust and Thrust to the thrust being maintained
										lemi->LastThrust=RStatus.MotorInfo[MotorNameNum].LastThrust;
										lemi->Thrust=lemi->LastThrust;
										//and stop any initial thrust from being set
										//need to have initial thrust to set correct duration  leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST;
										//and set the LAST_MAINTAIN_THRUST, so no initial thrust is set
										MainInst->flags|=ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST;
										RStatus.MotorInfo[MotorNameNum].flags&=~MOTOR_INFO_TABLE_MAINTAIN_THRUST; //clear flag
									}	//if (RStatus.MotorInfo[MotorNameNum].LastThrust!=0) 

									if (MainInst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
										//inst uses 2 motors
										//fprintf(stderr,"G1\n");
										MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[1],lemi->MotorNum[1]);									
										if (RStatus.MotorInfo[MotorNameNum].LastThrust!=0) {
											lemi->LastThrust=RStatus.MotorInfo[MotorNameNum].LastThrust;
											lemi->Thrust=lemi->LastThrust;
										}	
									} //if (MainInst->flags&ROBOT_INSTRUCTION_TWO_MOTORS)


									MainInst->flags|=ROBOT_INSTRUCTION_INSTRUCTION_START; //indicates instruction just started, used to determine if the target angle is in range or not - is better than using SENT_INITIAL_THRUST, because an initial determination of if the segment is out of range or not is necessary even though the SENT_INITIAL_THRUST flag is set here.

									//add this instruction to the condition list
									//fprintf(stderr,"add inst to cond list\n");
									AddCondInstruction(&ltask->iCond,MainInst);
								} //if (skip) {	
								//} //if (MainInst->InstNum==ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE) {
	
							break;  //ROBOT_INSTRUCTION_MOTOR_ANGLE:
							case ROBOT_INSTRUCTION_STORE_ANGLE:
								//store the angle associated with EthAccel[0], StoreAngle() only uses 1 Accel

								//get the EthAccels PCB
								skip=0;
								leai=(EthAccelsInstruction *)&MainInst->EAInst;
								leai->PCB[0]=Get_AccelsPCB_By_PCBNum(leai->PCBNum[0]);
								if (leai->PCB[0]==0) {
									fprintf(stderr,"No EthAccels PCB found for %s\n",RStatus.EthPCBInfo[leai->PCBNum[0]].Name);
									skip=1;
								} else { 

									leth[0]=leai->PCB[0];
									laccel=&leth[0]->Accel[leai->AccelNum[0]];
									//check to see if sample is recent
									CurrentTimeInMS=GetTimeInMS();
									if ((CurrentTimeInMS>laccel->TimeStamp) && ((CurrentTimeInMS-laccel->LastTimeStamp)>1500)) {
										//no sample received in 1 second - for now just print a warning- probably should AbortScript();
										fprintf(stderr,"Warning: No sample received from Accelerometer '%s' in over 1.5s\n",leai->SensorReadableName[0]);
									} 
									//get the Current Angle and store it in the script AngleVariable array
									lscript->AngleVariable[leai->AngleVariableIndex[0]]=leth[0]->Accel[leai->AccelNum[0]].EstAngle[leai->xyz[0]];
									//fprintf(stderr,"Store Angle %s [%d]=%03.2f\n",lscript->AngleVariableName[leai->AngleVariableIndex[0]],leai->AngleVariableIndex[0],lscript->AngleVariable[leai->AngleVariableIndex[0]]);
									//fprintf(stderr,"Set variable %s index=%d pcb=%s AccelNum=%d dimension=%d to Angle %03.1f\n\n",lscript->AngleVariableName[leai->AngleVariableIndex[0]],leai->AngleVariableIndex[0],ROBOT_PCB_NAMES[leai->PCBNum[0]],leai->AccelNum[0],leai->xyz[0],lscript->AngleVariable[leai->AngleVariableIndex[0]]);
								} //if (leai->PCB[0]==0) {

							break;  //ROBOT_INSTRUCTION_STORE_ANGLE
							case ROBOT_INSTRUCTION_START_LOOP:
								//start looping all instructions after this until EndLoop inst is reached
								//set loop flag and parameters 
								ltask->flags|=ROBOT_TASK_LOOPING;
								ltask->LoopCount=1;
								ltask->LoopEndCount=MainInst->NumLoops; //-1=no end
								ltask->StartLoopInst=MainInst; //to get back to start of loop
								//the next TurnMotor or MotorAccel inst will store pointer to first inst in loop if this flag is set, so when the loop ends, the code knows where to return to.
								//copy 2 KeyCodes to task
								ltask->KeyCode[0]=MainInst->KeyCode[0];
								ltask->KeyCode[1]=MainInst->KeyCode[1];
								ltask->LoopStartTime=RobotCurrentTime;
								fprintf(stderr,"Start Loop\n");
								fprintf(stderr,"Current Script Time %u\n",RobotCurrentTime-ltask->StartTime);



							break; //ROBOT_INSTRUCTION_START_LOOP
							case ROBOT_INSTRUCTION_END_LOOP:
								//end of sequence of insts to loop
								fprintf(stderr,"End Loop: ");
								//if there are still conditional instructions (a stage) being processed
								//hold this instruction in the task queue until the stage is done (iCond==0)
								if (ltask->iCond==0) { //no stage is still processing 
		
									//fprintf(stderr,"iCond==0 ");
									//if loop criteria have been fulfilled, stop looping, otherwise go back to the first inst in loop
									if ((ltask->LoopEndCount!=-1 || (ltask->flags&ROBOT_TASK_KEY_ENDING_LOOP))  && ltask->LoopEndCount<= ltask->LoopCount) {
										//if Loop does have an end count, or keypress is ending loop, and the LoopEndCount<= the LoopCount then end loop								//end loop
										fprintf(stderr,"Final loop (%d) complete\n",ltask->LoopCount);
										ltask->flags&=~(ROBOT_TASK_LOOPING|ROBOT_TASK_KEY_ENDING_LOOP);  //clear task looping flag
										ltask->StartLoopInst=0;  //just to be clean, clear the StartLoopInst pointer
										ltask->KeyCode[0]=0;
										ltask->KeyCode[1]=0;
										ltask->LoopEndCount=0;
										ltask->LoopCount=0;

									} else {  //if (ltask->LoopEndCount!=-1 && ltask->LoopEndCount<= ltask->LoopCount) {
										fprintf(stderr,"Loop (%d) complete\n",ltask->LoopCount);
										//set the EndLoop inst StartTime back to the Original StartTime

										ltask->LoopCount++;  //increment LoopCount
										//go back to first inst in loop
										MainInst=ltask->StartLoopInst;  //note that this is the StartLoop inst, so the MainInst=MainInst->next below will go to the first inst after the StartLoop inst. 
										//this is now done when running the script or should be: MainInst->StartTime=MainInst->OrigStartTime; //in case script is run a second time, the inst StartTime will be the original start time- otherwise there is a delay before the loop ends because the StartTime for this EndLoop inst becomes farther in the future.

										//looping again, so update the RobotCurrentTime, and add the current time to all TurnMotor insts until the EndLoop
										tempinst=MainInst->next;
										RobotCurrentTime=GetTimeInMS();
										//fprintf(stderr,"Set RobotCurrentTime to %u\n",RobotCurrentTime);
										fprintf(stderr,"Current Script Time %u\n",RobotCurrentTime-ltask->StartTime);

										//should possibly use WAIT_STAGE flag for loops
										//currently once a loop is entered, all following inst start times are offset to current time+ inst time
										//while(tempinst->InstNum!= ROBOT_INSTRUCTION_END_LOOP && tempinst!=0) {
										while(tempinst!=0) {
											//if (tempinst->InstNum==ROBOT_INSTRUCTION_TURN_MOTOR) {
												//tempinst->StartTime=(RobotCurrentTime-ltask->StartTime+tempinst->OrigStartTime);
												tempinst->StartTime+=(RobotCurrentTime-ltask->LoopStartTime);
												fprintf(stderr,"Set StartTime (%u) to %u\n",tempinst->OrigStartTime,tempinst->StartTime);
											//}
										tempinst=tempinst->next;
										} //while
										//if (tempinst!=0) {
												//add to the End Loop inst
										//		tempinst->StartTime=(RobotCurrentTime-ltask->StartTime+tempinst->OrigStartTime);										
										//}

										//update LoopStartTime 
										ltask->LoopStartTime=RobotCurrentTime;
		
									} //if (ltask->LoopEndCount!=-1 && ltask->LoopEndCount<= ltask->LoopCount) {
									//this is now done when running the script or should be MainInst->StartTime=MainInst->OrigStartTime; //in case script is run a second time, the inst StartTime will be the original start time- otherwise there is a delay before the loop ends because the StartTime for this EndLoop inst becomes farther in the future.
								} //if (ltask->iCond==0) { //no stage is still processing

						


							break; //ROBOT_INSTRUCTION_END_LOOP
							case ROBOT_INSTRUCTION_ANALOG_SENSOR:
								//AnalogSensor inst: Ends Stage if a touch sensor PercentPressed value is GT or LT some value

								//Currently all AnalogSensor instructions are part of a stage
								//so set the task WAIT_STAGE flag and record the inst (stage) starttime.
								ltask->flags|=ROBOT_TASK_WAIT_STAGE;
								//fprintf(stderr,"Set ROBOT_TASK_WAIT_STAGE for inst with OrigStartTime=%i\n",MainInst->OrigStartTime);
								ltask->StageNumber=MainInst->OrigStartTime;  //must be OrigStartTime so insts with same original time can be grouped together- Inst StartTime changes depending on when the inst is actually started


								leai=(EthAccelsInstruction *)&MainInst->EAInst;  //needed for checking sample timestamp below
								leai->PCB[0]=Get_AccelsPCB_By_PCBNum(leai->PCBNum[0]);
								if (leai->PCB[0]==0) {
									//fprintf(stderr,"No EthAccels PCB found for %s\n",ROBOT_PCB_NAMES[leai->PCBNum[0]]);
									fprintf(stderr,"No EthAccels PCB found for %s\n",RStatus.EthPCBInfo[leai->PCBNum[0]].Name);
									skip=1;
								}  


								leti=(EthAnalogSensorInstruction *)&MainInst->ETInst;
								leti->PCB=Get_AccelsPCB_By_PCBNum(leti->PCBNum);
								if (leti->PCB==0) {
									fprintf(stderr,"No EthAccelsTouch PCB found for %s\n",RStatus.EthPCBInfo[leti->PCBNum].Name);
									skip=1;
								}
								//leth[0]=leti->PCB;

								//not needed for AnalogSensor
								//MainInst->flags|=ROBOT_INSTRUCTION_INSTRUCTION_START; //indicates instruction just started

								//add this instruction to the condition list
								//fprintf(stderr,"add inst to cond list\n");
								AddCondInstruction(&ltask->iCond,MainInst);

							break; //ROBOT_INSTRUCTION_ANALOG_SENSOR
						} //switch(MainInst->InstNum) {
						//Done executing instructions or adding them to the conditional instruction list

						if (skip) { //one accel was not found so abort the script
							skip=0;
							AbortTask(&ltask);	
							ltask=0;  //task is done after AbortTask(), so clear local pointer
							//tlist=0; //abort the list of instructions waiting for a condition							
						} else { //if (skip) {


							//skip advancing to the next inst if holding an EndLoop inst (unless the EndLoop is in the same stage)
							if ((MainInst->InstNum!=ROBOT_INSTRUCTION_END_LOOP) || (ltask->flags&ROBOT_TASK_LOOPING)==0) {

								//if the next instruction=0, remove and free this Robot task						
								if (MainInst->next==0) {
									//could still be inst waiting for start time here MainInst=0;  //no more script instructions to process
									if (ltask->iCond==0) {  //keep task alive until all conditions are done
										//this RobotTask is done
										//fprintf(stderr,"task is done\n");
										RemoveRobotTask(ltask);
										FreeRobotTask(ltask);
										ltask=0;  //for ltask=ltask->next below
									} else { //if (ltask->iCond==0) {
										//fprintf(stderr,"CurrentRobotInst=0\n");
										MainInst=0; //no more instructions without an unmet condition (but are instructions with unmet conditions)
										ltask->CurrentRobotInst=0; //no more instructions w/o conditions, but some instructions with conditions still exist
									} //if (ltask->iCond==0) {  //keep task alive until all conditions are done
								} else { //if (MainInst->next==0) {
									//MainInst->next is not 0
									//advance to the next instruction
									MainInst=MainInst->next;
									ltask->CurrentRobotInst=MainInst;
								} //if (MainInst->next==0) {						
							} else { //if ((MainInst->InstNum!=ROBOT_INSTRUCTION_END_LOOP) || (ltask->flags&ROBOT_TASK_LOOPING)==0) {
								//if task instructions are currently in loop
								//one exception is that we need to advance 
								//fprintf(stderr,"%d=%d ",MainInst->InstNum,ROBOT_INSTRUCTION_END_LOOP);
//(MainInst->InstNum!=ROBOT_INSTRUCTION_END_LOOP || ltask->StartTime==MainInst->StartTime)
							} //if ((MainInst->InstNum!=ROBOT_INSTRUCTION_END_LOOP) || (ltask->flags&ROBOT_TASK_LOOPING)==0) {

						} //if (skip) {

						
					//} //					if (MainInst!=0 && (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) || ltask->StageNumber==MainInst->OrigStartTime) && RobotCurrentTime>=ltask->StartTime+MainInst->StartTime) {
					} //while (MainInst!=0 && ltask!=0 && (!(ltask->flags&ROBOT_TASK_WAIT_STAGE) || ltask->StageNumber==MainInst->OrigStartTime) && RobotCurrentTime>=(ltask->StartTime+MainInst->StartTime)) {					
				//END go through all script instructions and start any whose start time has passed.
				//*****END WHILE GO THROUGH INSTRUCTIONS HAPPENING NOW******



					//fprintf(stderr,"iCond=%p\n",ltask->iCond);

					//Process Instructions on the Conditional Instruction List					
					//see if any instruction conditions exist
					//Note that the next pointer in an instruction should not be followed here, because the instruction memory was allocated by LoadScript, and so the next points to the next instruction that was loaded by LoadScript. So the iCond Instruction list contains a list of pointers to individual instructions.
					//From here down can only be conditional instructions: MotorAccel and AnalogSensor
					if (ltask!=0 && ltask->iCond) {  //ltask can be set to 0 above
						ProcessCondInsts(ltask);
					} //if (ltask!=0 && ltask->iCond)


 					//fprintf(stderr,"18 ");
				} //if (ltask->kind&ROBOT_TASK_KIND_SCRIPT) {
				//fprintf(stderr,"19 ");
				if (ltask && ltask->CurrentRobotInst==0) {  //for now because above may free ltask when tinst->next==0
					fprintf(stderr,"next task\n");
					ltask=ltask->next;
				}
			}//while(ltask!=0) {
			//fprintf(stderr,"20 ");
			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				//LogRobotModelData("End Tasks");
			}
			//fprintf(stderr,"21 ");
		} //if (DisabledTimeConsumingProcesses<20)
		//fprintf(stderr,"22 ");
		} //if (RStatus.iTask!=0) {
		
		//fprintf(stderr,"23 ");
		//done with all tasks, if anything was disabled, re-enable it
		if (RStatus.iTask==0 && DisabledTimeConsumingProcesses) {

			fprintf(stderr,"Script done\n");


			if (DisabledRealTimeRender) {
				//re-eable "Render in real-time"
				DisabledRealTimeRender=0;
				RMInfo.flags|=ROBOT_MODEL_INFO_RENDER_IN_REALTIME;
				tc=GetFTControl("chkRobotModel_RealTimeRender");
				if (tc!=0) {
					tc->value=1;
					DrawFTControl(tc);
				} //if (tc!=0)
			} //if (DisabledRealTimeRender) {


			if (DisabledShowAccelData) {
				//re-enable "Show Data" for accelerometers
				DisabledShowAccelData=0;
				RStatus.flags|=ROBOT_STATUS_SHOW_ACCEL_DATA;
				tc=GetFTControl("chkAccel_ShowData");
				if (tc!=0) {
					tc->value=1;
					DrawFTControl(tc);
				} //if (tc!=0)
			} //if (DisabledShowAccelData)

			if (DisabledShowTouchData) {
				//re-enable "Show Data" for touch sensors
				DisabledShowTouchData=0;
				RStatus.flags|=ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA;
				tc=GetFTControl("chkAnalogSensor_ShowData");
				if (tc!=0) {
					tc->value=0;
					DrawFTControl(tc);
				} //if (tc!=0)
			} //if (DisabledShowTouchData)

			DisabledTimeConsumingProcesses=0;
		} //if (RStatus.iTask==0 && DisabledTimeConsumingProcesses) {


		

#if !(USE_RTAI | USE_RT_PREEMPT)
//	} //while(1) 
	} //	while (RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING) {
#endif //!(USE_RTAI | USE_RT_PREEMPT)

#if USE_RT_PREEMPT
	}  //while (keep_on_running) {
#endif //USE_RTAI


#if USE_RTAI
	}  //while (keep_on_running) {
	rt_task_delete(RT_Instructions_Task);
#endif //USE_RTAI


	//Note that this thread is only called when a user clicks on "Play Script" or a script (or task) is running. Probably tasks will be removed and just scripts will be the only things that run in this thread. 

	return(1);

} //int Thread_RobotTasksRobot(void) 






void ProcessCondInsts(RobotTask *ltask) {

	RobotInstruction *tinst;
	RobotInstructionList *tlist,*last;
	RobotScript *lscript;
	uint32_t RobotCurrentTime;
	Accels_PCB *leth[MAX_ANGLES_PER_INSTRUCTION];
	EthAccelsInstruction *leai;		
	EthMotorsInstruction *lemi;
	EthAnalogSensorInstruction *leti;
	Accelerometer *laccel;
	AnalogSensor *lanalog;
	float SegmentAngle,TargetAngle,toffset,SegmentAngle2,TouchSample;
	float ForceWeight,Velocity,Acceleration,Jerk,AccelAvg3,TempAngle,LargestAngle,HighestVelocity;
	int i,j,SameTimeStamp,InitialThrust,MotorNameNum,TempAccel;
	char LogStr[2048],tstr[2048];
	float ThrustmN,TotalForce,ExpectedAccel,RatioAV,DoubleThrustDivider;
	int WithGravity,ThrustNeeded,VelocityInCorrectDirection,AccelerationInCorrectDirection,ThrustInCorrectDirection;
	float FutureSegmentAngle,FutureSegmentVelocity;
	int FutureTargetIndex,FutureZeroVelocityIndex,FutureMinVelocityIndex,FutureMaxVelocityIndex;
	int IsMoving,InRange,AngNeedsToInc,IncAngDir,UseAngle,MovingAgainstGravity;
	float FutureAngle[NUM_FUTURE_SAMPLES],FutureVelocity[NUM_FUTURE_SAMPLES];
	int AngleInRange;
	float AccelVelocity;



	//if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
	//	LogRobotModelData("Proc Cond");
	//}
	//fprintf(stderr,"Proc Cond");
	//fprintf(stderr,"5 ");


	if (!ltask) {
		return;
	}

	UseAngle=0;
	lscript=ltask->iScript; //shorthand

	//only for tasks with the WAIT_STAGE flag set
	if (ltask->flags&ROBOT_TASK_WAIT_STAGE) { //this instruction (or set of instructions) has the WAIT_STAGE flag set
		ltask->flags|=ROBOT_TASK_WAIT_STAGE_DONE; //temporarily set the task WAIT_STAGE_DONE flag
	} 

	//StillTurnMotorUntil=0; //set this if any TurnMotorUntil instruction with WAIT_STAGE flag still has its condition unmet.  

	//if yes, for each instruction, check to see if their time is past or if their condition has been met 
	tlist=ltask->iCond; //go to start of list of instructions waiting for a condition
	while(tlist!=0) {			
		//fprintf(stderr,"6 ");				
		tinst=tlist->Inst; //shorthand for instruction (note that this has to be here, because tinst will not change if a condition is met and the instruction removed from the instruction list (tlist)- and tinst is used in the below if to check if the instruction need to start.


		RobotCurrentTime=GetTimeInMS();  //update RobotCurrentTime


		//if (RobotCurrentTime > ltask->StartTime + tlist->Inst->EMInst.Duration) {
		if (RobotCurrentTime > (ltask->StartTime + tinst->StartTime + tinst->EMInst.Duration)) {  //todo: && ! ltask->flags&NO_TIMEOUT/DOES_NOT_END/NO_DURATION
			//the instruction timed out (for TurnMotorUntilAngle, before the angle was reached, but for HoldAngle, is just a timeout)
			//fprintf(stderr,"7 ");

			//remove this condition from the list
			//note that there is no need to stop the motor since the thrust duration reached the end.
			if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {
					//MAINTAIN_THRUST flag was set. Currently on time out, thrust is maintained if MAINTAIN_THRUST flag is set

				fprintf(stderr,"Inst timed out, motor#%d, thrust=%d origstarttime=%u starttime=%u thrust maintained\n",tinst->EMInst.MotorNum[0],tinst->EMInst.Thrust,tinst->OrigStartTime,tinst->StartTime);
				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					sprintf(LogStr,"Inst timed out, motor#%d, thrust=%d origstarttime=%u starttime=%u thrust maintained",tinst->EMInst.MotorNum[0],tinst->EMInst.Thrust,tinst->OrigStartTime,tinst->StartTime);
					LogRobotModelData(LogStr);
				} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			} else { //if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {

				fprintf(stderr,"Inst timed out, motor#%d, thrust=%d origstarttime=%u starttime=%u set thrust=0\n",tinst->EMInst.MotorNum[0],tinst->EMInst.Thrust,tinst->OrigStartTime,tinst->StartTime);
				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					sprintf(LogStr,"Inst timed out, motor#%d, thrust=%d origstarttime=%u starttime=%u set thrust=0",tinst->EMInst.MotorNum[0],tinst->EMInst.Thrust,tinst->OrigStartTime,tinst->StartTime);
					LogRobotModelData(LogStr);
				} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

			} //if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {

			if (!(tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST)) {  //big difference between MAINTAIN_THRUST and LAST_MAINTAIN_THRUST (MAINTAIN_THRUST is inst that has flag set, LAST_MAINTAIN_THRUST is inst in next stage after MAINTAIN_THRUST was set in previous stage). 
					//MAINTAIN_THRUST flag was not set, so ok to set Thrust=0. Currently on time out, thrust is maintained if MAINTAIN_THRUST flag is set

				//It is possible that the Motor Speed might not be 0, so make sure to stop the associated motor
				lemi=(EthMotorsInstruction *)&tinst->EMInst;
				//leai=(EthAccelsInstruction *)&tinst->EAInst;
				if (lemi->Thrust!=0) {																	
					//fprintf(stderr,"timeout delete tlist=%x tlist->Inst=%x lemi=%x mac[0]=%x mac[1]=%x\n",tlist,tlist->Inst,lemi,lemi->mac[0],lemi->mac[1]);
					lemi->Thrust=0;//-lemi->Thrust; //brake
					TurnMotor(lemi->mac[0],lemi->MotorNum[0],lemi->Thrust,100,MOTOR_QUEUE);  //stop motor/brake for 10 ms	
					if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
						TurnMotor(lemi->mac[1],lemi->MotorNum[1],lemi->Thrust,100,MOTOR_QUEUE);  //stop second motor/brake for 10 ms	 if 2 motor inst
					} //if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
					lemi->LastThrust=0; //update the LastSpeed
				} //if (lemi->Thrust!=0) {
			} //if (tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST) {

			//set ROBOT_TASK_ABORT_TASK flag if ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT instruction flag is set
			if (tinst->flags&ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT) {
				ltask->flags|=ROBOT_TASK_ABORT_TASK;//AbortScript = 1;
				fprintf(stderr, "Aborting script (stage %d) because instruction timed out and ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT instruction flag is set\n",ltask->StageNumber);
				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					sprintf(LogStr, "Aborting script (stage %d) because instruction timed out and ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT instruction flag is set",ltask->StageNumber);
					LogRobotModelData(LogStr);
				} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			} //if (tinst->flags&ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT) {

			//if task is in stage and this is an inst without "REQUIRED" (was "HOLD_ANGLE", and before that was a TurnMotorUntil inst), the stage will end because the WAIT_STAGE_DONE (was WAIT_STAGE task flag)
			//will not get reset (was set) again below.

			//last=tlist;  //preserve the top of the list of instructions
			tlist=tlist->next; //go to next instruction with a condition
			//if WAIT_STAGE, below Delete not needed because without WAIT_STAGE task flag set, all insts in a stage will be deleted after this loop below. But does not matter.
			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				sprintf(LogStr,"delete tinst->OrigStartTime=%u Motor#%d\n",tinst->OrigStartTime,tinst->EMInst.MotorNum[0]);
				LogRobotModelData(LogStr);
			//fprintf(stderr,"delete tinst->OrigStartTime=%u Motor#%d\n",tinst->OrigStartTime,tinst->EMInst.MotorNum[0]);
			}
			DeleteCondInstruction(&ltask->iCond,tinst);//delete this instruction from the conditional inst list					
		} else { //if (RobotCurrentTime > ltask->StartTime +  tlist->Inst->EMInst.Duration) {
			//check to see if the condition of the instruction has been met, and if yes	(and not a REQUIRED {was HOLD_ANGLE and not NO_HOLD} inst)
			//stop the associated motor 
			//(note that we do not need to check to see if the instruction condition start time has passed, 
			//because the condition is only created after the instruction time past and the instruction was sent.)
			//check condition (accel angle)
			//note that this presumes that the accels are sending data, and the Accel sample data
			//stored in the Accel data structure is current

			//get a shorthand for the Instruction Condition structure 
			//leai=(EthAccelsInstruction *)&tlist->Inst->EAInst;
			//lemi=(EthMotorsInstruction *)&tlist->Inst->EMInst;
			leai=(EthAccelsInstruction *)&tinst->EAInst;
			lemi=(EthMotorsInstruction *)&tinst->EMInst;

			//IncAngDir=tinst->EAInst.IncAngDir;

			//update Instruction time: the time an individual instruction has been running (logs use this)
			tinst->InstTime=RobotCurrentTime - ltask->StartTime - tinst->StartTime;
			//update the TaskTime (ScriptTime): the time a script has been running (logs use this)
			ltask->TaskTime=RobotCurrentTime - ltask->StartTime;

			//get EthAccel MacConnection
			//todo: Probably should be just for MotorAccel and other insts that use accels- not AnalogSensor, and probably other conditional insts
			//todo: Could get leai->eth, and leai->eth2 at the time of starting script, to save time here
			//and all that would happen is that the first attemp to use leth[0] or leth[1] would cause a problem.
			//get the MAC_Connection for the EthAccel PCB, if not found, print error
			//leth[0]=Get_EthAccelsPCB_By_PCBName(leai->EthAccelsPCB);
			//skip=0;  //skip comparing angles if any EthAccels PCB(s) connected to the instruction cannot be found
			//for each accelerometer:
			for(j=0;j<tinst->NumAngles;j++) {
				//set pointer to EthAccel PCB
				leth[j]=leai->PCB[j];  //causes problem for touch sensor using leth[0]

				//before checking the angle to see if the accel angle is in the target range (condition is true)
				//only process if we have received a new sample (on the moving body segment)- otherwise we are analyzing the same situation more than once which can cause the speed to increase too much, and in addition, it only wastes time.

				//another thing to possibly check is if the motor is currently turning, but we have not received sample data
				//in more than 1.5 seconds- to stop running the inst. and stop the motor.

				SameTimeStamp=0;
				//for all active accelerometers, verify that the sample is fresh and not older than 1.5s
				laccel=&leth[j]->Accel[leai->AccelNum[j]];
				leai->CurrentSampleTimeStamp[j]=laccel->TimeStamp;
				//check for AbortScript and new accel sample
				//if (!AbortScript && leai->LastSampleTimeStamp[i]!=leai->CurrentSampleTimeStamp[i]) { //new accel sample
				
				if (RobotCurrentTime-leai->CurrentSampleTimeStamp[j]>19 ) { //not new accel sample (>19 ms- 10ms, but there can rarely be 1ms of gitter)
					//accel[i] does not have a new sample/
					//since samples are sent from each EthAccel every 10ms,
					//the accel timestamp should never be more than 10-11ms older than the CurrentTimeInMS
					SameTimeStamp=1;
					fprintf(stderr,"%u SameTimeStamp a=%d\n",ltask->TaskTime,j); 
					//fprintf(stderr,"1:%s missed a sample\n",ROBOT_ACCEL_NAMES[leai->AccelNameNum[1]]); 
					//fprintf(stderr,"accel[0] (%d) ts=%u accel[1] (%d) ts=%u\n",leai->AccelNum[0],leth[0]->Accel[leai->AccelNum[0]].TimeStamp,leai->AccelNum[1],leth[1]->Accel[leai->AccelNum[1]].TimeStamp); 

					//if there has been no timestamp in 1.5 second and the motor is turning, 
					//Stop the motor and print a message
					//sometimes LastSampleTimeStamp[1] can be more recent than the above GetTimeInMS() 
					if ((RobotCurrentTime-leai->CurrentSampleTimeStamp[j])>1500) {							
						//no sample received in 1.5 second 

						//if (lemi->Thrust!=0 || lemi->LastThrust!=0) {
							//motor is still turning
							if (tinst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR) { //AnalogSensor inst has no EthMotors connected
								TurnMotor(lemi->mac[0],lemi->MotorNum[0],0,10,MOTOR_QUEUE); //stop motor
								if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
									TurnMotor(lemi->mac[1],lemi->MotorNum[1],0,10,MOTOR_QUEUE); //stop motor
								}
								lemi->Thrust=0; 
								lemi->LastThrust=0;
							} //if (tinst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR)
						fprintf(stderr,"EthAccel PCB %s has not sent samples in >1.5 second. Ending task (stage %d).\n",RStatus.EthPCBInfo[leai->PCBNum[j]].Name,tinst->OrigStartTime); 
						//fprintf(stderr,"Timestamp=%u - %u > 1500\n",CurrentTimeInMS,leai->LastSampleTimeStamp[0]); 
						fprintf(stderr,"Timestamp=%u - %u > 1500\n",RobotCurrentTime,leai->CurrentSampleTimeStamp[j]); 
						leai->flags|=ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING;  //currently not used
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							sprintf(LogStr,"EthAccel PCB %s has not sent samples in >1.5 second. Ending task (stage %d).",RStatus.EthPCBInfo[leai->PCBNum[j]].Name,tinst->OrigStartTime); 
							LogRobotModelData(LogStr);
						} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						//AbortTask(&ltask);
						//ltask=0;  //task is done after AbortTask(), so clear local pointer
						//tlist=0; //abort the list of instructions waiting for a condition				
						ltask->flags|=ROBOT_TASK_ABORT_TASK; //AbortScript=1;
					} //if ((RobotCurrentTime-leai->CurrentSampleTimeStamp[i])>1500) {							
				} //if (RobotCurrentTime-leai->CurrentSampleTimeStamp[i]>19 ) { //not new accel sample (>19 ms- 10ms, but there can rarely be 1ms of gitter)
			}  //for(j=0;j<tinst->NumAngles;j++) {


			//fprintf(stderr,"8 ");
			if (!(ltask->flags&ROBOT_TASK_ABORT_TASK) && !SameTimeStamp) {
				//instruction acceleratometer samples are fresh and there has been no abort of the task/script

				//if a AnalogSensor inst, check Touch Sensor value and avoid all below code. Only needs above check that touch sample is fresh
				if (tinst->InstNum==ROBOT_INSTRUCTION_ANALOG_SENSOR) {
					leti=(EthAnalogSensorInstruction *)&tinst->ETInst;  //get shorthand again, because leti can change for different instructions

					//fprintf(stderr,"t ");
					//if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {	
					//	sprintf(LogStr,"Touch Time: %d",tinst->InstTime);
					//	LogRobotModelData(LogStr);
					//} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

					//if there is a delay wait to check the AnalogSensor value
					if (leti->Delay<=tinst->InstTime) {
						//Delay has passed, ok to check AnalogSensor value now
						//fprintf(stderr,"d ");
						//if AnalogSensor is GT or LT Value
						//get Touch sensor value
						TouchSample=leti->PCB->Analog[leti->AnalogSensorNum].Percent;
						//fprintf(stderr,"%03.1f ",TouchSample); 

#if 0 
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {	
							sprintf(LogStr,"Touch Sensor %s value %03.2f (GTorLT=%d) %03.2f, stage %d",ROBOT_ANALOG_SENSOR_NAMES[leti->AnalogSensorNameNum],TouchSample,leti->GTorLT,leti->Value,ltask->StageNumber);
							LogRobotModelData(LogStr);
						} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
#endif

						if (leti->GTorLT==0) {
							//if sample is > user value, stop stage
							//fprintf(stderr,"gt TS=%3.2f V=%3.2f SN=%d ",TouchSample,leti->Value,leti->AnalogSensorNum);
							if (TouchSample>leti->Value) {
								//fprintf(stderr,"TS ");
								ltask->flags|=ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE;  //end stage - this is not enough though, since, another instruction could clear this flag
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {	
									//sprintf(LogStr,"Touch Sensor %s value %03.2f > %03.2f, ending stage %d",ROBOT_ANALOG_SENSOR_NAMES[leti->AnalogSensorNameNum],TouchSample,leti->Value,ltask->StageNumber);
									sprintf(LogStr,"Touch Sensor %s value %03.2f > %03.2f, ending stage %d",RStatus.AnalogSensorInfo[leti->AnalogSensorNameNum].Name,TouchSample,leti->Value,ltask->StageNumber);
									LogRobotModelData(LogStr);
								} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							} //if (TouchSample>leti->Value) {
						} else { //if (leti->GTorLT==0) {
							//if sample is > user value, stop stage
							if (TouchSample<leti->Value) {
								ltask->flags|=ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE;  //end stage
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {	
									sprintf(LogStr,"Touch Sensor %s value %03.2f < %03.2f, ending stage %d",RStatus.AnalogSensorInfo[leti->AnalogSensorNameNum].Name,TouchSample,leti->Value,ltask->StageNumber);
									LogRobotModelData(LogStr);
								} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							} //if (TouchSample<leti->Value) {
						} //if (leti->GTorLT==0) {
					} //if (leti->Delay>=(ltask->StartTime-ltask->StageNumber)) {											
				} else {  //if (tinst->InstNum==ROBOT_INSTRUCTION_ANALOG_SENSOR) {
					//Not AnalogSensor instruction


					//Now that the samples are fresh, if this is the first time the instruction is being run,
					//if any accel has the Current_Angle or Current_Angle_Plus flag set, 
					//set the angle to the current angle or current angle plus an offset.
					//or if any angle variable is used, set that (previously stored) angle as the target angle now.
					//Also set the lowest and highest angles in the target angle range.
					//This is where leai->oAngle[] is set for all accelerometers too.
					if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
						for (j=0;j<tinst->NumAngles;j++) {
							//todo: probably SetCurrentAngle flag is not needed to check or be set here, since INSTRUCTION_START only happens once at the start of the instruction
//												if (!(tinst->flagSetCurrentAngle&(1<<j)) && (tinst->flagUseCurrentAngle&(1<<j) || tinst->flagUseCurrentAnglePlus&(1<<j)) ) {
							if ((tinst->flagUseCurrentAngle&(1<<j)) || (tinst->flagUseCurrentAnglePlus&(1<<j))) {
								//temporarily get the Segment Angle for Accel[0] 
								leai->CurrentAngle=leth[j]->Accel[leai->AccelNum[j]].EstAngle[leai->xyz[j]];
								SegmentAngle=leai->CurrentAngle;
								//fprintf(stderr,"0 CurrentAnglePlus 0 SegmentAngle=%4.2f Offset=%4.2f\n",SegmentAngle,leai->AngOffset[0]);		
								if (tinst->flagUseCurrentAngle&(1<<j)) {
									leai->oAngle[j][0]=SegmentAngle;	
								} else {
									leai->oAngle[j][0]=SegmentAngle+leai->AngOffset[j];		//USE_CURRENT_ANGLE_ADD
									//fprintf(stderr,"1 CurrentAnglePlus 0 SegmentAngle=%4.2f Offset=%4.2f\n",SegmentAngle,leai->AngOffset[0]);		
								} //if (tinst->flagUseCurrentAngle&(1<<j)) {
								leai->oAngle[j][1]=leai->oAngle[j][0]-leai->oDegOff[j][0]; //DegOff[0] and [1] are always positive
								leai->oAngle[j][2]=leai->oAngle[j][0]+leai->oDegOff[j][1]; 
								//tinst->flagSetCurrentAngle|=(1<<j); //set SetCurrentAngle flag, otherwise this code could try to set the CurrentAngle again
							} //if ((tinst->flagUseCurrentAngle&(1<<j)) || (tinst->flagUseCurrentAnglePlus&(1<<j))) {

							//if ETHACCELS_INSTRUCTION_USE_ANGLE_VARIABLE use the Angle value (stored earlier) from the Angle variable array
							if (tinst->flagUseAngleVariable&(1<<j)) {
								leai->oAngle[j][0]=lscript->AngleVariable[leai->AngleVariableIndex[j]];
								leai->oAngle[j][1]=leai->oAngle[j][0]-leai->oDegOff[j][0]; //DegOff[0] and [1] are always positive
								leai->oAngle[j][2]=leai->oAngle[j][0]+leai->oDegOff[j][1]; 
								//fprintf(stderr,"Use %s for Accel %d Angle[%d]=%03.2f LineNum=%d\n",lscript->AngleVariableName[leai->AngleVariableIndex[j]],j,leai->AngleVariableIndex[j],leai->oAngle[j][0],tinst->LineNum);
							} //if (tinst->flagUseAngleVariable&(1<<j)) {

						} //for j
					} //if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {



					//calculate relative angles for any RELATIVE_ANGLE paired accels
					j=0;
					while(j<tinst->NumAngles-1) {  //last accel cannot be a relative angle accel
						if (tinst->flagRelativeAngle&(1<<j)) {
							//Accel j uses a relative angle (from accel j+1)
							//set TargetAngle to Accel[1]+offset
							leai->CurrentAngle=leth[j]->Accel[leai->AccelNum[j]].EstAngle[leai->xyz[j]];
							//set Accel[0] TargetAngle to Accel[1] + user given offset  ***RELATIVE ANGLE IS A0 + A1 (NOT MINUS)***
							leai->oAngle[j][0]=leth[j+1]->Accel[leai->AccelNum[j+1]].EstAngle[leai->xyz[j+1]] + leai->AngOffset[j];												
							leai->oAngle[j][1]=leai->oAngle[j][0]-leai->oDegOff[j][0]; //DegOff[0] and [1] are always positive
							leai->oAngle[j][2]=leai->oAngle[j][0]+leai->oDegOff[j][1];  									

							//only print if LastUseAngle (or perhaps move below UseAngle code and use j==UseAngle)
							if (j==leai->LastUseAngle && RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									sprintf(LogStr,"TargetAngle updated: %s Accel[%d] (%s %s %02.1f %02.1f %02.1f)",lemi->MotorReadableName,j,leai->SensorReadableName[j],XYZ[leai->xyz[j]],leai->oAngle[j][0],leai->oAngle[j][1],leai->oAngle[j][2]);
									LogRobotModelData(LogStr);
							} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {														
							j++; //skip next accel (used to determine relative angle)
						} //if (tinst->flagRelativeAngle&(1<<j)) {
						j++; //go to next accel
					} //while(j<tinst->NumAngles-1) {

					//fprintf(stderr,"9 ");

					//START - DETERMINE WHICH ACCEL TO USE - for instructions with more than one accel, determine which accel to use
					//This section only applies to instructions with more than 1 accelerometer
					//Todo: revisit and make more simple, uniform
					//Determine which accelerometer to use. Methods currently available to determine which accelerometer to use:										
					//1) IF_A3_THEN_A1_ELSE_A2
					//2) IF_A2_THEN_A0_ELSE_A1
					//3) IF_A2_AND_A3_THEN_A0_ELSE_A1
					//4) FARTHEST_FROM_TARGET: Use the accelerometer that is farthest from its target angle
					//5) GYRO WITH MOST_VELOCITY: Use the accelerometer/gyroscope that has the most velocity
					//6) FIRST OUT OF RANGE (default method): Use the first accelereometer (Accel[0]) and if in range, use the next accelerometer, and continue this way until finding an accelerometer that is either out of range or is the last accelerometer.

					UseAngle=0;  //by default use Accel[0]
					//if number of accels in this inst is >1 and (currently) no accels are synced together
					if (tinst->NumAngles>1 && !(tinst->flags&ROBOT_INSTRUCTION_SYNC_TWO_ACCELS)) {

						//if IF_A3_THEN_A1_ELSE_A2 flag is set, if accel0 is not in range then use accel0 else (if accel2 is in range, use accel0, else use accel1)
						if (tinst->flags&ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2) {
							//IF_A3=if a0 then (a3 then a1 else a2) else a0
							//Determine if Angle of Accel0 is in range
							TempAngle=leth[0]->Accel[leai->AccelNum[0]].EstAngle[leai->xyz[0]];											
							if ((TempAngle>=leai->oAngle[0][1]) && (TempAngle<=leai->oAngle[0][2])) {
								//Angle of Accel0 is in range
								//Determine if the Angle of Accel3 is in range
								TempAngle=leth[3]->Accel[leai->AccelNum[3]].EstAngle[leai->xyz[3]];											
								if ((TempAngle>=leai->oAngle[3][1]) && (TempAngle<=leai->oAngle[3][2])) {
									//Angle of Accel3 is in range
									UseAngle=1;
								} else { //if (TempAngle>=leai->oAngle[3][1] && TempAngle<=leai->oAngle[3][2]) {
									UseAngle=2;
								} //elseif (TempAngle>=leai->oAngle[3][1] && TempAngle<=leai->oAngle[3][2]) {
							} else { //if (TempAngle>=leai->oAngle[0][1] && TempAngle<=leai->oAngle[0][2]) {
								UseAngle=0;
							} //elseif (TempAngle>=leai->oAngle[0][1] && leai->CurrentAngle<=leai->oAngle[0][2]) {

						} else { //if (tinst->flags&ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2) {

							//if IF_A2_THEN_A0_ELSE_A1 flag is set, if accel2 is in range, use accel0, otherwise use accel1
							if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1) {
								//Determine if the Angle of Accel2 is in range
								laccel=&leth[2]->Accel[leai->AccelNum[2]];
								TempAngle=laccel->EstAngle[leai->xyz[2]];											
								AngleInRange=0;
								if ((TempAngle>=leai->oAngle[2][1]) && (TempAngle<=leai->oAngle[2][2])) {
									AngleInRange=1;
								} else { //if ((TempAngle>=leai->oAngle[2][1]) && (TempAngle<=leai->oAngle[2][2])) {
									if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
										sprintf(LogStr,"Angle not in range for %s Accel[2] %s %02.1f (not >= %02.1f and <= %02.1f)",leai->SensorReadableName[2],XYZ[leai->xyz[2]],TempAngle,leai->oAngle[2][1],leai->oAngle[2][2]);
										LogRobotModelData(LogStr);
									} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
								} //if ((TempAngle>=leai->oAngle[2][1]) && (TempAngle<=leai->oAngle[2][2])) {
								//check for a required velocity unless AngleInRange and A2 operator is OR
								if ((tinst->flagRequireAccelVelocity&4) && (!(AngleInRange && tinst->flagRequireVelocityOperators[2]&2)) ) {  //4=accel2
									//Accel has a velocity requirement too
									if (tinst->flagRequireVelocityOperators[2]&2) {  //if OR then set AngleInRange=1 by default, otherwise will AND with existing AngleInRange value
										AngleInRange=1;
									}
									AccelVelocity=laccel->GSample.i[leai->xyz[2]];
									if (tinst->flagRequireVelocityOperators[2]&1) { 
										//>
										if (AccelVelocity<=tinst->RequiredVelocity[2]) {
											if (RStatus.flags&ROBOT_STATUS_MODEL_LOG && AngleInRange) {
												sprintf(LogStr,"Velocity not in range for %s Accel[2] %s (%02.1f is not > %02.1f)",leai->SensorReadableName[2],XYZ[leai->xyz[2]],AccelVelocity,tinst->RequiredVelocity[2]);
												LogRobotModelData(LogStr);
											} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
											AngleInRange=0; //velocity requirement not met
										}
									} else { //if (tinst->flagRequireVelocityOperators[2]&1) { 
										//<
										if (AccelVelocity>=tinst->RequiredVelocity[2]) {
											if (RStatus.flags&ROBOT_STATUS_MODEL_LOG && AngleInRange) {
												sprintf(LogStr,"Velocity not in range for %s Accel[2] %s (%02.1f is not < %02.1f)",leai->SensorReadableName[2],XYZ[leai->xyz[2]],AccelVelocity,tinst->RequiredVelocity[2]);
												LogRobotModelData(LogStr);
											} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
											AngleInRange=0; //velocity requirement not met
										}
									} //if (tinst->flagRequiredVelocityOperator2[2]&1) { 
								} //if (tinst->flagRequireAccelVelocity&4) {
								//A2 is in range, and either no RequiredVelocity, or RequiredVelocity is also in range 														
								if (AngleInRange) {
									//Angle of Accel2 is in range
									UseAngle=0;
								} else { //if (TempAngle>=leai->oAngle[2][1] && TempAngle<=leai->oAngle[2][2]) {
									UseAngle=1;
								} //if (TempAngle>=leai->oAngle[2][1] && leai->CurrentAngle<=leai->oAngle[2][2]) {
							} else { //if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1) {

								if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1) {
									//IF_A2_AND_A3=if a2 and a3 then a0 else a1, IF_A2_AND_A3=  if a2 and a3 then a0 else a1
									//Determine if Angle of Accel2 and Angle of Accel3 are in range
									laccel=&leth[2]->Accel[leai->AccelNum[2]];
									TempAngle=laccel->EstAngle[leai->xyz[2]];											
									AngleInRange=0;
									if ((TempAngle>=leai->oAngle[2][1]) && (TempAngle<=leai->oAngle[2][2])) {
										AngleInRange=1;
									} else {
										if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
											sprintf(LogStr,"Angle not in range for %s Accel[2] %s %02.1f (not >= %02.1f and <= %02.1f)",leai->SensorReadableName[2],XYZ[leai->xyz[2]],TempAngle,leai->oAngle[2][1],leai->oAngle[2][2]);
											LogRobotModelData(LogStr);
										} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									} //if ((TempAngle>=leai->oAngle[2][1]) && (TempAngle<=leai->oAngle[2][2])) {
									//check for a required velocity, unless AngleInRange and A2 operator is OR
									if ((tinst->flagRequireAccelVelocity&4)  && (!(AngleInRange && tinst->flagRequireVelocityOperators[2]&2)) ) {  //4=accel2
										//Accel has a velocity requirement too
										AccelVelocity=laccel->GSample.i[leai->xyz[2]];
										if (tinst->flagRequireVelocityOperators[2]&2) {  //bit2=AND/OR if OR then set AngleInRange=1 by default, otherwise will AND with existing AngleInRange value
											AngleInRange=1;
										}
										if (tinst->flagRequireVelocityOperators[2]&1) { 
											//>
											if (AccelVelocity<=tinst->RequiredVelocity[2]) {
												AngleInRange=0; //velocity requirement not met
											}
										} else { //if (tinst->flagRequireVelocityOperators[2]&1) { 
											//<
											if (AccelVelocity>=tinst->RequiredVelocity[2]) {
												AngleInRange=0; //velocity requirement not met
											}
										} //if (tinst->flagRequireVelocityOperators[2]&1) { 
									} //if (tinst->flagRequireAccelVelocity&4) {

									if (AngleInRange) {
										//Angle of Accel2 is in range
										//Determine if the Angle of Accel3 is also in range
										laccel=&leth[3]->Accel[leai->AccelNum[3]];
										TempAngle=laccel->EstAngle[leai->xyz[3]];											
										AngleInRange=0;
										if ((TempAngle>=leai->oAngle[3][1]) && (TempAngle<=leai->oAngle[3][2])) {
											AngleInRange=1;
										} else {
											if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
												sprintf(LogStr,"Angle not in range for %s Accel[3] %s %02.1f (not >= %02.1f and <= %02.1f)",leai->SensorReadableName[3],XYZ[leai->xyz[3]],TempAngle,leai->oAngle[3][1],leai->oAngle[3][2]);
												LogRobotModelData(LogStr);
											} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
										} //if ((TempAngle>=leai->oAngle[3][1]) && (TempAngle<=leai->oAngle[3][2])) {
										//check for a required velocity (unless A3 in range and operator is OR)
										if ((tinst->flagRequireAccelVelocity&8) && (!(AngleInRange && tinst->flagRequireVelocityOperators[3]&2)) ) {  //8=accel3
											//Accel has a velocity requirement too
											if (tinst->flagRequireVelocityOperators[3]&2) {  //if OR then set AngleInRange=1 by default, otherwise will AND with existing AngleInRange value
												AngleInRange=1;
											}
											AccelVelocity=laccel->GSample.i[leai->xyz[3]];
											if (tinst->flagRequireVelocityOperators[3]&1) { 
												//>
												if (AccelVelocity<=tinst->RequiredVelocity[3]) {
													if (RStatus.flags&ROBOT_STATUS_MODEL_LOG && AngleInRange) {
														sprintf(LogStr,"Velocity not in range for %s Accel[3] %s (%02.1f is not > %02.1f)",leai->SensorReadableName[3],XYZ[leai->xyz[3]],AccelVelocity,tinst->RequiredVelocity[3]);
														LogRobotModelData(LogStr);
													} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
													AngleInRange=0; //velocity requirement not met
												}
											} else { //if (tinst->flagRequireVelocityOperators[3]&1) { 
												//<
												if (AccelVelocity>=tinst->RequiredVelocity[3]) {
													if (RStatus.flags&ROBOT_STATUS_MODEL_LOG && AngleInRange) {
														sprintf(LogStr,"Velocity not in range for %s Accel[3] %s (%02.1f is not < %02.1f)",leai->SensorReadableName[3],XYZ[leai->xyz[3]],AccelVelocity,tinst->RequiredVelocity[3]);
														LogRobotModelData(LogStr);
													} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
													AngleInRange=0; //velocity requirement not met
												}
											} //if (tinst->flagRequireVelocityOperators[3]&1) { 
										} //if (tinst->flagRequireAccelVelocity&8) && (!(AngleInRange && tinst->flagRequireVelocityOperators[3]&2)) ) {  //8=accel3

										if (AngleInRange) {
											//Angle of Accel3 is also in range
											UseAngle=0;
										} else { //if (AngleInRange) {
											UseAngle=1;
										} //elseif (AngleInRange) {
									} else { //if ((AngleInRange) {
										UseAngle=1;
									} //if (AngleInRange) {


								} else { //if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1) {


									//if FARTHEST_FROM_TARGET flag is set, select the accel that is the farthest from the TargetAngle with no regard to if either accel is in range.
									if (tinst->flags&ROBOT_INSTRUCTION_FARTHEST_FROM_TARGET) {
										//compare distance of each Accel to their TargetAngle, and use the Accel with the highest distance
										j=0;
										LargestAngle=leth[0]->Accel[leai->AccelNum[0]].EstAngle[leai->xyz[0]];
										LargestAngle-=leai->oAngle[0][0];
										if (tinst->flagRelativeAngle&1) {
											j=2;  //skip relative angle accel
										} else {
											j=1; 
										}
										while (j<tinst->NumAngles) {
											TempAngle=leth[j]->Accel[leai->AccelNum[j]].EstAngle[leai->xyz[j]];
											TempAngle-=leai->oAngle[j][0];
											if (fabs(TempAngle)>fabs(LargestAngle)) {
												UseAngle=j;
												LargestAngle=TempAngle;
											} 
											if (tinst->flagRelativeAngle&(1<<j)) {
												j++;  //skip relative angle accel
											} 
											j++; //go to next accel
										} //while j

										if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
												sprintf(LogStr,"FarthestAngle: %s Accel[%d] (%s %s %02.1f)",lemi->MotorReadableName,UseAngle,leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],LargestAngle);
												LogRobotModelData(LogStr);
										} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {		

									} else { //if (tinst->flags&ROBOT_INSTRUCTION_FARTHEST_FROM_TARGET) {
				

										//if MOST_VELOCITY flag is set, select the accel that has the most velocity with no regard to if either accel is in range.
										//TODO: accomodate more than 1 relative angle
										if (tinst->flags&ROBOT_INSTRUCTION_MOST_VELOCITY) {
											//compare Velocity of each Accel, and use the Accel with the most Velocity
											laccel=&leth[0]->Accel[leai->AccelNum[0]]; //shortcut to Accel
											HighestVelocity=laccel->GSample.i[leai->xyz[0]];
											if (tinst->flagRelativeAngle&1) {
												j=2;  //skip relative angle accel
											} else {
												j=1; 
											}
											while (j<tinst->NumAngles) {
												laccel=&leth[j]->Accel[leai->AccelNum[j]]; //shortcut to Accel
												Velocity=laccel->GSample.i[leai->xyz[j]];
												if (fabs(Velocity)>fabs(HighestVelocity)) {
													UseAngle=j;
													HighestVelocity=Velocity;
												}

												if ((tinst->flagRelativeAngle&(1<<j)) || (tinst->flagIgnoreAccel&(1<<j))) {
													j++;  //skip relative angle (or ignored) accel
												} 
												j++; //go to next accel													
											} //while j

										} else { //if (tinst->flags&ROBOT_INSTRUCTION_MOST_VELOCITY) {

											//DEFAULT METHOD
											//Not choosing the Accel with the largest distance from target angle, or with the most velocity. 
											//Use the default method of choosing between multiple accels: 
											//try Accel[0] first, and if in range, use Accel[1], etc. until an accel that is out of range is found or the last accel is reached.
											j=0;
											while(j<tinst->NumAngles) {
												if (tinst->flagIgnoreAccel&(1<<j)) {
													//do not check any accels with the IgnoreAccel bit set
													j++;
												} else {  //if (tinst->flagIgnoreAccel&&(1<<j)) {
								
													if (leai->AngleKind[j]&ANGLE_KIND_ACCEL) {
														//accelerometer angle
														leai->CurrentAngle=leth[j]->Accel[leai->AccelNum[j]].EstAngle[leai->xyz[j]];											
													} else {
														//potentiometer angle 
														leai->CurrentAngle=leth[j]->Analog[leai->AnalogSensorNum[j]].Angle;
													}
													if (leai->CurrentAngle>=leai->oAngle[j][1] && leai->CurrentAngle<=leai->oAngle[j][2]) {
														//Accel j is in range, so go to the next accel

														//print a log message if instruction start or accel was out of range
														if ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || !(tinst->flagAngleInRange&(1<<j))) {
															tinst->flagAngleInRange|=(1<<j); //set AngleInRange flag
															if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
																sprintf(LogStr,"Angle in range '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f)",leai->SensorReadableName[j],XYZ[leai->xyz[j]],lemi->MotorReadableName,leai->CurrentAngle,leai->oAngle[j][1],leai->oAngle[j][0],leai->oAngle[j][2]);
																LogRobotModelData(LogStr);
															} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

															if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tAngle %d in range1 '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f\n",ltask->TaskTime,tinst->InstTime,j,leai->SensorReadableName[j],XYZ[leai->xyz[j]],lemi->MotorReadableName,leai->CurrentAngle,leai->oAngle[j][1],leai->oAngle[j][0],leai->oAngle[j][2]);
															} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			 
														} //if ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || !(tinst->flagAngleInRange&(1<<j))) {

														UseAngle=j; //just in case this is the last accel
														if (tinst->flagRelativeAngle&(1<<j)) {
															j++;  //skip relative angle accel
															//fprintf(stderr,"skiprel=%d ",j);
														} 
														j++; //go to next accel		
													} else {	//if (leai->CurrentAngle>=leai->oAngle[j][1] && leai->CurrentAngle<=leai->oAngle[j][2]) {
														//Accel j is not in range, so use this accel
														UseAngle=j;
														//Note: Do not clear the flagAngleInRange bit here, because this accel angle will be compared again below 
														//fprintf(stderr,"UA%d ",UseAngle);
#if 0 
														if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {																				
															sprintf(LogStr,"LUA=UA=%d Angle NOT in range '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f)",(leai->LastUseAngle==UseAngle),leai->SensorName[j],XYZ[leai->xyz[j]],lemi->MotorReadableName,leai->CurrentAngle,leai->oAngle[j][1],leai->oAngle[j][0],leai->oAngle[j][2]);
															LogRobotModelData(LogStr);
														} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
#endif
														//and exit while loop
														j=tinst->NumAngles;
													} //else if (leai->CurrentAngle>=leai->oAngle[j][1] && leai->CurrentAngle<=leai->oAngle[j][2]) {
												} //if (tinst->flagIgnoreAccel&&(1<<j)) {
											} //while j
										} //if (tinst->flags&ROBOT_INSTRUCTION_MOST_VELOCITY) {
									} //if (tinst->flags&ROBOT_INSTRUCTION_FARTHEST_FROM_TARGET) {
								} //if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1) {
							} //if (tinst->flags&ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1) {
						} //if (tinst->flags&ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2) {
					} //if (tinst->NumAngles>1) {


					//fprintf(stderr,"10 ");
					//fprintf(stderr,"UA%d ",UseAngle);

					//Apply any flags that change MaxThrust, etc.
					//1) CHECK IF_NOT_A#_MAXTHRUST, to see if inst sets MaxThrust
					//if (tinst->flags&ROBOT_INSTRUCTION_IF_NOT_A_MAXTHRUST) {
					if (tinst->flagInstAccelMaxThrust) { //at least one accel has IF_NOT_A_MAXTHRUST=
						//determine which accels (1, 2, 3, or 4) decide MaxThrust for other accel(s)
						//current MaxThrust is set for all other accels, so last
						for(TempAccel=0;TempAccel<tinst->NumAngles;TempAccel++) {
							if (tinst->flagInstAccelMaxThrust&(1<<TempAccel)) {


								//IF_NOT_A_MAXTHRUST =if a1 (a2, a3, or a4 for those instances) is out of range, then set MaxThrust= to User given value
								//Determine if Angle of MaxThrust deciding Accel is in range 
								TempAngle=leth[TempAccel]->Accel[leai->AccelNum[TempAccel]].EstAngle[leai->xyz[TempAccel]];											
								if ((TempAngle>=leai->oAngle[TempAccel][1]) && (TempAngle<=leai->oAngle[TempAccel][2])) {
									//TempAngle is in range
									//if it was previously not in range, then set MaxThrust back to what it was 
									//go through each accel
									//make use of InRange flag? no, since flag could eventually target specific accel tinst->flagAngleInRange|=(1<<j); //set AngleInRange flag 
									for(j=0;j<tinst->NumAngles;j++) {
										if (tinst->flagInstSetMaxThrust&(1<<j)) {
											//inst had set MaxThrust before this, so set MaxThrust back and clear flag
											lemi->MaxThrust[j]=lemi->LastMaxThrust[j];
											tinst->flagInstSetMaxThrust&=~(1<<j); //clear flag
										} //if (tinst->flagInstSetMaxThrust&(1<<j)) {
									} //for j
								} else {  //if ((TempAngle>=leai->oAngle[TempAccel][1]) && (TempAngle<=leai->oAngle[TempAccel][2])) {
									//TempAngle is out of range so set MaxThrust (unless already set)
									//go through each accel
									for(j=0;j<tinst->NumAngles;j++) {
										//probably should ignore accels with ignore flag
										if (!(tinst->flagInstSetMaxThrust&(1<<j))) {
											//inst had not set MaxThrust before this, so save current MaxThrust, set MaxThrust to inst MaxThrust, and set flag
											//have to be careful not to overwrite LastMaxThrust when for example A1=5 and A2=10 A1 sets to 5, then A2 would set LastThrust as 5
											lemi->LastMaxThrust[j]=lemi->MaxThrust[j];  //save current MaxThrust
										} //if (!(tinst->flagInstSetMaxThrust&(1<<j))) {
										lemi->MaxThrust[j]=tinst->InstMaxThrust[TempAccel]; //change to Inst MaxThrust
										tinst->flagInstSetMaxThrust|=(1<<j); //set flag indicating that the inst set the MaxThrust for this accel (j)
									} //for j
								} //elseif ((TempAngle>=leai->oAngle[TempAccel][1]) && (TempAngle<=leai->oAngle[TempAccel][2])) {
							} //for TempAccel
						} //if (tinst->flagInstAccelMaxThrust&(1<<j)) {
					} //if (tinst->flagInstAccelMaxThrust) {


					//if changing accels, reset the SEGMENT_ANGLE_IN_RANGE flag, because otherwise if accel0 is in range, and accel1 is not in range, the !InRange code below will set an initial thrust and that is a problem when MAINTAIN_THRUST was set in the previous instruction that used the same motor. 
//										leai->flags&=~ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE; 

					//get a pointer to the current accelerometer
					laccel=&leth[UseAngle]->Accel[leai->AccelNum[UseAngle]]; //shortcut to Accel
					//if the UseAngle uses a potentiometer to determine angle, velocity and acceleration, get a pointer to lanalog
					if (leai->AngleKind[UseAngle]&ANGLE_KIND_ANALOG) {
						lanalog=&leth[UseAngle]->Analog[leai->AnalogSensorNum[UseAngle]];
					}

					//If changing from Gyro to Accel, change TargetAngle by Gyro-Accel offset
					//X axis (Pitch) or Z axis (Roll)
					//todo: Note that this may be a problem if re-running a script- because the target angle may change
					if ( ((laccel->flags&ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_X) && leai->xyz[UseAngle]==0) ||  
							 ((laccel->flags&ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_Z) && leai->xyz[UseAngle]==2) ) {
						//Changing from Gyro X to Accel X on this accelerometer
						toffset=laccel->GyroAccelOffset[leai->xyz[UseAngle]];
						leai->oAngle[UseAngle][0]-=toffset;
						leai->oAngle[UseAngle][1]-=toffset;
						leai->oAngle[UseAngle][2]-=toffset;
						//output new TartgetAngle to Model log
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							sprintf(LogStr,"Changed TargetAngle '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f) Offset=%02.1f",leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],lemi->MotorReadableName,leai->CurrentAngle,leai->oAngle[UseAngle][1],leai->oAngle[UseAngle][0],leai->oAngle[UseAngle][2],toffset);
							//fprintf(stderr,"Angle reached\n");
							LogRobotModelData(LogStr);
						} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

					} //if (laccel->flags&ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_X) {


					//Moved here to add final TargetAngle to ModelLog
					//Add a log entry if UseAngle has changed (note that this can happen after an instruciton is started too)
					if (leai->LastUseAngle!=UseAngle) {
						//need? MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[0],lemi->MotorNum[0]);									
						leai->flags&=~ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; //clear initial thrust flag, because when changing accels a new initial thrust should be sent - todo: move to Accel based flagSentInitialThrust

						//Log change of accel for motor
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
//														sprintf(LogStr,"Changing Motor %s from Accel[%d] (%s %s) to Accel[%d] (%s %s). Thrust not set to 0",lemi->MotorReadableName,leai->LastUseAngle,leai->SensorName[leai->LastUseAngle],XYZ[leai->xyz[leai->LastUseAngle]],UseAngle,leai->SensorName[UseAngle],XYZ[leai->xyz[UseAngle]]);
							sprintf(LogStr,"Changing %s from Angle[%d] (%s %s %02.1f %02.1f %02.1f) to Angle[%d] (%s %s %02.1f %02.1f %02.1f)",lemi->MotorReadableName,leai->LastUseAngle,leai->SensorReadableName[leai->LastUseAngle],XYZ[leai->xyz[leai->LastUseAngle]],leai->oAngle[leai->LastUseAngle][0],leai->oAngle[leai->LastUseAngle][1],leai->oAngle[leai->LastUseAngle][2],UseAngle,leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],leai->oAngle[UseAngle][0],leai->oAngle[UseAngle][1],leai->oAngle[UseAngle][2]);
							LogRobotModelData(LogStr);
						} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tChanging %s from Angle[%d] (%s %s %02.1f %02.1f %02.1f) to Angle[%d] (%s %s %02.1f %02.1f %02.1f)\n",ltask->TaskTime,tinst->InstTime,lemi->MotorReadableName,leai->LastUseAngle,leai->SensorReadableName[leai->LastUseAngle],XYZ[leai->xyz[leai->LastUseAngle]],leai->oAngle[leai->LastUseAngle][0],leai->oAngle[leai->LastUseAngle][1],leai->oAngle[leai->LastUseAngle][2],UseAngle,leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],leai->oAngle[UseAngle][0],leai->oAngle[UseAngle][1],leai->oAngle[UseAngle][2]);
						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {		
					} else { //if (leai->LastUsedAccel!=UseAngle) {
						//UseAngle is the same as the LastUsedAccel
						//make log entry for initial UseAngle
						if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
							//instruction is starting - first pass

							//Log which accel motor is using
							if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
//														sprintf(LogStr,"Changing Motor %s from Accel[%d] (%s %s) to Accel[%d] (%s %s). Thrust not set to 0",lemi->MotorReadableName,leai->LastUseAngle,leai->SensorName[leai->LastUseAngle],XYZ[leai->xyz[leai->LastUseAngle]],UseAngle,leai->SensorName[UseAngle],XYZ[leai->xyz[UseAngle]]);
								sprintf(LogStr,"Initial: %s using Angle[%d] (%s %s %02.1f %02.1f %02.1f)",lemi->MotorReadableName,UseAngle,leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],leai->oAngle[UseAngle][0],leai->oAngle[UseAngle][1],leai->oAngle[UseAngle][2]);
								LogRobotModelData(LogStr);
							} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						} //if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
					} //if (leai->LastUsedAccel!=UseAngle) {


					//DONE: setting UseAngle and oAngle (original TargetAngle)

					//CALCULATE DISTANCE AND MOTION VALUES

					//Update the estimated angle of the body segment this accel is inside.
					//Note that during a motion, only use the gyroscope values, because 
					//they may be around 0.0 but only temporarily. Usually when, for example, 
					//when Gx=0.5 the accel is used to determine the body segment angle.

					//just set SegmentAngle to Accel[0]- the angle of the body segment relative to the ground
					//for MotorAccel instructions with 2 accels if Accel[0] is in range use Accel[1], otherwise use Accel[0]
					
					//if instruction Angle uses potentiometer to determine angle, use the AnalogSensor angle, velocity and acceleration information
					if (leai->AngleKind[UseAngle]&ANGLE_KIND_ANALOG) {
						//determine body segment angle using potentiometer angle
						leai->CurrentAngle=lanalog->Angle;
						leai->LastAngle=lanalog->LastAngle;											
						Velocity=lanalog->Velocity;										
						Acceleration=laccel->Acceleration.i[leai->xyz[UseAngle]];
						Jerk=0.0;
						leai->LastVelocity=lanalog->LastVelocity;
					} else { //										if (leai->AngleKind[UseAngle]&ANGLE_KIND_ANALOG) {										
						leai->CurrentAngle=laccel->EstAngle[leai->xyz[UseAngle]];
						leai->LastAngle=laccel->LastEstAngle[leai->xyz[UseAngle]];
						Velocity=laccel->GSample.i[leai->xyz[UseAngle]];
						Acceleration=laccel->Acceleration.i[leai->xyz[UseAngle]];
						Jerk=laccel->Jerk.i[leai->xyz[UseAngle]];
						leai->LastVelocity=laccel->LastGSample.i[leai->xyz[UseAngle]];
					} //										if (leai->AngleKind[UseAngle]&ANGLE_KIND_ANALOG) {

					//save Velocity										
					leai->Velocity=Velocity;


					//move back last 4 acceleration samples - todo: remove mixing accel of multiple accelerometers, perhaps make double array
					for(i=4;i>0;i--) {
						leai->Acceleration[i]=leai->Acceleration[i-1];
					}
					leai->Acceleration[0]=Acceleration;
					AccelAvg3=Acceleration+leai->Acceleration[1]+leai->Acceleration[2];
					//AccelAvg5=(AccelAvg3+leai->Acceleration[3]+leai->Acceleration[4])/5.0;
					AccelAvg3/=3.0;

					leai->LastAngleChange=leai->AngleChange; //save last angle change - can use laccel->EstAngleChange[xyz]
					leai->AngleChange=leai->CurrentAngle-leai->LastAngle; //calculate current angle change
				
					SegmentAngle=leai->CurrentAngle;  //shorthand to CurrentAngle										
					//SegmentAngleChange=leai->AngleChange;	//shorthand to AngleChange (velocity)
					//DeltaSegmentAngleChange=SegmentAngleChange-leai->LastAngleChange; //shorthand to change in last two angle changes (acceleration)

					//update remaining angle (distance)
					leai->RemainingAngle=leai->TotalAngle-SegmentAngle;


					//check for AbortScript if Velocity is too high
					if ((tinst->flagAbortVelocity&(1<<UseAngle)) && fabs(Velocity)>=leai->AbortVelocity[UseAngle]) {
						sprintf(LogStr,"ABORT VELOCITY: %s %s Velocity %4.2f >= abort velocity of %4.2f, aborting script (stage %d).",leai->SensorReadableName[UseAngle],  XYZ[leai->xyz[UseAngle]],Velocity,leai->AbortVelocity[UseAngle],ltask->StageNumber);
						ltask->flags|=ROBOT_TASK_ABORT_TASK;//AbortScript=1;
					} 
					//check for AbortScript if Angle is too high
					if ((tinst->flagAbortAngle&(1<<UseAngle)) && fabs(SegmentAngle)>=leai->AbortAngle[UseAngle]) {
						sprintf(LogStr,"ABORT ANGLE: %s %s SegmentAngle %4.2f >= abort angle of %4.2f, aborting script (stage %d).",leai->SensorReadableName[UseAngle],  XYZ[leai->xyz[UseAngle]],SegmentAngle,leai->AbortAngle[UseAngle],ltask->StageNumber);
						ltask->flags|=ROBOT_TASK_ABORT_TASK;//AbortScript=1;
					} 


					if (ltask->flags&ROBOT_TASK_ABORT_TASK) {
						fprintf(stderr,"%s\n",LogStr);
						if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							LogRobotModelData(LogStr);
						}
						ltask->flags&=~ROBOT_TASK_ABORT_TASK;//need in order to run another script after this aborted script ?
						AbortTask(&ltask);
						tlist=0; //abort the list of instructions waiting for a condition
						ltask=0;
						//AbortScript=0; //need in order to run another script after this aborted script
					} else { //if (ltask->flags&ROBOT_TASK_ABORT_TASK) {


															

						//Note that this overwrites anything CurrentAngle or Angle variable may set 
/* removed for now, just use angles in inst */
/*
						if (tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION) {
							//if OPPOSE_MOTION, set TargetAngle in direction opposite of motion, 
							//since motor needs to thrust against motion.
							leai->oAngle[UseAngle*3]=SegmentAngle-Velocity*0.04; //Opposite of Future Angle x4									 
							leai->oAngle[UseAngle*3+1]=leai->oAngle[UseAngle*3]-0.5;//leai->DegOff[0]; //DegOff[0] and [1] are aways positive
							leai->oAngle[UseAngle*3+2]=leai->oAngle[UseAngle*3]+0.5;//leai->DegOff[1];  
						} 

*/

						//for 2 accels, if accel[0] is not in range use accel[0], otherwise use accel[1]
						leai->Angle[0]=leai->oAngle[UseAngle][0];
						leai->Angle[1]=leai->oAngle[UseAngle][1];
						leai->Angle[2]=leai->oAngle[UseAngle][2];
						//currently there is no need for DegOff
						//leai->DegOff[0]=leai->oDegOff[UseAngle*2];
						//leai->DegOff[1]=leai->oDegOff[UseAngle*2+1];

						TargetAngle=leai->Angle[0]; //shorthand to target angle	


						//Get shorthand for MaxThrust for this Motor
						//and determine if with or against gravity
						//determine if with gravity or against gravity 
						//currently just checking that the magnitude of the current body segment angle is less than the magnitude of the target angle means that motor moving the body segment
						//is moving with gravity (is getting a gravity assist), and if the magnitude of the body segment angle is more than the magnitude of the target angle, the motor
						//is working against gravity (with gravity resist).
						//if magnitude of SegmentAngle is greater than magnitude of TargetAngle,
						// or both are different signs (the segment must cross 0 degrees), then
						//Segment is working against gravity- so the maximum thrust will be available.
						//lemi->MaxThrust is set in LoadScript() if user sets (same for lemi->MaxReverseThrust) 
						if ( (fabs(SegmentAngle)>fabs(TargetAngle) || 
								((SegmentAngle>0 && TargetAngle<0) || (SegmentAngle<0 && TargetAngle>0)) ) || 
								(tinst->flagIgnoreGravity&(1<<UseAngle)) ) {
							//against gravity (or ignore gravity- presumes against gravity)

							//If user and inst did not set MaxThrust (****MaxThrust is set in LoadScript() if user sets****)
							//set here depending on with or against gravity
							if (!(tinst->flagUserMaxThrust&(1<<UseAngle)) && !(tinst->flagInstSetMaxThrust&(1<<UseAngle))) {
								lemi->MaxThrust[UseAngle]=lemi->MaxThrustAG;
							}
							//same for MaxReverseThrust
							if (!(tinst->flagUserMaxReverseThrust&(1<<UseAngle))) {
								lemi->MaxReverseThrust[UseAngle]=lemi->MaxThrustWG;  //note with gravity, since is in reverse
							}
							WithGravity=0;
						} else {  //if ( (fabs(SegmentAngle)>fabs(TargetAngle) || ...
							//with gravity
							if (!(tinst->flagUserMaxThrust&(1<<UseAngle)) && !(tinst->flagInstSetMaxThrust&(1<<UseAngle))) {
								lemi->MaxThrust[UseAngle]=lemi->MaxThrustWG;
							}
							if (!(tinst->flagUserMaxReverseThrust&(1<<UseAngle))) {
								lemi->MaxReverseThrust[UseAngle]=lemi->MaxThrustAG;  //note against gravity, since is in reverse
							}
							WithGravity=1;
						} //if ( (fabs(SegmentAngle)>fabs(TargetAngle) || ...

						//If MotorAngleAssociation has ignore gravity flag set, set WithGravity=0
						if (tinst->flagIgnoreGravity&(1<<UseAngle)) {
							WithGravity=0;	
						} 								

						//Set IsMoving if the Segment is moving
						//if (Velocity>=1.0 || Velocity<=-1.0) {
						//Note that MotionThreshold can be different than GYROSCOPE_CALIBRATION_MINIMUM (the lowestr gyroscope value before the acclerometer data is used instead)										
						//todo: MotionThreshold will need to be MotionThreshold[0] or [1]
						if (fabs(Velocity)>leai->MotionThreshold[UseAngle]) {  //each Motor+Accelerometer Angle association can have their own IsMoving velocity threshold
							IsMoving=1;
						} else {
							IsMoving=0;
						} 

						//Determine if Segment is moving against gravity
						if ((SegmentAngle>0 && Velocity<0) || (SegmentAngle<0 && Velocity>0)) {
							MovingAgainstGravity=1;
						} else {
							MovingAgainstGravity=0;
						}
				
						//Determine if moving in correct direction
						VelocityInCorrectDirection=0;
						AccelerationInCorrectDirection=0;
						if (SegmentAngle<TargetAngle) {
							//Segment Angle needs to increase -30 to -5, -10 to 10, etc.
							AngNeedsToInc=1;
							if (Velocity>0) {
								VelocityInCorrectDirection=1;
							}
							if (Acceleration>0) {
								AccelerationInCorrectDirection=1;
							} 
						} else {
							//Segment Angle needs to decrease -20 to -30, 20 to 10, etc.
							AngNeedsToInc=0;
							if (Velocity<0) {
								VelocityInCorrectDirection=1;
							} 
							if (Acceleration<0) {
								AccelerationInCorrectDirection=1;
							} 
						} //if (SegmentAngle<TargetAngle) {


						//Calculate the force exerted by the segment from gravity using the segments current angle
						//The force that assists or resists the motor from the segment depends on which motor is associated with the accel.
						ForceWeight=fabs(sinf(SegmentAngle*PI/180.0))*leai->Weight[UseAngle];

						//Determine the motor thrust needed to produce the desired target velocity
						//if with gravity, the weight of the segment assists the motor
						//if not moving, there is a force (weight) of static friction to overcome 
						//but if moving, the existing velocity either adds to or subtracts from the motor thrust/force. 
						//need to use momentum. Using acceleration does not really work, because the segment could have a high velocity in the 
						//opposite direction, but be slowing down so it has a negative acceleration, but still the motor needs
						//to meet that strong opposing velocity.
						//Here we need to determine how much thrust is needed to counter any opposing momentum and produce the desired velicity, below we want to calculate what the change in acceleration will be from the chosen motor thrust.

						//Need to add TargetVelocity and TargetAccel/MaxAccel
						//Need to add static friction

						if (tinst->flags&ROBOT_INSTRUCTION_DOUBLE_THRUST) {
							DoubleThrustDivider=2.0;			
						} else {
							DoubleThrustDivider=1.0;
						}

						if (WithGravity) {
							if (VelocityInCorrectDirection) {
								//if moving in correct direction, any velocity is assisting so use minimum thrust 
								//note that for 2motors+1accel only Motor[0] parameters are currently used for both motors.
								//one problem is sometimes more than min is needed
								ThrustNeeded=GetNeededThrust(lemi->MotorNameNum[0],0,WithGravity); //get	thrust (force) needed												
								//ThrustNeeded=GetNeededThrust(lemi->MotorNameNum[0],fabs(ForceWeight)/DoubleThrustDivider,WithGravity); //get	thrust (force) needed												

							} else {
								//if moving in the wrong direction, any velocity is resisting so add fabs(velocity*mass)
								//the motor will need to thrust against the backwards velocity.
								//ThrustNeeded=GetNeededThrust(leai->MotorNameNum,fabs(Velocity*leai->Mass)/DoubleThrustDivider,WithGravity); //get	thrust (force) needed
								//ThrustNeeded=GetNeededThrust(leai->MotorNameNum,fabs(Velocity*leai->Mass)/DoubleThrustDivider,WithGravity);
								ThrustNeeded=GetNeededThrust(lemi->MotorNameNum[0],fabs(ForceWeight)/DoubleThrustDivider,WithGravity);
							} //if (VelocityInCorrectDirection) {
						} else { //if (WithGravity) {
							//Against Gravity
							//add Weight of segment 
							if (VelocityInCorrectDirection) {
								//if moving in the correct direction subtract any existing (assisting) momentum (not force): fabs(velocity*mass)
								//ThrustNeeded=GetNeededThrust(leai->MotorNameNum,(fabs(ForceWeight)-fabs(Velocity*leai->Mass))/DoubleThrustDivider,WithGravity);
								ThrustNeeded=GetNeededThrust(lemi->MotorNameNum[0],fabs(ForceWeight)/DoubleThrustDivider,WithGravity);
							} else {
								//or else if moving in the wrong direction, add the existing (opposing) force: fabs(velocity*mass)
								//ThrustNeeded=GetNeededThrust(leai->MotorNameNum[0],(fabs(ForceWeight)+fabs(Velocity*leai->Mass))/DoubleThrustDivider,WithGravity);
								ThrustNeeded=GetNeededThrust(lemi->MotorNameNum[0],fabs(ForceWeight)/DoubleThrustDivider,WithGravity);
							} //if (VelocityInCorrectDirection) {
						} //if (WithGravity) {

						//shorthand for leai->IncAngDir[0]
						IncAngDir=leai->IncAngDir[UseAngle][0];  //[0][0]=Accel[0]Motor[0] - presumes motor 0 - maybe UseMotor

						//Set correct sign for ThrustNeeded. 
						if ((IncAngDir==1 && !AngNeedsToInc) || (IncAngDir==-1 && AngNeedsToInc)) {
							ThrustNeeded=-ThrustNeeded;
						} 


						//Determine if LastThrust was in the correct direction
						//note that if LastThrust==0, ThrustInCorrectDirection will = 0
						ThrustInCorrectDirection=0;
						if (AngNeedsToInc) {  //SegmentAngle needs to increase to reach TargetAngle
							if (IncAngDir==1) {
								if (lemi->LastThrust>0) {
									ThrustInCorrectDirection=1;
								} 
							} else {
								if (lemi->LastThrust<0) {
									ThrustInCorrectDirection=1;
								} 
							} //if (IncAngDir==1) {
						} else { //if (AngNeedsToInc) {
							//SegmentAngle needs to decrease to reach TargetAngle
							if (IncAngDir==1) {
								if (lemi->LastThrust<0) {
									ThrustInCorrectDirection=1;
								} 
							} else {
								if (lemi->LastThrust>0) {
									ThrustInCorrectDirection=1;
								} 
							} //if (IncAngDir==1) {
						} //if (AngNeedsToInc) {  


																								
						//START THEORETICAL- NOT USED IN MOTION CODE
						//Calculate estimated velocity and estimated acceleration
						//ThrustmN=RStatus.MotorInfo[leai->MotorNameNum].Thrust[ThrustNeeded];  //Thrust in milliNewtons
						//for now just use actual thrust
						ThrustmN=RStatus.MotorInfo[lemi->MotorNameNum[0]].Thrust[abs(lemi->Thrust)];  //Thrust in milliNewtons
						//estimate acceleration from (Thrust+weight)/segment mass+existing accel
						if (SegmentAngle>TargetAngle) {
							//Segment Angle needs to decrease so make ThrustmN negative
							TotalForce=-ThrustmN;
							if (WithGravity) {
								//weight assists
								TotalForce-=fabs(ForceWeight);
							} else {
								//weight resists
								TotalForce+=fabs(ForceWeight);
							}
						} else { //if (SegmentAngle>TargetAngle) {
							//Segment Angle needs to increase so ThrustmN is positive
							TotalForce=ThrustmN;
							if (WithGravity) {
								//weight assists
								TotalForce+=fabs(ForceWeight);
							} else {
								//weight resists
								TotalForce-=fabs(ForceWeight);
							}
						} //if (SegmentAngle>TargetAngle) {
						ExpectedAccel=(TotalForce/leai->Mass[UseAngle]);//+Acceleration;
						//ExpectedVelocity=Velocity+ExpectedAccel;
						RatioAV=Acceleration/Velocity;
						//END THEORETICAL- NOT USED IN MOTION CODE



//										if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							//sprintf(LogStr,"ForceWeight: '%s %s' '%s' %02.2fdeg %4.0f/%4.0f Thrust=%d ThrustmN=%4.0f ThrustNeed=%4.0f",leai->SensorName,XYZ[leai->xyz[0]],lemi->MotorReadableName,SegmentAngle,ForceWeight,leai->Weight,ThrustNeeded,ThrustmN,RStatus.MotorInfo[lemi->MotorNameNum].Thrust[ThrustNeeded]);
//											sprintf(LogStr,"ForceWeight: '%s %s' '%s' %02.2fdeg %7.0f Weight=%7.0fg sin=%7f",leai->SensorName,XYZ[leai->xyz[0]],leai->MotorReadableName,SegmentAngle,ForceWeight,leai->Weight,sinf(SegmentAngle*PI/180.0));
							//sprintf(LogStr,"Angle reached by %s motor %d: %02.2f (%02.2f to %02.2f)",ROBOT_PCB_NAMES[lemi->PCBNum],lemi->MotorNum,SegmentAngle,leai->Angle[1],leai->Angle[2]);
							//fprintf(stderr,"MNN=%d\n",leai->MotorNameNum);

							//LogRobotModelData(LogStr);
//										} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

						//log motion data
						if (tinst->flags&ROBOT_INSTRUCTION_MOTION_LOG) {
							//fprintf(tinst->MotionLogfptr,"%d\t%d\t%d\t%4.0f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\n",RobotCurrentTime-ltask->StartTime-tinst->StartTime,lemi->Thrust,ThrustNeeded,ForceWeight,SegmentAngle,Velocity,Acceleration,ExpectedAccel,RatioAV);
							fprintf(tinst->MotionLogfptr, "%d\t%d\t%d\t%4.0f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\t%4.2f\n", tinst->InstTime, lemi->Thrust, ThrustNeeded, ForceWeight, SegmentAngle, Velocity, Acceleration, AccelAvg3,Jerk,ExpectedAccel, RatioAV);
						}



						//check for special flag: user set NO_MOTION_NO_THRUST flag, then if !IsMoving, set Thrust=0, and skip below thrust analysis
						if (!IsMoving && (tinst->flagNoMotionNoThrust&(1<<UseAngle))) {
							//set Thrust=0 and skip below thrust analysis

							lemi->Thrust=0;			
							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tThrust=0: V=N !IsMoving NO_MOTION_NO_THRUST\n",ltask->TaskTime,tinst->InstTime);
							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
						} else { //if (!IsMoving && (tinst->flagNoMotionNoThrust&(1<<UseAngle))) {



//START: SEE IF SEGMENT ANGLE IS IN TARGET ANGLE RANGE  
//**originally this was to catch a second angle- above check for InRange, only checked the first accel, and set UseAngle=1 if Accel 0 was in range, so this would check accel[1]. But now there are different criteria for an instruction to succeed (ex: IF_A2, etc.), so there is no need for this code anymore.
							InRange=0; //presume SegmentAngle is not in range
							if (SegmentAngle>=leai->Angle[1] && SegmentAngle<=leai->Angle[2]) {
								//SEGMENT ANGLE IN RANGE (condition is met)  
								InRange=1;	



								//if the segment angle was not already in range print "Angle range reached" message.
								//if (!(leai->flags&ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE)) {  
								if (!(tinst->flagAngleInRange&(1<<UseAngle))) {  

//**TODO - eventually comment below code out- currently just verify in model logs if this code is ever reached

									//fprintf(stderr,"StartTime=%d Motor#%d Angle %02.2f (%02.2f to %02.2f)\n",tinst->OrigStartTime,tinst->EMInst.MotorNum,SegmentAngle,leai->Angle,leai->Angle2); 
									//fprintf(stderr,"Angle reached by %s motor %d: %02.2f (%02.2f to %02.2f)\n",ROBOT_PCB_NAMES[lemi->PCBNum],lemi->MotorNum,SegmentAngle,leai->Angle[1],leai->Angle[2]);
									//fprintf(stderr,"Angle reached '%s' '%s' %02.2f (%02.2f to %02.2f)\n",leai->SensorName,leai->MotorReadableName,SegmentAngle,leai->Angle[1],leai->Angle[2]);
									if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
										sprintf(LogStr,"Angle in range2 '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f)",leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],lemi->MotorReadableName,SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2]);
										//sprintf(LogStr,"Angle range reached '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f)",leai->SensorName[UseAngle],XYZ[leai->xyz[UseAngle]],lemi->MotorReadableName,SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2]);
										//fprintf(stderr,"Angle reached\n");
										LogRobotModelData(LogStr);
									} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tAngle %d in range2 '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f\n",ltask->TaskTime,tinst->InstTime,UseAngle,leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],lemi->MotorReadableName,leai->CurrentAngle,leai->oAngle[UseAngle][1],leai->oAngle[UseAngle][0],leai->oAngle[UseAngle][2]);
									} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			 

									//set ANGLE_IN_RANGE flag
									//leai->flags|=ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE;
									tinst->flagAngleInRange|=(1<<UseAngle); //set AngleInRange flag   
									//SENT_INITIAL flag is set below

/*
									if (!(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) &&
												(tinst->flags&ROBOT_INSTRUCTION_INITIAL_THRUST)) {

										//If user specified INITIAL_THRUST flag, set thrust to UserThrust, even if segment is in range (used to thrust one leg at the same time as lifting the opposite leg)
										lemi->Thrust=lemi->UserThrust;
										if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
											fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSet Thrust to UserThrust %d\n",ltask->TaskTime,tinst->InstTime,lemi->UserThrust);
										}

										//fprintf(stderr,"INITIAL_THRUST flag, set Thrust=%d\n",lemi->Thrust);
									} //if (!(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) && ...
*/


									//Only set SENT_INITIAL_THRUST flag if this is not a REQUIRED or NO_HOLD (was is a HOLD_ANGLE) inst that just moved into range, because otherwise thrust analysis will not occur until the segment falls out of range.
									//if (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE) {  
									if (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))) {  
										//thrust analysis will set a thrust, and if different from the current thrust, an inst will get sent to the motor
										leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST;  //allows thrust analysis todo: verify that this is still needed (Angle starts in range)
									} 

								} //if (!(tinst->flagAngleInRange&(1<<UseAngle))) {  
							}//if (SegmentAngle>leai->Angle[1] && SegmentAngle<leai->Angle[2] ) {



							//If Segment is not in range, or is in range but moving and is not a REQUIRED or NO_HOLD inst (was unless was a HOLD_ANGLE inst) then set thrust
							//if (!InRange || (IsMoving && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE))) {
							//removed check for REQUIRED, because there can be more than one inst required in a stage
								//if (!InRange || (IsMoving && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))))) {
								if (!InRange || (IsMoving && (!(tinst->flags&ROBOT_INSTRUCTION_NO_HOLD)))) {
								//SEGMENT ANGLE NOT IN RANGE OR IS IN RANGE AND IS NOT "REQUIRED" AND IS NOT A "NO_HOLD" INST
								//was: SEGMENT ANGLE NOT IN RANGE OR HOLD_ANGLE INST SEGMENT ANGLE IS MOVING

								//note that the segment may be in range and then fall out of range
								//was: if the SegmentAngle is not in range and either has not sent an initial thrust, or was previously in the target angle range, then print "Angle out of range" to log
								//if (!InRange && (!(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) || (leai->flags&ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE))) {
								//if the SegmentAngle is not in range and either this instruction has just started, or was previously in the target angle range, then print "Angle out of range" to log
								if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle))) ) {  

									if ( (tinst->flagAngleInRange&(1<<UseAngle)) || (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START)) {
										//if either angle was out of range in the same stage, or is out of range and the instruction just started
										if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
											//sprintf(LogStr,"Angle out of range: %s motor %d: Angle %02.2f (%02.2f to %02.2f)",ROBOT_PCB_NAMES[lemi->PCBNum],lemi->MotorNum,SegmentAngle,leai->Angle[1],leai->Angle[2]);
											sprintf(LogStr,"Angle out of range: '%s %s' '%s' %02.2f (%02.2f<%02.2f<%02.2f)",leai->SensorReadableName[UseAngle],XYZ[leai->xyz[UseAngle]],lemi->MotorReadableName,SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2]);
											LogRobotModelData(LogStr);
											//fprintf(stderr,"Sent=%d InRange=%d\n",(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST),(leai->flags&ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE));
										} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									} //if ( (tinst->flagAngleInRange&(1<<UseAngle)) || (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START)) {
								} //if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle))) ) {   
							} //if (!InRange || (IsMoving && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))))) {




//START: OPPOSE_MOTION - todo: probably should not oppose motion if slowly moving against gravity
							if (tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION) {
								//Instruction has the OPPOSE_MOTION flag set

								//print out Angle, Velocity, Accel and Thrust if FUTURE_LOG is set
								if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
									//log FutureTargetIndex, current and future SegmentAngle
									sprintf(LogStr,"%d\t%d\tAng\tTargetIdx=%d\t\t(%4.1f<%4.1f<%4.1f)\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureTargetIndex,leai->Angle[1],leai->Angle[0],leai->Angle[2],SegmentAngle);
									tstr[0]=0;
									for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
										sprintf(tstr,"%s\t%4.2f",tstr,FutureAngle[i]);
									}
									fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);

									//log FutureZeroVelocityIndex and Current and Future Segment Velocity
									sprintf(LogStr,"%d\t%d\tVel\tZero=%d\t%d\t\t\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureZeroVelocityIndex,VelocityInCorrectDirection,Velocity);
									tstr[0]=0;				
									for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
										sprintf(tstr,"%s\t%4.2f",tstr,FutureVelocity[i]);
									}
									fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);

									//log thrust and Segment Accleration
									sprintf(LogStr,"%d\t%d\tAccel\tThrust=%d (%d)\t%d\t\t%4.2f\t%4.2f",ltask->TaskTime,tinst->InstTime, lemi->Thrust,WithGravity,AccelerationInCorrectDirection,Acceleration,AccelAvg3);
									tstr[0]=0;				
									for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
										sprintf(tstr,"%s\t%4.2f",tstr,Acceleration);
									}
									fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
								} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

								//If segment is moving, motion needs to be thrusted against.
								//if segment is not moving- maintain the current thrust. Note that this is different from the non OPPOSE_THRUST thrust analysis code where no motion will stop all thrust (set thrust=0).

								//need this code for above model log "Angle out of range"- if these flags are not cleared here, then the above prints every 10ms sample											
								if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle))) ) {  
										//Set the motor speed/thrust - First time or after the joint angle moved out of the range or the joint is InRange but moving. For CONSTANT_THRUST set thrust back to user specified thrust.												
										//unless the MAINTAIN_THRUST flag is set in MotorInfo, clear if set
										MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[0],lemi->MotorNum[0]);
		
										//for OPPOSE_MOTION, MAINTAIN_THRUST is not relevant, so clear this flag
										leai->flags&=~ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; //clear initial thrust flag

										//leai->flags&=~ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE; //reset ANGLE_IN_RANGE flag
										tinst->flagAngleInRange&=~(1<<UseAngle); //reset ANGLE_IN_RANGE flag

								} //if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle))) ) {

								if (IsMoving) {  
									//Segment is moving and so code needs to oppose this motion

									//is motion in direction of thrust?
									//if yes: reverse thrust
									//if no: set/maintain thrust (opts: 1:use VelToThr factor only, 2:VelToThr and increase thrust to MaxThrust if Thrust is already >=VelToThr 3:inc up to MaxThrust {tends to be too slow - is current method in non oppose motion code})

									//set Thrust= Velocity*VelocityToThrustFactor in opposite direction
									//TVR can be different for positive and negative directions
									if (Velocity>0) { 
										//positive direction
										lemi->Thrust= (int)(fabs(Velocity) * leai->ThrustVelocityRatio[UseAngle][0]);
									} else { //if (Velocity>0) { 
										//negative direction
										lemi->Thrust= (int)(fabs(Velocity) * leai->ThrustVelocityRatio[UseAngle][1]);
									} //if (Velocity>0) { 
									if (lemi->Thrust>lemi->MaxThrust[UseAngle]) {  //clamp Thrust to max thrust (done in TurnMotor anyway, but for logging)
										lemi->Thrust=lemi->MaxThrust[UseAngle];
									} 
									//determine if we need to negate thrust
									if ((Velocity>0 && IncAngDir==1) || (Velocity<0 && IncAngDir==-1)) {
										lemi->Thrust=-lemi->Thrust;
									}  
									if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										fprintf(tinst->FutureLogfptr,"%d\t%d\t\tOPPOSE_MOTION Vel=%03.2f TVR=%03.2f Thrust=%d IncAngDir=%d\n",ltask->TaskTime,tinst->InstTime,Velocity,leai->ThrustVelocityRatio[UseAngle][(Velocity<=0)],lemi->Thrust,IncAngDir);
									} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			


#if 0 //more OPPOSE_MOTION options
									//if Velocity=+ and IncAngDir=+ and LastThrust=+, or 
									//Velocity=+ and IncAngDir=- and LastThrust=-, or 
									//Velocity=- and IncAngDir=+ and LastThrust=-, or
									//Velocity=- and IncAngDir=- and LastThrust=+,
									//then motion is in direction of thrust and needs to be reversed
									if ((Velocity>0 && IncAngDir && lemi->LastThrust>0) || (Velocity>0 && IncAngDir==-1 && lemi->LastThrust<0) || (Velocity<0 && IncAngDir && lemi->LastThrust<0) || (Velocity<0 && IncAngDir==-1 && lemi->LastThrust>0) {
										//set thrust in opposite direction 


									} else { //if ((Velocity>0 && IncAngDir && lemi->LastThrust>0) || ...
										//thrust is in the opposite direction of motion (or LastThrust==0), no need to reverse

										//options:
										//(current option) 1: if no existing thrust, then set Thrust to Velocity*VelocityToThrustFactor
										//2: if fabs(LastThrust)>=Velocity*VelocityToThrustFactor, then increment Thrust to MaxThrust
                    //3: increment up to MaxThrust (tends to be too slow, and is how non OPPOSE_MOTION code functions)
							
										//set Thrust= Velocity*VelocityToThrustFactor in opposite direction
										lemi->Thrust= fabs(Velocity)*leai->ThrustVelocityRatio;
										//determine if we need to negate thrust
										if ((Velocity>0 && IncAngDir) || (Velocity<0 && IncAngDir==-1)) {
											lemi->Thrust=-lemi->Thrust;
										}  


									} //if ((Velocity>0 && IncAngDir && lemi->LastThrust>0) ||
#endif  //more OPPOSE_MOTION options 

#if 0  //old code 
									//Is Acceleration in same direction as Velocity?
									if ((Acceleration>0 && Velocity<0) || (Acceleration>0 && Velocity>0)) { 
										//Acceleration is in the opposite direction of Velocity
										//Will speed reach zero (reverse) within 50ms?
										i=0;
										FutureSegmentVelocity=Velocity;
										FutureZeroVelocityIndex=0;
										while(i<5 && FutureZeroVelocityIndex==0) {		
											FutureSegmentVelocity+=Acceleration;
											if (Velocity>0.0) { 
												if (FutureZeroVelocityIndex==0 && FutureSegmentVelocity<=0.0) {
													FutureZeroVelocityIndex=i;
												} 
											} else { //Velocity>0.0
												if (FutureZeroVelocityIndex==0 && FutureSegmentVelocity>=0.0) {
													FutureZeroVelocityIndex=i;
												} 
											} //Velocity>0.0
											i++;
										} //while(i<5 && FutureZeroVelocityIndex==0)

										if (FutureZeroVelocityIndex==0) {
											//current acceleration will not reverse speed within 50ms
											//increase thrust unless thrust was recently changed
											if (lemi->ThrustCount>2 && fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
												IncreaseThrust(lemi,AngNeedsToInc,IncAngDir);
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t\t\tOPPOSE_MOTION Inc: Accel=Y, No rev in 50\n",tinst->InstTime);
												} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

											} //if (lemi->ThrustCount>2) {

										} else { //if (FutureZeroVelocityIndex==0) 
											//otherwise if current acceleration will make velocity cross 0 (reverse speed) within 50ms, then set Thrust=0
											lemi->Thrust=0;
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tOPPOSE_MOTION Thrust=0 Accel=Y, vel rev in 50\n",ltask->TaskTime,tinst->InstTime);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

										} //if (FutureZeroVelocityIndex==0)


									} else { //if ((Acceleration>0 && Velocity<0) || (Acceleration>0 && Velocity>0)) { 
										//Acceleration is in the same direction	as Velocity
										//Thrust in opposite (opposing) direction of Velocity?
										if ((Velocity>0 && lemi->Thrust<=0 && leai->IncAngDir>0) ||
											 (Velocity>0 && lemi->Thrust>=0 && leai->IncAngDir<0) ||
											 (Velocity<0 && lemi->Thrust>=0 && leai->IncAngDir>0) ||
											 (Velocity<0 && lemi->Thrust<=0 && leai->IncAngDir<0)) {
											//Thrust is in opposite direction of velocity (note: includes Thrust==0)
											//increase thrust unless thrust was recently changed
											if (lemi->ThrustCount>2 && fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
												IncreaseThrust(lemi,AngNeedsToInc,IncAngDir);
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tOPPOSE_MOTION Inc: Accel=Y, No rev in 50\n",ltask->TaskTime,tinst->InstTime);
												} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
											} //if (lemi->ThrustCount>2) {
										} else { //if ((Velocity>0 && lemi->Thrust<0 && leai->IncAngDir>0) || ...
											//Thrust is with motion (in the wrong way)
											//set Thrust to ThrustNeeded
											//lemi->Thrust=ThrustNeeded;
											lemi->Thrust=AngNeedsToInc*IncAngDir-(!AngNeedsToInc)*IncAngDir;//ThrustNeeded; 
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tOPPOSE_MOTION Thrust=ThrustNeeded thrust in wrong dir\n",ltask->TaskTime,tinst->InstTime);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
							
										} // //if ((Velocity>0 && lemi->Thrust<0 && leai->IncAngDir>0) || ...

									} //if ((Acceleration>0 && Velocity<0) || (Acceleration>0 && Velocity>0)) { 

#endif //old code

								} else { //if (IsMoving) {
									//segment in question is not moving
									//if not moving, and OPPOSE_MOTION_MAINTAIN flag is not set
									//stop motor thrust (set Thrust=0), otherwise motor thrust is maintained
									if (!(tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_MAINTAIN)) {
										lemi->Thrust=0;  //body segment is not moving and OPPOSE_MOTION_MAINTAIN flag not set, so stop thrust
									}

								} //if (IsMoving) {
					
							//END: OPPOSE_MOTION

							} else { //if (tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION) {
								//Instruction does not have OPPOSE_MOTION flag set


								//fprintf(stderr,"11 ");

								//SET INITIAL THRUST IF NECESSARY (not InRange). Note that (InRange && INSTRUCTION_START) currently does not set initial thrust
								//Otherwise do thrust analysis

								// if not InRange, or (is InRange and) IsMoving and is either just starting, or not REQUIRED and not a NO_HOLD inst (which cannot fall out of range). If InRange and not moving, or moving, but not starting or is a REQUIRED or a NO_HOLD inst, then no initial thrust analysis should be done.  
								//if (!InRange || (IsMoving && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE))) {
								//  commented below line because: angles that are !InRange, not START, and REQUIRED do not get thruist analysis. In addition, the below logic splits into Initial Thrust, Thrust Analysis, or No Thrust Analysis

								//if (!InRange || (IsMoving && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || !(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))))) {
								//	if (!InRange || (IsMoving && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle))))) {

								//SEGMENT ANGLE NOT IN RANGE OR IS IN RANGE AND MOVING AND IS NOT REQUIRED AND NOT NO_HOLD
								//was: SEGMENT ANGLE NOT IN RANGE OR HOLD_ANGLE INST SEGMENT ANGLE IS MOVING

								//note that the segment may be in range and then fall out of range
								//was: if the SegmentAngle is not in range and either has not sent an initial thrust, or was previously in the target angle range, then print "Angle out of range" to log
								//if (!InRange && (!(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) || (leai->flags&ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE))) {
								//if the SegmentAngle is not in range and either this instruction has just started, or was previously in the target angle range, or (is using constant thrust and 1: passed over the range w/o ever being InRange, or 2) changed Accels because one accel went out of range), then print "Angle out of range" to log

								if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flagAngleInRange&(1<<UseAngle)) || ((tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST) && ((leai->LastAngNeedsToInc[UseAngle]!=AngNeedsToInc) || UseAngle!=leai->LastUseAngle)) )) {


									//Set the motor speed/thrust - First time or after the joint angle moved out of the range or the joint is InRange but moving. For CONSTANT_THRUST set thrust back to user specified thrust.												
									//unless the MAINTAIN_THRUST flag is set in MotorInfo, clear if set
									MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[0],lemi->MotorNum[0]);
									//if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
									//only if this is the start of a new instruction

									//leai->flags&=~ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE; //reset ANGLE_IN_RANGE flag
									tinst->flagAngleInRange&=~(1<<UseAngle); //reset ANGLE_IN_RANGE flag


									//if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0 || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
									//was: set initial thrust (either constant thrust, or is out of range and initial thrust has not been set)
									if ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
										//set initial thrust (either constant thrust, or is out of range and inst is just starting)
											


//															if ((tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST) ||  (tinst->flags&ROBOT_INSTRUCTION_INITIAL_THRUST)) {
										if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST) {

											//User set CONSTANT_THRUST or INITIAL_THRUST flag, so use user specified initial thrust
											//todo: One unwanted side effect of the INITIAL_THRUST flag, is that if the HOLD_ANGLE flag is also set, and the angle falls out of range, the InitialThrust will be set again, which probably should be fixed. Maybe use SENT_INITIAL_THRUST_AGAIN
											if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS) {
												//user set constant thrust bias (different constant thrust depending on direction to target: positive or negative)
												if (AngNeedsToInc) {
													//TargetAngle in positive direction
													lemi->Thrust=IncAngDir*leai->ConstantThrustBias[UseAngle][0];
												} else {
													//TargetAngle in negative direciton																		
													lemi->Thrust=-IncAngDir*leai->ConstantThrustBias[UseAngle][2];
												}
											} else {  //if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS) {
												//No CONSTANT_THRUST_BIAS, use User given Motor Initial thrust then
												InitialThrust=lemi->UserThrust; //thrust= thrust user specified
												if (AngNeedsToInc) {
													lemi->Thrust=IncAngDir*InitialThrust;
												} else {
													lemi->Thrust=-IncAngDir*InitialThrust;
												} 
											} //if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS) {

											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\tINITIAL SET CONST THRUST=%d. Segment %s out of range\t\tSegAngle=%4.2f TargetAngle=%4.2f %4.2f %4.2f lemi->UserThrust=%d IncAngDir=%d\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle],SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2],lemi->UserThrust,IncAngDir);
												} else { //if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
													if (UseAngle!=leai->LastUseAngle) {
														fprintf(tinst->FutureLogfptr,"%d\t%d\t\tAccel change. SET CONST THRUST=%d. Segment %s out of range\t\tSegAngle=%4.2f TargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle],SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2]);
													} else { //if (UseAngle!=leai->LastUseAngle) {
														//if (leai->LastAngNeedsToInc[UseAngle]!=AngNeedsToInc) {
															fprintf(tinst->FutureLogfptr,"%d\t%d\t\tSegment passed over Target. SET CONST THRUST=%d. Segment %s out of range\t\tSegAngle=%4.2f TargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle],SegmentAngle,leai->Angle[1],leai->Angle[0],leai->Angle[2]);
														//} else { //if (leai->LastAngNeedsToInc[UseAngle]!=AngNeedsToInc) {
															
														//} //if (leai->LastAngNeedsToInc[UseAngle]!=AngNeedsToInc) {
													}	//if (UseAngle!=leai->LastUseAngle) {
												} //if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			

											//for now just store AngNeedsToInc since currently the segment going over the Target without ever being InRange, only matters for CONSTANT_THRUST
											leai->LastAngNeedsToInc[UseAngle]=AngNeedsToInc;

										} else {	//if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST) {
											//User did not set CONSTANT_THRUST

											//USE_MINIMUM_THRUST or USE_USER_INITIAL_THRUST
											//InitialThrust=fabs(ThrustNeeded);  //fabs because correct sign is calculated above, but below expects InitialThrust to always be positive
											InitialThrust=lemi->UserThrust;
											//possibly set lemi->LastThrust==0 so that below code will send a TurnMotor command
											//alt: just set initial thrust to 1 or 2 and let the thrust analysis code adjust the thrust.




											//SegmentAngle is the body segment current angle, leai->Angle[0] is the target angle
											if (SegmentAngle<TargetAngle) { 
												//joint angle is less than target angle, need to increase joint angle (ex:-30 to 0, 0 to 30)
												//make sure TargetSpeed has correct sign
												leai->TargetSpeed=fabs(leai->TargetSpeed); //TargetSpeed should be positive to increase joint angle
												//do not set lemi->Thrust if MAINTAIN_THRUST flag is set, 
												//(LAST_MAINTAIN_THRUST flag (set above on inst start if MAINTAIN_FLAG was set on MOTOR is cleared below)
												if (!(tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST)) {
													lemi->Thrust=IncAngDir*InitialThrust; 
												}
											} else { //if (SegmentAngle<TargetAngle) {
												//joint angle is more than target angle, need to decrease joint angle (ex:30 to 0,0 to -30)
												leai->TargetSpeed=-fabs(leai->TargetSpeed); //TargetSpeed should be negative to decrease joint angle
												if (!(tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST)) {
													lemi->Thrust=-IncAngDir*InitialThrust;
												} //
											} //if (SegmentAngle<TargetAngle) {


											if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {
												//tinst->flags&=~ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST; //clear flag, so thrust analysis can proceed after this
												lemi->Thrust=lemi->LastThrust;
												//Need to either set LastThrust=0, or add to MODEL_LOG, so thrust will get set when people use the "S" key to start at a stage > the first stage
												//cannot set LastThrust=0 because DecreaseThrust uses LastThrust of 0, lemi->LastThrust=0; //only so below code will recognize that Thrust has changed and send another TurnMotor command, so Model window can recognize when people use "S" key to skip to a stage> the first stage
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tMAINTAINING LAST THRUST (%d) Segment %s out of range\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
												}
												//if LastThrust > MaxThrust, then reduce to new MaxThrust
												if (lemi->Thrust>lemi->MaxThrust[UseAngle]) {
													lemi->Thrust=lemi->MaxThrust[UseAngle];
													if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
														fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tLastThrust>MaxThrust[UseAngle] reducing Thrust to %d for Segment %s\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle]);
													}
												} //if (lemi->Thrust>lemi->MaxThrust[UseAngle]) {
												//commented below, so Model Log can see Thrust getting set to last maintained thrust, 10ms of thrust analysis will not matter
												//in addition IncreaseThrust and DecreaseThrust used by ThrustAnalysis depend on LastThrust being correct
												//***in addition, this flag should never be set before the below TurnMotor code, because that updates the duration of the TurnMotor, which needs to be extended, even if the Thrust stays the same (MAINTAIN_THRUST flag is set)
												//leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; //so Thrust analysis will start immediately
											} else {  //if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {
												//NOT MAINTAINING THRUST OF LAST STAGE
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tINITIAL THRUST=%d Segment %s out of range\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust,leai->SensorReadableName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
												}
											} //if (tinst->flags&ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST) {

											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSegment %s out of range\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,leai->SensorReadableName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			
										//leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; //move below to skip below to skip below thrust analysis code?
										} //else if (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST) {

									} else { //if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0 || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
										if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tNO INITIAL THRUST SENT_INITIAL_THRUST=%x Segment %s out of range\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST),leai->SensorReadableName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
										} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			
									} //if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0 || (tinst
		
								} //if (!InRange && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || (leai-...



								//moved below code into above 2 Angle reached code, speeds up motion analysis 
								//if a HOLD_ANGLE is InRange and IsMoving we need to do Thrust Analysis, so set the SENT_INITIAL_THRUST flag.
								//Perhaps there should be a separate PERFORM_THRUST_ANALYSIS flag, but generally the model is, send an initial thrust, then do thrust analysis after that. It might be good to remove the concept of an initial thrust- but it allows a user to specify an initial thrust.				
/*								
								if (InRange && IsMoving && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
									leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST;  //allows thrust analysis
								} 
*/


								//no thrust analysis (no thrusting against motion) in rare case when segment is InRange but IsMoving, and MovingAgainstGravity, for some motor-accel combinations like FootMotor->LowerLegX, KneeMotor->UpperLegX, LegMotor->TorsoX, HeadMotor->HeadX. Because thrusting with gravity causes oscilations and usually is not necessary since gravity pulls the segment back into the target range.

								//Or if segment is InRange and IsMoving and towards the TargetAngle (VelocityInCorrectDirection)
								//added, if (moving against gravity and V<MaxV) or VelocityInCorrectDirection (moving toward TargetAngle even if >MaxVelocity), do not thrust

								//added requirement of: V<MaxV to allow thrust analysis when InRange, but V=N and V>MaxV 
								//this can cause a problem when the segment is InRange- probably this exception of too high a velocity
								//against gravity, and so the potential need for a thrust with gravity should only occur when the
								//body segment is not in the target range (!InRange)

								//be careful with fabs(Velocity)<leai->MaxVelocity[UseAngle]) because could be negative (!VelocityInCorrectDirection) - if Velocity>MaxVelocity- do thrust analysis to stop/slow the motion whether moving against gravity or with gravity

//added back to thrust if InRange and moving faster than MaxVelocity

								//remove  && fabs(Velocity)<leai->MaxVelocity[UseAngle]? - after all, body segment is InRange- and gravity may pull it back or slow it- otherwise make range smaller- could add another flag- UNLESS_OVER_MAX_VELOCITY

//set thrust=0 if the NoThrustWhen moving against gravity in range flag is set, and if OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET is not set or is set but the target is moving against gravity and the instruction is not just starting


								//Only perform thrust analysis on an instruction after it has started
								if (!(tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START)) {

//														if ( (tinst->flagNoThrustWhenMAGIR&(1<<UseAngle)) && (!(tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET) || MovingAgainstGravity) && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))) && InRange && IsMoving && ((MovingAgainstGravity && fabs(Velocity)<leai->MaxVelocity[UseAngle]) ||  VelocityInCorrectDirection)) {
									if ( (tinst->flagNoThrustWhenMAGIR&(1<<UseAngle)) && (!(tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET) || MovingAgainstGravity) && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))) && InRange && IsMoving && (fabs(Velocity)<leai->MaxVelocity[UseAngle] && (MovingAgainstGravity ||  VelocityInCorrectDirection))) {

										//NO THRUST ANALYSIS - InRange && IsMoving
										//and V<MaxV and either MovingAgainstGravity or V=Y
										//set Thrust=0 otherwise an existing Thrust might continue when the segment is InRange
										lemi->Thrust=0;
										//each sample that an instruction runs must be accounted for in the future log
										if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {																		
											if (MovingAgainstGravity) {
												sprintf(LogStr,"%d\t%d\t\tAng=%4.2f Vel=%4.2f Accel=%4.2f T=%d\tThrust=0: In Range and moving against gravity %4.1f %4.1f",ltask->TaskTime,tinst->InstTime, SegmentAngle,Velocity,Acceleration,lemi->Thrust,leai->Angle[1],leai->Angle[2]);
											} else {
												sprintf(LogStr,"%d\t%d\t\tAng=%4.2f Vel=%4.2f Accel=%4.2f T=%d\tThrust=0: In Range and moving toward TargetAngle %4.1f %4.1f",ltask->TaskTime,tinst->InstTime, SegmentAngle,Velocity,Acceleration,lemi->Thrust,leai->Angle[1],leai->Angle[2]);
											} 	
											fprintf(tinst->FutureLogfptr, "%s\n",LogStr);
										} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {	
					
										//if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
											//fprintf(tinst->FutureLogfptr,"%d\t\t\tThrust=0: Targ<brake Vel=Y\n",tinst->InstTime);
										//	fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tIn Range && IsMoving\n",ltask->TaskTime,tinst->InstTime);
										//} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										//fprintf(stderr,"%d InRange && IsMoving (%s)\n",tinst->InstTime,leai->SensorName[UseAngle]);
									} else { //		if ( ((tinst->flagNoThrustWhenMAGIR&(1<<UseAngle))&&!(tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET)) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE) && InRange && IsMoving && ((MovingAgainstGravity && fabs(Velocity)<leai->MaxVelocity[UseAngle]) ||  VelocityInCorrectDirection)
						
										//DOES THRUST ANALYSIS
										//Currently only do thrust analysis on angles that are !InRange or are InRange and IsMoving and not a NO_HOLD inst (inst ends when InRange). Note that because there can be multiple REQUIRED insts in one stage, they may get thrust analysis if InRange and IsMoving, but it does not matter since if all REQUIRED insts are InRange, the stage will end anyway. 
										if (!InRange || (IsMoving && !(tinst->flags&ROBOT_INSTRUCTION_NO_HOLD)))  { 


											//BELOW SYNC CODE NEEDS TO BE REFINED- examine next 50ms and only slow down if FutAngles50 are > MaxSpread and Thrust_count>2
											//Check to see if this instruction is syncing to another EthAccel, and if yes 
											//see if this accel needs to wait for the other to catch up.
											if ((tinst->flags&ROBOT_INSTRUCTION_SYNC_TWO_ACCELS) && leth[1]!=0) { //leth[1]!=0 so no crash
												//this accelerometer must stay in sync with a second accelerometer
												//this accel is not in the target range, so determine if this accel is closer
												//to the TargetAngle than the second accel, if yes, then slow down and skip the lower
												//thrust analysis section. If no, then continue on to the lower thrust analysis section.
												//Since they have the same TargetAngle (currently- that may change later on), only the
												//actual angles need to be compared. If SegmentAngle-Accel[1]<-MaxSpread, this accel needs to slow down.
												//Note MaxSpread is always positive.
												//SegmentAngle<TargetAngle													
												//  ex: a0=-30 a1=-32 target=0 maxspread=1 a0-a1=2 2>1 (slow down)
												//  ex2: a0=10 a1=12 target=30 maxspread=1 a0-a1=-2 -2<1 (no need to slow down)  
												//SegmentAngle>TargetAngle													
												//  ex: a0=-4 a1=-2 target=-30 maxspread=1 a0-a1=-2 -2<-1 (slow down)
												//  ex2: a0=10 a1=8 target=5 maxspread=1 a0-a1=2 2>-1 (no need to slow down)  
												SegmentAngle2=leth[1]->Accel[leai->AccelNum[1]].EstAngle[leai->xyz[1]];
												if ( (SegmentAngle<TargetAngle &&
														 (SegmentAngle-SegmentAngle2)>leai->MaxSpread) ||
														 (SegmentAngle>TargetAngle &&
														 (SegmentAngle-SegmentAngle2)<-leai->MaxSpread) ) {  

													//need to slow down to let other accel catch up
													leai->flags|=ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL;  //set flag
													//decrease thrust to +-1
													//currently motor can only thrust to increase speed (and decrease thrust to lower speed)- 
													//there is curently no active braking- where thrust is away from TargetAngle.
													//so thrust can only be in direction of TargetAngle
													if (lemi->Thrust>0 && ThrustInCorrectDirection) {
															lemi->Thrust--;  //slow down
															if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDec SYNC Thr=Y Thr-- angs:%3.1f %3.1f\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,SegmentAngle2);
															} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													}
													if (lemi->Thrust<0 && ThrustInCorrectDirection) {
															lemi->Thrust++;  //slow down
															if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc SYNC Thr=Y Thr++ angs:%3.1f %3.1f\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,SegmentAngle2);
															} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													}  
												} else {
													//no need to slow down to let other accel catch up
													leai->flags&=~ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL;  //clear flag
												} //if ( (SegmentAngle<TargetAngle &&
											} //if (tinst->flags&ROBOT_INSTRUCTION_SYNC_TWO_ACCELS) {


											if (leai->flags&ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL) {
												//this motor needs to wait for a second accel to catch up
												//create a future log entry if enabled, so there are no empty timestamps in the future log file
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tWAIT FOR SYNC, skip thrust analysis angs:%3.1f %3.1f\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,SegmentAngle2);
												} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

											} else { //if (leai->flags&ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL) 

												//there is no need to wait for a second accel to catch up, so perform regular
												//thrust analysis

//#if 0 //NO THRUST ANALYSIS

												//====================================================
												//              THRUST ANALYSIS SECTION
												//====================================================

												//we do not want to analyze the effects of the last thrust when just starting an instruction where a SegmentAngle is initially out of range. We need to send an initial thrust and then start the actual analysis on the next sample. In theory that wastes 30 ms of samples (3) but currently, the motion model I am using is to set the initial thrust and then inc/dec from there. There is a case where one inst might be synchronously (and quickly) following another instruction, but probably thrust would be set to 0 after any complete instruction. It really remains for the future to determine clearly if an initial thrust can be included in the thrust analysis code section.

//fprintf(stderr,"12 ");

												if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) && !(tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {


//fprintf(stderr,"13 ");
													//if ((tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)==0) {
													//this instruction does not have the "constant thrust" flag set, so we can change the existing thrust
													//note that currently the sample where initial thrust is set passes through here- should not be an issue because code waits 30ms before inc/dec thrust, and allows check to see if braking needed.

													if (leai->WaitAndSeeTime==0 && (leai->flags&ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION)==0) {
														//skip Thrust Analysis section if WaitAndSee>0 (waiting after reaching target for motion to settle, or to detect an external (non motor thrust caused) motion) or if the DECREASING_OSCILLATION flag is set (because a decreasing oscillation was detected- so changing Thrust is on hold).



														//Velocity in correct direction?
														if (VelocityInCorrectDirection) {
															//*******************************************
															//VELOCITY IS IN THE CORRECT DIRECTION
															//*******************************************

															//target within 300ms at current speed and accel? 
															//find if and where SegmentAngle crosses TargetAngle 
															//and if and where Segment velocity crosses 0
															i=-1;
															FutureSegmentAngle=SegmentAngle;
															FutureSegmentVelocity=Velocity;
															//FutureVelocityMinusFriction=Velocity;
															FutureTargetIndex=-1;
															FutureZeroVelocityIndex=-1;
															FutureMinVelocityIndex=-1;
															FutureMaxVelocityIndex=-1;
															//FutureFrictionIndex=(int)fabs(Velocity/0.9); //takes about Velcity/0.6 samples to reach Velocity=0 when Thrust=0
															//FutureTargetFrictionIndex=-1;
															//FutureSegmentAngleMinusFriction=SegmentAngle;
															while(i<NUM_FUTURE_SAMPLES) { // && (FutureTargetIndex==0 || FutureZeroVelocityIndex==0)) {		
																if (AngNeedsToInc) { //SegmentAngle<TargetAngle
																	//SegmentAngle needs to increase to reach TargetAngle
																	//check for SegmentAngle crossing target
																	if (FutureTargetIndex<0 && FutureSegmentAngle>=TargetAngle) { 
																		//at current rate, SegmentAngle will cross target angle in ix10ms
																		FutureTargetIndex=i+1; //remember index where SegmentAngle crosses target
																	}
																	//check for SegmentVelocity crossing 0 - velocity is positive
																	if (FutureZeroVelocityIndex<0 && FutureSegmentVelocity<=0.0) {
																		FutureZeroVelocityIndex=i+1;
																	}
																	//check for SegmentVelocity crossing MinVelocity
																	if (FutureMinVelocityIndex<0 && FutureSegmentVelocity<leai->MinVelocity[UseAngle]) {
																		FutureMinVelocityIndex=i+1;
																	} 
																	//check for SegmentVelocity crossing MaxVelocity
																	if (FutureMaxVelocityIndex<0 && FutureSegmentVelocity>leai->MaxVelocity[UseAngle]) {
																		FutureMaxVelocityIndex=i+1;
																	} 
																	//check for SegmentVelocity crossing Zero due to friction (Thrust=0)
																	//note since it is *0.9, FutureVelocityMinusFriction will never cross 0.0
																	//if (!FutureFrictionIndex && FutureVelocityMinusFriction<=1.0) {
																	//	FutureFrictionIndex=i+1;
																	//} 
																	//check for SegmentAngle crosses TargetAngle using FutureVelocityMinusFriction (Thrust=0)
																	//if (FutureTargetFrictionIndex<0 && FutureSegmentAngleMinusFriction>=TargetAngle) {
																	//	FutureTargetFrictionIndex=i+1;
																	//} 
																} else { //AngNeedsToInc //SegmentAngle>=TargetAngle
																	//SegmentAngle needs to decrease to reach TargetAngle
																	//check for SegmentAngle crossing target
																	if (FutureTargetIndex<0 && FutureSegmentAngle<=TargetAngle) { 
																		//at current rate, SegmentAngle will cross target angle in ix10ms
																		FutureTargetIndex=i+1; //remember index where SegmentAngle crosses target
																	}
																	//check for SegmentVelocity crossing 0 - velocity is negative
																	if (FutureZeroVelocityIndex<0 && FutureSegmentVelocity>=0.0) {
																		FutureZeroVelocityIndex=i+1;
																	} 
																	//check for SegmentVelocity crossing MinVelocity (note MinVelocity is always positive)
																	if (FutureMinVelocityIndex<0 && FutureSegmentVelocity>-leai->MinVelocity[UseAngle]) {
																		FutureMinVelocityIndex=i+1;
																	} 
																	//check for SegmentVelocity crossing MaxVelocity (note MaxVelocity is always positive)
																	if (FutureMaxVelocityIndex<0 && FutureSegmentVelocity<-leai->MaxVelocity[UseAngle]) {
																		FutureMaxVelocityIndex=i+1;
																	} 
																	//check for SegmentVelocity crossing Zero due to friction (Thrust=0)
																	//note since it is *0.9, FutureVelocityMinusFriction will never cross 0.0
																	//if (!FutureFrictionIndex && FutureVelocityMinusFriction>=-1.0) {
																	//	FutureFrictionIndex=i+1;
																	//} 
																	//check for SegmentAngle crosses TargetAngle using FutureVelocityMinusFriction (Thrust=0)
																	//if (FutureTargetFrictionIndex<0 && FutureSegmentAngleMinusFriction<=TargetAngle) {
																	//	FutureTargetFrictionIndex=i+1;
																	//} 
																} //AngNeedsToInc

																FutureSegmentVelocity+=Acceleration;
																//FutureSegmentAngle+=FutureSegmentVelocity*0.01;  //add velocity and acceleration to SegmentAngle
																FutureSegmentAngle+=Velocity*0.01;  //add only velocity to SegmentAngle
																//FutureVelocityMinusFriction*=0.9; //
																//FutureSegmentAngleMinusFriction+=FutureVelocityMinusFriction*0.01; //add Velocity lowered by friction to SegmentAngle
																//FutureSegmentAngleMinusFriction+=fabs(Velocity/0.9)*0.01;
																//only need to store for future log:
																//FutureAngle[i]=FutureSegmentAngle;
																//FutureVelocity[i]=FutureSegmentVelocity;
																//FutureFrictionAngle[i]=FutureSegmentAngleMinusFriction;

																i++;
															} //while(i<NUM_FUTURE_SAMPLES && (FutureTargetIndex==0 || FutureZeroVelocityIndex==0))
															if (FutureMinVelocityIndex<0) {
																FutureMinVelocityIndex=NUM_FUTURE_SAMPLES;
															} 
															if (FutureMaxVelocityIndex<0) {
																FutureMaxVelocityIndex=NUM_FUTURE_SAMPLES;
															} 

															//log future motion data
															if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
									
																//log FutureTargetIndex, current and future SegmentAngle
																sprintf(LogStr,"%d\t%d\tAng\tTargetIdx=%d\t\t(%4.1f<%4.1f<%4.1f)\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureTargetIndex,leai->Angle[1],leai->Angle[0],leai->Angle[2],SegmentAngle);
																//sprintf(LogStr,"%d\tAng\tTargetIdx=%d Frict=%d\t\t%4.2f",tinst->InstTime, FutureTargetIndex,FutureTargetFrictionIndex,SegmentAngle);
																//tstr[0]=0;
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,FutureAngle[i]);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																//tstr[0]=0;
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//  sprintf(tstr,"%s\t%4.2f",tstr,FutureFrictionAngle[i]);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

																//log FutureZeroVelocityIndex and Current and Future Segment Velocity
																sprintf(LogStr,"%d\t%d\tVel\tZero=%d Min=%d Max=%d\t%d\t\t\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureZeroVelocityIndex,FutureMinVelocityIndex,FutureMaxVelocityIndex,VelocityInCorrectDirection,Velocity);
																//sprintf(LogStr,"%d\tVel\tZero=%d Min=%d Frict=%d\t%d\t\t%4.2f",tinst->InstTime, FutureZeroVelocityIndex,FutureMinVelocityIndex,FutureFrictionIndex,VelocityInCorrectDirection,Velocity);  //move Velocity over 1 column for ease in reading
																//tstr[0]=0;				
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,FutureVelocity[i]);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

																//log thrust and Segment Accleration
																//sprintf(LogStr,"%d\tAccel\tThrust=%d (%d)\t%d\t%4.2f",tinst->InstTime, lemi->Thrust,WithGravity,AccelerationInCorrectDirection,Acceleration);
																sprintf(LogStr,"%d\t%d\tAccel\tThrust=%d (%d)\t%d\t\t\t\t%4.2f\t%4.2f",ltask->TaskTime,tinst->InstTime, lemi->Thrust,WithGravity,AccelerationInCorrectDirection,Acceleration,AccelAvg3);  //move Acceleration over 2 columns for ease in reading
																//tstr[0]=0;				
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,Acceleration);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);
															} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {


															//======================================
															//START V=Y MOTION THRUST ANALYSIS CODE	
															//========================================
												
															//first check for OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET flag
															//If InRange and IsMoving and V=Y, apply the TVR method to oppose motion
															if ((tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET) && InRange && IsMoving) {
																//use the TVR method to try to stop the motion (is duplicate of below V=N code, todo: possibly move to a function)
																//set Thrust= Velocity*VelocityToThrustFactor in opposite direction
																//TVR can be different for positive and negative directions
																if (Velocity>0) { 
																	//positive direction
																	lemi->Thrust= (int)(Velocity * leai->ThrustVelocityRatio[UseAngle][0]);
																} else { //if (Velocity>0) { 
																	//negative direction
																	lemi->Thrust= (int)(-Velocity * leai->ThrustVelocityRatio[UseAngle][1]);
																} //if (Velocity>0) { 
																//lemi->Thrust is positive at this point
																//Because InRange and V=Y, check against MaxReverseThrust (MaxBrakeThrust)
																if (lemi->Thrust>lemi->MaxReverseThrust[UseAngle]) {  //clamp Thrust to max reverse thrust
																	lemi->Thrust=lemi->MaxReverseThrust[UseAngle];
																} 
																//determine if we need to negate thrust (V=N)
																if ((Velocity>0 && IncAngDir==1) || (Velocity<0 && IncAngDir==-1)) {
																	lemi->Thrust=-lemi->Thrust;
																}  
																if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																	fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tV=Y OPPOSE_MOTION_IN_RANGE Seg=%03.2f Targ=%03.2f Vel=%03.2f TVR=%03.2f Thrust=%d IncAngDir=%d\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,TargetAngle,Velocity,leai->ThrustVelocityRatio[UseAngle][(Velocity<=0)],lemi->Thrust,IncAngDir);
																} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			
											


															} else { //if ((tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET) && InRange && IsMoving) {
														
																//OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET flag not set


																//If Velocity>MaxVelocity and Thrust==0, 
								
																if (FutureTargetIndex<1000 && !(tinst->flags&ROBOT_INSTRUCTION_NO_STOP_ON_TARGET)) {
																	//Target is within 1000ms at current rate - Planning stop (and NO_STOP_ON_TARGET flag is not set)

																	//The segment can actually accelerate even with Thrust=0, so there are times where a reverse thrust to stop is necessasry. But when InRange=1 && IsMoving==0 (<2dps), Thrust should=0.
																	//V=Y A=X, if TargetIndex<15 and |V|*2>TargetIdx, then need to brake, set Thrust=0 
																	//if (FutureTargetIndex<=3) {
																	//there are 2 parameters Length of Braking (FutureTargetIndex) and Strength of Braking (Velocity*10>FutureTargetIndex)  *10.0 means that from 0-9 samples Velocity must be <1 or else Thrust will be decreased (Thrust will get set to 0 once Segment is InRange and V<2), and from 10-19 samples Velocity must be <2.
									
																	//See if Emergency braking is needed (nearing target and velocity is too fast to stop on the target)
																	//One problem is that if the weight of gravity on a segment is large, the brake may actually reverse thrust with gravity and it can cause a quick reversal of the velocity. So added:
//MovingAgainstGravity toward the target - not nearly as much brake is needed- let the velocity be high (use a lower velocity multiplier/factor)
//but !MovingAgainstGravity (moving with gravity) toward the target more braking is needed, do not let the velocity be high (use a higher velocity multipier/factor). 
																	//velocity factor V*X: target is Y ms away, Thrust will be decreased until Velocity is < Z
																	//V*9 is 0-8 <1, 9-17 <2, 18-26 <3  (target is 0-80ms away, V=0, 90-170ms V=1, etc.)
																	//V*8 is 0-7 <1, 8-15 <2, 16-23 <3 
																	//V*7 is 0-6 <1, 7-13 <2, 14-20 <3
																	//if (FutureTargetIndex<=25 && fabs(Velocity)*10.0>FutureTargetIndex) {
																	//if (FutureTargetIndex<=40 && fabs(Velocity)*7.0>FutureTargetIndex) {
																	if (FutureTargetIndex<=25 && 
( (MovingAgainstGravity && fabs(Velocity)*5.0>FutureTargetIndex) ||  
(!MovingAgainstGravity && fabs(Velocity)*7.0>FutureTargetIndex) ) ) {
																		//This piece of code under this if is not great 

																		//changing this if, has a large effect on balance. For example, allowing a reverse thrust here, caused a lot of problems- possibly with the knees, and perhaps feet- where setting 0 makes balancing much more likely. This code is basically the code that brings the segment to a stop at the target by setting thrust=0 or reverse (braking) seeks to stop all motion as soon as possible.    
																		//Moving too quickly to target, and target is near (250ms away)
																		//if ThrustInCorrectDirection and V>MaxV, set Thrust to opposite direction to start brake. Set T=IncAngDir if (AngNeedsToInc==0) or T=-IncAngDir if otherwise
																		if (Velocity>leai->MaxVelocity[UseAngle]) {
																			if (ThrustInCorrectDirection) {
																				//T=Y, need to reverse Thrust (start brake)  
																				if (AngNeedsToInc) {
																					lemi->Thrust=-IncAngDir;
																				} else {
																					lemi->Thrust=IncAngDir;
																				} 
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTART BRAKE: Targ<=25 V>MaxV\n",ltask->TaskTime,tinst->InstTime);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																			} else {	//if (ThrustInCorrectDirection) { 
																				//T=N and V>MaxV, already braking, need to brake more																						
																				DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					//fprintf(tinst->FutureLogfptr,"%d\t\t\tThrust=0: Targ<brake Vel=Y\n",tinst->InstTime);
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tINC BRAKE DecThrust: Targ<=25 V*5>Targ\n",ltask->TaskTime,tinst->InstTime);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																			} //if (ThrustInCorrectDirection) {
																		} //if (Velocity>leai->MaxVelocity[UseAngle]) {
																	} else { //if (FutureTargetIndex<=25) {																	
																		//FutureTargetIndex>25ms away
																		//Will speed cross 0 at same time (+/-2) that SegmentAngle crosses Target?
																		if (FutureZeroVelocityIndex>=0) {
																			//SegmentAngle will cross TargetAngle and Speed will cross zero within 1000ms

																			//Does Segment speed cross zero before Segment will cross Target?
																			if (FutureZeroVelocityIndex<FutureTargetIndex-3) {
																				//if not accelerating and Thrust<ThrustNeeded+1 increase thrust (careful of Thrust sign)
																				//if (!AccelerationInCorrectDirection && ((fabs(lemi->Thrust)<fabs(ThrustNeeded) && fabs(Velocity)<=leai->MaxVelocity) || fabs(Velocity)<leai->MinVelocity)) {
																				//if (!AccelerationInCorrectDirection && fabs(lemi->Thrust)<fabs(ThrustNeeded) && fabs(Velocity)<=leai->MaxVelocity) {
																				//make sure that we only ever go NeededThrust+1 otherwise too much velocity develops, unless user overrides with MAXTHRUST flag.
																				//added check for <MaxVelocity because when AccelInWrongDir FutureMax

																				//if (!AccelerationInCorrectDirection && FutureMaxVelocityIndex>5 && fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
																				//Note that FutureMinVelocityIndex<5 restricts increasing thrust until the MinVelocity is only 40ms away  //
																				if (!AccelerationInCorrectDirection && FutureMinVelocityIndex<=5 && FutureMaxVelocityIndex>=5) {
																					//(!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded))) {
																					//added 0.5*leai->MaxVelocity because should not increase if velocity is near Max even if decel is high
																					if (FutureTargetIndex>5) {// && fabs(Velocity)< 0.5*leai->MaxVelocity[UseAngle]) { //within 50ms do not increase
																						if (!ThrustInCorrectDirection && lemi->LastThrust!=0) { //braking
																							//stop brake
																							lemi->Thrust=0;
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tStop Brake T=0: will stop too soon Targ>5 MinVelInd<5 T=N\n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} else {
																							//increase thrust
																							IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: will stop too soon Targ>5 MinVelInd<5 \n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} //if (!ThrustInCorrectDirection) {
																					} //if (FutureTargetIndex>5) {
																				} //if (!AccelerationInCorrectDirection ...
																			} else { //if (FutureZeroVelocityIndex<FutureTargetIndex) {
																				//is FutureZeroVelocityIndex> than FutureTargetIndex+3? - going too fast
																				if (FutureZeroVelocityIndex>FutureTargetIndex+3) {
																					//if accelerating and fabs(Thrust)>0 decrease thrust (careful of Thrust sign)
																					if (AccelerationInCorrectDirection && (fabs(Velocity)>leai->MinVelocity[UseAngle] || fabs(Acceleration)>leai->MaxAccel[UseAngle])) { 
																						//a Thrust=0 can powerfully brake, and so only use Thrust=0 near the target

																						//if (fabs(lemi->Thrust)>0 && (FutureTargetIndex<40 || fabs(Velocity)>leai->MaxVelocity)) {
																						//if (fabs(lemi->Thrust)>0 && (FutureTargetIndex<40 || FutureMaxVelocityIndex<5)) {
																						//possibly may want to add (TICD || fabs(Velocity)>MaxVelocity- but FMVI<5 should cover that) 
																						if (ThrustInCorrectDirection && (FutureTargetIndex<40 || FutureMaxVelocityIndex<5 || fabs(Acceleration)>leai->MaxAccel[UseAngle])) {

																							//decrease thrust
																							DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t\t\tDec: will stop too late\n",tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} //if (fabs(lemi->Thrust)>1 ...
																					} //if (AccelerationInCorrectDirection ...
																				} //if (FutureZeroVelocityIndex>FutureTargetIndex+3) {
																			} //if (FutureZeroVelocityIndex<FutureTargetIndex) {
																		} else { //if (FutureZeroVelocityIndex>=0)
																			//Velocity=0 is not within next 100ms 
																			//Target is <=25ms
																			//see if need to dec thrust (>MinV or A>MaxAccel, and (T=Y or V>MaxV or A>MaxA)
																			if (AccelerationInCorrectDirection && (fabs(Velocity)>leai->MinVelocity[UseAngle] || fabs(Acceleration)>leai->MaxAccel[UseAngle]) && (ThrustInCorrectDirection || fabs(Velocity)>leai->MaxVelocity[UseAngle] || fabs(Acceleration)>leai->MaxAccel[UseAngle])) { //will go in reverse 
																				//if (FutureTargetIndex< 50 || fabs(Velocity)>leai->MaxVelocity || fabs(lemi->Thrust)>fabs(ThrustNeeded)) {
																				//if (fabs(Velocity)>leai->MaxVelocity || fabs(lemi->Thrust)>fabs(ThrustNeeded)) {
																				//if (fabs(Velocity)>leai->MaxVelocity) {
																					//if ((ThrustNeeded>0 && lemi->Thrust>-1) || (ThrustNeeded<0 && lemi->Thrust<1)) { 
																					//if ((ThrustNeeded>0 && lemi->Thrust>0) || (ThrustNeeded<0 && lemi->Thrust<0)) { 

																						//decrease thrust (if T=Y lower thrust magnitude, if T=N, increase brake by decreasing thrust)
																						DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDec: Targ<25 Zero>28 V=Y A=Y V>Min Need=%d T=%d\n",ltask->TaskTime,tinst->InstTime,ThrustNeeded,ThrustInCorrectDirection);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					//} //if ((ThrustNeeded>0
																				//} //if (fabs(Velocity)>leai->MaxVelocity)

																			} //if (AccelerationInCorrectDirection ...

																		} //if (FutureTargetIndex<=25) {


																	} //if (FutureZeroVelocityIndex>=0) 
																} else { //if (FutureTargetIndex<1000 && !(tinst->flags&ROBOT_INSTRUCTION_NO_STOP_ON_TARGET)) {
																	//V=Y and Target is more than 1000ms away, still too far away to start braking (Thrust=0) unless Velocity>MaxVelocity or A>MaxAccel or ROBOT_INSTRUCTION_NO_STOP_ON_TARGET flag is set (user wants to reach target angle with a certain velocity)
//remove analysis when velocity is in correct direction and target is more than 1s away
//other than:
//1) if fabs(Velocity)>MaxVelocity (going too fast already) then:
//a) if Thrust=Y, set Thrust=0 (START BRAKE)
//b) if Thrust=N, (BRAKING)
 //1) if Accel=Y Dec (INCREASE BRAKE)
 //2) if Accel=N Inc (DECREASE BRAKE)
//2) or else if MaxVelocity is within 50ms and fabs(Thrust)>0 and fabs(Velocity)>MinVelocity, then DecreaseThrust 
//3) or else (see below was:)if MinVelocity is within 50ms and Thrust<MaxThrust then IncreaseThrust

																	//1 Velocity is greater than MaxVelocity, stop thrust
																	if (fabs(Velocity)>leai->MaxVelocity[UseAngle]) {
																		//if ThrustInCorrrectDirection, set Thrust=0																				
																		if (ThrustInCorrectDirection) {
																			lemi->Thrust=0;
																			if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTART BRAKE Thrust=0: V=Y V>MaxV (%3f) T=Y \n",ltask->TaskTime,tinst->InstTime,leai->MaxVelocity[UseAngle]);
																			} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																		} else { //if (ThrustInCorrectDirection) {
																			//!ThrustInCorrectDirection, Brake with reverse thrust (no need to check MaxReverseThrust- is done in DecreaseThrust())
																				//If Accel=Y (in correct direction) Dec (INCREASE BRAKE)
																				if (AccelerationInCorrectDirection) {
																					DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																					if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tINCREASE BRAKE Dec: V=Y V>MaxV T=N A=Y\n",ltask->TaskTime,tinst->InstTime);
																					} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				} else { //if (AccelerationInCorrectDirection) {
																					if (lemi->LastThrust!=0) {
																						//Important: T=N include T=0, Inc when T=0 will inc thrust, not dec brake
																						IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDECREASE BRAKE Inc: V=Y V>MaxV T=N T!=0 A=N\n",ltask->TaskTime,tinst->InstTime);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					} //if (lemi->LastThrust!=0) {
																				} //if (AccelerationInCorrectDirection) {
																		} //if (ThrustInCorrectDirection) {
																	} else { //if (fabs(Velocity)>leai->MaxVelocity[UseAngle]) {
																		//***V is <=MaxV ***
																		//if approaching MaxVelocity or Accel>MaxAccel, decrease thrust
//																				if (FutureMaxVelocityIndex<5 && ThrustInCorrectDirection && fabs(lemi->Thrust)>0 && fabs(Velocity)>leai->MinVelocity[UseAngle]) {
																		if (FutureMaxVelocityIndex<5) {
																			//MaxV is approaching
																			if (ThrustInCorrectDirection) {
																				//T=Y, set T=0
																				lemi->Thrust=0;
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTART BRAKE Thrust=0: V=Y MaxVFI<5 T=Y \n",ltask->TaskTime,tinst->InstTime);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																			} else { //if (ThrustInCorrectDirection) {
																				//MaxVFI<5, T=N
																				//if MinVFI<5 then set T=0 
																				if (FutureMinVelocityIndex<5) {
																					lemi->Thrust=0;
																					if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\t(In between MaxV and MinV) Thrust=0: V=Y MaxVFI<5 MinVFI<5 T=N \n",ltask->TaskTime,tinst->InstTime);
																					} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				} else { //if (FutureMinVelocityIndex<5) {
																					//MaxVFI<5, T=N, MinVFI>=5
																					DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																					if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {																		
																						fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tBRAKE MORE: Dec V=Y MaxVFI<5 T=N MinVFI>=5\n",ltask->TaskTime,tinst->InstTime);
																					} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				} //if (FutureMinVelocityIndex<5) {
																			} //if (ThrustInCorrectDirection) {
																		} else { //if (FutureMaxVelocityIndex<5 &&
																			//Maximum V is not approaching- need to speed up or slow down? 
																			//*** V<MaxV, MaxVFI>=5, 
																			//*** 1) if V<MinV 
																			//      a) if T=Y, 
																			//         1) if A=Y: if A>MaxA:dec else:inc. if A=N:inc
																			//			b) if T=N, 
																			//				 1) if A=Y: if A>MaxA:dec(inc brake) else:inc(dec brake). if A=N:T=(AngNeedsToInc/IncAngDir){stop brake and reverse} 
																			//    2) else V>MinV but
				                              //       a) if MinV is approaching (A can only = N) 
																			//				  1) if T=Y:inc T=N:T=(AngNeedsToInc/IncAngDir){stop brake and reverse} 
																			if (fabs(Velocity)<leai->MinVelocity[UseAngle]) {
																				//V=Y but V<MinV (could be acceling up to speed or decelling down from MaxAccel)
																				if (ThrustInCorrectDirection) {
																					//T=Y
																					if (AccelerationInCorrectDirection) {
																						//T=Y A=Y 
																						if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																							DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDec: V=Y V<MinV T=Y A=Y A>MaxA (%3.1f)\n",ltask->TaskTime,tinst->InstTime,leai->MaxAccel[UseAngle]);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {																	
																						} else { //if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																							//A=Y but A<MaxA (accel can be increased to MaxA)
																							IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: V=Y V<MinV T=Y A=Y A<MaxA\n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} //if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																					} else { //if (AccelerationInCorrectDirection) {
																						//T=Y A=N Accel can be increased to MaxA
																						IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: V=Y V<MinV T=Y A=N\n",ltask->TaskTime,tinst->InstTime);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					} //if (AccelerationInCorrectDirection) {
																				} else { //if (ThrustInCorrectDirection) {
																					//T=N (BRAKING)
																					if (AccelerationInCorrectDirection) {
																						//T=N A=Y 
																						if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																							//Accel>MaxA increase brake (decrease thrust) 
																							//Note: Dec will Inc brake even when T=0
																							DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tINC BRAKE Dec: V=Y V<MinV T=N A=Y A>MaxA (%3.1f)\n",ltask->TaskTime,tinst->InstTime,leai->MaxAccel[UseAngle]);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {																	
																						} else { //if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																							//A=Y but A<MaxA (accel can be increased to MaxA so decrease brake)
																							//Important: if T=0, Inc will Inc thrust, not dec brake
																							if (lemi->LastThrust!=0) {
																								//V<MinV T=N A=Y A<MaxA Stop Brake and set thrust to 1 in correct direction
																								if(AngNeedsToInc) {
																									lemi->Thrust=IncAngDir;
																								} else {
																									lemi->Thrust=-IncAngDir;
																								} 
																								if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																									fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTOP BRAKE & THRUST T=%d: V=Y V<MinV T=N A=Y A<MaxA\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust);
																								} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							} //if (lemi->LastThrust!=0) {
																						} //if (fabs(Acceleration)>leai->MaxAccel[UseAngle]) {
																					} else { //if (AccelerationInCorrectDirection) {
																						//T=N A=N Stop Brake and set thrust to 1 in correct direction
																						if(AngNeedsToInc) {
																							lemi->Thrust=IncAngDir;
																						} else {
																							lemi->Thrust=-IncAngDir;
																						} 
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTOP BRAKE & THRUST T=%d: V=Y V<MinV T=N A=N\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					} //if (AccelerationInCorrectDirection) {

																				} //if (ThrustInCorrectDirection) {
																			} else { //if (fabs(Velocity)<leai->MinVelocity[UseAngle]) {
																				//V=Y V>MinV See if Min Velocity is approaching (A can only = N, so no threat of A>MaxA))
																				//  if T=Y:inc T=N:T=(AngNeedsToInc/IncAngDir){stop brake and reverse} 
																				if (FutureMinVelocityIndex<5) {  //MinVelocity is approaching
																					//MinVFI<5 (A can only = N)
																					if (ThrustInCorrectDirection) {  
																						//already thrusting, so increase thrust
																						IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							//fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: MinVelInd<5 Thr=Y Thr<MaxThr ThrCnt>2\n",ltask->TaskTime,tinst->InstTime);
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tMINV Inc: V=Y T=Y (A=N) V>MinV but MinVFI<5\n",ltask->TaskTime,tinst->InstTime);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					} else { //if (ThrustInCorrectDirection) {  
																						//MinVFI<5,T=N (BRAKING)
																						//T=N A=N Stop Brake and set thrust to 1 in correct direction
																						if(AngNeedsToInc) {
																							lemi->Thrust=IncAngDir;
																						} else {
																							lemi->Thrust=-IncAngDir;
																						} 
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tSTOP BRAKE & THRUST T=%d: V=Y (A=N) T=N V>MinV but MinVFI<5\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

#if 0 
																						//if Thrust==0 increase or else Thrust=0
																						if (lemi->LastThrust!=0) {
																							lemi->Thrust=0;
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tMINV STOP BRAKE T=0: V=Y MaxVFI>=5 MinVFI<5 T=N\n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} else { //if (lemi->LastThrust!=0) {
																							IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tMINV Inc: V=Y T=0 MaxVFI>=5 MinVFI<5\n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						} //if (lemi->LastThrust!=0) {
#endif
																					} //if (ThrustInCorrectDirection) {  
																				} else { //if (FutureMinVelocityIndex<5

																					//Neither VmaxFI nor VminFI are a problem (V is within range, and neither MinV or MaxV are approaching) 

																					//the default is to maintain thrust unless:
																					//1) if T=Y and (A>MaxAccel or (V>MinV && !USE_MAX_VELOCITY)), then decrease thrust
																					if (ThrustInCorrectDirection && ((AccelerationInCorrectDirection && (fabs(Acceleration)>leai->MaxAccel[UseAngle])) || (fabs(Velocity)>leai->MinVelocity[UseAngle] && !(tinst->flags&ROBOT_INSTRUCTION_USE_MAX_VELOCITY)))) {
																						DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							if (AccelerationInCorrectDirection && (fabs(Acceleration)>leai->MaxAccel[UseAngle])) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tA>MaxA (%3.1f) Dec: V=Y T=Y A=Y\n",ltask->TaskTime,tinst->InstTime,leai->MaxAccel[UseAngle]);
																							} else { //if (AccelerationInCorrectDirection && (fabs(Acc
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tV>MinV Dec: V=Y T=Y A=Y\n",ltask->TaskTime,tinst->InstTime);
																							} //else if (AccelerationInCorrectDirection && (fabs(Acc						
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) 
																					} //if (ThrustInCorrectDirection && ((AccelerationInCorrectDirection && ...

																					//for now do nothing, but possibly, if braking, set T=0 
#if 0 
																					//if BRAKING V=Y 
																					if (!ThrustInCorrectDirection) {
																						//if MinVelocity is approaching and !ThrustInCorrectDirection (braking) then set Thrust=0
																						if (FutureMinVelocityIndex<5) {
																						} else { //if (FutureMinVelocityIndex<5) {
																							//Braking (!ThrustInCorrectDirection), V<Vmax, V not near MaxV, V>Vmin and Vmin not nearing 
																							//Decrease brake if decellerating and not near MaxV
																							//if V=Y A=N !ThrustInCorrectDirection FutureMaxVelocityIndex>5   
																							if (!AccelerationInCorrectDirection && FutureMaxVelocityIndex>5 && fabs(lemi->Thrust)>0) {
																								IncreaseThrust(lemi,AngNeedsToInc,IncAngDir);
																								if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																									fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDecrease Brake (Inc): Thr=N V=Y A=N MaxVelInd>5\n",ltask->TaskTime,tinst->InstTime);
																								} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							} // if (!AccelerationInCorrectDirection && 
																						} //if (FutureMinVelocityIndex<5) {
																					} //if (!ThrustInCorrectDirection) {
#endif
																				} ////if (FutureMinVelocityIndex<5) {
																			} //if (fabs(Velocity)<leai->MinVelocity[UseAngle]) {
													
																		} //if (FutureMaxVelocityIndex<5 &&
																	} //if (fabs(Velocity)>leai->MaxVelocity[UseAngle]) {
											
#if 0 
																	if (AccelerationInCorrectDirection) {
																		//Both Velocity and Acceleration are in the correct direction
																		//see if there is a surplus of thrust
																		//if (fabs(Velocity)>leai->MaxVelocity || fabs(lemi->Thrust)>=fabs(ThrustNeeded)) {
																		//if (fabs(Velocity)>leai->MaxVelocity) {
																		//
																		//if (FutureMaxVelocityIndex<5 && fabs(lemi->Thrust)>0)) {  //max velocity is less than 50ms away
																		//if ((fabs(Velocity)>leai->MinVelocity[UseAngle] || FutureMaxVelocityIndex<5) && fabs(lemi->Thrust)>0) {  //max velocity is less than 50ms away
																		//Note: ThrustInCorrectDirection replaced fabs(lemi->Thrust)>0 because Thrust sometimes needs to be reversed (emergency braking) when Velocity>MaxVelicity
																		//if ((fabs(Velocity)>leai->MinVelocity[UseAngle] || FutureMaxVelocityIndex<=5) && (ThrustInCorrectDirection || fabs(Velocity)>leai->MaxVelocity[UseAngle])) {  //max velocity is less than 50ms away


																		if (FutureMinVelocityIndex>5 && FutureMaxVelocityIndex<=5 && (ThrustInCorrectDirection || fabs(Velocity)>leai->MaxVelocity[UseAngle]) && lemi->ThrustCount>2 && fabs(Velocity)>leai->MedianVelocity[UseAngle]) {  //(allows Thrust to go in reverse) max velocity is less than 50ms away
																		//if (FutureMinVelocityIndex>5 && FutureMaxVelocityIndex<=5 && ThrustInCorrectDirection) {  // (only goes to Thrust=0) max velocity is less than 50ms away																		
																			//yes Velocity>MinVelocity or Thrust is higher in magnitude than needed																
																			if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				//if (fabs(lemi->Thrust)>fabs(ThrustNeeded)) {
																				//	fprintf(tinst->FutureLogfptr,"%d\t\t\tDec: V=Y A=Y Need=%d T>TN\n",tinst->InstTime,ThrustNeeded);
																				//} else {
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDec: Targ>100 V=Y A=Y FMin>5 FMax<=5 V>Vmed Need=%d\n",ltask->TaskTime,tinst->InstTime,ThrustNeeded);
																				//} 
																			} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

																			DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);

																		} //if (fabs(Velocity)>leai->MinVelocity[UseAngle] || (FutureMaxVelocityIndex<5 && fabs(lemi->Thrust)>0))
*/
																	} else { //if (AccelerationInCorrectDirection) {
																		//Velocity is in the correct direction, but Acceleration is in the wrong direction
																		//the Segment is slowing down  lemi->ThrustCount>2
																		//if (fabs(Velocity)<=leai->MaxVelocity && fabs(lemi->Thrust)<=fabs(ThrustNeeded)) {
																		if (FutureMinVelocityIndex<=5) {	  //DO NOT ADD MORE CONDITIONS- SEE ELSE															
																			//never thrust more than ThrustNeeded, unless user specifies with MAX_THRUST, or else too much velocity can develop.
																			//if (fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
																			//if ((!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded)) && fabs(Velocity)<0.5*leai->MaxVelocity[UseAngle]) {
																			if (lemi->ThrustCount>2 && (!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded) || FutureMinVelocityIndex==0)) {
																				//increase thrust
																				IncreaseThrust(lemi,AngNeedsToInc,IncAngDir);
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: Targ>100 V=Y A=N V<MinV or FMin<=5\n",ltask->TaskTime,tinst->InstTime);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																			} //if (!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
																		} else { //if (fabs(Velocity)<=leai->MinVelocity[UseAngle])
																			//Velocity is not less than MinVelocity, and the MinVelocity is more than 5 samples away.
																			//There is an important case where the velocity is quite high even though decellerating, and so if the Velocity is over the MinVelocity, Thrust should be decreased to 0, otherwise a large velocity can build up over time by the Velocity getting closer and closer to the MaxVelocity, but adding 5*Acceleration puts the Velocity < MinVelocity.
																			//Simply, if you have a high velocity (but not MaxVelocity), you do not want to be thrusting.
																			if (ThrustInCorrectDirection && lemi->ThrustCount>2 && fabs(Velocity)>leai->MedianVelocity[UseAngle]) { // && fabs(lemi->Thrust)>0) {
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDec: Targ>100 V=Y A=N TICD TC>2 V>Vmed Need=%d\n",ltask->TaskTime,tinst->InstTime,ThrustNeeded);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

																				DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																			} //if (ThrustInCorrectDirection) {

																		} //if (!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
									
																	} //if (AccelerationInCorrectDirection) {

#endif //if 0 																																		
																} ////if (FutureTargetIndex<1000 && !(tinst->flags&ROBOT_INSTRUCTION_NO_STOP_ON_TARGET))
															} //if ((tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET) && InRange && IsMoving) {

														} else { //if (VelocityInCorrectDirection) {


															//======================================================
															//VELOCITY IS IN THE WRONG DIRECTION (Velocity=No) (V=N) (Target is more than 1000ms away)
															//======================================================
															//if velocity is not in the correct direction, there is no need to calculate if the target angle would be passed in the near future.
															//Note that when Velocity is in the wrong direction (as opposed to when Velocity is in the correct direction), Thrust may need to increase more than ThrustNeeded so use lemi>=ThrustNeeded. 


															//print out Angle, Velocity, Accel and Thrust if FUTURE_LOG is set
															if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																//log FutureTargetIndex, current and future SegmentAngle
																sprintf(LogStr,"%d\t%d\tAng\tTargetIdx=%d\t\t(%4.1f<%4.1f<%4.1f)\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureTargetIndex,leai->Angle[1],leai->Angle[0],leai->Angle[2],SegmentAngle);
																//tstr[0]=0;
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,FutureAngle[i]);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

																//log FutureZeroVelocityIndex and Current and Future Segment Velocity
																sprintf(LogStr,"%d\t%d\tVel\tZero=%d\t%d\t\t\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureZeroVelocityIndex,VelocityInCorrectDirection,Velocity); //move Velocity over 1 column for ease in reading 
																//tstr[0]=0;				
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,FutureVelocity[i]);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

																//log thrust and Segment Accleration
																sprintf(LogStr,"%d\t%d\tAccel\tThrust=%d (%d)\t%d\t\t\t\t%4.2f\t%4.2f",ltask->TaskTime,tinst->InstTime, lemi->Thrust,WithGravity,AccelerationInCorrectDirection,Acceleration,AccelAvg3);  //move Acceleration over 2 columns for ease in reading
																//tstr[0]=0;				
																//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {
																//	sprintf(tstr,"%s\t%4.2f",tstr,Acceleration);
																//}
																//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
																fprintf(tinst->FutureLogfptr, "%s\n",LogStr);
															} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {



															//Start Velocity in wrong direction (V=N) Thrust setting code

															//if InRange==1 and IsMoving==0 set Thrust=0, note IsMoving is when fabs(Velocity)>MotionThreshold) and so can vary depending on the body segment.
															if (InRange && !IsMoving) {
																lemi->Thrust=0;			
																if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																	fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tThrust=0: V=N InRange !IsMoving\n",ltask->TaskTime,tinst->InstTime);
																} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
//#endif
#if 0  
															//If the TargetAngle is within +-1 degree of TargetAngle, Thrust=0, |velocity|<10dps (alt: and accel is in opposite direction of velocity (slowing down)) - another safe guard against oscillating around a target angle
															if ((SegmentAngle>(TargetAngle-1.0)) && (SegmentAngle<(TargetAngle+1.0)) && lemi->Thrust==0 && (fabs(Velocity)<15.0 || (fabs(Velocity)<25.0 && AccelerationInCorrectDirection))) {
																if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																	fprintf(tinst->FutureLogfptr,"%d\t\t\tNothing: V=N Targ +-1 degree...\n",tinst->InstTime);
																} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
#endif 
															} else { //if (InRange && !IsMoving) {   //if ((SegmentAngle>(TargetAngle-1.0)) && (SegmentAngle<(TargetAngle+1.0)) && lemi->Thrust==0 && fabs(Velocity)<15.0) {

																//TODO: Need to revisit and improve this code: target could be near, ThrustNeeded should possibly be removed to make more generic, this code assumes incremental thrust feedback model (not ThrustVelocityRatio method).

																//V=N and is not (InRange && !IsMoving)
																//so is either not InRange and Moving away from target, or not InRange and Moving  

																//move TVR code above InRange && !IsMoving if?
																//If user set HOLD_ANGLE=+#:-# then use TVR method to set opposing thrust
																//this can result in a much quicker increasing, and/or proportional opposing thrust
																//if (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE_TVR) {
																if (tinst->flags&ROBOT_INSTRUCTION_TVR) {
																	//set Thrust= Velocity*VelocityToThrustFactor in opposite direction
																	//TVR can be different for positive and negative directions
																	if (Velocity>0) { 
																		//positive direction
																		lemi->Thrust= (int)(Velocity * leai->ThrustVelocityRatio[UseAngle][0]);
																	} else { //if (Velocity>0) { 
																		//negative direction
																		lemi->Thrust= (int)(-Velocity * leai->ThrustVelocityRatio[UseAngle][1]);
																	} //if (Velocity>0) {
																	//lemi->Thrust is now positive 
																	//When V is small (like 0.5), the Angle can be far out of range, but Thrust will only be set to 0 or 1. So to counter this problem: if !InRange and T<MinTVRThrust, set T=MinTVRThrust
																	if (!InRange && lemi->Thrust<leai->MinTVRThrust[UseAngle]) {
																		lemi->Thrust=leai->MinTVRThrust[UseAngle];
																	}	//if (!InRange && lemi->Thrust<leai->MaxTVRThrust[UseAngle]) {																				
																	//Currently if InRange and moving away from target (V=N), TVR thrust is viewed as braking. But if out of range and moving away from target (V=N), TVR thrust is viewed as thrusting toward target range until V=Y. 
																	if (InRange) {
																		if (lemi->Thrust>lemi->MaxReverseThrust[UseAngle]) {  //clamp Thrust to max thrust (done in TurnMotor anyway, but for logging)
																			lemi->Thrust=lemi->MaxReverseThrust[UseAngle];
																		} 
																		//determine if we need to negate thrust (V=N)
																		if ((Velocity>0 && IncAngDir==1) || (Velocity<0 && IncAngDir==-1)) {
																			lemi->Thrust=-lemi->Thrust;
																		}  
																	} else {
																		//Not InRange
																		if (lemi->Thrust>lemi->MaxThrust[UseAngle]) {  //clamp Thrust to max thrust (done in TurnMotor anyway, but for logging)
																			lemi->Thrust=lemi->MaxThrust[UseAngle];
																		} 
																		//determine if we need to negate thrust (V=N)
																		if ((Velocity>0 && IncAngDir==1) || (Velocity<0 && IncAngDir==-1)) {
																			lemi->Thrust=-lemi->Thrust;
																		}  
																	} 
																	if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																		//fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tV=N HOLD_ANGLE_TVR Seg=%03.2f Targ=%03.2f Vel=%03.2f TVR=%03.2f Thrust=%d IncAngDir=%d\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,TargetAngle,Velocity,leai->ThrustVelocityRatio[UseAngle][(Velocity<=0)],lemi->Thrust,IncAngDir);
																		fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tV=N TVR Seg=%03.2f Targ=%03.2f Vel=%03.2f TVR=%03.2f Thrust=%d IncAngDir=%d\n",ltask->TaskTime,tinst->InstTime,SegmentAngle,TargetAngle,Velocity,leai->ThrustVelocityRatio[UseAngle][(Velocity<=0)],lemi->Thrust,IncAngDir);
																	} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			
												

																} else { //if (tinst->flags&ROBOT_INSTRUCTION_TVR) { //if (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE_TVR) {
																	//user not using TVR method
																

//#if 0 


																	//Is Acceleration in the correct direction?
																	if (AccelerationInCorrectDirection) {
																		//Velocity going in wrong direction but slowing down
																		if (ThrustInCorrectDirection || lemi->LastThrust==0) {
																			//Thrust is in the correct direction or =0																								

																			//For the torso and other gravity sensitive body segments, 
																			//if V=N and A=Y and MAG=Y, generally the brake should be decreased
																			//but if MAG=N, the brake should be maintained.
																			//will velocity reverse within 50ms?
																			i=0;
																			FutureSegmentVelocity=Velocity;
																			FutureZeroVelocityIndex=0;
																			while(i<5 && FutureZeroVelocityIndex==0) {		
																				FutureSegmentVelocity+=Acceleration;
																				if (Velocity>0.0) { 
																					if (FutureZeroVelocityIndex==0 && FutureSegmentVelocity<=0.0) {
																						FutureZeroVelocityIndex=i+1;
																					} 
																				} else { //Velocity>0.0
																					if (FutureZeroVelocityIndex==0 && FutureSegmentVelocity>=0.0) {
																						FutureZeroVelocityIndex=i+1;
																					} 
																				} //Velocity>0.0
																				i++;
																			} //while(i<5 && FutureZeroVelocityIndex==0)

																			if (FutureZeroVelocityIndex==0) {
																				//current acceleration will not reverse velocity within 50ms
																				//increase thrust unless thrust was recently changed
																				//if (lemi->ThrustCount>2) {
																					//never thrust more than ThrustNeeded, unless user specifies with MAX_THRUST, or else too much velocity can develop.
																					//if (fabs(lemi->Thrust)<=fabs(ThrustNeeded)) {
																					//if (!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
																						IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																						if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																							//fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: V=N A=Y T<TNeed (%d) No V reverse in 50ms\n",ltask->TaskTime,tinst->InstTime,ThrustNeeded);
																							fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: V=N A=Y T=Y No V reverse in 50ms\n",ltask->TaskTime,tinst->InstTime);
																						} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					//} //if (!ThrustInCorrectDirection || fabs(lemi->Thrust)<fabs(ThrustNeeded)) {
																			//} //if (lemi->ThrustCount>2) {

																			} else { //if (FutureZeroVelocityIndex==0) 
																				//V=N A=Y T=0 or T=Y and V will reverse within 50ms at the current accel

																				 
																		
																				//unless MotorAngleAssociation has ignore gravity flag set (WithGravity=0)
																				//then just set Thrust=0

																				//If MotorAngleAssociation has ignore gravity flag set, WithGravity=0
																				if (WithGravity) {
																					//MotorAngleAssociation is set to take gravity into account
																					if (MovingAgainstGravity) {
																						//V=N A=Y T=0 or T=Y if MovingAgainstGravity DecreaseThrust (Decrease Brake), because the momentum from that brake plus the accel from gravity may cause the body segment to start falling rapidly once V=Y
																						if (lemi->LastThrust!=0) {
																							//DecreaseThrust (decrease brake)
																							DecreaseThrust(lemi,AngNeedsToInc,IncAngDir,1,UseAngle);
																							if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																								fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC BRAKE Dec: V=N A=Y T=Y (V=Y w/in 50ms)\n",ltask->TaskTime,tinst->InstTime);
																							} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) 
																						} //if (lemi->LastThrust!=0) {
																					} else { //if (MovingAgainstGravity)
																						//if !MovingAgainstGravity (moving with gravity) do nothing (maintain thrust), because without the brake, the force of gravity may cause the body segment to fall more rapidly
																					} //else if (MovingAgainstGravity)
																				} else {
																					//MotorAngleAssociation is set to ignore gravity
																					//so just set Thrust=0
																					//lemi->Thrust=0;
																					//if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					//	fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tEND WRONG DIR BRAKE T=0: V=N A=Y T=Y (V=Y w/in 50ms)\n",ltask->TaskTime,tinst->InstTime);
																					//} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

																				} //if (WithGravity) {

																			} //else if (FutureZeroVelocityIndex==0) 


																		} else { //if (ThrustInCorrectDirection) {
																				//Thrust!=0 and is in the wrong direction, start brake by setting thrust to IncAngDir
																				//was: set thrust=GetNeededThrust 
																				//alt: set thrust to IncAngDir (+-1)
																				//alt2: IncreaseThrust
																				//lemi->Thrust=ThrustNeeded; //ThrustNeeded was already calculated above
																				//lemi->Thrust=AngNeedsToInc*IncAngDir-(!AngNeedsToInc)*IncAngDir;//ThrustNeeded; 
																				if (AngNeedsToInc) {
																					lemi->Thrust=IncAngDir;
																				} else {
																					lemi->Thrust=-IncAngDir;
																				}
																				if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																					//fprintf(tinst->FutureLogfptr,"%d\t\t\tThrustNeeded: %d Vel=N Accel=Y Thrust=N\n",tinst->InstTime,ThrustNeeded);
																					fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tTHRUST IN WRONG DIR (T=IncAngDir) T=%d: V=N A=Y T=N\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust);
																				} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {


																		} //if (lemi->Thrust!=0 && ThrustInCorrectDirection) {


																	} else { //if (AccelerationInCorrectDirection)
																		//Both Velocity and Acceleration are not in the correct direction - speeding up in the wrong direction

																		if (ThrustInCorrectDirection) {
																			//Thrust is not 0 and is in the correct direction, 
																			//Thrust is less than or equal to the estimated ThrustNeeded
																			//or the Velocity is greater than the MaximumVelocity
																			//if (fabs(lemi->Thrust)<=fabs(ThrustNeeded)) {
																			//if (fabs(lemi->Thrust)<=fabs(ThrustNeeded) || fabs(Velocity)>leai->MaxVelocity[UseAngle]) {
																			//if (fabs(Velocity)>leai->MaxVelocity[UseAngle]) {
																				//increase thrust if 30ms since last thrust change
																				//if (lemi->ThrustCount>2) {
																				//never thrust more than ThrustNeeded, unless user specifies with MAX_THRUST, or else too much velocity can develop.
																					IncreaseThrust(lemi,AngNeedsToInc,IncAngDir,UseAngle);
																					if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																						fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInc: V=N A=N T=Y\n",ltask->TaskTime,tinst->InstTime);
																					} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																			//} //if (lemi->ThrustCount>2) {
																			//} //if (fabs(lemi->Thrust)<=fabs(ThrustNeeded)) else - keep trying with ThrustNeeded+1
																		} else { //if (ThrustInCorrectDirection) {
																			//Thrust is 0 or in the wrong direction
																			//needs a quick change 
																			//alt: set thrust to IncAngDir (+-1)
																			//lemi->Thrust=ThrustNeeded; //ThrustNeeded was already calculated above
																			if (AngNeedsToInc) {
																				lemi->Thrust=IncAngDir;
																			} else {
																				lemi->Thrust=-IncAngDir;
																			}
																			if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
																				//fprintf(tinst->FutureLogfptr,"%d\t\t\tThrustNeeded: %d Vel=N Accel=N Thrust=N\n",tinst->InstTime,ThrustNeeded);
																				fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tIncAngDir T=%d: V=N A=N T=N\n",ltask->TaskTime,tinst->InstTime,lemi->Thrust);
																			} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

																		} //if (ThrustInCorrectDirection) {
																	} //if (AccelerationInCorrectDirection)

																} //if (tinst->flags&ROBOT_INSTRUCTION_TVR) { //if (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE_TVR) {
															} //if (InRange && !IsMoving) {    //if ((SegmentAngle>(TargetAngle-1.0)) && (SegmentAngle<(TargetAngle+1.0)) && lemi->Thrust==0 && fabs(Velocity)<15.0) {
														} //if (VelocityInCorrectDirection) {


													} //if (leai->WaitAndSeeTime==0 && (leai->flags&ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION)==0) {

												} //if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST) && (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)==0) {


//END THRUST ANALYSIS SECTION

//#endif //NO THRUST ANALYSIS


											} //if (leai->flags&ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL) {



										} else { //if (!InRange || (IsMoving && !(tinst->flags&ROBOT_INSTRUCTION_NO_HOLD)))  { 

											//Need to fill future log with timestamp, otherwise would be timestamps missing
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInRange and NotMoving (or has ROBOT_INSTRUCTION_NO_HOLD flag): NO THRUST ANALYSIS Segment %s\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,leai->SensorReadableName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			

										}  //elseif (!InRange || (IsMoving && !(tinst->flags&ROBOT_INSTRUCTION_NO_HOLD)))  { 

									} //elseif ( ((tinst->flagNoThrustWhenMAGIR&(1<<UseAngle))&&!(tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET)) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE) && InRange && IsMoving && ((MovingAgainstGravity && fabs(Velocity)<leai->MaxVelocity[UseAngle]) ||  VelocityInCorrectDirection) 

								} //if !(tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) - do not perform thrust analysis if inst just started

								//END !INRANGE OR ISMOVING AND INSTRUCTION IS STARTING OR CONDITION WAS NOT MET OR HOLD_ANGLE INST SEGMENT ANGLE IS MOVING

#if 0 
								} else { //if (!InRange || (IsMoving && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || !(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))))) {
									//NO INITIAL THRUST ANALYSIS IS NEEDED
									//need a future log entry here if no
										
									if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
											fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInRange INSTRUCTION_START: NO INITIAL THRUST ANALYSIS Segment %s\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,leai->SensorName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);
										} else { //if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
											fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tInRange NOT INSTRUCTION_START: NO THRUST ANALYSIS Segment %s\tTargetAngle=%4.2f %4.2f %4.2f\n",ltask->TaskTime,tinst->InstTime,leai->SensorName[UseAngle],leai->Angle[1],leai->Angle[0],leai->Angle[2]);

										} //elseif (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
									} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {			

									//if (tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) {
											//tinst->flags&=~ROBOT_INSTRUCTION_INSTRUCTION_START; //clear start flag
									//}

								} //elseif (!InRange || (IsMoving && ((tinst->flags&ROBOT_INSTRUCTION_INSTRUCTION_START) || !(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD))))) { 
#endif



							} //else if (tinst->flags&ROBOT_INSTRUCTION_OPPOSE_MOTION) {


//fprintf(stderr,"14 ");

		
							//If WaitAndSee>0 or DECREASING_OSCILLATION flag is set, we need to check here, because inside the above "if (!InRange..." excludes counting down WaitAndSee when the Segment and is InRange but is not moving (IsMoving=0). So this code needs to happen whether the SegmentAngle is moving or not.
							if ((leai->WaitAndSeeTime>0) || (leai->flags&ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION)) {
								//reached TargetAngle, set thrust=0, and now waiting for motion to stop or for any motion from an external source to become evident, and checking for a decreasing oscillation (oscillation that slows motion to a stop).

										//log future motion data - for velocity in wrong direction we only need Angle, Velocity, Accel and Thrust
								if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
	
									//log FutureTargetIndex, current and future SegmentAngle
									sprintf(LogStr,"%d\t%d\tAng\tTargetIdx=%d\t\t(%4.1f<%4.1f<%4.1f)\t%4.2f",ltask->TaskTime,tinst->InstTime, FutureTargetIndex,leai->Angle[1],leai->Angle[0],leai->Angle[2],SegmentAngle);
									fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

									//log FutureZeroVelocityIndex and Current and Future Segment Velocity
									sprintf(LogStr,"%d\tVel\tZero=%d\t%d\t\t\t%4.2f",tinst->InstTime, FutureZeroVelocityIndex,VelocityInCorrectDirection,Velocity);  //move Velocity over 1 column
									//tstr[0]=0;				
									//FutureVelocity[0]=0.0;
									//for(i=0;i<NUM_FUTURE_SAMPLES;i++) {  //esimate velocity 300ms into the future
									//for(i=0;i<5;i++) {  //esimate velocity 50ms into the future
									//	FutureVelocity[i]+=Acceleration;
									//	sprintf(tstr,"%s\t%4.2f",tstr,FutureVelocity[i]);
									//}
									//fprintf(tinst->FutureLogfptr, "%s%s\n",LogStr,tstr);
									fprintf(tinst->FutureLogfptr, "%s\n",LogStr);

									//log thrust and Segment Accleration
									sprintf(LogStr,"%d\t%d\tAccel\tThrust=%d (%d)\t%d\t\t\t\t%4.2f",ltask->TaskTime,tinst->InstTime, lemi->Thrust,WithGravity,AccelerationInCorrectDirection,Acceleration);  //move Accel over 2 column
									fprintf(tinst->FutureLogfptr, "%s\n",LogStr);
								} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {


								if (leai->WaitAndSeeTime>0) {
									leai->WaitAndSeeTime--;
									if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tReached Target Time=%d ms\n",ltask->TaskTime,tinst->InstTime,leai->WaitAndSeeTime*10);
									} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
								} //if (leai->WaitAndSeeTime>0) {


								//START DECREASING OSCILLATION CODE:  Check for decreasing oscillation
								if (Velocity>leai->LastVelocity) {
									leai->CurrentDirection=1; //Velocity moving in positive direction
								} else {
									leai->CurrentDirection=-1; //Velocity moving in negative direction
								} 
								if (leai->SampleCount==0) {
									//first sample since WaitAndSeeTime
									leai->LastDirection=leai->CurrentDirection;
									leai->SampleCount++;
								} else {
									//not first sample since WaitAndSeeTime
									if (leai->CurrentDirection!=leai->LastDirection) {
										//there has been a change in direction
										if (leai->LastDirection==1) {
											//last velocity was in positive direction, so found positive maximum
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: PeakPlus Vel=%4.2f Pk#%d\n",ltask->TaskTime,tinst->InstTime,leai->LastVelocity,leai->NumPeaks);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

											leai->LastPeakPlus=leai->PeakPlus; //save last PeakPlus
											leai->PeakPlus=leai->LastVelocity;
										} else { //if (leai->LastDirection==1) {
											//last velocity was in negative direction, so found negative minimum
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: PeakMinus Vel=%4.2f Pk#%d\n",ltask->TaskTime,tinst->InstTime,leai->LastVelocity,leai->NumPeaks);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

											leai->LastPeakMinus=leai->PeakMinus; //save last PeakMinus
											leai->PeakMinus=leai->LastVelocity;
										} // if (leai->LastDirection==1) {
										leai->NumPeaks++;
										if (leai->NumPeaks==1) { //first peak found
											leai->SampleCount=1; //set SampleCount to start of cycle
										} //if (leai->NumPeaks==1) {
										if (leai->NumPeaks==2) { //cycle is complete
											leai->SampleCount++;
											leai->NumPeriod++; 
											if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: Found Period #%d SampleCount=%d\n",ltask->TaskTime,tinst->InstTime,leai->NumPeriod,leai->SampleCount);
											} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

											if (leai->NumPeriod==2) { //there has been an earlier period
												//If the period of this cycle is less, the PeakPlus less, and PeakMinus more than the last cycle, then this is a decreasing oscillation
												if (leai->SampleCount<=leai->Period) {
													//SampleCount of this period is less than or equal to the last
													//if last peak was plus and PeakPlus<=LastPeakPlus or last peak was minus and PeakMinus>=LastPeakMinus, then this is a decreasing oscillation.
													if ((leai->LastDirection==1 && leai->PeakPlus<=leai->LastPeakPlus) ||
															(leai->LastDirection==-1 && leai->PeakMinus>=leai->LastPeakMinus)) {
															//this Segment is experiencing a decreasing oscillation so set that flag to keep thrust=0 and wait for the motion to stop
														leai->flags|=ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION;
														if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
															fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: *Found Dec period=%d ms*\n",ltask->TaskTime,tinst->InstTime,leai->SampleCount*10);
														} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													} else { //((leai->LastDirection==1 ...
														//otherwise this is definitely not a decreasing oscillation and make sure that the DECREASING_OSCILLATION flag is not set																			
														leai->flags&=~ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION;
														if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
															fprintf(tinst->FutureLogfptr,"%d\t\t\tDEC_OSC: Found period with <SampleCount but >peak\n",tinst->InstTime);  //shifted oscillation
														} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													} //((leai->LastDirection==1 ...															
												} else { //if (leai->SampleCount<=leai->Period)
													//otherwise this is definitely not a decreasing oscillation and make sure that the DECREASING_OSCILLATION flag is not set																			
													leai->flags&=~ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION;
													if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
														fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: Found Inc period\n",ltask->TaskTime,tinst->InstTime);
													} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {

												} //if (leai->SampleCount<=leai->Period)
												//set NumPeriod back to 1 because we found a period, and to use to compare to any next period
												leai->NumPeriod=1;																		
											} //if (leai->NumPeriod==2) {
											leai->Period=leai->SampleCount; //store the SampleCount of this period																		 
											leai->SampleCount=1;  //set SampleCount=1 since there is always one remaining sample needed to detect a peak, and to prepare for next possible period
											leai->NumPeaks=1;  //set NumPeaks=1 to start looking for the next peak (only one more is needed, since the last peak can be used)
										} //if (leai->NumPeaks==2) {
										leai->LastDirection=leai->CurrentDirection; //save the direction of Velocity
									} else { //if (leai->CurrentDirection!=leai->LastDirection) {
										//CurrentDirection is the same as LastDirection
										leai->SampleCount++;
										if (leai->NumPeriod>0) { 
											//we have found at least one oscillating period (a plus and minus peak)
											//if the SampleCount>Period then we know this cannot be a decreasing oscillation
											//it means that there was an oscillation, but now the body segment is moving beyond that initial oscillation range. So reset the DECREASING_OSCILLATION flag so that THRUST_ANALYSIS can continue to strive toward a TargetAngle or to OPPOSE_MOTION.									
											if (leai->SampleCount>leai->Period && (leai->flags&ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION)) {	
												//Current SampleCount in same direction is larger than period of last oscillation
												//this cannot be a decreasing oscillation so clear that flag. 							
												leai->flags&=~ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION;
												if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
													fprintf(tinst->FutureLogfptr,"%d\t%d\t\t\tDEC_OSC: SampleCount %d>%d\n",ltask->TaskTime,tinst->InstTime,leai->SampleCount,leai->Period);
												} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
												leai->NumPeriod=0; //reset NumPeriod, we want to start fresh with a new period
												leai->NumPeaks=0; 
											} //if (leai->SampleCount>leai->Period ...
										} //if (leai->NumPeriod>0) {
									} //if (leai->CurrentDirection!=leai->LastDirection) {
								} //if (leai->SampleCount==0) {
								//END DECREASING OSCILLATION CODE:  Check for decreasing oscillation


							} //if ((leai->WaitAndSeeTime>0) || (leai->flags&ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION)) {






//END: SEE IF SEGMENT ANGLE IS IN TARGET ANGLE RANGE, AND SET MOTOR THRUST


							//clear the robot instruction start flag - done everytime even though only needs to be done once, because is faster just to clear it than to check to see if it needs to be cleared.
							//cannot easily be done above after an angle reached or out of range, because inst could have 2 accels, and changing between would then clear the flag for the second accel.
							//tinst->flags&=~ROBOT_INSTRUCTION_INSTRUCTION_START;
							//tinst->flags&=~ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST; //clear flag, so threust analysis can proceed after this
							//update the LastUseAngle to the current UseAngle
							leai->LastUseAngle=UseAngle;  //update LastUseAngle. LastUseAngle needs to be in instruction struct, because more than one inst can use the 1 motor 2 accels arrangement.
				
							tinst->flags&=~(ROBOT_INSTRUCTION_INSTRUCTION_START|ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST);
	
							//if InRange determine if wait stage is done
							if (InRange) {
								//Note that not thrusting when a segment is moving against gravity should only apply to the X dimension, and the Z dimension of the torso. Thinking more about this- I decided to leave the against gravity thrust=0 code for the leg Z too, since the principle still somewhat applies.
//Mostly this principle applies to the lower legs and torso X and Z
//												if ((IsMoving==0 || (MovingAgainstGravity && (leai->xyz[UseAngle]==0 || (leai->xyz[UseAngle]==2 && leai->AccelNameNum[UseAngle]==ACCEL_NAME_TORSO)))) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
								//if ((IsMoving==0 || (MovingAgainstGravity && leai->xyz[UseAngle]!=1)) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
								//if (IsMoving==0  && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
								if (IsMoving==0  && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD)))) {
								//if ((IsMoving==0 || (MovingAgainstGravity && leai->xyz[UseAngle]!=1 && !VelocityInCorrectDirection)) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
								//if ((IsMoving==0 || (MovingAgainstGravity && leai->xyz[UseAngle]!=1 && !((lemi->MotorNameNum[0]==MOTOR_LEFT_FOOT || lemi->MotorNameNum[0]==MOTOR_RIGHT_FOOT || lemi->MotorNameNum[0]==MOTOR_LEFT_ANKLE || lemi->MotorNameNum[0]==MOTOR_RIGHT_ANKLE) && VelocityInCorrectDirection))) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
//												if (IsMoving==0 && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
									if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
										//if (IsMoving==0) {
											//Segment is not moving and is HOLD_ANGLE so add InstTime so there are no lapses in FutureLog
										sprintf(LogStr,"%d\t%d\t\tAng=%4.2f Vel=%4.2f Accel=%4.2f T=%d\t\tThrust=0: In Range and not moving (range= %4.1f to %4.1f)",ltask->TaskTime,tinst->InstTime, SegmentAngle,Velocity,Acceleration,lemi->Thrust,leai->Angle[1],leai->Angle[2]);
										//} else {
										//	sprintf(LogStr,"%d\t%d\t\tAng=%4.2f Vel=%4.2f Accel=%4.2f T=%d\t\tIn Range and moving against gravity (Thrust=0) %4.1f %4.1f",ltask->TaskTime,tinst->InstTime, SegmentAngle,Velocity,Acceleration,lemi->Thrust,leai->Angle[1],leai->Angle[2]);
										//} 
										fprintf(tinst->FutureLogfptr, "%s\n",LogStr);
									} //if (tinst->flags&ROBOT_INSTRUCTION_FUTURE_LOG) {
									//there are cases where TargetAngle is InRange and IsMoving==0 where Thrust!=0, ex: Velocity is slow and so TargetIndex is >3, so Thrust will not get set to 0 by motion code, so set Thrust=0 here.
									//The alternative is to keep thrusting towards the target, but for now, I think only thrust when InRange, when IsMoving. THe important thing is to not start motion.
									if (lemi->Thrust!=0 || lemi->LastThrust!=0) {
										lemi->Thrust=0;//-lemi->Thrust; //brake
										TurnMotor(lemi->mac[0],lemi->MotorNum[0],lemi->Thrust,10,MOTOR_QUEUE);  //stop motor/brake for 10 ms	
										if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
											TurnMotor(lemi->mac[1],lemi->MotorNum[1],lemi->Thrust,10,MOTOR_QUEUE);  //stop second motor/brake for 10 ms	
										} 
										lemi->LastThrust=lemi->Thrust; //update the LastSpeed so below code will not send TurnMotor() again
									} //if (lemi->Thrust!=0 || lemi->LastThrust!=0) {																	


								} //if (IsMoving==0 ...

								last=tlist;  //preserve the current top of the instruction list - for below code to delete it if not HoldAngleWithMotor

								//for stages with only HOLD_ANGLE insts, (does not have HAS_END_AT_TARGET), the stage can only time out
								//and because the WAIT_STAGE_DONE bit only gets cleared when out of range, clear it here to keep the stage going until all stage insts timeout. 
								//if ((tinst->flags&ROBOT_INSTRUCTION_WAIT_STAGE) && !(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST) && (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)) {
								if ((tinst->flags&ROBOT_INSTRUCTION_WAIT_STAGE) && !(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST) && (!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD)))) {
									ltask->flags&=~ROBOT_TASK_WAIT_STAGE_DONE; //so clear the stage done bit set above
								} 


								//if a MotorAccel inst with no HOLD_ANGLE flag set reached target, sent TurnMotor with 0 thrust if necessary. Note that there can be other insts with no HOLD_ANGLE flag set, waiting to get InRange, so we cannot just wait to stop motor before deleting the instruction in the below code after the stage ends. Or if ROBOT_INSTRUCTION_CONSTANT_THRUST is set, stop thrust here because the thrust analysis section is skipped and the motor would not ever stop. ROBOT_INSTRUCTION_CONSTANT_THRUST is just for testing a constant unchanging thrust.


								//if (((tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)==0) || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
								if (((!(tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD)))==0) || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
									//InRange but not a HOLD_ANGLE instruction

									//because CONSTANT_THRUST can get here numerous times, only issue TurnMotor(0) if Thrust!=0 and LastThrust!=0
									//unless the inst has a MAINTAIN_THRUST flag, set thrust=0
									if (!(tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST)) {
										if (lemi->Thrust!=0 || lemi->LastThrust!=0) {
											lemi->Thrust=0;//-lemi->Thrust; //brake
											TurnMotor(lemi->mac[0],lemi->MotorNum[0],lemi->Thrust,10,MOTOR_QUEUE);  //stop motor/brake for 10 ms	
											if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
												TurnMotor(lemi->mac[1],lemi->MotorNum[1],lemi->Thrust,10,MOTOR_QUEUE);  //stop second motor/brake for 10 ms	
											} 
											lemi->LastThrust=lemi->Thrust; //update the LastSpeed so below code will not resend TurnMotor()
										} //if (lemi->Thrust!=0 || lemi->LastThrust!=0) {																	
									} //if (!(tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST)) {
		
									//Delete the inst here because there could be other MotorAccel insts with no HOLD_ANGLE flag that are still not in range, and this inst would continue to be evaluated if not deleted.
									//an alternative is to just mark it as complete or know to ignore if no HOLD_ANGLE flag set in above analysis code.
									//only delete instruction if is not HOLD_ANGLE (not CONSTANT_THRUST)
									//if ((tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)==0) {
									if (tinst->flags&(ROBOT_INSTRUCTION_REQUIRED|ROBOT_INSTRUCTION_NO_HOLD)) {
										//fprintf(stderr,"Stage %i Inst w/o HOLD_ANGLE in range\n",tinst->OrigStartTime);

										if (!(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST)) {
											//only delete an inst with no HOLD_ANGLE here if it does not have the WAIT_STAGE flag set- because if WAIT_STAGE is set, it will be deleted below								
											if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
												sprintf(LogStr,"Delete inst (hold all angles) stage=%d LineNum=%d ost=%d inum=%d motnum=%d",ltask->StageNumber,tinst->LineNum,tinst->OrigStartTime,tinst->InstNum,lemi->MotorNum[0]); 
												//LogRobotModelData(LogStr);
											} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

											DeleteCondInstruction(&ltask->iCond,tinst); 
										} //if (!(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST)) {
										//do not go to next instruction for an inst that has no HOLD_ANGLE, because below code processes an inst that has HOLD_ANGLE
										//tlist=last->next; //go to next instruction with a condition
										tlist=last; //below next will move to next instruction with a condition
									} //if ((tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)==0) {
								}  //if (((tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE)==0) || (tinst->flags&ROBOT_INSTRUCTION_CONSTANT_THRUST)) {
								

							} //if (InRange) {

						} //else if (!IsMoving && (tinst->flagNoMotionNoThrust&(1<<UseAngle))) {

						//need to clear the ROBOT_TASK_WAIT_STAGE_DONE flag here if any REQUIRED inst last accels are InRange, (a stage timing out will not reach this code)
						//for an inst without HOLD_ANGLE but with multiple angles (NumAngles>0) the last accel angle needs to be in range (all earlier angles are in range)
						if (((tinst->flags&ROBOT_INSTRUCTION_REQUIRED) || !(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST)) && 
									(tinst->flags&ROBOT_INSTRUCTION_WAIT_STAGE)) {
							if (!InRange || !(tinst->flagAngleInRange&tinst->flagAccelEndsStage)) {
									ltask->flags&=~ROBOT_TASK_WAIT_STAGE_DONE; //so clear the stage done bit set above
									//otherwise if required inst is InRange and is the last accel in the inst, the stage done flag will remain set
									//this way more than one inst can be required before the stage will end.
							} //if (!InRange || !(tinst->flagAngleInRange&tinst->flagAccelEndsStage)) {
						} //if (((tinst->flags&ROBOT_INSTRUCTION_REQUIRED) || !(ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST)) && 
									//(tinst->flags&ROBOT_INSTRUCTION_WAIT_STAGE)) {




						//PERFORM ACTUAL TURNMOTOR COMMAND
						//see if we need to issue a TurnMotor and then which direction that should be in
						//if the condition had been met before this, or the speed has changed, issue a new TurnMotor command
						//lemi->Thrust is set above

						//first check if an inst changed the MaxThrust and Thrust Analysis made no change to thrust (but only if not constant thrust because user may not set MaxThrust when using CONSTANT_THRUST flag- so default {is low usually, for example, around 5} would be used):
						if (!(tinst->flags&(ROBOT_INSTRUCTION_CONSTANT_THRUST|ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS)) && abs(lemi->Thrust)>abs(lemi->MaxThrust[UseAngle])) {
							//MaxThrust changed and is being exceeded, so set to Thrust to the MaxThrust:
							if (lemi->Thrust<0) {
								lemi->Thrust=-lemi->MaxThrust[UseAngle];
							} else {
								lemi->Thrust=lemi->MaxThrust[UseAngle];
							}
						} //if (abs(lemi->Thrust)>abs(lemi->MaxThrust[UseAngle])) {

						//if ((leai->flags&ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE) || (leai->LastThrust!=lemi->Thrust)) {
						//If thrust has changed (or this is the start of a new inst w/o last inst having MAINTAIN_THRUST flag set), send TurnMotor inst to EthMotors PCB
						//or if the current motor thrust is greater than the MaxThrust as a result of an instruction changing MaxThrust (tinst->flagInstSetMaxThrust&(1<<j)). Is done here, because not sure what Thrust Analysis will do if anything before this. If it sets a thrust, then Inc or Dec Thrust funcs will check MaxThrust, but if not checking against MaxThrust needs to be done here.
#if 0 
						if (lemi->LastThrust!=lemi->Thrust || 
							(((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0) && 
							 (!(RStatus.MotorInfo[MotorNameNum].flags&MOTOR_INFO_TABLE_MAINTAIN_THRUST)))) {
#endif
						if (lemi->LastThrust!=lemi->Thrust || 
							!(leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)) {
																				
							TurnMotor(lemi->mac[0],lemi->MotorNum[0],lemi->Thrust,ltask->StartTime+tinst->StartTime+lemi->Duration-RobotCurrentTime,MOTOR_QUEUE);
							if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
								//change thrust magnitude if opposite IncAngDir
								if (leai->IncAngDir[UseAngle][0]!=leai->IncAngDir[UseAngle][1]) {
									TurnMotor(lemi->mac[1],lemi->MotorNum[1],-lemi->Thrust,ltask->StartTime+tinst->StartTime+lemi->Duration-RobotCurrentTime,MOTOR_QUEUE); //opposite dir 
								} else {
									TurnMotor(lemi->mac[1],lemi->MotorNum[1],lemi->Thrust,ltask->StartTime+tinst->StartTime+lemi->Duration-RobotCurrentTime,MOTOR_QUEUE);  //same dir
								} 	
							} //if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
							//set initial thrust flag (first thrust of instruction), so thrust analysis code will start to analyze motion.
							//if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0) {
							//	leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; 
							//} 
							lemi->LastThrust=lemi->Thrust; //update the LastThrust
							lemi->ThrustCount=0; //reset ThrustCount (how many samples (10ms) we have been thrusting)
						} else { //if (leai->LastThrust!=lemi->Thrust) {
							lemi->ThrustCount++; //increase ThrustCount- increases even if Thrust=LastThrust=0
							//fprintf(stderr,"TC=%d\n",leai->ThrustCount);
						} //if (lemi->LastThrust!=lemi->Thrust) {


						//moved this code out of the above if LastThrust!=Thrust because SENT_INITIAL_THRUST for HOLD_ANGLE insts
						//needs to be set, or else thrust analysis does not occur
						//set initial thrust flag (first thrust of instruction), so thrust analysis code will start to analyze motion.
						if ((leai->flags&ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST)==0) {
							leai->flags|=ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST; //***this is the only place this flag should get set (except above when an angle falls into range - todo: verify that instance is needed)
						}  

					} //elseif (AbortScript)
				} //elseif (tinst->InstNum==ROBOT_INSTRUCTION_ANALOG_SENSOR) {


				//fprintf(stderr,"next inst with cond\n");
				if (tlist!=0) {
					//fprintf(stderr,"15 ");
					tlist=tlist->next; //go to next instruction with a condition
				}

				

			} //if (!AbortScript && !SameTimeStamp) {


		} //ifelse if (RobotCurrentTime > ltask->StartTime + tinst->StartTime + tinst->EMInst.Duration) {  


		//Abort from Condition Instruction while
		if (ltask->flags&ROBOT_TASK_ABORT_TASK) {
			fprintf(stderr,"Instruction aborted script.\n"); 

			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				sprintf(LogStr,"Instruction aborted script."); 
				LogRobotModelData(LogStr);
			} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

			if (ltask) {
				AbortTask(&ltask);
				ltask->flags&=~ROBOT_TASK_ABORT_TASK;
				ltask=0;  //task is done after AbortTask(), so clear local pointer
				tlist=0; //abort the list of instructions waiting for a condition				
			} //if (ltask)
		} //if (ltask->flags&ROBOT_TASK_ABORT_TASK) {


		//if (AbortScript || (leai->flags&ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING)) {
		//	leai->flags&=~ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING;  //clear in case script is run again
		//} //if (AbortScript || (leai->flags&ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING)) {



	} //while(tlist!=0) { 						
	//end going through list of all instructions that still are waiting for a condition to be met


//fprintf(stderr,"16 ");
#if 0 
	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		sprintf(LogStr,"END COND INSTS");  //Indiate an end of processing conditional instructions- used to make sure loop is finishing within 10ms everytime. 
		LogRobotModelData(LogStr);
	} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
#endif

	//Done going through conditional instructions, for an inst or inst group with WAIT_STAGE, see if this stage is complete
//						if (ltask!=0 && ltask->iCond && !(ltask->flags&ROBOT_TASK_WAIT_STAGE)) {
	//if (ltask!=0 && !(ltask->flags&ROBOT_TASK_WAIT_STAGE)) {
	if (ltask!=0 && (ltask->flags&(ROBOT_TASK_WAIT_STAGE_DONE|ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE))) {
		//End of stage
		if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			if (ltask->flags&ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE) {
				sprintf(LogStr,"END STAGE %d (touch)",ltask->StageNumber);
			} else {
				sprintf(LogStr,"END STAGE %d",ltask->StageNumber);
			}  
			LogRobotModelData(LogStr);
		} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		ltask->flags&=~ROBOT_TASK_WAIT_STAGE_DONE; //clear flag for next possible WAIT_STAGE
		ltask->flags&=~ROBOT_TASK_WAIT_STAGE; //clear WAIT_STAGE flag for above while (tinst!=0 && ltask!=0 && ((!(ltask->flags&ROBOT_TASK_WAIT_STAGE) ...
		ltask->flags&=~ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE;  //clear ROBOT_TASK_ANALOG_SENSOR_ENDED_STAGE flag
		ltask->flags&=~ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST; //clear (if later stage has only HOLD_ANGLE, not clearing would not keep them running until timeout)
		//fprintf(stderr,"END STAGE %d\n",ltask->StageNumber);

		//go through all conditional instructions (of this stage), stop their motors and delete them (unless LAST_STAGE>CurrentStage, the inst will continue on to the next stage)
		//if (ltask->iCond!=0 && (ltask->flags&ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST)) {
		if (ltask->iCond!=0) {

			//go through all instructions in the condition list one more time
			//and turn off the motors unless LastStage>CurrentStage
			tlist=ltask->iCond; //go to start of list of instructions waiting for a condition
			last=tlist;  //preserve the current top of the instruction list - for below code to delete it if necessary
			while(tlist!=0) {							
				tinst=tlist->Inst; //shorthand for instruction  
				//if no LAST_STAGE flag or the LastStage is less than or equal to the current stage, stop the motor and delete the inst. Otherwise the inst continues to the next stage.  
				if (!(tinst->flags&ROBOT_INSTRUCTION_LAST_STAGE) ||  (tinst->LastStage<=ltask->StageNumber)) {
					//make sure associated motor(s) are issued Thrust=0 inst for 10ms to make sure the motors are off before deleting any instruction


					lemi = (EthMotorsInstruction *)&tinst->EMInst;
					//fprintf(stderr,"G1 ");
					if (tinst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR) { //AnalogSensor inst has no EthMotors		
						//fprintf(stderr,"G2 ");								
						//if (tinst->flags&ROBOT_INSTRUCTION_HOLD_ANGLE) {
						if (tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST) {  //user set MAINTAIN_THRUST (note that all motors are set to 0 after last stage)
							//store the current Thrust for the next instruction that turns this motor
							//set LastThrust in MotorInfo table (needed for inst after MAINTAIN_THRUST flag to know to set initial thrust to last thrust)
							//fprintf(stderr,"G3 ");
							if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
								sprintf(LogStr,"MAINTAIN_THRUST %s",RStatus.MotorInfo[MotorNameNum].Name); 
								LogRobotModelData(LogStr);
							} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
							MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[0], lemi->MotorNum[0]);
							//if angle is reached Thrust=0
							RStatus.MotorInfo[MotorNameNum].LastThrust=lemi->Thrust;
							lemi->LastThrust=lemi->Thrust; //set here
							RStatus.MotorInfo[MotorNameNum].flags|=MOTOR_INFO_TABLE_MAINTAIN_THRUST;
							if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
								MotorNameNum=GetMotorNameNumByPCBNum(lemi->PCBNum[1], lemi->MotorNum[1]);
								//fprintf(stderr,"G4\n");
								RStatus.MotorInfo[MotorNameNum].LastThrust=lemi->Thrust;
								RStatus.MotorInfo[MotorNameNum].flags|=MOTOR_INFO_TABLE_MAINTAIN_THRUST;
								lemi->LastThrust=lemi->Thrust; //set here- perhaps should be LastThrust[1]
							} //if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS)
						} else { //if (tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST)
							//stop the motor
							if (lemi->Thrust != 0 || lemi->LastThrust != 0) {
								//motor is still turning
								//fprintf(stderr,"bottom delete\n");
								TurnMotor(lemi->mac[0], lemi->MotorNum[0], 0, 10,MOTOR_QUEUE); //stop motor
								if (tinst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) { //inst has two motors
									TurnMotor(lemi->mac[1], lemi->MotorNum[1], 0, 10,MOTOR_QUEUE); //stop motor
								}
								lemi->Thrust=0; 
								lemi->LastThrust=0;
							} //if (lemi->Thrust != 0 || lemi->LastThrust != 0) {
						} //if (tinst->flags&ROBOT_INSTRUCTION_MAINTAIN_THRUST) {
					} //if (tinst->InstNum!=ROBOT_INSTRUCTION_ANALOG_SENSOR) {
					//Delete each instruction in the stage
					//fprintf(stderr,"Delete inst ost=%d num=%d\n",tinst->OrigStartTime,tinst->InstNum);
					if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						sprintf(LogStr,"Delete inst stage=%d LineNum=%d ost=%d inum=%d motnum=%d",ltask->StageNumber,tinst->LineNum,tinst->OrigStartTime,tinst->InstNum,lemi->MotorNum[0]); 
						//LogRobotModelData(LogStr);
					} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					DeleteCondInstruction(&ltask->iCond,tinst); 
					//DeleteCondInstruction(&ltask->iCond,last->Inst); 

				} else { //if (!(tinst->flags&ROBOT_INSTRUCTION_LAST_STAGE) || (tinst->LastStage<=ltask->StageNumber)) {
					//inst has LAST_STAGE flag and inst LastStage is > this stage StageNumber
					fprintf(stderr,"\nError: code should never get here tinst->flags=%u\n\n",tinst->flags);

				} //if (!(tinst->flags&ROBOT_INSTRUCTION_LAST_STAGE) ||  (tinst->LastStage<=ltask->StageNumber)) {

				tlist=tlist->next; //go to next instruction with a condition
			} //while(tlist!=0) {

			//also reset the STAGE_HAS_END_AT_TARGET flag
			ltask->flags&=~ROBOT_TASK_WAIT_STAGE_HAS_REQUIRED_INST;
		} //if (ltask->iCond!=0) {
	} //if (ltask!=0 && (ltask->flags&ROBOT_TASK_WAIT_STAGE_DONE)) {

	if (ltask && ltask->iCond==0 && ltask->CurrentRobotInst==0) { 
		fprintf(stderr,"Done with all conditional instructions\n");
		//if no more conditional instructions remain and no more unexecuted script instructions remain
		//then this RobotTask is done
		//Stop all motors
		StopAllMotors();
		RemoveRobotTask(ltask);
		FreeRobotTask(ltask);
		ltask=0;  //for ltask=ltask->next below
		//close LogFile if still open (user forgot to add CloseLogFile() at end of script)
		if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			//fprintf(stderr,"no more conditional tasks CloseRobotLogFile()\n");
			CloseRobotModelLogFile();
		} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {

	} //if (ltask && ltask->iCond==0 && ltask->CurrentRobotInst==0) {
	//fprintf(stderr,"17 ");				

	//end if any instructions with a conditions exist

} //ProcessCondInsts(RobotTask *ltask)
