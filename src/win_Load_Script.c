//win_Load_Script.c
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
#include "win_Load_Script.h"
#include "Free3D.h" //for GetFileNameFromPath
#include "robot_motor_pic_instructions.h" //for ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE
#if Linux
#include <strings.h>
#endif


extern ProgramStatus PStatus; //for info messages
extern RobotStatus RStatus; //for robot settings

//is defined here because there is a gcc warning if in win_Load_Script.h 
const char *ETHACCEL_ACQUISITION_KINDS[NUM_ETHACCEL_ACQUISITION_KINDS] = { "SINGLE_SAMPLE","POLLING","ABSOLUTE_INTERRUPT","RELATIVE_INTERRUPT" };

const char *RobotInstructionNames[NUM_ROBOT_INSTRUCTIONS] = { "StartLogging","LogScriptFileName","StopLogging","StartAccelerometer","StopAccelerometer",
"StartAnalogSensor","StopAnalogSensor","TurnMotor","MotorAngle","StoreAngle","StartLoop","EndLoop","AnalogSensor","StartBalancing","StopBalancing"};

int winLoadScript_AddFTWindow(void)
{
FTWindow twin,*twin2;
int newx,newy;

twin2=GetFTWindow("winLoadScript");
if (twin2!=0) {
	newx=twin2->x+300;
	newy=twin2->y+20;
} else {
	newx=300;
	newy=100;
}

memset(&twin,0,sizeof(FTWindow));
strcpy(twin.name,"winLoadScript");
strcpy(twin.title,"Load Script");
strcpy(twin.ititle,"Load Script");
twin.x=newx;
twin.y=newy;
twin.w=500;
twin.h=370;
twin.OnOpen=(FTControlfuncw *)winLoadScript_OnOpen;
twin.AddFTControls=(FTControlfuncw *)winLoadScript_AddFTControls;


//change dir to scripts folder
chdir(RStatus.CurrentRobotScriptFolder);

//fprintf(stderr,"create window main_CreateFTWindow\n");
CreateFTWindow(&twin);
twin2=GetFTWindow(twin.name);
DrawFTWindow(twin2);

//fprintf(stderr,"end main_CreateFTWindow\n");
return 1;
} //end winLoadScript_AddFTWindow


int winLoadScript_AddFTControls(FTWindow *twin)
{
FTControl *tcontrol,*tcontrol2;
//FTItem titem;
int fh,fw;


if (PStatus.flags&PInfo) {
	if (!twin) {
	  fprintf(stderr,"Adding controls for window '%s'\n",twin->name);
	} else {
	  fprintf(stderr,"Error: twin==0 in winLoadScript_AddFTControls()\n");
		return(0);
	}
}

fw=twin->fontwidth;
fh=twin->fontheight;

tcontrol=(FTControl *)malloc(sizeof(FTControl));

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadScriptPath");
tcontrol->type=CTTextBox;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->x2=fw*60;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowX2;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblLoadScriptName");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*10;
tcontrol->flags=CGrowY1|CGrowY2;
strcpy(tcontrol->text,"filename:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtLoadScriptName");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*11;
tcontrol->y1=fh*21;
tcontrol->x2=fw*60;
//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
//tcontrol->flags=CGrowX1|CGrowY1|CGrowX2|CGrowY2;
tcontrol->flags=CGrowX2|CGrowY1|CGrowY2;
AddFTControl(twin,tcontrol);




memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"foLoadScript");
tcontrol->type=CTFileOpen;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
tcontrol->x2=fw*60;
tcontrol->y2=fh*20;
tcontrol->flags=CGrowX2|CGrowY2|CVScroll;//|CHScroll;
tcontrol2=GetFTControl("txtLoadScriptPath");
tcontrol->TextBoxPath=tcontrol2; //so the textbox control with the path will automatically get updated when the file open control path changes
tcontrol2=GetFTControl("txtLoadScriptName");
tcontrol->TextBoxFile=tcontrol2; //so this control will automatically get updated with the currently selected filename in the file open control
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadScript_OK");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"OK");
tcontrol->x1=fw*5;
tcontrol->y1=fh*23;
tcontrol->x2=fw*13;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_LoadScript_OK_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnLoadScript_Cancel");
tcontrol->type=CTButton;
strcpy(tcontrol->text,"Cancel");
tcontrol->x1=fw*14;
tcontrol->y1=fh*23;
tcontrol->x2=fw*22;
tcontrol->y2=fh*23+32;
tcontrol->flags=CGrowY1|CGrowY2;
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_LoadScript_Cancel_Click;
AddFTControl(twin,tcontrol);



if (tcontrol!=0) {
	free(tcontrol);
}

if (PStatus.flags&PInfo) {
  fprintf(stderr,"Done adding FTControls for window %s\n",twin->name);
}

//#endif
return(1);
} //end winLoadScript_AddFTControls


int winLoadScript_OnOpen(FTWindow *twin)
{
	FTControl *tc;

	//update the list of loaded scripts
	tc = GetFTControl("ddScripts");
	UpdateScriptList(tc);

	//set the window focus to file open control for mousewheel to work scrolling
	tc = GetFTControl("foLoadScript");
	if (tc != 0) {
		FT_SetFocus(twin, tc);
		return(1);
	}
	return(0);
}


void btn_LoadScript_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
CloseFTWindow(twin);
}

unsigned int btn_LoadScript_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
	char *tstr,tstr2[FTMedStr],tstr3[FTMedStr],tmsg[FTMedStr];
	FTControl *tcontrol,*tcontrol2;//,*tc;
	RobotScript *lscript;
	int ExitLoop,MultipleFiles,cur,cur2,FoundError;


	//Load script(s) from file

	//get path from control
	tcontrol=GetFTControl("foLoadScript");
	tcontrol2=GetFTControl("txtLoadScriptName");

	//go through list of file names and load all scripts
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

		lscript=LoadRobotScriptFile(tstr3);
		if (lscript!=0) {
			//add this script to the global script list- done automatically in LoadScript


			if (!MultipleFiles) {
					sprintf(tmsg,"Script %s loaded",lscript->name);
					FTMessageBox(tmsg,FTMB_NONE,"Robot - Info",500);
			} //if (!MultipleFiles) {

		} else {//		if (lscript!=0) {
			FoundError=1;
			sprintf(tmsg,"Error loading script: %s",tstr3);
			FTMessageBox(tmsg,FTMB_NONE,"Robot - Info",500);	
		}//		if (lscript!=0) {
	} //while(!ExitLoop) {


	if (!FoundError) {
		if (MultipleFiles) {
			sprintf(tmsg,"Scripts loaded");
			FTMessageBox(tmsg,FTMB_NONE,"Robot - Info",500);
		} //if (MultipleFiles) {

/*
		//update the list of loaded scripts
		tc=GetFTControl("ddScripts");
		if (tc!=0) {
			UpdateScriptList(tc);
			//select the current script- only after updating it- otherwise ilist==0
			SelectFTItemByName(tc->ilist,lscript->name,0);
		}
*/
		CloseFTWindow(twin);
		return(1);
	} else {
		CloseFTWindow(twin);
		return(0);
	}

} //void btn_LoadScript_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)

RobotScript* LoadRobotScriptFile(char *filename) 
{
	FILE *fptr;
	char tstr[FTMedStr];
	RobotScript *lscript;
	int fSize,bSize,result,reload;
	char *buf;
	FTControl *tc;

	
	reload=0;
	//see if this script is already loaded
	lscript=GetRobotScript(GetFileNameFromPath(filename));
	if (lscript) {
		//if yes, unload the already loaded script and reload the new script
		UnloadRobotScript(lscript);
		lscript=0;
		reload=1;
	} 

	fptr=fopen(filename,"rb");
	if (fptr==0) {
		sprintf(tstr,"Could not open file '%s'\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		fprintf(stderr,"Could not open file '%s'\n",filename);
		return(0);
	} //fptr!=0


	// determine file size:
#if Linux
	fseeko(fptr,0,SEEK_END);
	fSize=ftello(fptr);
#endif
#if WIN32
	_fseeki64(fptr,0,SEEK_END);
	fSize=_ftelli64(fptr);
#endif
	rewind(fptr);

	// allocate memory to contain the whole file:
	buf=(char *)malloc(fSize+1);//+1 for buf[fSize]=0
	if (buf == NULL) {
		sprintf(tstr,"Allocating memory for script file failed.\n");
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		return(0);
	}

	//read in the entire file
	//copy the file into the buffer:
	result=fread(buf,1,fSize,fptr);
	if (result != fSize) {
		sprintf(tstr,"Error reading script file %s\n",filename);
		FTMessageBox(tstr,FTMB_OK,"Error",0);
		return(0);
	}

	fclose(fptr);
	buf[fSize]=0; //close string of buffer - so strlen will give correct size
	bSize=strlen(buf);


	//todo add a function to AddScriptText(char *script)
	//so that inline scripts can be added

	lscript=LoadRobotScript(GetFileNameFromPath(filename),buf,bSize);
	if (lscript!=0) {
		//store the filename (includes path)	
		strcpy(lscript->filename,filename);
		lscript->flags|=ROBOT_SCRIPT_IS_FROM_FILE;

		if (reload) {
			fprintf(stderr,"Script file %s reloaded\n",filename);
		} else {
			fprintf(stderr,"Script file %s loaded\n",filename);
		}

		//update the list of loaded scripts
		tc=GetFTControl("ddScripts");
		if (tc!=0) {
			UpdateScriptList(tc);
			//select the current script- only after updating it- otherwise ilist==0
			SelectFTItemByName(tc->ilist,lscript->name,0);
		}


	} //	if (lscript!=0) {


	free(buf);

	return(lscript);

} //RobotScript* LoadRobotScriptFile(char *filename) 



RobotScript* LoadRobotScript(char *name,char *buf,int bSize) 
{
	char tstr[FTMedStr],tstr2[FTMedStr];
	char tline[FTMedStr];
	RobotScript *lscript;
	RobotInstruction *linst;
	int j,k,k2,match,InstNum,SeqNum,FoundMatch,MatchedFlag,ExitLoop;
	char *param;
	int NumLines,CurLine,CurChar;
	FTControl *tc;
	//float BrakeDur; 
	//int AccelNameNum[2];//,MotorNameNum;
	MotorAngleAssociation *MotorAngAssoc;
	EthMotorsInstruction *lemi;
	EthAccelsInstruction *leai;
	EthAnalogSensorInstruction *leti;
	//float UserMaxVelocity,UserMinVelocity;
	int UserSetMaxVelocity[4],UserSetMinVelocity[4];
	//float UserMaxAccel;
	int UserSetMaxAccel[4];
	//float UserMinAccel;
	int UserSetMinAccel[4];
  char TVR[4][20],CTB[4][20];
	char InstText[1024];
	int InstLen;
	int TVRlen,CTBlen,NumAngle;
	unsigned int UseDefaultTVR[4];
	int GTorLT,AngleNum;
	float Value;

	//todo:
	//unload any scripts with the same name


	//go through each line and each word (separated by spaces or a comma)

	//determine total number of lines in order to allocate space for a pointer to each line for strtok
	NumLines=0;
	j=0;
	while(j<bSize) {
		if (buf[j]==10 || (buf[j]!=10 && j==bSize-1)) {
			NumLines++;
		}
		j++;
	}  //while(j<bSize)



	//initialize script
	lscript=0;

	//go through each line in the script file
	//Robot scripts are text files that can only have a comment (preceeded by #) or a Robot Motor instruction
	//Robot Motor instructions currently have the form:
	//Duration (in ms),instruction

	//When a script is loaded, each instruction is allocated memory and attached to a linked list of instructions
	//so remember to free this allocated memory before existing the Robot program. 
	CurLine=0;
	CurChar=0;
	SeqNum=0; //instruction sequence number
	while(CurLine<NumLines) { 
		//get a line from the file text
		GetLineFromText(tline,buf+CurChar);
		InstLen=strlen(tline);
		CurChar+=InstLen;
		if (InstLen>1) {
			memcpy(InstText,tline,InstLen-1);  //do not copy line feed
			InstText[InstLen-1]=0; //terminate string
		} 
		CurLine++;
		//get a word on this line
		param=strtok(tline,",\n"); //comma or \n delimited (for first read, but all later strtoks also have space and #)

		//either this is a comment (preceeded by a '#') or a start time
		if (param!=0) {
			if (param[0]!=35) { //# not comment

				//if this is the first instruction initialize the script and instruction list
				if (lscript==0) {
					lscript=malloc(sizeof(RobotScript));
					if (lscript==0) {
						sprintf(tstr,"Could not allocate memory for script\n");
						FTMessageBox(tstr,FTMB_OK,"Error",0);							
						return(0);
					} //if (lscript==0) {
					memset(lscript,0,sizeof(RobotScript));					
					//store the name of the script
					//strcpy(lscript->name,GetFileNameFromPath(filename));
					strcpy(lscript->name,name);
				} //if (lscript==0) {

				//add instruction
				if (lscript->iRobotInst==0) { //no instructions yet
					//allocate memory for the first instruction
					lscript->iRobotInst=malloc(sizeof(RobotInstruction));
					if (lscript->iRobotInst==0) {
						sprintf(tstr,"Could not allocate memory for script's Instruction list\n");
						FTMessageBox(tstr,FTMB_OK,"Error",0);		
						free(lscript);					
						return(0);
					} //if (lscript->iRobotInst==0) {
					linst=lscript->iRobotInst;
				} else { //if (lscript->iRobotInst==0)
					//there already is at least 1 instruction
					//allocate memory for the next instruction
					linst->next=malloc(sizeof(RobotInstruction));
					if (linst->next==0) {
						sprintf(tstr,"Could not allocate memory for instruction\n");
						FTMessageBox(tstr,FTMB_OK,"Error",0);		
						FreeRobotScript(lscript);
						return(0);
					} //if (linst->next==0) {
					linst=linst->next;
				} //if (lscript->iRobotInst==0) { //no instructions yet

				memset(linst,0,sizeof(RobotInstruction)); //clear instruction 

				//copy start time to instruction
				//StartTime can be "END" - run the instruction at the end of all other instructions
				linst->StartTime=atoi(param);
				linst->OrigStartTime=linst->StartTime;  //save for WAIT_STAGE insts whose StartTime can change

				lemi=(EthMotorsInstruction *)&linst->EMInst;
				leai=(EthAccelsInstruction *)&linst->EAInst;

				//add text of instruction to linst
				if (InstLen>1024) {
					memcpy(linst->InstText,InstText,1024);
					fprintf(stderr,"Trimmed Instruction text on line %d (to 1024 chars) for log:\n%s\n",CurLine,InstText);
					linst->InstText[1023]=0; //terminate string
				} else {
					memcpy(linst->InstText,InstText,InstLen);
				}


				//get the instruction name
				param=strtok(NULL,"(,\n #"); //(,comma,\n,space,or # delimited
				if (param==0) {
					sprintf(tstr,"No Instruction on line %d\n",CurLine);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					FreeRobotScript(lscript);
					return(0);
				} //if (param==0) {
		
				//switch on instruction name
				match=0;
				InstNum=0;
				while(match==0 && InstNum<NUM_ROBOT_INSTRUCTIONS) {
					if (!strcmp(param,RobotInstructionNames[InstNum])) {  
						//found a match
						match=1;
						linst->InstNum=InstNum; //save Instruction number
						linst->SeqNum=SeqNum+1; //save instruction sequence number	
						linst->LineNum=CurLine; //save Instruction Line Number
						SeqNum++; //and increment 
						//read in data for this section
						switch(InstNum) {
							case ROBOT_INSTRUCTION_START_LOGGING:
							case ROBOT_INSTRUCTION_LOG_SCRIPT_FILENAME:
							case ROBOT_INSTRUCTION_STOP_LOGGING:
								param=strtok(NULL,")\n");  //go to the next word or line
								//currently we can ignore the rest of the line								
							break;
							case ROBOT_INSTRUCTION_START_ACCELEROMETER: //StartAccel(PCBName,AccelMask,AcqKind)
							case ROBOT_INSTRUCTION_STOP_ACCELEROMETER:	//StopAccel(PCBName,AccelMask,AcqKind)
							case ROBOT_INSTRUCTION_START_ANALOG_SENSOR:  //StartAnalogSensor(PCBName,SensorMask,AcqKind)
							case ROBOT_INSTRUCTION_STOP_ANALOG_SENSOR:  //StopAnalogSensor(PCBName,SensorMask,AcqKind)
								//get EthAccel PCB name (can be ALL_ETH_ACCEL)
								param=strtok(NULL,",\n #)"); //comma,\n,space,#, or ) delimited
								if (param==0) {
									sprintf(tstr,"No EthAccels PCB name on line %d\n",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								leai->PCBNum[0]=GetPCBNameNum(param);
								if (leai->PCBNum[0]==-1) {
									if (strcmp(param,"ALL_ETHACCELS")) {  //for ALL_ETHACCELS leai->PCBNum[0] will = -1
										sprintf(tstr,"No known EthAccels PCB name %s on line %d\n",param,CurLine);
										FTMessageBox(tstr,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} 	
								}
								//strcpy(leai->EthAccelsPCB,param);
								
								//get the AccelMask (which accels on the PCB, could work for ALL_ETHACCELS- but probably don't want to- user should put 7 here for all 3- and should be in decimal - not hex currently)
								//to do- make hex
								param=strtok(NULL,",\n #)"); //comma,\n,space,#, or ) delimited
								if (param==0) {
									if (InstNum==ROBOT_INSTRUCTION_START_ACCELEROMETER || InstNum==ROBOT_INSTRUCTION_STOP_ACCELEROMETER ) {
										sprintf(tstr,"No Accel mask on line %d (1=accel 0, 2=accel 1, 3= accels 1 & 2, 7=all 3 accels, etc.)\n",CurLine);
									} else {
										sprintf(tstr,"No Touch sensor mask on line %d (in hex: 1=sensor 0, 2=sensor 1, 3= sensors 1 & 2, 7f=all 15 sensors, etc.)\n",CurLine);
									}
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								//leai->AccelMask=atoi(param);
								leai->AccelMask=(int)strtol(param,0,16);
								//fprintf(stderr,"Mask=%x\n",leai->AccelMask);
								

								//get the Acquisition Method (SINGLE_SAMPLE,RELATIVE_INTERRUPT,ABSOLUTE_INTERRUPT,POLLING)
								param=strtok(NULL,",\n #)"); //comma,\n,space,#, or ) delimited
								if (param==0) {
									sprintf(tstr,"No Acquisition method (RELATIVE_INTERRUPT,SINGLE_SAMPLE, etc.)given on line %d\n",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								leai->AcqKind=GetEthAccelAcqNum(param);
								if (leai->AcqKind==-1) {
									sprintf(tstr,"No known Acquisition method name %s on line %d\n",param,CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);	
								}

								//I decided to make set accel threshold a different instruction SetAccelThreshold
								//but AccelThreshold can be set in a single command to the PCB firmware

//							End:
//							case ROBOT_INSTRUCTION_START_ACCELEROMETER: //StartAccel(PCBName,AccelMask,AcqKind)
//							case ROBOT_INSTRUCTION_STOP_ACCELEROMETER:	//StopAccel(PCBName,AccelMask,AcqKind)
//							case ROBOT_INSTRUCTION_START_ANALOG_SENSOR:  //StartAnalogSensor(PCBName,SensorMask,AcqKind)
//							case ROBOT_INSTRUCTION_STOP_ANALOG_SENSOR:  //StopAnalogSensor(PCBName,SensorMask,AcqKind)

							break;
							case ROBOT_INSTRUCTION_TURN_MOTOR:
							case ROBOT_INSTRUCTION_MOTOR_ANGLE:
							//case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE:							
							//case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_RELATIVE_ANGLE:
//							case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE_WAIT:	

							//current format: TurnMotor(MOTOR_LEFT_SIDE,Thrust,Duration)
							//old format: TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,Thrust,Duration)

							//old format: TurnMotorUntilAngle(MOTOR_LEFT_SIDE,Thrust,Duration,ACCEL_LEFT_UPPER_LEG,Z,BETWEEN,3,-1,1,WAIT_STAGE)
							//old format: TurnMotorUntilRelativeAngle(MOTOR_LEFT_SIDE,Thrust,Duration,ACCEL_LEFT_UPPER_LEG,Z,ACCEL_TORSO,Z,BETWEEN,3,-1,1,WAIT_STAGE)
							//current format, option 1: 1 motor with 2 accels (if accel[0] in range accel[1] is used): MotorAngle(MOTOR_LEFT_SIDE,Thrust,Duration,ACCEL_LEFT_UPPER_LEG,Z,CurrentAngle,-1,1,ACCEL_TORSO,Z,3,-1,1,WAIT_STAGE|HOLD_ANGLE)
							//current format, option 2: 2 motors with 1 angle (accel or pot) (motors are turned with the same thrust with a single timeout- code knows to use IncAngDir for each motor to turn each motor correct CW or CCW): 		MotorAngle(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,Thrust,Duration,ACCEL_LEFT_UPPER_LEG,Z,3,-1,1,WAIT_STAGE|HOLD_ANGLE)
							//current format, option 3: 2 motors with 2 angles (motors are turned to first angle, if that angle is in range, motors are turned to second angle)

//can have up to 3 angles

//note that for a two-motor one-accel instruct with different constant or initial thrusts for each motor, the code needs to change UserThrust to an array[2], probably the model playback code would need to be changed too. Two motors with 1 accel but different constant thrusts can be done with 2 separate insts.

								//UserTVROverride=0;
								for(j=0;j<MAX_ANGLES_PER_INSTRUCTION;j++) {
									UseDefaultTVR[j]=0xf; //start presuming all accels will get default TVR values 0=CW 1=CCW 2=MinThrust
								} //for j
								//linst->flagAccelEndsStage=(1<<MAX_ANGLES_PER_INSTRUCTION)-1; //initially all possible Accels can end a stage, but accels and flags below can change these flag bits
								j=0;
								while (j>-1) {
									//get a word on this line Motor name
									param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
									if (param==0) {
										if (j==0) {
											sprintf(tstr,"No Motor Name on line %d\n",CurLine);
										} else {
											sprintf(tstr,"No Motor Name or Thrust on line %d\n",CurLine);
										} 
										FTMessageBox(tstr,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);

										return(0);
									} //if (param==0) {

									if (j==0 || (j==1 && !strncmp(param,"MOTOR",5))) {
										//Todo: get this all from MotorInfo and/or EthMotorsPCB structs

										//lemi->MotorNum=atoi(param);
										lemi->MotorNum[j]=GetMotorNum(param);
										if (lemi->MotorNum[j]==-1) {
											sprintf(tstr,"Motor name %s unrecognized on line %d\n",param,CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);
										} //if (lemi->MotorNum[j]==-1)

										//fprintf(stderr,"MotorNum[j]=%x ",lemi->MotorNum[j]);

										//from motor name # get EthMotors and mac
										lemi->MotorNameNum[j]=GetMotorNameNum(param);
										//fprintf(stderr,"MotorNameNum[j]=%d ",lemi->MotorNameNum[j]);

										//lemi->PCBNum[j]=GetPCBNameNum((char *)ROBOT_MOTOR_NAMES_PCB[lemi->MotorNameNum[j]]);								
										lemi->PCBNum[j]=RStatus.MotorInfo[lemi->MotorNameNum[j]].PCBNum;										
										//fprintf(stderr,"PCBNum=%d ",lemi->PCBNum);
										if (lemi->PCBNum[j]==-1) {
											sprintf(tstr,"No known EthMotors PCB associated with %s (%d) on line %d\n",param,lemi->MotorNum[j],CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);	
										}


										//get the MAC_Connection for ease in calling TurnMotor commands when running the script.
										//but note that this is done again when running the script because script may be loaded before EthAccels are powered on and identified.
										lemi->mac[j]=Get_MAC_Connection_By_PCBNum(lemi->PCBNum[j]);

										//fprintf(stderr,"Found motor %s\n",ROBOT_MOTOR_NAMES[lemi->MotorNameNum[j]]);

										if (j==1) {
											linst->flags|=ROBOT_INSTRUCTION_TWO_MOTORS;
											//fprintf(stderr,"found 2 motors\n");
											//get next word
											param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited	
										} //if (j==1)

									} //if (j==0 || (j==1 && !strncmp(param,"MOTOR",5))) {

									if (j==1) {
										j=-1; //exit while
									} else {
										j++; //see if second motor is given
									} //if (j==1) {

								} //while(j>-1)


								//get the next word on this line (Speed)
								//MotorSpeed (TurnMotor())  or TargetSpeed (TurnMotorUntil...())
								//param=strtok(NULL,",\n #)"); //comma,\n,space,#, or ) delimited
								if (param==0) {
									if (InstNum==ROBOT_INSTRUCTION_TURN_MOTOR) {
										sprintf(tstr,"No motor thrust level on line %d\n",CurLine);
									} else {
										//sprintf(tstr,"No target speed on line %d\n",CurLine);
										sprintf(tstr,"No motor thrust on line %d\n",CurLine);
									}
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {

								//User thrust - initial thrust of motor (higher results in faster motion), 0=use minimum thrust (balance,hold angle)
								lemi->UserThrust=atoi(param);	
								//make sure is not greater than max 
								for(j=0;j<1+((linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS)>0);j++) {
									/*if (lemi->UserThrust>RStatus.MotorInfo[lemi->MotorNameNum[j]].MaxThrustAG) {
										fprintf(stderr,"User thrust of %d too high, reduced to max thrust Against Gravity of %d\n",lemi->UserThrust,RStatus.MotorInfo[lemi->MotorNameNum[j]].MaxThrustAG);
										lemi->UserThrust=RStatus.MotorInfo[lemi->MotorNameNum[j]].MaxThrustAG;
									} */
/*
									if (lemi->UserThrust>16) {
										fprintf(stderr,"User thrust of %d too high, reduced to max thrust to 16 on line %d\n",lemi->UserThrust,CurLine);
										lemi->UserThrust=16;
									} 
*/
								} //for j

								//set default max motor thrust here - note for 2motors+1accel, MaxThrust is currently the same for both motors.
								//MotorNameNum=GetMotorNameNum(GetMotorName(lemi->PCBNum,lemi->MotorNum));
								lemi->MaxThrustAG=RStatus.MotorInfo[lemi->MotorNameNum[0]].MaxThrustAG;
								lemi->MaxThrustWG=RStatus.MotorInfo[lemi->MotorNameNum[0]].MaxThrustWG;



								if (InstNum==ROBOT_INSTRUCTION_TURN_MOTOR) {
									//Motor Speed (-7 to +7)
									lemi->Thrust=atoi(param);
									if (lemi->Thrust<-ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE) { //note is now +-20
											fprintf(stderr,"Warning: Motor speed %d is too large on line %d, trimming to -%d on line %d\n",lemi->Thrust,CurLine,ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE,CurLine);
											lemi->Thrust=-ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;
										} 
										if (lemi->Thrust>ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE) {
											fprintf(stderr,"Warning: Motor speed %d is too large on line %d, trimming to %d on line %d\n",lemi->Thrust,CurLine,ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE,CurLine);
											lemi->Thrust=ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;
										} 
										//note- currently TurnMotorUntilAngle uses the user speed- does not change the speed (but does change motor direction)								
										//leai->InitialSpeed=lemi->Thrust;  //save because software can overwrite Speed for motor moves with a condition
										//set Big speed based on user speed and small speed to 1
										//leai->BigSpeed=abs(lemi->Thrust);
										//leai->SmallSpeed=1;
								} else { //if (i==ROBOT_INSTRUCTION_TURN_MOTOR) {
#if 0 
									//Target Speed (in degrees/sec)
									leai->TargetSpeed=atof(param);								
									//we cannot correct the sign here because the current angle would need to be known- it is done later in win_Tasks if this script is run.
									//correct magnitude if necessary
									if (leai->TargetSpeed<-100.0) {
										fprintf(stderr,"Warning: Target speed %07.3f is too large on line %d, trimming to -100.0 degrees/sec\n",leai->TargetSpeed,CurLine);
										leai->TargetSpeed=-100.0;										
									}

									if (leai->TargetSpeed>100.0) {
										fprintf(stderr,"Warning: Target speed %07.3f is too large on line %d, trimming to 100.0 degrees/sec\n",leai->TargetSpeed,CurLine);
										leai->TargetSpeed=100.0;
									}
									//put TargetSpeed in 10ms (sample increments), so it can be compared with Change in degress at each sample interval
									//ex: 50 deg/s becomes 0.5 degrees of change/sample (0.5 degrees of change every 10ms)
									leai->TargetSpeed/=100; //todo: use SAMPLE_TIMER_INTERVAL
									//set when script starts running leai->TargetSpeed0=leai->TargetSpeed; //save original target speed because EAInst.TargetSpeed is reduced to 0 when braking

									//Calculate BrakeDecel from TargetSpeed
									//todo: BrakeDecel can be smaller when gravity is working with brake
									leai->BrakeDecel=leai->TargetSpeed*0.1; //10%  (make negative?)
									//Calculate BrakeAngle: the amount of angle (distance) needed to come to a complete stop.
									//Dist=1/2at^2
									//t=fabs(TargetSpeed0/BrakeDecel)  (ex: fabs(50degrees/s / -5 degrees/s/s) = 10s, 10s are needed to reach TargetSpeed0 (v=at, 50=5*10) - note because samples are 100hz (every 10ms), this time can be thought of as 10 samples - or 100ms.									

									BrakeDur=leai->TargetSpeed/leai->BrakeDecel;  //ex: 50/5= 10
									leai->BrakeAngleTotal=fabs(0.5*leai->BrakeDecel*BrakeDur*BrakeDur);  //S=1/2at^2 ex: 0.5*5*10^2=2.5 degrees
									//2.5 degrees needed to come to a complete stop. At each 10ms sample, TargetVelocity is changed by:
									//TargetVelocity=RemainingAngle/BrakeAngle)*TargetVelocity0  (RemainingAngle<=BrakeAngle) 
									//So currently change in velocity is linear (constant decceleration), while distance decreases exponentially.
#endif								
								} //if (InstNum==ROBOT_INSTRUCTION_TURN_MOTOR) {											
	

								//get the next word on this line (Duration) - note that for 2motors+1accel Duration is the same for both motors.
								param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
								if (param==0) {
									sprintf(tstr,"No motor duration/instruction timeout on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								lemi->Duration=atoi(param);
								if (lemi->Duration<0) {
									lemi->Duration=0;
								} 
								//linst->EndTime=linst->StartTime+lemi->Duration;



								//read in accelerometers
								if (InstNum == ROBOT_INSTRUCTION_MOTOR_ANGLE) {
									//add a "turn motor until angle" condition to this instruction

									param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
									if (param==0) {
										sprintf(tstr,"No Accelerometer or Potentiometer name on line %d",CurLine);
										FTMessageBox(tstr,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (param==0) {

									j=0;
									while(j>-1) {  //for each Angle determine if an acclerometer or potentiometer

										leai->AccelNameNum[j]=GetAccelNameNum(param);
										leai->AngleKind[j]=0;
										if (leai->AccelNameNum[j]==-1) {
											//see if this is an analog sensor (potentiometer)
											leai->AnalogSensorNameNum[j]=GetAnalogSensorNameNum(param);
											if (leai->AnalogSensorNameNum[j]==-1) {
												sprintf(tstr,"No known Accelerometer or Analog Sensor (Potentiometer) named '%s' on line %d",param,CurLine);
												FTMessageBox(tstr,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);	
											} else { //if (leai->AnalogSensorNameNum[j]==-1) {
												leai->AngleKind[j]=ANGLE_KIND_ANALOG; //angle is analog sensor
											} //if (leai->AnalogSensorNameNum[j]==-1) {
										} else { //if (leai->AccelNameNum[j]==-1) {
											leai->AngleKind[j]=ANGLE_KIND_ACCEL; //angle is accelerometer
										} //if (leai->AccelNameNum[j]==-1) {

										//Get EthAccel PCB num from AccelNameNum[j] or AnalogSensorNameNum[j]
										if (leai->AngleKind[j]==ANGLE_KIND_ACCEL) { 
											leai->PCBNum[j]=RStatus.AccelInfo[leai->AccelNameNum[j]].PCBNum;
											//get Accel num on PCB
											leai->AccelNum[j]=RStatus.AccelInfo[leai->AccelNameNum[j]].Num;
										} else {
											if (leai->AngleKind[j]==ANGLE_KIND_ANALOG) {
												leai->PCBNum[j]=RStatus.AccelInfo[leai->AnalogSensorNameNum[j]].PCBNum;
												leai->AnalogSensorNum[j]=RStatus.AnalogSensorInfo[leai->AnalogSensorNameNum[j]].Num;
											} 
										} //if (leai->AngleKind[j]==ANGLE_KIND_ACCEL) {

										if (leai->PCBNum[j]==-1) {
											sprintf(tstr,"No known EthAccels PCB name for angle '%s' on line %d",param,CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);	
										}

										//get the next word on this line (Accel dimension x,y,)  (was: 0,1, or 2)
										param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
										if (param==0) {
											fprintf(stderr,"No angle dimension (x,y,z = 0,1,2) on line %d",CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);
										} //if (param==0) {
										//fprintf(stderr,"param=%s, (int)param[0]=%d\n",param,(int)param[0]);
										if (param[0]==88 || param[0]==89 || param[0]==90) {  //X,Y, or Z
											leai->xyz[j]=(int)param[0]-88;	
											//fprintf(stderr,"Param[0]=X Y or Z %d\n",(int)param[0]);
										} else {
											if (param[0]==120 || param[0]==121 || param[0]==122) {  //x,y,or z
												leai->xyz[j]=(int)param[0]-120;	
												//fprintf(stderr,"Param[0]=x y or z %d\n",(int)param[0]);
											} else {
												sprintf(tstr,"Accelerometer dimension needs to be X,Y, or Z) on line %d",CurLine);
												FTMessageBox(tstr,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
//												leai->xyz[j]=atoi(param);  //number 0,1,2
											}
										} //if (param=='X' || param=='Y' || param='Z') {
										if (leai->xyz[j]<0 || leai->xyz[j]>2) {
											//fprintf(stderr,"Accelerometer dimension needs to be x,y, or z or 0,1, or 2) on line %d\n",CurLine);
											fprintf(stderr,"Angle dimension needs to be X,Y, or Z) on line %d\n",CurLine);
										}

										//fprintf(stderr,"Found Accel: %s %s(%d) NameNum=%d pcb=%s AccelNum=%d\n",ROBOT_ACCEL_NAMES[leai->AccelNameNum[j]],param,leai->xyz[j],leai->AccelNameNum[j],ROBOT_ACCEL_NAMES_PCB[leai->AccelNameNum[j]],leai->AccelNum[j]);


										//get the next word on this line (TargetAngle, Variable, CurrentAngle, or CurrentAnglePlus)
										param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited


										FoundMatch=0;
										//check to see if "CurrentAnglePlus" is specified
										//perhaps change to CurrentAngle+3 or CurrentAngle-3, etc. 
#if Linux
										if (strncasecmp(param,"CurrentAnglePlus",16)==0) {
#endif
#if WIN32
										if (_strnicmp(param,"CurrentAnglePlus",16)==0) {
#endif
											//fprintf(stderr,"Using current angle\n");
											FoundMatch=1;
											linst->flagUseCurrentAnglePlus|=(1<<j); //set flag
											//leai->Angle[0]=0.0;  //todo: determine why this is set to 0.0

									
											//with CurrentAnglePlus comes a floating point value offset that follows it
											param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
											if (param==0) {
												sprintf(tstr,"No CurrentAnglePlus offset on line %d",CurLine);
												FTMessageBox(tstr,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
											} //if (param==0) {
											leai->AngOffset[j]=atof(param);
											//fprintf(stderr,"CurrentAnglePlus j=%d offset = %5.2f on line %d\n",j,leai->AngOffset[j],CurLine);											
										} else { //if (strncasecmp(param,"CurrentAnglePlus",16)==0


											//check to see if "CurrentAngle" is specified
											//note that this is in the above else, because otherwise matches for CurrentAnglePlus
	#if Linux
											if (strncasecmp(param,"CurrentAngle",12)==0) {
	#endif
	#if WIN32
											if (_strnicmp(param,"CurrentAngle",12)==0) {
	#endif
												//fprintf(stderr,"Using current angle\n");
												FoundMatch=1;
												linst->flagUseCurrentAngle|=(1<<j); //set flag
												leai->oAngle[j][0]=0.0;
											} //if (strncasecmp(param,"CurrentAngle",12)==0) {
										} //if (strncasecmp(param,"CurrentAnglePlus",16)==0) {


										if (FoundMatch) {
											//CurrentAngle or CurrentAngle[]

											//check to see if "...[variable]" is specified
											//can be: CurrentAngle[var], CurrentAnglePlus[var]  (was also: Angle[var])	
											//So the '[' character can be at 12, 16, or 5 
											//if (param[5]=='[' || param[12]=='[' || param[16]=='[') {			
											if (param[12]=='[' || param[16]=='[') {			
												//There is an angle variable name given	
												//get the variable name
												k=0;
												k2=0;
												FoundMatch=0;
												while(k<strlen(param)) {
													if (FoundMatch && param[k]!=']') {
														tstr[k2]=param[k];
														k2++;
													} else {
														tstr[k2]=0;
													}
													if (param[k]=='[') {
														FoundMatch=1;
													}
													k++;
												} //while

										
												//if CurrentAngle[var] or CurrentAnglePlus[var] see if variable already exists, if no, then add to variable list. Otherwise the inst has Angle[var], so verify that variable already exists in variable list.
								
												if ((linst->flagUseCurrentAngle&(1<<j)) || (linst->flagUseCurrentAnglePlus&(1<<j))) {  //CurrentAngle or CurrentAnglePlus used on this accel 
							
													//see if variable already exists
													for(k=0;k<lscript->NumAngleVariables;k++) {
														if (!strcmp(lscript->AngleVariableName[k],tstr)) {
															sprintf(tstr2,"Error: Angle Variable '%s' already exists, line %d",tstr,CurLine);
															FTMessageBox(tstr2,FTMB_OK,"Error",0);
															FreeRobotScript(lscript);
															return(0);
														} //if (!strcmp(lscript->AngleVariableName[k],tstr)) {
													} //for(k

													//fprintf(stderr,"New angle variable '%s', stored at index %d\n",tstr,lscript->NumAngleVariables);
													//store the variable name in the script variable name array
													strcpy(lscript->AngleVariableName[lscript->NumAngleVariables],tstr);
											
													if (linst->flagStoreAngleVariable&(1<<j)) {
														leai->AngleVariableIndex[j]=lscript->NumAngleVariables;
													} else {
														leai->AngleVariableIndex[j]=-1;
													}
													lscript->NumAngleVariables++;
													leai->oAngle[j][0]=0.0;  //set Angle[0]=0.0 since it will not be used
												} //if (linst->flagStoreAngleVariable&(1<<j)) {

											} //if (param[5]=='[' || param[12]=='[' || param[16]=='[') {				

										} else { //if (FoundMatch) {

											//Not CurrentAngle, must be either a number or an angle variable name
											//if not CurrentAngle[] or CurrentAnglePlus[] then can only be a number or an Angle variable (was Angle[] but now is just variable name)

											//if not a number, can only be a variable
											//check for +, -, ., and 0-9
											if (!(param[0]==43 || param[0]==45 || param[0]==46 || (param[0]>=48 && param[0]<=57))) {

												//verify that this variable already exists
												FoundMatch=-1;							
												for(k=0;k<lscript->NumAngleVariables;k++) {
													if (!strcmp(lscript->AngleVariableName[k],param)) {
														FoundMatch=k;
													} //if (!strcmp(lscript->AngleVariableName[k],param)) {
												} //for(k
												if (FoundMatch==-1) {
													sprintf(tstr2,"Error: Unknown Angle Variable '%s' on line %d",param,CurLine);
													FTMessageBox(tstr2,FTMB_OK,"Error",0);
													FreeRobotScript(lscript);
													return(0);
												} //if (FoundMatch==-1) { 
												//otherwise store the angle variable index
												//fprintf(stderr,"Angle variable '%s' used, index=%d\n",tstr,FoundMatch);
												linst->flagUseAngleVariable|=(1<<j);
												leai->AngleVariableIndex[j]=FoundMatch;
												//fprintf(stderr,"Use Variable %s for Accel %d, Index=%d, on line %d\n",lscript->AngleVariableName[FoundMatch],j,FoundMatch,CurLine);
											} //if (!(param[0]==43 || param[0]==45 || param[0]==46 || (param[0]>=48 && param[0]<=57))) {


										} //if (FoundMatch) {


										if (!(linst->flagUseCurrentAngle&(1<<j)) && !(linst->flagUseCurrentAnglePlus&(1<<j)) && !(linst->flagUseAngleVariable&(1<<j))) { //no need to worry about STORE_ANGLE flags since USE_CURRENT_ANGLE flag would be set
											//just a regular Angle (TargetAngle)
											leai->oAngle[j][0]=atof(param);  //note that oAngle can also be a relative offset if the RELATIVE_ANGLE flag is given
											//fprintf(stderr,"Found angle specified as number: %f\n",atof(param));
										} //if (!(linst->flagUseCurrentAngle&(1<<j)) && !(...


					
										//Get DegreesMinus
										//get the next word on this line (Angle[1])
										param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
										if (param==0) {
											sprintf(tstr,"No DegreesMinus on line %d",CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);
										} //if (param==0) {
										leai->oAngle[j][1]=leai->oAngle[j][0]+atof(param);											
										leai->oDegOff[j][0]=fabs(atof(param)); //save offset for use current angle
										if (atof(param)>0) {
												fprintf(stderr,"Warning: DegreesMinus %f is >0 on line %d\n",atof(param),CurLine);
												leai->oAngle[j][1]=leai->oAngle[j][0]-atof(param);
												fprintf(stderr,"Setting minimum target angle to %f degrees.\n",leai->oAngle[j][1]);
										}
										//if (j==0) { 
										//	leai->DegOff[0]=atof(param);  //copy to DegOff[0] since win_Tasks.c does not do this
										//	leai->Angle[1]=leai->Angle[0]-leai->DegOff[0];
									//	}

										//get DegreesPlus
										//get the next word on this line (Angle[2])
										param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
										if (param==0) {
											sprintf(tstr,"No DegreesPlus on line %d",CurLine);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											FreeRobotScript(lscript);
											return(0);
										} //if (param==0) {
										leai->oAngle[j][2]=leai->oAngle[j][0]+atof(param);											
										leai->oDegOff[j][1]=fabs(atof(param)); //save offset for use current angle
										if (atof(param)<0) {
												fprintf(stderr,"Warning: DegreesPlus %f is <0 on line %d\n",atof(param),CurLine);
												leai->oAngle[j][2]=leai->oAngle[j][0]+fabs(atof(param));
												fprintf(stderr,"Setting maximum target angle to %f degrees.\n",leai->oAngle[j][2]);
										}
										//if (j==0) { 
										//	leai->DegOff[1]=atof(param);  //copy to DegOff[0] since win_Tasks.c does not do this
										//	leai->Angle[2]=leai->Angle[0]+leai->DegOff[1];
									//	}

										//fprintf(stderr,"oAngle[%d]=%2.1f %2.1f %2.1f\n",j,leai->oAngle[j*3],leai->oAngle[j*3+1],leai->oAngle[j*3+2]);

										linst->NumAngles++; //one Accel is complete

										//get the next word on this line flags or second accel 
										param=strtok(NULL,",\n |)"); //comma,\n,space, |, or ) delimited 
										if (param!=0) {
											//if (!strncmp(param,"ACCEL",5) || (j==0 && InstNum==ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_RELATIVE_ANGLE)) {
											if ((!strncmp(param, "ACCEL",5) || !strncmp(param, "POT",3)) && (j<MAX_ANGLES_PER_INSTRUCTION)) { //j==0) {
												j++;//j=1;  //loop again to get the second accel
												//linst->flags|=ROBOT_INSTRUCTION_TWO_ACCELS; //so code knows to load EthAccel of Accel[1]
												//fprintf(stderr,"found 2 accels\n");
											} else {
												j=-1; //exit while
											} 
										} else {
											j=-1; //exit while
										} //if (param!=0)

									//} //for(j=0;j<1+(InstNum==ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_RELATIVE_ANGLE);j++) {
									} //while(j>-1) {  //for each ACCEL



									//get the next word on this line (flags)
									//current flags: WAIT_STAGE,END_ON_TIMEOUT
									//WAIT_ALL was removed, because in a script that has a WAIT_STAGE, starttimes should be viewed more like symbolic stages, because the WAIT_STAGE inst will last for a variable time, and to guarantee that later stages will execute- they should have starttimes of WAIT_STAGE starttime+1,2,3,4, etc. to be sure their starttime has passed no matter how long any WAIT_STAGE inst lasts. So to wait all, a script should just set an inst 1ms behind the WAIT_STAGE inst if that inst should not be executed until the WAIT_STAGE inst is done. To use WAIT_STAGE indicates that this script is a "stage-based" script, where one set of instructions wait for the last set to end, instead of a "time-based" script, where all instructions execute at precise times. Of course, times are still checked in stage-based scripts, but probably no stage-based script would make use of that. 											
									//param=strtok(NULL,",\n #)|"); //comma,\n,space,#,) or | delimited
									memset(UserSetMaxVelocity,0,sizeof(int)*4);
									memset(UserSetMinVelocity,0,sizeof(int)*4);
									memset(UserSetMaxAccel,0,sizeof(int)*4);
									memset(UserSetMinAccel,0,sizeof(int)*4);
									ExitLoop=0;
									while(!ExitLoop) {
										MatchedFlag=0;
										if (!(strcmp(param,"WAIT_STAGE"))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_WAIT_STAGE;											
										} 
										if (!(strcmp(param,"END_ON_TIMEOUT"))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_END_SCRIPT_ON_TIMEOUT;
										}
										if (!(strcmp(param,"MOTION_LOG"))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_MOTION_LOG;
											//add path and first part of filename - note for 2motors+1accel, name of first motor is used
											strncpy(tstr,lscript->name,strlen(lscript->name)-3);  //get filename minus file extension
											tstr[strlen(lscript->name)-3]=0;
#if Linux
											sprintf(linst->MotionLogFileName,"%s/motion/Motion_%s_%s_%s",RStatus.CurrentRobotLogFolder,tstr,RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[0]].Name);
#endif
#if WIN32
											sprintf(linst->MotionLogFileName,"%s\\motion\\Motion_%s_%s_%s",RStatus.CurrentRobotLogFolder,tstr,RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[0]].Name);
#endif
										}  //if(!(strcmp(param,"MOTION_LOG"))) {
										if (!(strcmp(param,"FUTURE_LOG"))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_FUTURE_LOG;
											//add path and first part of filename
											strncpy(tstr,lscript->name,strlen(lscript->name)-3);  //get filename minus file extension
											tstr[strlen(lscript->name)-3]=0;
											sprintf(linst->FutureLogFileName,"%s_S%d_%s_%s",tstr,linst->StartTime,RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[0]].Name);
//#if Linux
//											sprintf(linst->FutureLogFileName,"%s/future/Future_%s_S%d_%s_%s",RStatus.CurrentRobotLogFolder,tstr,linst->StartTime,RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[0]].Name);											
//#endif
//#if WIN32
//											sprintf(linst->FutureLogFileName,"%s\\future\\Future_%s_S%d_%s_%s",RStatus.CurrentRobotLogFolder,tstr,linst->StartTime,RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[0]].Name);
//#endif
										}  //if(!(strcmp(param,"FUTURE_LOG"))) {

										if (!(strcmp(param,"DOUBLE_THRUST"))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_DOUBLE_THRUST;
										}
										if (!(strncmp(param,"CONSTANT_THRUST",15))) {
											MatchedFlag=1;
											linst->flags|=ROBOT_INSTRUCTION_CONSTANT_THRUST;
											//check to see if user gave a constant thrust bias
											if (!(strncmp(param,"CONSTANT_THRUST=",16))) {
												//UserCTBOverride=1;
												linst->flags |= ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS;
												j=16; 
												k=0;
												CTBlen=0;
												while(j!=strlen(param)) {
													if (param[j]==0x3a) { //found colon 0x3a //was comma 0x2c
														CTB[0][CTBlen]=0;
														CTBlen=0;
														k++;
													} else { //if (param[j]==0x2c) {
														CTB[k][CTBlen]=param[j];
														CTBlen++;
													} //if (param[j]==0x2c) {
													j++;
												} //while
												//for now set CTB for all accels
												for(j=0;j<linst->NumAngles;j++ ) {
													//set constant thrust when target in positive direction, todo: add for motor=1 too
													leai->ConstantThrustBias[j][0]=fabs(atof(CTB[0]));	//CTB[j][0]: accel=j motor=0 (positive direction)
													if (k==1) { 
														//todo: add for motor=1 too
														//terminate second word, CTB when negative direction
														CTB[1][CTBlen]=0;
														leai->ConstantThrustBias[j][2]=fabs(atof(CTB[1])); //CTB[j][2]: accel=j motor=0 (negative direction)
													} else {
														//no second number given so CTB + and - are identical
														leai->ConstantThrustBias[j][2]=fabs(atof(CTB[0]));  //CTB[j][2]: accel=j motor=0 (negative direction)
													} //if (k==1) {
												} //for j
												//fprintf(stderr,"User set Constant Thrust Ratio plus=%03.2f minus=%03.2f\n",leai->ConstantThrustBias[0],leai->ConstantThrustBias[4]);

											} //if (!(strncmp(param,"CONSTANT_THRUST=",16))) {
										} //if (!(strncmp(param,"CONSTANT_THRUST",15))) {

										if (!(strncmp(param, "REQUIRED",8))) {
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_REQUIRED;
										} //if (!(strncmp(param, "REQUIRED",8))) {

										if (!(strncmp(param, "NO_HOLD",7))) {
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_NO_HOLD;
										} //if (!(strncmp(param, "NO_HOLD",7))) {

										if (!(strcmp(param, "IF_A2")) || !(strncmp(param,"IF_A2_THEN",10)) ) { //IF_A2_THEN_A0_ELSE_A1
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1;
											linst->flagIgnoreAccel|=4;  //1<<2 - so no Motor-Angle Association is checked
											//linst->flagAccelEndsStage&=~4; //if Accel2 determines which accel to use, then Accel2 cannot end the stage
										} //if (!(strncmp(param, "IF_A2",5))) {

										if (!(strncmp(param, "IF_A3",5))) { //IF_A3_THEN_A1_ELSE_A2
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2;
											linst->flagIgnoreAccel|=8;  //1<<3 - so no Motor-Angle Association is checked
											//linst->flagAccelEndsStage&=~8; //if Accel3 determines which accel to use, then Accel3 cannot end the stage
										} //if (!(strncmp(param, "IF_A3",5))) {

										if (!(strncmp(param, "IF_A2_AND_A3",12))) { //IF_A2_AND_A3_THEN_A0_ELSE_A1
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1;
											linst->flagIgnoreAccel|=0xc;  //mask A2 (0x4) and A3 (0x8) - so no Motor-Angle Association is checked
											//linst->flagAccelEndsStage&=~0xc; //if Accel2 and Accel3 determine which accel to use, then Accel2 and Accel3 cannot end the stage
										} //if (!(strncmp(param, "IF_A2_AND_A3",12))) {

										if (!(strncmp(param, "IF_NOT_A1_MAXTHRUST=",20)) || !(strncmp(param, "IF_NOT_A2_MAXTHRUST=",20)) || !(strncmp(param, "IF_NOT_A3_MAXTHRUST=",20)) || !(strncmp(param, "IF_NOT_A4_MAXTHRUST=",20))) { //IF_NOT_A4_MAXTHRUST=#
											MatchedFlag=1;
											j=(int)param[8]-48;
											if (j<1 || j>4) {
												sprintf(tstr2,"Error: Incorrect character in position [8] of IF_NOT_A#_MAXTHRUST flag on line %d",CurLine);
												FTMessageBox(tstr2,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
											}	
											//linst->flags |= ROBOT_INSTRUCTION_IF_NOT_A_MAXTHRUST;  //technically do not need this flag can use below flagInstAccelMaxThrust
											linst->flagInstAccelMaxThrust|=(1<<j); //0x10=A4, 0x8=A3, 0x4=A2,0x2=A1
											linst->flagIgnoreAccel|=(1<<j);  //1<<4 - so no Motor-Angle Association is checked
											//linst->flagAccelEndsStage&=~(1<<j); //if Accel4 determines MaxThrust, then Accel4 cannot end the stage
										  linst->InstMaxThrust[j]=abs(atoi(&param[20]));  //Store MaxThrust value for this A# in this array. There can be multiple MaxThrust A#s in a single inst. 
											//fprintf(stderr,"Inst may set MaxThrust=%d depends on Accel %d (flagInstAccelMaxThrust=0x%x) on line %d\n",linst->InstMaxThrust[j],j,linst->flagInstAccelMaxThrust,CurLine);

											//Currently this instruction sets MaxThrust for all Accels
											for(j=0;j<MAX_ANGLES_PER_INSTRUCTION;j++) {
												//Set MaxThrust for all possible accels
												linst->flagInstMaxThrust|=(1<<j); 
												//fprintf(stderr,"Inst may set MaxThrust of Accel %d to %d on line %d\n",j,linst->InstMaxThrust[j],CurLine);
											} //for j
										} //if (!(strncmp(param, "IF_NOT_A1_MAXTHRUST",20)...


										if (!(strncmp(param, "VELOCITY_A",9))) { //ex: VELOCITY_A3<-10 or VELOCITY_A3_OR<-1, Accel condition requires certain velocity to be true
											//Currently this is just for accels that are ignored (use IF_A2, etc.) 
											MatchedFlag=1;
											AngleNum=(int)param[10]-48;
											if (AngleNum<1 || AngleNum>4) {
												sprintf(tstr2,"Error: Incorrect character in position [10] of VELOCITY_A# flag on line %d",CurLine);
												FTMessageBox(tstr2,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
											}
											if (!(strncmp(&param[11], "_OR",3))) {
												linst->flagRequireVelocityOperators[AngleNum]|=2; //bit 1: 0=AND 1=OR
												GTorLT=(int)param[14];
												Value=atof(&param[15]);	
											} else {
												GTorLT=(int)param[11];
												Value=atof(&param[12]);	
											}
											if (GTorLT != 60  && GTorLT != 62) {  //< 60  > 62
												sprintf(tstr2,"Error: Operator in VELOCITY_A# flag can only be < or > on line %d",CurLine);
												FTMessageBox(tstr2,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
											}
											//linst->flags |= ROBOT_INSTRUCTION_REQUIRE_VELOCITY;
											linst->flagRequireAccelVelocity|=(1<<AngleNum); //0x10=A4, 0x8=A3, 0x4=A2,0x2=A1
											linst->RequiredVelocity[AngleNum]=Value;
											if (GTorLT==62) {
												linst->flagRequireVelocityOperators[AngleNum]|=1; //0=LT 1=GT
											} 
											//linst->flagIgnoreAccel|=(1<<j);  //1<<4 - so no Motor-Angle Association is checked
											//linst->flagAccelEndsStage&=~(1<<j); //if Accel4 determines MaxThrust, then Accel4 cannot end the stage
											if (GTorLT==60) {
												//fprintf(stderr,"Inst requires velocity of A%d to be less than %3.2f on line %d\n",AngleNum,Value,CurLine);
											} else {
												//fprintf(stderr,"Inst requires velocity of A%d to be greater than %3.2f on line %d\n",AngleNum,Value,CurLine);
											} 
										} //if (!(strncmp(param, "VELOCITY_A",9))) {

										if (!(strncmp(param, "TVR",3))) {
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_TVR;
											//fprintf(stderr,"TVR\n");
											//check for TVR= form
											if (param[3]>=48 && param[3]<=51) {
												linst->flagTVR|=(1<<(param[3]-48));  //TVR0,TVR1,etc.
											}
											if (strlen(param)==3) { //just TVR command
												linst->flagRelativeAngle=0xf;
												//UseDefaultTVR[0-2] are already set to 0xf, so nothing more to do here
											} else { //if (strlen(param)==3) { //just TVR command
												//could be TVR= or TVR0,TVR1, etc. 
												if (strlen(param)==4) { //just TVR0,TVR1,TVR2,or TVR3 command
													if (param[3]>=48 && param[3]<=51) {
														j=param[3]-48;
														//already are set for(k=0;k<3;k++) {
														//	UseDefaultTVR[k]|=(1<<(param[4]-48));  //set TVR to default (below)
														//}
														fprintf(stderr,"Using default TVR for TVR%d flag (cw:%03.1f,ccw:%03.1f) on line %d\n",j,DEFAULT_TVR,DEFAULT_TVR,CurLine);
													} else { //if (param[4]>=48 && param[4]<=51) {
														sprintf(tstr2,"Error: Incorrect character in position [4] of TVR flag on line %d",CurLine);
														FTMessageBox(tstr2,FTMB_OK,"Error",0);
														FreeRobotScript(lscript);
														return(0);													
													} //if (param[3]>=48 && param[3]<=51) {
												} else { //if (strlen(param)==4) { //just TVR0,TVR1,TVR2,or TVR3 command
													j=4; //
													if (param[3]==61) { //=
														j=4; //offset without # (TVR=)
													} else {
														if (param[4]==61) { //=
															j=5; //offset with # (TVR0=)
														} else {
															sprintf(tstr2,"Error: Unknown character in position [3] or [4] of TVR flag on line %d",CurLine);
															FTMessageBox(tstr2,FTMB_OK,"Error",0);
															FreeRobotScript(lscript);
															return(0);													
														} //if (param[4]==61) { //=
													}  //if (param[3]==61) { //=

													//User entered in TVR numbers (either as TVR=# or TVR#=#): 
													//UserTVROverride=1;
													//determine how many of three numbers are given (CW TVR:CCW TVR:MinTVRThrust)
													k=0;
													TVRlen=0;
													while(j!=strlen(param)) {
														if (param[j]==0x3a) { //found colon 0x3a //was comma 0x2c
															TVR[k][TVRlen]=0;
															TVRlen=0;
															k++; //move to next number (can be one to three numbers)
														} else { //if (param[j]==0x2c) {
															TVR[k][TVRlen]=param[j];
															TVRlen++; 
														} //if (param[j]==0x2c) {
														j++;
													} //while j
													//set TVR for specified accelerometer
													if (k==0) { //no user numbers, set one or all accels to default TVR
															sprintf(tstr2,"Error: TVR= flag but no user thrusts given on line %d",CurLine);
															FTMessageBox(tstr2,FTMB_OK,"Error",0);
															FreeRobotScript(lscript);
															return(0);		
													} //if (k==0) {
													if (param[3]==61) { //TVR= (all accels)
														NumAngle=linst->NumAngles;
														j=0;
													} else {
														j=param[3]-48; //TVR#= (one accel)
														NumAngle=param[3]-47;  //single AccelNum+1
													}
													while(j<NumAngle) {													
														if (k>-1) {	//TVR[0] is set
															leai->ThrustVelocityRatio[j][0]=fabs(atof(TVR[0]));	//TVR [0]: accel=j motor=0 (positive direction)									
															UseDefaultTVR[0]&=~(1<<j); //clear bit for this accel, so below does not set to default TVR values
															//fprintf(stderr,"User set Accel%d ThrustVelocityRatioCW=%03.2f\n",j,leai->ThrustVelocityRatio[j][0]);				
														} 
														if (k>0) {	//TVR[1] is set
															leai->ThrustVelocityRatio[j][1]=fabs(atof(TVR[1]));	//TVR [1]: accel=j motor=0 (negative direction)													
															UseDefaultTVR[1]&=~(1<<j); //clear bit for this accel, so below does not set to default TVR values
															//fprintf(stderr,"User set Accel%d ThrustVelocityRatioCCW=%03.2f\n",j,leai->ThrustVelocityRatio[j][1]);				
														} else {
															//
														}
														if (k>1) {	//MinTVRThrust is set
															leai->MinTVRThrust[j]=fabs(atoi(TVR[2]));	//TVR [2]: MinTVRThrust
															UseDefaultTVR[2]&=~(1<<j); //clear bit for this accel, so below does not set to default TVR values
															//fprintf(stderr,"User set Accel%d MinTVRThrust=%0d\n",j,leai->MinTVRThrust[j]);
														} 
														j++;
													} //while(j<=NumAngle) {													

													//fprintf(stderr,"User set ThrustVelocityRatioP=%03.2f ThrustVelocityRatioN=%03.2f\n",leai->ThrustVelocityRatio[0],leai->ThrustVelocityRatio[4]);
												} //if (strlen(param)==4) { //just TVR0,TVR1,TVR2,or TVR3 command
											} //if (strlen(param)==3) { //just TVR command
										} //if (!(strncmp(param, "TVR",3))) {



/*
										//todo: add HOLD_ANGLE0=, HOLD_ANGLE1=, etc.
										if (!(strncmp(param, "HOLD_ANGLE",10))) {
											MatchedFlag=1;
											linst->flags |= ROBOT_INSTRUCTION_HOLD_ANGLE;
											//fprintf(stderr,"HOLD_ANGLE\n");
											//check for HOLD_ANGLE= form
											if (!(strncmp(param, "HOLD_ANGLE=",11))) {
												linst->flags |= ROBOT_INSTRUCTION_HOLD_ANGLE_TVR;  //tells thrust analysis in task code to use TVR when velocity in the wrong direction (away from target angle) V=N
												UserTVROverride=1;
												j=11; 
												k=0;
												TVRlen=0;
												while(j!=strlen(param)) {
													if (param[j]==0x3a) { //found colon 0x3a //was comma 0x2c
														TVR[0][TVRlen]=0;
														TVRlen=0;
														k++;
													} else { //if (param[j]==0x2c) {
														TVR[k][TVRlen]=param[j];
														TVRlen++;
													} //if (param[j]==0x2c) {
													j++;
												} //while
												//set TVR when positive direction, todo: add for motor=1 too
												//for now, add for all accelerometers
												for(j=0;j<linst->NumAngles;j++) {
													leai->ThrustVelocityRatio[j][0]=fabs(atof(TVR[0]));	//TVR [0]: accel=j motor=0 (positive direction)
													if (k==1) { //second value give (-+:-)
														//todo: add for motor=1 too
														//terminate second word, TVR when negative direction
														TVR[1][TVRlen]=0;
														leai->ThrustVelocityRatio[j][1]=fabs(atof(TVR[1])); //TVR [8]: accel=j motor=0 (negative direction)
													} else {
														//no second number given so TVR + and - are identical
														leai->ThrustVelocityRatio[j][1]=fabs(atof(TVR[0]));  //TVR [8]: accel=j motor=0 (negative direction)
													} //if (k==1) {
												} //for j
												//fprintf(stderr,"User set ThrustVelocityRatioP=%03.2f ThrustVelocityRatioN=%03.2f\n",leai->ThrustVelocityRatio[0],leai->ThrustVelocityRatio[4]);
											} //if (!(strncmp(param, "HOLD_ANGLE=",11))) {
										} //if (!(strcmp(param, "HOLD_ANGLE"))) {
*/
										if (!(strncmp(param, "RELATIVE_ANGLE",14))) {
											MatchedFlag=1;
											if (param[14]==48) { //RELATIVE_ANGLE0
												linst->flagRelativeAngle|=1;
												linst->flagIgnoreAccel|=2;  //ignore A1- so no Motor-Angle Association is checked
												//linst->flagAccelEndsStage&=~2; //if Accel0 is a relative angle Accel1 cannot end the stage
												//fprintf(stderr,"Relative Angle Accel 0 line %d\n",CurLine);
												leai->AngOffset[0]=leai->oAngle[0][0]; //store relative angle offset
											}
											if (param[14]==49) { //RELATIVE_ANGLE1
												if (linst->flagRelativeAngle&1) {
													sprintf(tstr2,"Error: RELATIVE_ANGLE0 and RELATIVE_ANGLE1 cannot be in the same instruction, use RELATIVE_ANGLE2 for two relative angles in one instruction on line %d",CurLine);
													FTMessageBox(tstr2,FTMB_OK,"Error",0);
													FreeRobotScript(lscript);
													return(0);													
												}
												linst->flagRelativeAngle|=2;
												linst->flagIgnoreAccel|=4;  //ignore A2- so no Motor-Angle Association is checked
												//linst->flagAccelEndsStage&=~4; //if Accel1 is a relative angle Accel2 cannot end the stage
												//fprintf(stderr,"Relative Angle Accel 1 line %d\n",CurLine);
												leai->AngOffset[1]=leai->oAngle[1][0]; //store relative angle offset
											}
											if (param[14]==50) { //RELATIVE_ANGLE2
												if (linst->flagRelativeAngle&2) {
													sprintf(tstr2,"Error: RELATIVE_ANGLE1 and RELATIVE_ANGLE2 cannot be in the same instruction on line %d",CurLine);
													FTMessageBox(tstr2,FTMB_OK,"Error",0);
													FreeRobotScript(lscript);
													return(0);													
												}
												linst->flagRelativeAngle|=4;
												linst->flagIgnoreAccel|=8;  //ignore A3- so no Motor-Angle Association is checked
												//linst->flagAccelEndsStage&=~8; //if Accel2 is a relative angle Accel3 cannot end the stage
												//fprintf(stderr,"Relative Angle Accel 2 line %d\n",CurLine);
												leai->AngOffset[2]=leai->oAngle[2][0]; //store relative angle offset
											}
											if (strlen(param)==14) { //RELATIVE_ANGLE (not number indicating which accel is the relative angle)
												if (linst->NumAngles<4) {
													fprintf(stderr,"Presuming Accel 0 is Relative Angle on line %d\n",CurLine);
													linst->flagRelativeAngle|=1;
													linst->flagIgnoreAccel|=2;  //ignore A1- so no Motor-Angle Association is checked
													//linst->flagAccelEndsStage&=~2; //if Accel0 is a relative angle Accel1 cannot end the stage
												}
												if (linst->NumAngles==4) {
													fprintf(stderr,"Presuming Accel 0 and Accel 2 are Relative Angle on line %d\n",CurLine);
													linst->flagRelativeAngle|=5;
													linst->flagIgnoreAccel|=0xa;  //ignore A1 and A3- so no Motor-Angle Association is checked
													//linst->flagAccelEndsStage&=~0xa; //if Accel0 and Accel2 are relative angle Accel1 and Accel3 cannot end the stage
												}
											} //if (strlen(param)==14) { //RELATIVE_ANGLE
											if (linst->NumAngles<2) {
												sprintf(tstr2,"Error: RELATIVE_ANGLE requires at least 2 accels on line %d",CurLine);
												FTMessageBox(tstr2,FTMB_OK,"Error",0);
												FreeRobotScript(lscript);
												return(0);
											} //if (linst->NumAngles<2) {
												//fprintf(stderr,"Relative Angle flag %x\n",linst->flagRelativeAngle);

										}//if (!(strncmp(param, "RELATIVE_ANGLE",14))) {

										//todo: OPPOSE_MOTION0 and OPPOSE_MOTION1 option
										//todo: remove =, and make user use TVR flag for TVR values  
										if (!(strncmp(param, "OPPOSE_MOTION",13))) {  //Currently only for all accels 
											//forms: 
											//  1: OPPOSE_MOTION    (use default ThrustVelocityFactor (TVF))
											//  2: OPPOSE_MOTION=#1   (override TVF with #1 {float})
											//  3: OPPOSE_MOTION=#1:#2 (override TVF with #1 TVF for cw motion, and #2 TVF for ccw motion {biases thrust in one direction, used, for example, to bias a robot ankle to thrust more to the outside when stepping with the other leg}   
											MatchedFlag=1;
											if (!(strncmp(param, "OPPOSE_MOTION_IN_RANGE",22))) {
												//OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET can be used with HOLD_ANGLE (and not OPPOSE_MOTION), so set this flag separately
												linst->flags |= ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET;
											} else {
												linst->flags |= ROBOT_INSTRUCTION_OPPOSE_MOTION;
											} //if (!(strncmp(param, "OPPOSE_MOTION_IN_RANGE",22))) {
											//HOLD_ANGLE is now assumed unless NO_HOLD flag linst->flags |= ROBOT_INSTRUCTION_HOLD_ANGLE;  //currently requires HOLD_ANGLE flag
											if (!(strncmp(param, "OPPOSE_MOTION_MAINTAIN",22))) {
												linst->flags|=ROBOT_INSTRUCTION_OPPOSE_MOTION_MAINTAIN; //Thrust=0 when IsMoving=0
											} 

											//check for forms 2, 3 and 4,  
											if (!(strncmp(param, "OPPOSE_MOTION=",14)) || !(strncmp(param, "OPPOSE_MOTION_MAINTAIN=",23)) || !(strncmp(param, "OPPOSE_MOTION_IN_RANGE=",23)) )  {
												//skip for now because there are 2 accels with 4 potential plus minus TVRs
												//if (UserTVROverride) {  //check to see if TVROverride plus/minus params have already been given
												//	fprintf(stderr,"Found multiple plus/minus Thrust-Velocity Ratio parameters, using first given: %03.1f %03.1f on line %d\n",
													
												//UserTVROverride=1;
												for(k=0;k<3;k++) {
													UseDefaultTVR[k]=0xe; //for now set accel 1-3 to default TVR
												} //for k

												if (!(strncmp(param, "OPPOSE_MOTION=",14))) { 	
													j=14; //OPPOSE_MOTION=
												} else {
													j=23;  //OPPOSE_MOTION_MAINTAIN= or OPPOSE_MOTION_IN_RANGE=
												} 
												k=0;
												TVRlen=0;
												while(j!=strlen(param)) {
													if (param[j]==0x3a) { //found colon 0x3a //was comma 0x2c
														TVR[0][TVRlen]=0;
														TVRlen=0;
														k++;
													} else { //if (param[j]==0x2c) {
														TVR[k][TVRlen]=param[j];
														TVRlen++;
													} ////if (param[j]==0x2c) {
													j++;
												} //while
												//set TVR when positive direction, todo: add for motor=1 too
												leai->ThrustVelocityRatio[0][0]=fabs(atof(TVR[0]));	//TVR [0]: accel=0 motor=0 (positive direction)
												leai->ThrustVelocityRatio[1][0]=fabs(atof(TVR[0]));	//TVR [2]: accel=1 motor=0 (positive direction)
												if (k==1) { 
													//todo: add for motor=1 too
													//terminate second word, TVR when negative direction
													TVR[1][TVRlen]=0;
													leai->ThrustVelocityRatio[0][1]=fabs(atof(TVR[1])); //TVR [4]: accel=0 motor=0 (negative direction)
													leai->ThrustVelocityRatio[1][1]=fabs(atof(TVR[1])); //TVR [6]: accel=1 motor=0 (negative direction)
												} else {
													//no second number given so TVR + and - are identical
													leai->ThrustVelocityRatio[0][1]=fabs(atof(TVR[0]));  //TVR [4]: accel=0 motor=0 (negative direction)
													leai->ThrustVelocityRatio[1][1]=fabs(atof(TVR[0]));  //TVR [6]: accel=1 motor=0 (negative direction)
												} //if (k==1) {
												//fprintf(stderr,"User set ThrustVelocityRatioP=%03.2f ThrustVelocityRatioN=%03.2f\n",leai->ThrustVelocityRatio[0],leai->ThrustVelocityRatio[4]);
											} //if (!(strncmp(param, "OPPOSE_MOTION=",14))) {
										}//if (!(strncmp(param, "OPPOSE_MOTION",13))) {

										if (!(strncmp(param, "MAX_THRUST",10))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MAX_THRUST=",11))) { 
												strcpy(tstr,&param[11]);
												//tstr[strlen(param)]=0;
												for(j=0;j<linst->NumAngles;j++) {
													lemi->MaxThrust[j]=atoi(tstr);
													linst->flagUserMaxThrust|=(1<<j);
												} //for
											} else { // MAX_THRUST#=
												//MAX_THRUST0= (or 1, 2, 3)
												strcpy(tstr,&param[12]); //copy int after MAX_THRUST0=
												for(j=0;j<linst->NumAngles;j++) {
													if (param[10]==48+j) { //MAX_THRUST0=
														MatchedFlag=1;
														lemi->MaxThrust[j]=atoi(tstr);
														linst->flagUserMaxThrust|=(1<<j);
														//fprintf(stderr,"User set MaxThrust for gyro %d in inst to %d with flagUserMaxThrust=%x on line %d\n",j,lemi->MaxThrust[j],linst->flagUserMaxThrust,CurLine);
													} 
												} //for j
											} //if (!(strncmp(param, "MAX_THRUST=",11)) { 
										} //if (!(strncmp(param, "MAX_THRUST",10))) {

										if (!(strncmp(param, "MAX_REVERSE_THRUST",18))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MAX_REVERSE_THRUST=",19))) { 
												strcpy(tstr,&param[19]);
												for(j=0;j<linst->NumAngles;j++) {
													lemi->MaxReverseThrust[j]=atoi(tstr);
													linst->flagUserMaxReverseThrust|=(1<<j);
												} //for
											} else { //if (!(strncmp(param, "MAX_REVERSE_THRUST=",19)) { 
												//MAX_REVERSE_THRUST0= (or 1, 2, 3)
												strcpy(tstr,&param[20]);  //copy int after MAX_REVERSE_THRUST0=
												for(j=0;j<linst->NumAngles;j++) {
													if (param[18]==48+j) { //MAX_REVERSE_THRUST0
														MatchedFlag=1;
														lemi->MaxReverseThrust[j]=atoi(tstr);
														linst->flagUserMaxReverseThrust|=(1<<j);
														//fprintf(stderr,"User set MaxReverseThrust for gyro %d in inst to %d on line %d\n",j,lemi->MaxReverseThrust[j],CurLine);
													} //if (param[19]==48+j) { //MAX_REVERSE_THRUST0 
												} //for j
											} //if (!(strncmp(param, "MAX_REVERSE_THRUST=",19)) { 
										} //if (!(strncmp(param, "MAX_REVERSE_THRUST",18))) {

										if (!(strncmp(param, "MIN_VELOCITY",12))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MIN_VELOCITY=",13))) { 
												strcpy(tstr,&param[13]);
												for(j=0;j<linst->NumAngles;j++) {
													leai->MinVelocity[j]=atof(tstr);
													//linst->flagUserMinVelocity|=(1<<j);
													UserSetMinVelocity[j]=1;  //todo:remove if possible
												} //for J
											} else { //if (!(strncmp(param, "MIN_VELOCITY=",13)) { 
												//MIN_VELOCITY0= (or 1, 2, 3)
												strcpy(tstr,&param[14]);  //get float at end of MIN_VELOCITY0=
												for(j=0;j<linst->NumAngles;j++) {
													if (param[12]==48+j) { //get number of MIN_VELOCITY0
														MatchedFlag=1;
														leai->MinVelocity[j]=atof(tstr);
														//linst->flagUserMinVelocity|=(1<<j);
														UserSetMinVelocity[j]=1;  //todo: remove if possible
														//fprintf(stderr,"User set MinVelocity for gyro %d in inst to %3.1f on line %d\n",j,leai->MinVelocity[j],CurLine);
													} //if (param[12]==48+j) { //MIN_VELOCITY0
												} //for j
											} //if (!(strncmp(param, "MIN_VELOCITY=",13)) { 
											//fprintf(stderr,"User set MinVelocity for all gyros in inst to %f\n",leai->MinVelocity[0]);
										} //if (!(strncmp(param, "MIN_VELOCITY=",13))) {

										if (!(strncmp(param, "MAX_VELOCITY",12))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MAX_VELOCITY=",13))) { 
												strcpy(tstr,&param[13]);
												for(j=0;j<linst->NumAngles;j++) {
													leai->MaxVelocity[j]=atof(tstr);
													//linst->flagUserMaxVelocity|=(1<<j);
													UserSetMaxVelocity[j]=1;  //todo:remove if possible
												} //for
											} else { //if (!(strncmp(param, "MAX_VELOCITY=",13)) { 
												strcpy(tstr,&param[14]); //get float value after MAX_VELOCITY0=
												for(j=0;j<linst->NumAngles;j++) {
													if (param[12]==48+j) { //0, 1, 2, 3
														MatchedFlag=1;
														leai->MaxVelocity[j]=atof(tstr);
														//linst->flagUserMaxVelocity|=(1<<j);
														UserSetMaxVelocity[j]=1;  //todo: remove if possible
														//fprintf(stderr,"User set MaxVelocity for gyro %d in inst to %3.1f on line %d\n",j,leai->MaxVelocity[j],CurLine);													
													} //if (param[12]==48+j) { //MAX_VELOCITY0
												} //for j
											} //if (!(strncmp(param, "MAX_VELOCITY=",13)) { 
											//fprintf(stderr,"User set MaxVelocity for all gyros in inst to %f\n",leai->MaxVelocity[0]);
										} //if (!(strncmp(param, "MAX_VELOCITY",12))) {


										if (!(strncmp(param, "MAX_ACCEL",9))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MAX_ACCEL=",10))) { 
												strcpy(tstr,&param[10]); //MAX_ACCEL=
												for(j=0;j<linst->NumAngles;j++) {
													leai->MaxAccel[j]=atof(tstr);
													//linst->flagUserMaxAccel|=(1<<j);
													UserSetMaxAccel[j]=1;  //todo:remove if possible
												} //for
												//fprintf(stderr,"User set MaxAccel for all gyros in inst to %3.1f\n",leai->MaxAccel[0]);
											} else { //if (!(strncmp(param, "MAX_ACCEL=",10)) { 
												strcpy(tstr,&param[11]);  //get float value after MAX_ACCEL0=												
												for(j=0;j<linst->NumAngles;j++) {
													if (param[9]==48+j) {  //0,1,2,3 
														MatchedFlag=1;
														leai->MaxAccel[j]=atof(tstr);
														//linst->flagUserMaxAccel|=(1<<j);
														UserSetMaxAccel[j]=1;  //todo: remove if possible
														//fprintf(stderr,"User set MaxAccel for gyro %d in inst to %3.1f on line %d\n",j,leai->MaxAccel[j],CurLine);													
													} //if (param[10]==48+j) { //MAX_ACCEL0
												} //for j
											} //if (!(strncmp(param, "MAX_ACCEL=",10)) { 
											
										} //if (!(strncmp(param, "MAX_ACCEL",9))) {

										if (!(strncmp(param, "MIN_ACCEL",9))) {
											MatchedFlag=1;
											if (!(strncmp(param, "MIN_ACCEL=",10))) { 
												strcpy(tstr,&param[11]);
												for(j=0;j<linst->NumAngles;j++) {
													leai->MinAccel[j]=atof(tstr);
													//linst->flagUserMinAccel|=(1<<j);
													UserSetMinAccel[j]=1;  //todo:remove if possible
												} //for
												//fprintf(stderr,"User set MinAccel for all gyros in inst to %f\n",leai->MinAccel[0]);
											} else { //if (!(strncmp(param, "MIN_ACCEL=",10)) {
												//MIN_ACCEL0= (or 1, 2, 3) 
												strcpy(tstr,&param[11]);  //get float after MIN_ACCEL0=
												for(j=0;j<linst->NumAngles;j++) {
													if (param[9]==48+j) { //MIN_ACCEL0
														MatchedFlag=1;
														leai->MinAccel[j]=atof(tstr);
														//linst->flagUserMinAccel|=(1<<j);
														UserSetMinAccel[j]=1;  //todo: remove if possible
														//fprintf(stderr,"User set MinAccel for gyro %d in inst to %3.1f on line %d\n",j,leai->MinAccel[j],CurLine);													
													} //if (param[10]==48+j) { //MIN_ACCEL0
												} //for j
											} //if (!(strncmp(param, "MIN_ACCEL=",10)) { 
											
										} //if (!(strncmp(param, "MIN_ACCEL",9))) {

										if (!(strncmp(param, "ABORT_VELOCITY",14))) {
											MatchedFlag=1;
											if (param[14]==48+j) { //ABORT_VELOCITY0=
												strcpy(tstr,&param[16]);
												for(j=0;j<linst->NumAngles;j++) {
													leai->AbortVelocity[j]=atof(tstr);
													linst->flagAbortVelocity|=(1<<j);
													//fprintf(stderr,"User set AbortVelocity for Accel/Gyro %d to %4f\n",j,leai->AbortVelocity[[j]);
												} //for
											} //if (param[14]==48+j) {
										} //if (!(strncmp(param, "ABORT_VELOCITY",14)) { 

										if (!(strncmp(param, "ABORT_ANGLE",11))) {
											MatchedFlag=1;
											if (param[12]==48+j) { //ABORT_ANGLE0=
												strcpy(tstr,&param[14]);
												for(j=0;j<linst->NumAngles;j++) {
													leai->AbortAngle[j]=atof(tstr);
													linst->flagAbortAngle|=(1<<j);
													//fprintf(stderr,"User set AbortAngle for Accel %d to %4f\n",j,leai->AbortVelocity[[j]);
												} //for
											} //if (param[12]==48+j) {
										} //if (!(strncmp(param, "ABORT_ANGLE",11)) { 


										if (!(strncmp(param, "NO_MOTION_NO_THRUST",19))) {
											MatchedFlag=1;
											if (strlen(param)==19) { 
												for(j=0;j<linst->NumAngles;j++) {
													linst->flagNoMotionNoThrust|=(1<<j);
												} //for J
												//fprintf(stderr,"User set NoMotionNoThrust flag for all gyros in inst on line %d\n",CurLine);
											} else { //if (strlen(param)==19) { 
												for(j=0;j<linst->NumAngles;j++) {
													if (param[20]==48+j) { //NO_MOTION_NO_THRUST0
														MatchedFlag=1;
														linst->flagNoMotionNoThrust|=(1<<j);
														//fprintf(stderr,"User set NoMotionNoThrust flag for gyro %d on line %d\n",CurLine);													
													} //if (param[20]==48+j) { //NO_MOTION_NO_THRUST0
												} //for j
											} //if (strlen(param)==19) {  											
										} //if (!(strncmp(param, "NO_MOTION_NO_THRUST",19))) {



										if (!(strncmp(param, "ABORT_ON_TIMEOUT", 23))) {
											MatchedFlag = 1;
											//fprintf(stderr,"User set Abort on timeout\n");
											linst->flags |= ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT;
										}

										if (!(strncmp(param, "SYNC_MAX_SPREAD=",16))) {  //todo: add a second accel pair?
											MatchedFlag=1;
											strcpy(tstr,&param[16]);
											leai->MaxSpread=atof(tstr);
											//fprintf(stderr,"User set MaxSpread to %4.2f\n",lemi->MaxSpread);
											if (leai->MaxSpread<0) {
											  //MaxSpread is always positive
												fprintf(stderr,"MaxSpread is always positive, changing %03.2f to %03.2f on line %d\n",leai->MaxSpread,fabs(leai->MaxSpread),CurLine);
												leai->MaxSpread=fabs(leai->MaxSpread);
											} //if (leai->MaxSpread<0) {
											linst->flags |= ROBOT_INSTRUCTION_SYNC_TWO_ACCELS;
										}

										if (!(strncmp(param, "LAST_STAGE=",11))) {  
											//maybe I will do away with this flag - using MAINTAIN_THRUST makes looking at each stage clearer, but LAST_STAGE make it clear what changes are happening in a stage- and decreases chances of errors because there is only 1 inst
											MatchedFlag=1;
											strcpy(tstr,&param[11]);
											linst->LastStage=atof(tstr);
											fprintf(stderr,"User set LastStage to %d on line %d\n",linst->LastStage,CurLine);
											fprintf(stderr, "***LAST_STAGE is deprecated*** use MAINTAIN_THRUST. User set LastStage to %d\n", linst->LastStage);
											linst->flags |= ROBOT_INSTRUCTION_LAST_STAGE;
										}

										if (!(strncmp(param, "MAINTAIN_THRUST", 15))) {
											MatchedFlag = 1;
											//strcpy(tstr, &param[15]);
											//linst->LastStage = atof(tstr);
											//fprintf(stderr,"MAINTAIN_THRUST Line %d\n",CurLine);
											linst->flags |= ROBOT_INSTRUCTION_MAINTAIN_THRUST;
										}

										//NO_STOP_ON_TARGET - skip the code that tries to stop exactly at the target with V=0
										if (!(strncmp(param, "NO_STOP_ON_TARGET", 17))) {
											MatchedFlag = 1;
											//strcpy(tstr, &param[15]);
											//linst->LastStage = atof(tstr);
											//fprintf(stderr,"User set LastStage to %d\n",linst->LastStage);
											linst->flags |= ROBOT_INSTRUCTION_NO_STOP_ON_TARGET;
										}

/*
										if (!(strncmp(param, "INITIAL_THRUST",14))) {
											MatchedFlag=1;
											//strcpy(tstr,&param[15]);
											//lemi->UserThrust=atof(tstr);
											//fprintf(stderr,"User set InitialThrust to %d\n",lemi->UserThrust);
											linst->flags |= ROBOT_INSTRUCTION_INITIAL_THRUST;
										}
*/

										if (!(strncmp(param, "MOST_VELOCITY", 13))) {
											MatchedFlag = 1;
											//fprintf(stderr,"User set MOST_VELOCITY\n");
											linst->flags |= ROBOT_INSTRUCTION_MOST_VELOCITY;
										}


										if (!(strncmp(param, "FARTHEST_FROM_TARGET",20))) {
											MatchedFlag = 1;
											//fprintf(stderr,"User set FARTHEST_FROM_TARGET\n");
											linst->flags |= ROBOT_INSTRUCTION_FARTHEST_FROM_TARGET;
										}

										if (!(strncmp(param, "USE_MAX_VELOCITY",15))) {
											MatchedFlag = 1;
											//fprintf(stderr,"User set USE_MAX_VELOCITY\n");
											linst->flags |= ROBOT_INSTRUCTION_USE_MAX_VELOCITY;
										}

/*
										if (!(strncmp(param, "USE_MIN_VELOCITY",15))) {
											MatchedFlag = 1;
											//fprintf(stderr,"User set USE_MIN_VELOCITY\n");
											linst->flags |= ROBOT_INSTRUCTION_USE_MIN_VELOCITY;
										}
*/
										if (!MatchedFlag) {
											//flag was not matched
											if (param[0]==35) { //comment
												MatchedFlag=1;  //so no error message, but exit loop because no flags can be after a comment character #
											} //if (param==35) {
											ExitLoop=1;
											//fprintf(stderr,"Flag %s not recognized\n",param);
										} else { //if (!MatchedFlag) {
											param=strtok(NULL,",\n )|"); //comma,\n,space, ) or | delimited
											if (param==0) {
												ExitLoop=1;
												//fprintf(stderr,"Exiting flag loop\n");
											} //if (param==0) {
										} //(!MatchedFlag)

										
										
									} //while(param!=0)


									if (!MatchedFlag) {
										//flag was not matched
										sprintf(tstr,"Instruction Flag '%s' not recognized on line %d",param,CurLine);
										FTMessageBox(tstr,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //(!MatchedFlag)


									//done getting all flags

									//one MotorAngle inst can have 2 motors and 1 accel - note that this code is not currently implemented in win_tasks.c
									//Now get the direction that the motor needs to turn to increase the angle
									//in order to know- how do I turn the motor specified in the MotorAngle instruction to cause the accel (first - accel[0]- for relative angle comparisons) angle specified in the MotorAngle instruction to increase the angle?


									//fprintf(stderr,"%d Accels for inst on line %d\n",linst->NumAngles,CurLine);
									//fprintf(stderr,"flagRelativeAngle=%x 1<<(NumAngles-1)=%x\n",linst->flagRelativeAngle,1<<(linst->NumAngles-1));

									//SAFETY CHECKS

									//check of last relative angle has one more angle
									if (linst->flagRelativeAngle&(1<<(linst->NumAngles-1))) {
										sprintf(tstr2,"Error: RELATIVE_ANGLE requires another accel on line %d",CurLine);
										FTMessageBox(tstr2,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (linst->NumAngles<2) {


									//check that there are enough accels for IF_A2, IF_A3, and IF_A2_AND_A3 
									if ((linst->flags&ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2) && linst->NumAngles<4) {
										sprintf(tstr2,"Error: IF_A3 requires at least 4 accels on line %d",CurLine);
										FTMessageBox(tstr2,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (linst->NumAngles<2) {
									if ((linst->flags&ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1) && linst->NumAngles<3) {
										sprintf(tstr2,"Error: IF_A2 requires at least 3 accels on line %d",CurLine);
										FTMessageBox(tstr2,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (linst->NumAngles<2) {
									if ((linst->flags&ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1) && linst->NumAngles<4) {
										sprintf(tstr2,"Error: IF_A2_AND_A3 requires at least 4 accels on line %d",CurLine);
										FTMessageBox(tstr2,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (linst->NumAngles<2) {


									//determine the last accel that can end a stage
									linst->flagAccelEndsStage=0;
									for(j=0;j<linst->NumAngles;j++) {
										if (!(linst->flagIgnoreAccel&(1<<j))) {
											linst->flagAccelEndsStage=(1<<j); //1=accel0,2=accel1,4=accel2, etc.  	
										}
									}
									if (!linst->flagAccelEndsStage) {
										fprintf(stderr,"Warning: No accelerometers that can end the stage in instruction on line %d\n",CurLine);
									} 

									//fprintf(stderr,"NumAngles=%d, linst->flagAccelEndsStage 0x%x on line %d\n",linst->NumAngles,linst->flagAccelEndsStage,CurLine);

									//fprintf(stderr,"NumActiveAccels=%d for Line %d\n",linst->NumActiveAccels,CurLine);

									//Handle the case of two motors separately:
									if (linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
										for (j=0;j<2;j++) {
											//fprintf(stderr,"j=%d\n",j);
											//two motors+ one or more accels
											//get the MotorAngleAssociation for both Motor+Accel combinations so we can determine if the Thrust needs to be reversed when two motors are sync'd (turned at the same time with the same Thrust).
											//for each accel
											for(k=linst->NumAngles-1;k>-1;k--) {  //go backwards because the task code should refer to the MotorAngleAssociation properties (mass, etc.) for Accel[0]
												//get the pointer to the Motor Angle Association
												MotorAngAssoc=GetMotorAngleAssociation(linst,j,k); //lemi-


												if (MotorAngAssoc==0) {
													if (!(linst->flagIgnoreAccel&(1<<k))) {
//													sprintf(tstr,"No Motor-Accelerometer Angle Association found for PCB %s Motor %d, with Accel %s num %d, angle x,y,z=%d, Script='%s' on line %d.",RStatus.pcb[lemi->PCBNum[j]].Name,lemi->MotorNum[j],RStatus.pcb[leai->PCBNum[k]].Name,leai->AccelNum[k],leai->xyz[k],lscript->name,CurLine);
														if (leai->AngleKind[j]==ANGLE_KIND_ACCEL) {
															sprintf(tstr,"No Motor-Angle Association found for Motor %s with Accel %s angle x,y,z=%d on line %d.",RStatus.MotorInfo[lemi->MotorNameNum[j]].Name,RStatus.AccelInfo[leai->AccelNameNum[k]].Name,leai->xyz[k],CurLine);
														} else {
															sprintf(tstr,"No Motor-Angle Association found for Motor %s with Analog %s angle x,y,z=%d on line %d.",RStatus.MotorInfo[lemi->MotorNameNum[j]].Name,RStatus.AnalogSensorInfo[leai->AnalogSensorNameNum[k]].Name,leai->xyz[k],CurLine);
														} //
														FTMessageBox(tstr,FTMB_OK,"Error",0);
														FreeRobotScript(lscript);
														return(0);
													} //if (!(linst->flagIgnoreAccel&(1<<k))) {
												} else {
													//note k is accel and j is motor (0,1 positive direction, 2,3 negative direction)
													leai->IncAngDir[k][j]=MotorAngAssoc->IncAngDir;  //need IncAngDir from MotorAssoc
													//fprintf(stderr,"2Motors: IncAngDir[%d][0]=%d line %d\n",k,leai->IncAngDir[k][0],CurLine);
													if (UseDefaultTVR[0]&(1<<k)) {  //User did not override default TVR CW
														leai->ThrustVelocityRatio[k][j*2]=MotorAngAssoc->ThrustVelocityRatio;  //positive TVR [a][0] and [a][2]
														//fprintf(stderr,"Setting accel %d TVR CW to %03.1f on line %d\n",k,MotorAngAssoc->ThrustVelocityRatio,CurLine);
													} //if (UseDefaultTVR[0]&(1<<k)) {  //User did not override default TVR CW
													if (UseDefaultTVR[1]&(1<<k)) {  //User did not override default TVR CCW
														leai->ThrustVelocityRatio[k][j*2+1]=MotorAngAssoc->ThrustVelocityRatio;  //negative TVR [a][1] and [a][3]
														//fprintf(stderr,"Setting accel %d TVR CCW to %03.1f on line %d\n",k,MotorAngAssoc->ThrustVelocityRatio,CurLine);
													} //if (UseDefaultTVR[1]&(1<<k)) {  //User did not override default TVR CCW
													if (UseDefaultTVR[2]&(1<<k)) {  //User did not override default TVR CCW
														leai->MinTVRThrust[k]=abs(MotorAngAssoc->IncAngDir);
														//fprintf(stderr,"Setting accel %d MinTVRThrust to %0d on line %d\n",k,leai->MinTVRThrust[k],CurLine);
													} //if (UseDefaultTVR[2]&(1<<k)) {  //User did not override default TVR CCW
													//fprintf(stderr,"IncAngDir %d = %d\n",k*2+j,leai->IncAngDir[k][j]);
												} //if (MotorAngAssoc==0) {
											} //for k
										} //for j
									} else { //if (linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
										//only 1 motor

										for(k=linst->NumAngles-1;k>-1;k--) {  //go backwards because the task code should refer to the MotorAngleAssociation properties (mass, etc.) for Accel[0] 
											if (!((k==1) && (linst->flags&ROBOT_INSTRUCTION_SYNC_TWO_ACCELS))) { //no need for MotorAngle Association for a second accelerometer if the SYNC flag is set

//												MotorAngAssoc=GetMotorAngleAssociation(lemi->MotorNameNum[0],leai->AccelNameNum[k],leai->xyz[k]);
												MotorAngAssoc=GetMotorAngleAssociation(linst,0,k);

												if (MotorAngAssoc==0) {
													if (!(linst->flagIgnoreAccel&(1<<k))) {	//these accels do not require a Motor-Angle Association
														if (leai->AngleKind[0]==ANGLE_KIND_ACCEL) {
															sprintf(tstr,"No Motor-Angle Association found for Motor %s with Accel %s angle x,y,z=%d on line %d.",RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AccelInfo[leai->AccelNameNum[k]].Name,leai->xyz[k],CurLine);
														} else { 
															sprintf(tstr,"No Motor-Angle Association found for Motor %s with Analog %s angle x,y,z=%d on line %d.",RStatus.MotorInfo[lemi->MotorNameNum[0]].Name,RStatus.AnalogSensorInfo[leai->AnalogSensorNameNum[k]].Name,leai->xyz[k],CurLine);
														} //if (leai->AngleKind==ANGLE_KIND_ACCEL) {
														FTMessageBox(tstr,FTMB_OK,"Error",0);
														FreeRobotScript(lscript);
														return(0);
													} //if (!(linst->flagIgnoreAccel&(1<<k))) {
												} else {
													//note: IncAngDir[0] and IncAngDir[2], for Motor[0]+Accel[0], Motor[0]+Accel[1] (because UseAccel*2 is used in win_Tasks)
													leai->IncAngDir[k][0]=MotorAngAssoc->IncAngDir;  // need IncAngDir from MotorAssoc [accel][motor]
													//fprintf(stderr,"1Motor: IncAngDir[%d][0]=%d line %d\n",k,leai->IncAngDir[k][0],CurLine);
													if (UseDefaultTVR[0]&(1<<k)) {  //User did not override default TVR CW
														leai->ThrustVelocityRatio[k][0]=MotorAngAssoc->ThrustVelocityRatio;  //positive TVR [a][0] and [a][2]
														//fprintf(stderr,"Setting accel %d TVR CW to %03.1f on line %d\n",k,MotorAngAssoc->ThrustVelocityRatio,CurLine);
													} //if (UseDefaultTVR[0]&(1<<k)) {  //User did not override default TVR CW
													if (UseDefaultTVR[1]&(1<<k)) {  //User did not override default TVR CCW
														leai->ThrustVelocityRatio[k][1]=MotorAngAssoc->ThrustVelocityRatio;  //negative TVR [a][1] and [a][3]
														//fprintf(stderr,"Setting accel %d TVR CCW to %03.1f on line %d\n",k,MotorAngAssoc->ThrustVelocityRatio,CurLine);
													} //if (UseDefaultTVR[1]&(1<<k)) {  //User did not override default TVR CCW
													if (UseDefaultTVR[2]&(1<<k)) {  //User did not override default TVR CCW
														leai->MinTVRThrust[k]=abs(MotorAngAssoc->IncAngDir);
														//fprintf(stderr,"Setting accel %d MinTVRThrust to %0d on line %d\n",k,leai->MinTVRThrust[k],CurLine);
													} //if (UseDefaultTVR[2]&(1<<k)) {  //User did not override default TVR CCW
													//fprintf(stderr,"IncAngDir %d = %d\n",k*2+j,leai->IncAngDir[k][j]);
												} //if (MotorAngAssoc==0) {
											} //if (!((k==1) && (linst->flags&ROBOT_INSTRUCTION_SYNC_TWO_ACCELS)))

										} //for k
									} //if (linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {


									for(k=0;k<linst->NumAngles;k++) {
										//this code is the same whether 2motors and/or 2 accels:
										if (MotorAngAssoc!=0) {											
											leai->Weight[k]=MotorAngAssoc->Weight;
											leai->Mass[k]=MotorAngAssoc->Mass;
											leai->MotionThreshold[k]=MotorAngAssoc->MotionThreshold;
											leai->WaitAndSeeDelay[k]=MotorAngAssoc->WaitAndSeeDelay;
											
											//set MinVelocity and MaxVelocity while we have MotorAngAssoc
											//if user did not set them in the instruction flags											
											if (!UserSetMinVelocity[k]) {
												leai->MinVelocity[k]=MotorAngAssoc->MinVelocity;
											}
											if (!UserSetMaxVelocity[k]) {
												leai->MaxVelocity[k]=MotorAngAssoc->MaxVelocity;
											}
											leai->MedianVelocity[k]=(leai->MaxVelocity[k]+leai->MinVelocity[k])/2.0;
											if (!UserSetMaxAccel[k]) {
												leai->MaxAccel[k]=MotorAngAssoc->MaxAccel;
											}
											if (!UserSetMinAccel[k]) {
												leai->MinAccel[k]=MotorAngAssoc->MinAccel;
											}
											if (MotorAngAssoc->flags&MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY) {
												linst->flagIgnoreGravity|=(1<<k);
											} //if (MotorAngAssoc->flags&MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY) {
											if (MotorAngAssoc->flags&NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE) {
												linst->flagNoThrustWhenMAGIR|=(1<<k);
											} //if (MotorAngAssoc->flags&NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE) {
										} //if (MotorAngAssoc!=0) {

									} //for k

								
								} //if (InstNum == ROBOT_INSTRUCTION_MOTOR_ANGLE) {


								//if (InstNum!=ROBOT_INSTRUCTION_TURN_MOTOR) {
									//set max speed associated with accelerometer angle assocaited with this instruction
									//leai->MaxSpeed=RStatus.AccelInfo[leai->AccelNameNum[0]].MaxSpeed[leai->xyz[0]]/100; //todo: use SAMPLE_TIMER_INTERVAL instead of /100 - units are currently Degrees/Sampleinterval (currently Degrees/10^-2s degrees per 10ms or .01s) 
								//} 


								//if TargetSpeed is more than MaxSpeed, decrease TargetSpeed here
								//if (fabs(leai->TargetSpeed)>leai->MaxSpeed) {
								//	leai->TargetSpeed=leai->MaxSpeed;
								//}

								//get body segment name, and a readble motor name text for the Model Log
								for(k=0;k<linst->NumAngles;k++) {
									if (leai->AngleKind[k]==ANGLE_KIND_ACCEL) {
										//accelerometer
										//strcpy(leai->SensorName[k],RStatus.AccelInfo[leai->AccelNameNum[k]].Name);
											strcpy(leai->SensorReadableName[k],RStatus.AccelInfo[leai->AccelNameNum[k]].ReadableName);
//										GetSensorName(leai->SensorName[k],leai->PCBNum[k],leai->AccelNum[k],);
									} else {
										//potentiometer - todo: change SensorName to AngleName?
										//note that currently there is no AnalogSensor "Readable Name" (potentiometers or touch sensors), so just use Name
										strcpy(leai->SensorReadableName[k],RStatus.AnalogSensorInfo[leai->AnalogSensorNameNum[k]].Name);
										//GetSensorName(leai->SensorName[k],leai->PCBNum[k],leai->AccelNum[k]);
									} 
								}

								//fprintf(stderr,"lemi=%p\n",lemi);
								//fprintf(stderr,"lemi->PCBNum[0]=%p\n",lemi->PCBNum[0]);
								//fprintf(stderr,"lemi->MotorNum[0]=%d\n",lemi->MotorNum[0]);
								//fprintf(stderr,"Motor[0] readable name=%s\n",GetMotorReadableName_By_PCB(lemi->PCBNum[0],lemi->MotorNum[0]);							
								
								//for now just store the readable name of the first motor
								strcpy(lemi->MotorReadableName,GetMotorReadableName_By_PCB(lemi->PCBNum[0],lemi->MotorNum[0]));  
								//lemi->MotorNameNum=lemi->MotorNameNum; //Get number of Motor Name so task can easily GetNeededThrust

								//user should not need to add DOUBLE_THRUST flag
								if (linst->flags&ROBOT_INSTRUCTION_TWO_MOTORS) {
									linst->flags|=ROBOT_INSTRUCTION_DOUBLE_THRUST;
								} 

#if 0 
								//set flagAccelEndsStage, which accel ends the stage (usually last, but is next to last if last accel is second accel in relative angle, or is IF_A2 or IF_A3 accel)
								linst->flagAccelEndsStage=1<<(linst->NumAngles-1); //set AccelEndsStage flag to last accel bit (1 accel=1, 2acc=2, 3acc=4,4acc=8)
								//check to see if last accel is second half of a relative angle by >>1 and comparing to flagRelativeAngle
								if ((linst->flagAccelEndsStage>>1)&linst->flagRelativeAngle) {  //is last accel a relative angle accel?
									linst->flagAccelEndsStage>>=1; //if yes, shift to accel before (last relative angle/accel)
								} 
								if (linst->flags&(ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1|ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2|ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1)) { //is this inst an IF_A2 or IF_A3 inst?
									linst->flagAccelEndsStage>>=1; //if yes,shift accel that can end stage one more place, to exclude A2
									if (linst->flags&ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1) {
										linst->flagAccelEndsStage>>=1; //shift accel that can end stage one more place, to exclude A3
									}
								} //								if (linst->flags&(ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1|ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2|ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1)) 

								if (linst->flags&ROBOT_INSTRUCTION_IF_NOT_A4_MAXTHRUST) {
									linst->flagAccelEndsStage>>=1; //shift accel that can end stage one more place, to exclude A4
								}
#endif

								//fprintf(stderr,"Name %s Num %d\n",leai->MotorReadableName,leai->MotorNameNum);

								leai->origflags=leai->flags;  //preserve flags, because scripts can be run more than once, and when run their flags get changed
								linst->origflags=linst->flags;  //preserve flags, because scripts can be run more than once, and when run their flags get changed
				
								//Done with this line- there is no need to strtok the carriage return, 
								//because the loop just goes to the next line

//							End:
//							case ROBOT_INSTRUCTION_TURN_MOTOR:
//							case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE:							
//							case ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE_WAIT:	
							break;  //ROBOT_INSTRUCTION_MOTOR_ANGLE: 
							case ROBOT_INSTRUCTION_STORE_ANGLE:
								//store an angle into a variable during real-time (while the script is running)
								//ex: 0,StoreAngle(ACCEL_LOWER_LEFT_LEG,Z,LLLZ0)	
						
								//get the Accel

								param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
								if (param==0) {
									sprintf(tstr,"No Accelerometer name on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {


								leai->AccelNameNum[0]=GetAccelNameNum(param);
								if (leai->AccelNameNum[0]==-1) {
									sprintf(tstr,"No known Accelerometer named '%s' on line %d",param,CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);	
								}
								//Get EthAccel PCB num from leai->AccelNameNum[0] 

								leai->PCBNum[0]=RStatus.AccelInfo[leai->AccelNameNum[0]].PCBNum;
								if (leai->PCBNum[0]==-1) {
									sprintf(tstr,"No known EthAccels PCB name '%s' from accel '%s' on line %d",RStatus.AccelInfo[leai->AccelNameNum[0]].Name,param,CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);	
								}
								//get Accel num on PCB
								leai->AccelNum[0]=leai->AccelNameNum[0]%3;

								//get the next word on this line (Accel dimension x,y,z 0,1, or 2)
								param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
								if (param==0) {
									fprintf(stderr,"No accelerometer dimension (x,y,z = 0,1,2) on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								//fprintf(stderr,"param=%s, (int)param[0]=%d\n",param,(int)param[0]);
								if (param[0]==88 || param[0]==89 || param[0]==90) {  //X,Y, or Z
									leai->xyz[0]=(int)param[0]-88;	
									//fprintf(stderr,"Param[0]=X Y or Z %d\n",(int)param[0]);
								} else {
									if (param[0]==120 || param[0]==121 || param[0]==122) {  //x,y,or z
										leai->xyz[j]=(int)param[0]-120;	
										//fprintf(stderr,"Param[0]=x y or z %d\n",(int)param[0]);
									} else {
										leai->xyz[0]=atoi(param);
									}
								} //if (param=='X' || param=='Y' || param='Z') {
								if (leai->xyz[0]<0 || leai->xyz[0]>2) {
									fprintf(stderr,"Accelerometer dimension needs to be x,y, or z or 0,1, or 2) on line %d\n",CurLine);
								}

								//get the angle variable name
								param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
								if (param==0) {
									fprintf(stderr,"No Angle Variable name given on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
							
								//see if variable already exists
								for(k=0;k<lscript->NumAngleVariables;k++) {
									if (!strcmp(lscript->AngleVariableName[k],param)) {
										sprintf(tstr2,"Error: Angle Variable '%s' already exists, line %d",param,CurLine);
										FTMessageBox(tstr2,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (!strcmp(lscript->AngleVariableName[k],param)) {
								} //for(k

								//fprintf(stderr,"New angle variable '%s', stored at index %d\n",param,lscript->NumAngleVariables);
								//store the variable name in the script variable name array
								strcpy(lscript->AngleVariableName[lscript->NumAngleVariables],param);							
								linst->flagStoreAngleVariable=1;  
								leai->AngleVariableIndex[0]=lscript->NumAngleVariables;
								for(j=1;j<MAX_ANGLES_PER_INSTRUCTION;j++) {
									leai->AngleVariableIndex[1]=-1;
								}
								lscript->NumAngleVariables++;
								//GetSensorName(leai->SensorName[0],leai->PCBNum[0],leai->AccelNum[0]);
								strcpy(leai->SensorReadableName[0],RStatus.AccelInfo[leai->AccelNameNum[0]].ReadableName);


							break; //ROBOT_INSTRUCTION_STORE_ANGLE
							case ROBOT_INSTRUCTION_START_LOOP:  //StartLoop(keyword,KeyWord1,KeyWord2) keyword- # of loops, ex: StartLoop(3) (loop 3 times)
								param=strtok(NULL,",");  //go to the next word (number of times to loop)
								if (param==0) {
									sprintf(tstr,"No number of Loops on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								linst->NumLoops=atoi(param); //-1=indef, 0=none, 1+=number of times to loop

								param=strtok(NULL,",");  //go to the next word (KeyCode1)
								if (param==0) {
									sprintf(tstr,"No KeyCode1 (key to end loop after last inst, ex: AnyKey or Key=31) on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								if (!strcmp(param,"Any")) {
									linst->KeyCode[0]=0; //AnyKey=0 any key press stop looping after last inst in loop sequence
								} else {
									//todo: add error checking
									//linst->KeyCode[0]=atoi(&param[4]); //
									linst->KeyCode[0]=atoi(param);
									fprintf(stderr,"KeyCode1=%d\n",linst->KeyCode[0]);
								} //if (!strcmp(param,"Any")) {

								param=strtok(NULL,")");  //go to the next word (KeyCode2)
								if (param==0) {
									sprintf(tstr,"No KeyCode2 (key to end loop instantly, ex: AnyKey or Key=31) on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								if (!strcmp(param,"Any")) {
									linst->KeyCode[1]=0; //AnyKey=0 any key press stop looping after last inst in loop sequence
								} else {
									//todo: add error checking
									linst->KeyCode[1]=atoi(param); //
									fprintf(stderr,"KeyCode2=%d\n",linst->KeyCode[1]);
								} //if (!strcmp(param,"Any")) {

								param=strtok(NULL,"\n");  //go to the next line

								//currently we can ignore the rest of the line								
							break; //ROBOT_INSTRUCTION_START_LOOP
							case ROBOT_INSTRUCTION_END_LOOP:
								param=strtok(NULL,"\n");  //go to the next line
								//currently we can ignore the rest of the line								
							break; //ROBOT_INSTRUCTION_END_LOOP
							case ROBOT_INSTRUCTION_ANALOG_SENSOR:
								param=strtok(NULL,",");  //go to the next word (KeyCode1)
								if (param==0) {
									sprintf(tstr,"No Analog Sensor name found on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								leti=(EthAnalogSensorInstruction *)&linst->ETInst;  //shorthand
								leti->AnalogSensorNameNum=GetAnalogSensorNameNum(param);
								if (leti->AnalogSensorNameNum==-1) {
									sprintf(tstr,"No known Analog Sensor named '%s' on line %d",param,CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);	
								}
								//Get EthAccel PCB num from AnalogSensorNameNum 

								leti->PCBNum=RStatus.AnalogSensorInfo[leti->AnalogSensorNameNum].PCBNum;
								if (leti->PCBNum==-1) {
									sprintf(tstr,"No known EthAccelsTouch PCB name '%s' from Analog Sensor '%s' on line %d",RStatus.AnalogSensorInfo[leti->AnalogSensorNameNum].Name,param,CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);	
								}
								leai=(EthAccelsInstruction *)&linst->EAInst;  //shorthand

								leai->PCBNum[0]=leti->PCBNum; //currently EAInst needs PCBNum[0] too, in tasks loop to make sure that touch sample timestamps are recent
								//get Analog Sensor num on PCB
								leti->AnalogSensorNum=leti->AnalogSensorNameNum%2+1; //sensornums are 1,2

								//get the next word on this line (GT or LT)
								param=strtok(NULL,",\n )"); //comma,\n,space, or ) delimited
								if (param==0) {
									fprintf(stderr,"No GT or LT on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								//fprintf(stderr,"param=%s, (int)param[0]=%d\n",param,(int)param[0]);
								if (!strncmp(param,"GT",2)) {
									leti->GTorLT=0;
								} else {
									if (!strncmp(param,"LT",2)) {
										leti->GTorLT=1;
									} else {
										fprintf(stderr,"No GT or LT on line %d",CurLine);
										FTMessageBox(tstr,FTMB_OK,"Error",0);
										FreeRobotScript(lscript);
										return(0);
									} //if (!strncmp(param,"LT",2)) {
								} //if (!strncmp(param,"GT",2)) {

								//fprintf(stderr,"Found Accel: %s %s(%d) NameNum=%d pcb=%s AccelNum=%d\n",ROBOT_ACCEL_NAMES[leai->AccelNameNum[j]],param,leai->xyz[j],leai->AccelNameNum[j],ROBOT_ACCEL_NAMES_PCB[leai->AccelNameNum[j]],leai->AccelNum[j]);

								//get the next word on this line (Value)
								param=strtok(NULL,","); //comma delimited
								leti->Value=atof(param);

								if (leti->GTorLT) {
									//fprintf(stderr,"End stage if AnalogSensor %s (Namenum=%d) (num=%d) is < %3.2f\n",leti->AnalogSensorNameNum,ROBOT_ANALOG_SENSOR_NAMES[leti->AnalogSensorNameNum],leti->AnalogSensorNum,leti->Value);
								} else {
									//fprintf(stderr,"End stage if AnalogSensor %s (Namenum=%d) (num=%d) is > %3.2f\n",leti->AnalogSensorNameNum,ROBOT_ANALOG_SENSOR_NAMES[leti->AnalogSensorNameNum],leti->AnalogSensorNum,leti->Value);
								} 

								//get the next word on this line (Delay) - number of ms to wait in stage until checking AnalogSensor value
								param=strtok(NULL,",)"); //comma, or ) delimited
								if (param==0) {
									sprintf(tstr,"No AnalogSensor Delay on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								leti->Delay=atoi(param);
								if (leti->Delay<0) {
									leti->Delay=0;
								} 

								//get the next word on this line (Duration) - needed to know when conditional AnalogSensor inst times out
								//todo: probably this should be moved to the RobotInstruction struct
								param=strtok(NULL,",)"); //comma, or ) delimited
								if (param==0) {
									sprintf(tstr,"No AnalogSensor duration/instruction timeout on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
								} //if (param==0) {
								lemi->Duration=atoi(param);
								if (lemi->Duration<0) {
									lemi->Duration=0;
								} 

								//AnalogSensor insts needs to have a HOLD_ANGLE flag set so that any stage they are in will end correctly
								//todo: change that either by (make a new flag, and/or change the HOLD_ANGLE code in tasks loop)
								//HOLD_ANGLE is now assumed for all insts in a stage///linst->flags |= ROBOT_INSTRUCTION_HOLD_ANGLE; 

								param=strtok(NULL,"\n");  //go to the next line

								//currently all AnalogSensor insts belong to a WAIT_STAGE
								linst->flags|=ROBOT_INSTRUCTION_WAIT_STAGE;

								leai->origflags=leai->flags;  //preserve flags, because scripts can be run more than once, and when run their flags get changed
								linst->origflags=linst->flags;  //preserve flags, because scripts can be run more than once, and when run their flags get changed

							break; //ROBOT_INSTRUCTION_ANALOG_SENSOR
							case ROBOT_INSTRUCTION_START_BALANCING:
								param=strtok(NULL,"\n");  //go to the next line
								//currently we can ignore the rest of the line

								if (!StartBalancing()) {
									fprintf(stderr,"StartBalancing() failed.\n");
								} //								
								tc=GetFTControl("btnBalance");
								if (tc!=0) {
									if (strcmp(tc->text,"Start Balancing")) {
										strcpy(tc->text,"Start Balancing"); //change to start balancing
										DrawFTControl(tc); //and redraw
									} 
								} //if (tc!=0) {
							break;
							case ROBOT_INSTRUCTION_STOP_BALANCING:
								param=strtok(NULL,"\n");  //go to the next line
								//currently we can ignore the rest of the line
								//see if the winTasks Balance button is correctly labeled 
								if (!StopBalancing()) {
									fprintf(stderr,"StopBalancing() failed.\n");
								} //								
								tc=GetFTControl("btnBalance");
								if (tc!=0) {
									if (strcmp(tc->text,"Start Balancing")) {
										strcpy(tc->text,"Start Balancing"); //change to start balancing
										DrawFTControl(tc); //and redraw
									} 
								} //if (tc!=0) {
							break;
							default: //this could only be a programming error- not a user (script) error
									sprintf(tstr,"Programming error: Matched but unknown command on line %d",CurLine);
									FTMessageBox(tstr,FTMB_OK,"Error",0);
									FreeRobotScript(lscript);
									return(0);
							break;	
						} //switch(InstNum) {
					} //if (!strcmp(param,MotorInstructionName[i])) {  
					InstNum++; //go to next possible instruction name
				} //while(match==0 && i<NUM_ROBOT_INSTRUCTIONS) {
				if (!match) {
					sprintf(tstr,"Unknown command %s on line %d\n",param,CurLine);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					FreeRobotScript(lscript);
					return(0);
				} //if (!match) {						
	



			} //if (param[0]!=35) { //# not comment				
		} //			if (param!=0) {

	} //while(CurLine<NumLines) 

	AddRobotScript(lscript);	//add Robot script to Robot's list of scripts


	return(lscript);
} //RobotScript* LoadRobotScript(char *name,char *buf,int bSize) 



int GetLineFromText(char *tline,char *buf)
{
	int i,len;

	len=strlen(buf);
	i=0;
	while(i<len) {
		if (buf[i]==10) {
			tline[i]=10; //include chr(1)?
			tline[i+1]=0; //terminate string  
			return(i);
		} else {
			tline[i]=buf[i];
		}
		i++;
	} //while

	return(len);  //no chr(10) in string
} //int GetLineFromText(char *tline,char *buf)


int UnloadRobotScript(RobotScript *lscript) 
{
	FTControl *tc;

	if (lscript!=0) {
		RemoveRobotScript(lscript);
		FreeRobotScript(lscript);
		//update the dropdown control list of loaded scripts
		tc=GetFTControl("ddScripts");
		if (tc!=0) {
			UpdateScriptList(tc);
			if (tc->ilist!=0) {
				if (tc->ilist->cur>1) {
					tc->ilist->cur--;
					SelectFTItemByNum(tc->ilist,tc->ilist->cur,0);
//		SelectFTItemByName(tc->ilist,lscript->name,0);
				} //if (tc->ilist.cur>1) {
			} //if (tc->ilist!=0) {

		} //if (tc!=0) {
		return(1);
	} //if (lscript!=0) {
	return(0);
} //int UnloadRobotScript(RobotScript *lscript) 


//add (an already allocated) script to the main Robot list of loaded scripts
int AddRobotScript(RobotScript *lscript)
{
	RobotScript *iscript;

	if (lscript==0) {
		return(0);
	}

	if (RStatus.iScript==0) { 
		//no scripts yet
		RStatus.iScript=lscript;
		return(1);
	}
	
	iscript=RStatus.iScript;

	//go to end of list
	while(iscript->next!=0) {
		iscript=iscript->next;
	} //while

	iscript->next=lscript;
	return(1);

} //int AddRobotScript(RobotScript *lscript)

//Remove a script from the main Robot list of scripts
//doesn't free the script- FreeScript() does that
int RemoveRobotScript(RobotScript *lscript)
{
	RobotScript *lscript2,*last;

	lscript2=RStatus.iScript;
	last=0;
	while(lscript2!=0) {
		if (lscript2==lscript) {
			if (last==0) {
				RStatus.iScript=lscript->next;
				return(1);
			} else {
				last->next=lscript->next;
				return(1);
			}//last==0
		} //if (lscript2==lscript) {
		last=lscript2;
		lscript2=lscript2->next;
	} //while
	return(1);
} //int RemoveRobotScript(RobotScript *lscript)


//free any malloc'd memory for a single script
int FreeRobotScript(RobotScript *lscript)
{
	RobotInstruction *linst,*next;

	//scripts
	if (lscript != 0) {
		if (lscript->iRobotInst != 0) {
			//free all Robot instructions
			linst=lscript->iRobotInst;
			while(linst!=0) {
				next=linst->next;
				free(linst);
				linst=next;
			} //while(linst!=0) {
		} //if (lscript->iRobotInst!=0) {

		free(lscript); //finally free the rest of the memory allocated to the script
	} //if (lscript!=0) {

	return(1);
} //int FreeRobotScript(RobotScript *lscript)

//free the malloc'd memory for all loaded scripts
int FreeAllRobotScripts(void)
{
//	RobotScript *lscript;
	FTControl *tc;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"FreeAllRobotScripts()\n");
	}

	while(RStatus.iScript!=0) {
		UnloadRobotScript(RStatus.iScript);  //sets RStatus.iScript=RStatus.iScript->next, and does free, alt is RemoveRobotScript and FreeRobotScript
	}  //while

	tc=GetFTControl("ddScripts");
	if (tc!=0) {
		UpdateScriptList(tc);
	}

	return(1);
} //int FreeAllRobotScripts(void)


//todo: there is sometimes a crash because lscript=0x2, somehow need to lock RStatus when it is being initialized
//fill a dropdown control with Script names
int UpdateScriptList(FTControl *tcontrol)
{
	RobotScript *lscript;
	FTItem titem;

	if (tcontrol!=0) {
		strcpy(tcontrol->text,""); //clear text in dropdown textbox area (should be done in FreeThought?)
		//fill control with script names
		//free any existing itemlist
		DelFTItemList(tcontrol->ilist);
//		strcpy(tcontrol->text,""); //erase any selected script
		lscript=RStatus.iScript;
		while(lscript!=0) {
			if (lscript->flags&ROBOT_SCRIPT_IS_FROM_FILE) { //do not show internal scripts
				memset(&titem,0,sizeof(FTItem));
				strcpy(titem.name,lscript->name);
				AddFTItem(tcontrol,&titem);
			} //if (!(lscript->flags&ROBOT_SCRIPT_IS_FROM_FILE))
			lscript=(RobotScript *)lscript->next;
		} //while
	} //if (tcontrol!=0)
	DrawFTControl(tcontrol); //redraw the dropdown
return(1);
} //int UpdateScriptList(FTControl *tcontrol)


//return a pointer to a RobotScript that matches the name given
RobotScript *GetRobotScript(char *name)
{
	RobotScript *lscript;

	lscript=RStatus.iScript;

	while(lscript!=0) {
		if (!strcmp(lscript->name,name)) {
			return(lscript);
		}
		lscript=lscript->next;
	} //while
	return(0);
} //RobotScript *GetRobotScript(char *name)

int GetEthAccelAcqNum(char *Name)
{
	int i;

	i=0;
	while(i<NUM_ETHACCEL_ACQUISITION_KINDS) {
		if (!strcmp(Name,ETHACCEL_ACQUISITION_KINDS[i])) {
			return(i);
		}
		i++;
	} //while

return(-1);
} //int *GetEthAccelAcqNum(char *)


#if 0 
int GetSensorName(char *Name,int PCBNum,int SensorNum,uint32_t SensorKind)
{
	int SensorNameNum;

	//could go directly from RStatus.EthAccelsPCBInfo[].AccelInfo->ReadableName

	if (SensorKind&ANGLE_KIND_ACCEL) {
		//get accel name num
		SensorNameNum=GetAccelNameNum_By_PCB(PCBNum,SensorNum);
		if (SensorNameNum==-1) {
			fprintf(stderr,"No Accel NameNum for PCBNum=%d AccelNum=%d\n",PCBNum,SensorNum);
			return(0);
		} else {
			strcpy(Name,RStatus.AccelInfo[SensorNameNum].ReadableName);
			return(1);
		}
	} else { //	if (SensorKind&ANGLE_KIND_ACCEL) {
		//potentiometer
		//get pot name num
		SensorNameNum=GetAnalogSensorNameNum_By_PCB(PCBNum,SensorNum);
		if (SensorNameNum==-1) {
			fprintf(stderr,"No Potentiometer NameNum for PCBNum=%d Num=%d\n",PCBNum,SensorNum);
			return(0);
		} else {
			//strcpy(Name,RStatus.AnalogInfo[SensorNameNum].ReadableName);
strcpy(Name,RStatus.AnalogSensorInfo[SensorNameNum].Name);
			return(1);
		}
	} //	if (SensorKind&ANGLE_KIND_ACCEL) {
} //int GetSensorName(char *Name,int PCBNum,int SensorNum)
#endif
