//win_Motors.c
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
#include <sys/socket.h>  //sys/socket.h does not have SO_REUSEADDR
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h> //for gethostbyname
#endif

#include "freethought.h"
#include "Robot.h"
#include "win_Motors.h"
#include "win_Robot.h"
#include "robot_motor_mcu_instructions.h"

#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  //WIN32


#define INITIAL_WIN_MOTOR_STRENGTH 6  //initial motor strength for Motors in motor window
//#define DEFAULT_MOTOR_BUFLEN 512

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons

unsigned int winMotorsFlags;
//winMotorsFlags
#define ROBOT_MOTOR_NUMBER_CHANGE	0x00000001 //m key was pressed, next key is motor number (0-b)
#define ROBOT_MOTOR_STRENGTH_CHANGE	0x00000002 //s key was pressed, next key is motor strength (0-7)
#define ROBOT_MOTOR_DURATION_CHANGE	0x00000004 //d key was pressed, next key is duration (0-b)
#define ROBOT_MOTOR_OPPOSITE_CHANGE	0x00000008 //o key was pressed, next key is c (complement) or p (pair)

int Pair; //when moving a motor with a key press, move the opposite side too
int PairOpposite; //move both motors in opposite directions when Pair is selected
int Complement; //when moving a motor with a key press, move the complementary motor too
int ComplementOpposite; //move both complmentary motors in opposite directions when Complemnent is selected

int winMotors_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winMotors");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winMotors");
	strcpy(twin.title,"Motors");
	strcpy(twin.ititle,"Motors");
	twin.x=630;
	twin.y=50;
	twin.w=550;
	twin.h=240;
	twin.Keypress=(FTControlfuncwk *)winMotors_key;
	twin.AddFTControls=(FTControlfuncw *)winMotors_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winMotors_OnOpen;
	twin.OnClose=(FTControlfuncw *)winMotors_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winMotors_AddFTWindow



int winMotors_AddFTControls(FTWindow *twin)
{
int fw,fh,i;
FTControl *tcontrol,*tcontrol2;
FTItem *titem;
//MAC_Connection *lmac;

#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winMotors'\n");

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
strcpy(tcontrol->name,"lblMotorPCBNum");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"PCB:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
//strcpy(tcontrol->name,"txtMotorPCBName");
strcpy(tcontrol->name,"ddMotorPCBNum");
tcontrol->type=CTDropdown;
tcontrol->x1=fw*6;
tcontrol->y1=fh;
tcontrol->x2=fw*31;
tcontrol->y2=fh*2+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)ddMotorPCBNum_OnChange;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorMACStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*32;
tcontrol->y1=fh;
tcontrol->x2=fw*50;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"MAC: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*51;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtMotorDestIPAddressStr");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*55;
tcontrol->y1=fh;
tcontrol->x2=fw*74;
tcontrol->y2=fh*2+fh/2;
//strcpy(tcontrol->text,"192.168.1.2");
//see if a name exists for this PCB yet
//if (lmac!=0) {
		//strcpy(tcontrol->text,"192.168.1.1");
//	strcpy(tcontrol->text,lmac->DestIPAddressStr);
//} 
//tcontrol->OnChange=(FTControlfuncwc *)txtMotorDestIPAddress_OnChange;
AddFTControl(twin,tcontrol);



//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorNum");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Motor #:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"ddMotorNum");

tcontrol->type=CTDropdown;
//strcpy(tcontrol->text,"1");  //select first item
tcontrol->value=1; //select first item
//tcontrol->FolderTabControl=tFTC;
//tcontrol->FolderTab=tFT;
tcontrol->x1=fw*9;
tcontrol->y1=fh*3;
tcontrol->x2=fw*15;
tcontrol->y2=fh*4+fh/2;
tcontrol->x3=fw*13;
tcontrol->window=twin;
AddFTControl(twin,tcontrol);
tcontrol2=GetFTControl(tcontrol->name);
//add items
titem=(FTItem *)malloc(sizeof(FTItem));
memset(titem,0,sizeof(FTItem));

//titem.hotkey[0]=0x78;  //x
//titem.hotkey[1]=0x58;  //X
//titem.click[0]=(FTControlfuncwcxy *)btnExit_Click;
//tcontrol.ilist=AddFTItem(tcontrol.ilist,&titem);
for(i=0;i<16;i++) {
	sprintf(titem->name,"%x",i);
	AddFTItem(tcontrol2,titem);
}
free(titem);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorDir");
tcontrol->type=CTLabel;
tcontrol->x1=fw*16;
tcontrol->y1=fh*3;
tcontrol->x2=fw*27;
tcontrol->y2=fh*4+fh/2;
strcpy(tcontrol->text,"Direction: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"ddMotorDir");
tcontrol->type=CTDropdown;
tcontrol->value=1; //select first item
tcontrol->x1=fw*28;
tcontrol->y1=fh*3;
tcontrol->x2=fw*45;
tcontrol->y2=fh*4+fh/2;
//tcontrol->Keypress=(FTControlfuncwck *)txtMyMessage_Keypress;
//tcontrol->OnChange=(FTControlfuncwc *)cmbRobotNum_OnChange;
tcontrol->window=twin;
AddFTControl(twin,tcontrol);
//add items
titem=(FTItem *)malloc(sizeof(FTItem));
tcontrol2=GetFTControl("ddMotorDir");
memset(titem,0,sizeof(FTItem));
strcpy(titem->name,"clockwise");
//titem.hotkey[0]=0x78;  //x
//titem.hotkey[1]=0x58;  //X
//titem.click[0]=(FTControlfuncwcxy *)btnExit_Click;
//tcontrol.ilist=AddFTItem(tcontrol.ilist,&titem);
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"counter-clockwise");
AddFTItem(tcontrol2,titem);
free(titem);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblDutyCycleClk");
tcontrol->type=CTLabel;
tcontrol->x1=fw*46;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Duty Cycle Clock: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtDutyCycleClk");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*65;
tcontrol->y1=fh*3;
tcontrol->x2=fw*75;
//strcpy(tcontrol->text,"1000");
sprintf(tcontrol->text,"%d",ROBOT_MOTORS_DEFAULT_MOTOR_DUTY_CYCLE_CLK);
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblDutyCycleClkus");
tcontrol->type=CTLabel;
tcontrol->x1=fw*76;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"us");
AddFTControl(twin,tcontrol);

//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorStrength");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
tcontrol->x2=fw*11;
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,"Strength: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"ddMotorStrength");
tcontrol->type=CTDropdown;
tcontrol->value=INITIAL_WIN_MOTOR_STRENGTH; //select item- initial strength, todo: allow winMotor initial strength to be set in config  
tcontrol->x1=fw*12;
tcontrol->y1=fh*5;
tcontrol->x2=fw*28;//24;
tcontrol->y2=fh*6+fh/2;
//tcontrol->x3=fw*21;
tcontrol->window=twin;
AddFTControl(twin,tcontrol);
//add items
titem=(FTItem *)malloc(sizeof(FTItem));
tcontrol2=GetFTControl("ddMotorStrength");
memset(titem,0,sizeof(FTItem));
strcpy(titem->name,"Stop");
AddFTItem(tcontrol2,titem);
for(i=1;i<21;i++) {
	sprintf(titem->name,"%d/20 speed",i);	
	AddFTItem(tcontrol2,titem);
} //for i
free(titem);



memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblNumClkInDutyCycle");
tcontrol->type=CTLabel;
tcontrol->x1=fw*35;
tcontrol->y1=fh*5;
tcontrol->x2=fw*(35+24);
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,"Num Clks in Duty Cycle: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtNumClkInDutyCycle");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*(35+24);
tcontrol->y1=fh*5;
tcontrol->x2=fw*(35+28);
//strcpy(tcontrol->text,"14");
sprintf(tcontrol->text,"%d",ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE);
AddFTControl(twin,tcontrol);



//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorDuration");

tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*7;
tcontrol->x2=fw*11;
tcontrol->y2=fh*8+fh/2;
strcpy(tcontrol->text,"Duration: ");
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text

AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"ddMotorDuration");
tcontrol->type=CTDropdown;
tcontrol->value=3; //3=100 ms   select item
//tcontrol->FolderTabControl=tFTC;
//tcontrol->FolderTab=tFT;
tcontrol->x1=fw*12;
tcontrol->y1=fh*7;
tcontrol->x2=fw*22;
tcontrol->y2=fh*8+fh/2;
tcontrol->x3=fw*20;
tcontrol->window=twin;
AddFTControl(twin,tcontrol);
//add items
titem=(FTItem *)malloc(sizeof(FTItem));
tcontrol2=GetFTControl("ddMotorDuration");
memset(titem,0,sizeof(FTItem));
strcpy(titem->name,"0");  //to stop motor when using arrow keys
//titem.hotkey[0]=0x78;  //x
//titem.hotkey[1]=0x58;  //X
//titem.click[0]=(FTControlfuncwcxy *)btnExit_Click;
//tcontrol.ilist=AddFTItem(tcontrol.ilist,&titem);
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"50");
//titem.hotkey[0]=0x78;  //x
//titem.hotkey[1]=0x58;  //X
//titem.click[0]=(FTControlfuncwcxy *)btnExit_Click;
//tcontrol.ilist=AddFTItem(tcontrol.ilist,&titem);
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"100");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"150");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"200");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"250");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"350");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"500");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"750");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"1000");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"2000");
AddFTItem(tcontrol2,titem);
strcpy(titem->name,"3000");
AddFTItem(tcontrol2,titem);

free(titem);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMotorDuration2");
tcontrol->type=CTLabel;
tcontrol->x1=fw*23;
tcontrol->y1=fh*7;
tcontrol->x2=fw*26;
tcontrol->y2=fh*8+fh/2;
strcpy(tcontrol->text,"ms");
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);


//Both sides - when moving a motor, move its complimentary motor if one exists
//for example move left foot also moves right foot, move right knee also moves left knee
//and in the direction that causes the connected joints to move in the same direction
//for example left foot moves clockwise and right foot moves counterclockwise, 
//but left leg and right leg both move clockwise or counterclockwise together



//move both complementary motors simultaneously
Complement=0;
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkComplement");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*28;
tcontrol->y1=fh*7;
tcontrol->x2=fw*(38+3);
tcontrol->y2=fh*8+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)chkComplement_OnChange;
strcpy(tcontrol->text,"Complement");
AddFTControl(twin,tcontrol);

//move both complementary motors in opposite directions simultaneously
ComplementOpposite=0;
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkCompOpp");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*42;
tcontrol->y1=fh*7;
tcontrol->x2=fw*(50+3);
tcontrol->y2=fh*8+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)chkCompOpp_OnChange;
strcpy(tcontrol->text,"Comp opp");
AddFTControl(twin,tcontrol);

//more both paired motors simultaneously
Pair=0;
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkPair");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*54;
tcontrol->y1=fh*7;
tcontrol->x2=fw*(58+3);
tcontrol->y2=fh*8+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)chkPair_OnChange;
strcpy(tcontrol->text,"Pair");
AddFTControl(twin,tcontrol);


//move both paired motors in opposite directions when Pair is selected
PairOpposite=0;
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkPairOpp");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*62;
tcontrol->y1=fh*7;
tcontrol->x2=fw*(70+3);
tcontrol->y2=fh*8+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)chkPairOpp_OnChange;
strcpy(tcontrol->text,"Pair Opp");
AddFTControl(twin,tcontrol);



//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblSendInstructionToMotorRaw");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*9;
tcontrol->x2=fw*47; //error: for some reason this needs to be longer than the text box position
tcontrol->y2=fh*10+fh/2;
strcpy(tcontrol->text,"Send raw instruction: (m:4 dir:1 str:4 dur:16)"); //46 chars
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtSendInstructionToMotorRaw");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*48;
tcontrol->y1=fh*9;
tcontrol->x2=fw*60;
tcontrol->y2=fh*10+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
//tcontrol->color[0]=white;  //bkground
//tcontrol->color[1]=black;  //frame
//tcontrol->textcolor[0]=black;//text
//tcontrol->textcolor[1]=white;//text background
//tcontrol->textcolor[2]=white;//selected text
//tcontrol->textcolor[3]=blue;//selected text background
AddFTControl(twin,tcontrol);


//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGetMotorData");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*11;
tcontrol->x2=fw*17;
tcontrol->y2=fh*12+fh/2;
strcpy(tcontrol->text,"Data received: ");
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGetMotorData");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*18;
tcontrol->y1=fh*11;
tcontrol->x2=fw*28;
tcontrol->y2=fh*12+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
//tcontrol->color[0]=white;  //bkground
//tcontrol->color[1]=black;  //frame
//tcontrol->textcolor[0]=black;//text
//tcontrol->textcolor[1]=white;//text background
//tcontrol->textcolor[2]=white;//selected text
//tcontrol->textcolor[3]=blue;//selected text background
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkDisableMotors");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*30;
tcontrol->y1=fh*11;
tcontrol->x2=fw*47;
tcontrol->y2=fh*12+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)chkDisableMotors_OnChange;
if (RStatus.flags&ROBOT_STATUS_MOTORS_DISABLED) {
	tcontrol->value=1;
}
strcpy(tcontrol->text,"Disable Motors");
AddFTControl(twin,tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSendInstructionToMotor");
tcontrol->type=CTButton;
tcontrol->x1=fw*21;
tcontrol->y1=fh*13;
tcontrol->x2=fw*40;
tcontrol->y2=fh*18;
strcpy(tcontrol->text,"Send Instruction");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSendInstructionToMotor_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);

//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinMotorsExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*48;
tcontrol->y1=fh*13;
tcontrol->x2=fw*68;
tcontrol->y2=fh*18;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinMotorsExit_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);


#if 0 
//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnMotorLeftArrow");

tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*22;
tcontrol->x2=fw*20;
tcontrol->y2=fh*26;
//tcontrol->image=FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//tcontrol->image=FTLoadBitmap("images/left_arrow_01.bmp");
strcpy(tcontrol->text,"<");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnMotorLeftArrow_Click;

tcontrol->color[0]=ltgray; //button color background no press
tcontrol->color[1]=dkgray; //button down color (and toggle color)
tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnMotorStop");

tcontrol->type=CTButton;
tcontrol->x1=fw*21;
tcontrol->y1=fh*22;
tcontrol->x2=fw*40;
tcontrol->y2=fh*26;
//tcontrol->image=FTLoadBitmapAndScale("images/stop_01.bmp",tcontrol->x2-tcontrol->x1,tcontrol->y2-tcontrol->y1);
strcpy(tcontrol->text,"STOP");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnMotorStop_Click;

tcontrol->color[0]=ltgray; //button color background no press
tcontrol->color[1]=dkgray; //button down color (and toggle color)
tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnMotorRightArrow");

tcontrol->type=CTButton;
tcontrol->x1=fw*41;
tcontrol->y1=fh*22;
tcontrol->x2=fw*60;
tcontrol->y2=fh*26;
//tcontrol->image=FTLoadBitmapAndScale("images/right_arrow_01.bmp",tcontrol->x2-tcontrol->x1,tcontrol->y2-tcontrol->y1);
strcpy(tcontrol->text,">");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnMotorRightArrow_Click;

tcontrol->color[0]=ltgray; //button color background no press
tcontrol->color[1]=dkgray; //button down color (and toggle color)
tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);

#endif

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



int Populate_EthMotors_Dropdown(FTControl *tc)
{
int NumMotorPCBs;
FTControl *tc2;
FTItem *titem;
MAC_Connection *lmac,*tmac;


//Delete all existing dropdown items
DelAllFTItems(tc);
//Go through each MAC_Connection and get all names that exist for this PCB yet
//lmac=Get_MAC_Connection_By_WindowFunction((FTControlfunc *)winMotors_AddFTWindow);
NumMotorPCBs=0;
if (tc!=0) {
	titem=(FTItem *)malloc(sizeof(FTItem));
	lmac=RStatus.iMAC_Connection;	
	//for all the MAC_Connections, compare the AddWindowFunction (could compare name to Motor too)
	while(lmac!=0) {
		if (lmac!=0) {
			if (lmac->AddWindowFunction==(FTControlfunc *)winMotors_AddFTWindow) {
				//strcpy(tcontrol->text,"192.168.1.1");
				memset(titem,0,sizeof(FTItem));
				sprintf(titem->name,"%s",lmac->pcb.Name);
				//sprintf(titem->name,"%i",lmac->Num);
				//strcpy(titem->name,lmac->Name);
				AddFTItem(tc,titem);
				if (NumMotorPCBs==0) {
					tmac=lmac; //remember the first Motor PCB
				}
				NumMotorPCBs++;
			} //if (lmac->AddWindowFunction==(FTControlfunc *)winMotors_AddFTWindow) {
		} 
		lmac=lmac->next;
	}//while(lmac!=0) {
	free(titem);
	if (NumMotorPCBs>0) {
		SelectFTItemByNum(tc->ilist,1,0); //select the first item
		lmac=tmac;
// is done in ddMotorPCBNum_OnChange, but only when there is a change
		//populate Mac address and IP
		tc2=GetFTControl("lblMotorMACStr");
		if (tc2!=0) {		
			sprintf(tc2->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc2);
		}
		tc2=GetFTControl("txtMotorDestIPAddressStr");
		if (tc2!=0) {		
			sprintf(tc2->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc2);
		}

	} //if (NumMotorPCBs>0) {

	//redraw dropdown
	DrawFTControl(tc);
	return(NumMotorPCBs);
} //tc!=0
return(0);
} //int Populate_EthMotors_Dropdown(FTControl *tc)


void winMotors_key(FTWindow *twin,KeySym key)
{
FTControl *tc;
int MotorNum,PCBNum,MotorStrength,MotorDuration;

    switch (key)	{
		case XK_Shift_L://-30:
		case XK_Shift_R: //-31:  //ignore shift keys
        break;
      case XK_Escape:	/*esc*/
				if (twin->FocusFTControl==0) {  //esc key and no control has focus
					//PStatus.flags|=PExit;
					//CloseFTWindow(twin);
					StopBalancing();
				}	     
        break;
      case XK_Left: //0xff51:  //left arrow key
#if Linux
      case XK_KP_Left: //0xff96:  //key pad left arrow (no num lock)
#endif
				if (twin->FocusFTControl==0) {  //only if no other controls have focus

					//change direction to counter-clockwise and send
					tc=GetFTControl("ddMotorDir");
					if (tc!=0) {
						tc->value=2;
						DrawFTControl(tc); //redraw control
						//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
						SendUserSettingsInstruction();
					}
				} //				if (twin->FocusFTControl==0) {  //only if no other controls have focus
        break;
      case XK_Right: //0xff52:  //right arrow key
#if Linux
      case XK_KP_Right: //0xff98:  //key pad right arrow (no num lock)
#endif
				if (twin->FocusFTControl==0) {  //only if no other controls have focus

					//change direction to counter-clockwise and send
					tc=GetFTControl("ddMotorDir");
					if (tc!=0) {
						tc->value=1;
						DrawFTControl(tc); //redraw control
						//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
						SendUserSettingsInstruction();
					}
				} //if (twin->FocusFTControl==0) {

        break;
      case XK_Down: //0xff54:  //down arrow key
#if Linux
      case XK_KP_Down: //0xff99:  //key pad down arrow (no num lock)
#endif
      case XK_KP_Begin: //0xff9d:  //key pad middle key (no num lock)
				//change duration of motor to 0 (stop)
				//tc=GetFTControl("ddMotorDuration");
				if (twin->FocusFTControl==0) {  //only if no other controls have focus
					tc=GetFTControl("ddMotorStrength");
					if (tc!=0) {
						tc->value=1;  //strength=0 (stop)
						DrawFTControl(tc); //redraw control
						//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
						SendUserSettingsInstruction();
					}
				} //				if (twin->FocusFTControl==0) {  //only if no other controls have focus
        break;
		case XK_KP_0: //0xffb0:  //0 key
		case XK_KP_1: //0xffb1:  //1 key
		case XK_KP_2: //0xffb2:  //2 key
		case XK_KP_3: //0xffb3:  //3 key
		case XK_KP_4: //0xffb4:  //4 key
		case XK_KP_5: //0xffb5:  //5 key
		case XK_KP_6: //0xffb6:		//6 key
		case XK_KP_7: //0xffb7:		//7 key
		case XK_KP_8: //0xffb8:		//8 key
		case XK_KP_9: //0xffb9:		//9 key
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
		case XK_A: //A key
#if Linux
		case XK_a: //a key
#endif
		case XK_B: 
#if Linux
		case XK_b: 
#endif
		case XK_C: 
#if Linux
		case XK_c: 
#endif
		case XK_D: 
#if Linux
		case XK_d: 
#endif
		case XK_E: 
#if Linux
		case XK_e: 
#endif
		case XK_F: 
#if Linux
		case XK_f: 
#endif

			if (winMotorsFlags&ROBOT_MOTOR_NUMBER_CHANGE) {
				//just pressed "m" key- change motor number
				winMotorsFlags&=~ROBOT_MOTOR_NUMBER_CHANGE;
				tc=GetFTControl("ddMotorNum");
				if (tc!=0) {
					MotorNum=(int)strtol((char *)&key,0,16);
					SelectFTItemByNum(tc->ilist,MotorNum+1,0);
					DrawFTControl(tc); //redraw control
				}
			} else { //if (winMotorsFlags&ROBOT_MOTOR_NUMBER_CHANGE) {
				//not changing a motor


#if Linux
				if (key==XK_d || key==XK_D) {
#endif
#if WIN32
				if (key==XK_D) {
#endif
					//if D key, toggle duration flag
					winMotorsFlags|=ROBOT_MOTOR_DURATION_CHANGE;
				} else {

					if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
						//change motor strength
						winMotorsFlags&=~ROBOT_MOTOR_STRENGTH_CHANGE;
						tc=GetFTControl("ddMotorStrength");
						if (tc!=0) {
							MotorStrength=(int)strtol((char *)&key,0,16);
							SelectFTItemByNum(tc->ilist,MotorStrength+1,0);
							DrawFTControl(tc); //redraw control
						}
					} else { //if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {

						if (winMotorsFlags&ROBOT_MOTOR_DURATION_CHANGE) {
							//change motor number
							winMotorsFlags&=~ROBOT_MOTOR_DURATION_CHANGE;
							tc=GetFTControl("ddMotorDuration");
							if (tc!=0) {
								MotorDuration=(int)strtol((char *)&key,0,16);
								SelectFTItemByNum(tc->ilist,MotorDuration+1,0);
								DrawFTControl(tc); //redraw control
							}
						} else { //if (winMotorsFlags&ROBOT_MOTOR_DURATION_CHANGE) {
							//B - move through EthMotors PCBs
#if Linux
							if (key==XK_b || key==XK_B) {
#endif
#if WIN32
							if (key==XK_B) {
#endif

								tc=GetFTControl("ddMotorPCBNum");
								if (tc!=0 && tc->ilist!=0) {					
									PCBNum=tc->ilist->cur;
				//					fprintf(stderr,"tc->ilist->cur=%d num=%d\n",tc->ilist->cur,tc->ilist->num);
									if (PCBNum+1>tc->ilist->num) {
										PCBNum=1;
									} else {
										PCBNum++;
									}
									SelectFTItemByNum(tc->ilist,PCBNum,0);
									DrawFTControl(tc); //redraw control
									ddMotorPCBNum_OnChange(twin,tc); //update MAC and IP
								}
							} else { //if (key==XK_b || key==XK_B) {
								//C - toggle complementary motors checkbox
#if Linux
								if (key==XK_c || key==XK_C) {
#endif
#if WIN32
								if (key==XK_C) {
#endif
									//check to see if "O" was pressed last
									if (winMotorsFlags&ROBOT_MOTOR_OPPOSITE_CHANGE) {
										//toggle "Comp Opp" checkbox
										tc=GetFTControl("chkCompOpp");
										if (tc!=0) {
											tc->value=!tc->value;
											ComplementOpposite=!ComplementOpposite; 
											DrawFTControl(tc);
										}		
										winMotorsFlags&=~ROBOT_MOTOR_OPPOSITE_CHANGE; //reset O flag

									} else {

										//toggle "Complement" checkbox
										tc=GetFTControl("chkComplement");
										if (tc!=0) {
											tc->value=!tc->value;
											Complement=!Complement; 
											DrawFTControl(tc);
										}		
									} //if (winMotorsFlags&ROBOT_MOTOR_OPPOSITE_CHANGE) {
								} //if (key==XK_c || key==XK_C) {
							} //if (key==XK_b || key==XK_B) {
						} //if (winMotorsFlags&ROBOT_MOTOR_DURATION_CHANGE) {
					} //if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
				} //if (winMotorsFlags&ROBOT_MOTOR_NUMBER_CHANGE) {
			} //if (key==XK_d || key==XK_D) {


			//change motor to # of keypress if no control is selected
//			if (twin->FocusFTControl==0) {
//			} //if (twin->FocusFTControl==0) {
    break;
		case XK_I: //I key
#if Linux
		case XK_i: //i key
#endif
			if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
				//change motor strength
				winMotorsFlags&=~ROBOT_MOTOR_STRENGTH_CHANGE;
				tc=GetFTControl("ddMotorStrength");
				if (tc!=0) {
					MotorStrength=(int)strtol((char *)&key,0,16);
					SelectFTItemByNum(tc->ilist,MotorStrength+1,0);
					DrawFTControl(tc); //redraw control
				}
			} //if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
		break; //I key

		case XK_J: //J key
#if Linux
		case XK_j: //j key
#endif
			if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
				//change motor strength
				winMotorsFlags&=~ROBOT_MOTOR_STRENGTH_CHANGE;
				tc=GetFTControl("ddMotorStrength");
				if (tc!=0) {
					MotorStrength=(int)strtol((char *)&key,0,16);
					SelectFTItemByNum(tc->ilist,MotorStrength+1,0);
					DrawFTControl(tc); //redraw control
				}
			} //if (winMotorsFlags&ROBOT_MOTOR_STRENGTH_CHANGE) {
		break; //J key

		case XK_M: 
#if Linux
		case XK_m: 
#endif
			//change motor number depending on next key pressed
			if (twin->FocusFTControl==0) {
				//record current time for timeout for second key press

				if (winMotorsFlags&ROBOT_MOTOR_NUMBER_CHANGE) {
					//reset change motor flag 
					winMotorsFlags&=~ROBOT_MOTOR_NUMBER_CHANGE;
				} else {
					//set change motor flag
					winMotorsFlags|=ROBOT_MOTOR_NUMBER_CHANGE;
				}
			} //if (twin->FocusFTControl==0) {
		break;


//O - toggle opposite dir (move paired motors in opposite directions when Both Sides is checked)
		case XK_O: //O key
#if Linux
		case XK_o: //o key
#endif

			//opposite is selected, change either c complemnent or p pair checkbox depending on next key pressed
			if (twin->FocusFTControl==0) {
				//record current time for timeout for second key press

				if (winMotorsFlags&ROBOT_MOTOR_OPPOSITE_CHANGE) {
					//reset change opposite flag 
					winMotorsFlags&=~ROBOT_MOTOR_OPPOSITE_CHANGE;
				} else {
					//set change opposite flag
					winMotorsFlags|=ROBOT_MOTOR_OPPOSITE_CHANGE;
				}
			} //if (twin->FocusFTControl==0) {


		break; //case XK_o: //o key


//P - toggle paired motor checkbox
		case XK_P: //P key
#if Linux
		case XK_p: //p key
#endif

			//check to see if "O" was pressed last
			if (winMotorsFlags&ROBOT_MOTOR_OPPOSITE_CHANGE) {
				//toggle "Pair Opp" checkbox
				tc=GetFTControl("chkPairOpp");
				if (tc!=0) {
					tc->value=!tc->value;
					PairOpposite=!PairOpposite; 
					DrawFTControl(tc);
				}		
				winMotorsFlags&=~ROBOT_MOTOR_OPPOSITE_CHANGE; //reset O flag


			} else {
				//toggle "Pair" checkbox
				tc=GetFTControl("chkPair");
				if (tc!=0) {
					tc->value=!tc->value;
					Pair=!Pair; //currently there is no SelectCheckBox() UnselectCheckBox() to call OnChange- when the value is changed programmatically
					DrawFTControl(tc);
				}		
			} //if (winMotorsFlags&ROBOT_MOTOR_OPPOSITE_CHANGE) {
		break;


		case XK_S: //S key
#if Linux
		case XK_s: //s key
#endif
			//change motor pulse strength depending on next key pressed
			if (twin->FocusFTControl==0) {
				//record current time for timeout for second key press

				//set change motor speed flag
				winMotorsFlags|=ROBOT_MOTOR_STRENGTH_CHANGE;
			} //if (twin->FocusFTControl==0) {
		break;



    default:
			//fprintf(stderr,"key=%x\n",key);
    break;
    }
} //void winMotors_key(FTWindow *twin,KeySym key)

int ddMotorPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol) {
	MAC_Connection *lmac;
	FTControl *tc;
	
	//fprintf(stderr,"OnChange\n");
	//if the user changed Motor PCBs update the MAC and IP
	lmac=Get_MAC_Connection_By_PCBName(tcontrol->text); //get remote MAC_Connection
	if (lmac!=0) {
		//MAC
		tc=GetFTControl("lblMotorMACStr");
		if (tc) {
			sprintf(tc->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc);
		} //tc
		//IP
		tc=GetFTControl("txtMotorDestIPAddressStr");
		if (tc) {
			sprintf(tc->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc);
		} //tc
		return(1);
	} //if (lmac==0) {
	return(0);
} //int ddMotorPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol) {

void btnSendInstructionToMotor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	FTControl *tc;
	int NumChars,NumBytes;
	unsigned char inst[512];
	int SentRawInstruction;

	//check to see if there is an instruction in the Send raw instruction text box

	SentRawInstruction=0;
	tc=GetFTControl("txtSendInstructionToMotorRaw");

	if (tc!=0) {
		if (strlen(tc->text)>0) { //there is a command there
			SentRawInstruction=1;
			//presumes instruction is in hexadecimal (without 0x)
			NumChars=strlen(tc->text);
			if (NumChars>0) {
				//convert ascii to numbers
				NumBytes=ConvertStringToBytes((unsigned char *)tc->text,inst); //NumBytes=NumChars/2
/*
				tkey[1]=0;
				for(i=0;i<NumChars/2;i++) {
					tkey[0]=tc->text[i*2];
					inst[i]=strtol(tkey,0,16);
					inst[i]<<=4;
					tkey[0]=tc->text[i*2+1];
					inst[i]|=strtol(tkey,0,16);//atoi(tkey);				
				} //for(i=0;i<NumChars/2;i++) {
*/
				//SendInstructionToMotor(inst,NumChars/2,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
				SendUserInstructionToMotor(inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
			} //if (NumChars>0) {
		} //	if (strlen(tc->text)>0) { //there is a command there
	} //	if (tc!=0) {

	if (!SentRawInstruction) {
	//send instruction based on dropdown boxes
		SendUserSettingsInstruction();
	} //if (!SentRawInstruction)
}  //void btnSendInstructionToMotor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int SendUserSettingsInstruction(void)
{
	FTControl *tc;
	int MotorNum,MotorDir,MotorStrength,MotorDur,MotorNum2,MotorNum2c;
	int DutyCycleClock,NumClksInDutyCycle;
	unsigned char inst[512];
	MAC_Connection *lmac;
//	MotorPair *lmi;
	int MotorNameNum,MotorNameNum2,MotorNameNum2c;

	//first check to see if the Duty Cycle Clock has changed 
	//and send an instruction for the PIC to update it's duty cycle clock register (PR3)

	
	tc=GetFTControl("txtDutyCycleClk");
	if (tc!=0) {
		if (strlen(tc->text)>0) {
			DutyCycleClock=atoi(tc->text);  //in us
			if (DutyCycleClock!=RStatus.MotorDutyCycleClock) {
				//update Motor duty cycle clock
	
			//for each leg:
				RStatus.MotorDutyCycleClock=DutyCycleClock;
				inst[0]=ROBOT_MOTORS_SET_MOTOR_DUTY_CYCLE_CLK_IN_USEC;
				//int value is sent Little Endian
				*(unsigned int *)(&inst[1])=DutyCycleClock;
				fprintf(stderr,"Updating the motor duty cycle clock to %d us\n",DutyCycleClock);
				SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
/*
				inst[1]=(DutyCycleClock&0xff);
				inst[2]=(DutyCycleClock&0xff00)>>0x8;
				inst[3]=(DutyCycleClock&0xff0000)>>0x10;
				inst[4]=(DutyCycleClock&0xff000000)>>0x18;
*/


			} //if (DutyCycleClock!=RStatus.MotorDutyCycleClock) {
		} //if (strlen(tc->text)>0) {
	}  //if (tc!=0) {


	tc=GetFTControl("txtNumClkInDutyCycle");
	if (tc!=0) {
		if (strlen(tc->text)>0) {
			NumClksInDutyCycle=atoi(tc->text);  //in us
			if (NumClksInDutyCycle!=RStatus.NumClksInMotorDutyCycle) {
				//update Motor duty cycle clock
				RStatus.NumClksInMotorDutyCycle=NumClksInDutyCycle;
				inst[0]=ROBOT_MOTORS_SET_NUM_CLK_IN_MOTOR_DUTY_CYCLE;
				//int value is sent Little Endian
				*(unsigned int *)(&inst[1])=NumClksInDutyCycle;
				fprintf(stderr,"Updating number of clock ticks in motor duty cycle to: %d\n",NumClksInDutyCycle);
				SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
/*
				inst[1]=(NumClksInDutyCycle&0xff);
				inst[2]=(NumClksInDutyCycle&0xff00)>>0x8;
				inst[3]=(NumClksInDutyCycle&0xff0000)>>0x10;
				inst[4]=(NumClksInDutyCycle&0xff000000)>>0x18;
*/
			} //if (NumClockInDutyCycle!=RStatus.NumClksInMotorDutyCycle) {
		}//if (strlen(tc->text)>0) {
	}  //if (tc!=0) {


	MotorNum=0;
	tc=GetFTControl("ddMotorNum");
	if (tc!=0) {
		MotorNum=tc->value-1;
	}

	MotorDir=0;
	tc=GetFTControl("ddMotorDir");
  if (tc!=0) {
		MotorDir=tc->value-1;
	}

	MotorStrength=3;
	tc=GetFTControl("ddMotorStrength");
	if (tc!=0) {
		MotorStrength=tc->value-1;  //0=stop
		//fprintf(stderr,"Motor Strength=%d\n",MotorStrength);
			//for each leg:
	}

	MotorDur=100;
	tc=GetFTControl("ddMotorDuration");
	if (tc!=0) {
		if (strlen(tc->text)>0) {
			//MotorDur=atoi(tc->text)/10;  //instruction duration is in centiseconds
			//instructions are now translated to number of Duty Cycle Clock ticks
			//  so 100ms with 1000us clock is 100, with 500us clock=200
			//MotorDur=atoi(tc->text)*1000/RStatus.MotorDutyCycleClock;  //instruction duration is divided by the Duty Cycle Clock because
			MotorDur=atoi(tc->text);  //send instruction duration in ms- don't send in duty cycle clocks, because the PIC firmware DutyCycle may not be the same and that might cause a motor to mistakenly run for a long time
			//that clock is Timer2 the interrupt for all motors.

		}
	}

  //fprintf(stderr,"MotorNum=%d Dir=%d Strength=%d Duration=%d\n",MotorNum,MotorDir,MotorStrength,MotorDur);

	//key[0]=(MotorNum<<4)|(MotorDir<<3)|MotorStrength;
	//key[1]=MotorDur&0xff;  //send Litle Endian
	//key[2]=(MotorDur&0xff00)>>8;

	//SendInstructionToRobot(ROBOT_MOTORS_SEND_B24_INST,key);
	inst[0]=ROBOT_MOTORS_SEND_4BYTE_INST;
	inst[1]=(MotorNum<<4);
//	inst[2]=(MotorDir<<7)|(MotorStrength*ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE/7); //scale to 1-7 from 1-20
	inst[2]=(MotorDir<<7)|(MotorStrength); //0-19
	inst[3]=(unsigned char)(MotorDur&0xff); //duration is sent little endian
	inst[4]=(unsigned char)((MotorDur&0xff00)>>0x8);

/*
	inst[1]=(MotorNum<<4)|(MotorDir<<3)|MotorStrength;
	inst[2]=(unsigned char)(MotorDur&0xff); //duration is sent little endian
	inst[3]=(unsigned char)((MotorDur&0xff00)>>0x8);
*/
	SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);


	//Send similar instruction to complementary motor if "Copmlement" is checked and there is a complementary motor.
	if (Complement) {
		//fprintf(stderr,"Complement\n");

		//first get mac
		tc=GetFTControl("ddMotorPCBNum");
		if (tc!=0) {
			lmac=Get_MAC_Connection_By_PCBName(tc->text); //get remote MAC_Connection
			if (lmac!=0) {
				//if this motor has a complementary motor, get the complementary Motor NameNum, then use that to get the Motor num (on PCB)
				MotorNameNum=GetMotorNameNumByPCBNum(lmac->pcb.Num,MotorNum);
				if (RStatus.MotorInfo[MotorNameNum].Complement[0]!=0) { //there is a complementary motor
					MotorNameNum2= GetMotorNameNum(RStatus.MotorInfo[MotorNameNum].Complement);
					MotorNum2=RStatus.MotorInfo[MotorNameNum2].Num;
					//see if motor cw and ccw directions are opposite
					if ((RStatus.MotorInfo[MotorNameNum].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MotorNameNum2].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)) {
						MotorDir=!MotorDir;
					}
					//motors need to move in opposite directions
					if (ComplementOpposite) {
						MotorDir=!MotorDir;
					}

					inst[1]=(MotorNum2<<4);
					inst[2]=(MotorDir<<7)|MotorStrength; //0-19
					SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
				} else {  //if (RStatus.MotorInfo[MotorNameNum].Complement[0]!=0) { //there is a complementary motor
					//do not print because some motors may not have opposite motors
					//fprintf(stderr,"Could not get MotorPair for PCB:%d MotorNum:%d. Has no opposite motor.\n",lmac->pcb.PCBNum,MotorNum);
				} //else if (RStatus.MotorInfo[MotorNameNum].Complement[0]!=0) { //there is a complementary motor
			} else {
				//this can mean that there is no opposite motor (as is the case for the neck and head motors)
				//fprintf(stderr,"Could not get MAC_Connection for PCB Name: %s\n",tc->text);
			} //if (lmac!=0) {
			//return(0);
		} //if (tc!=0) {

	} //if (Complement) {


	//Send similar instruction to paired motor if "Pair" is checked and there is a paired motor.
	if (Pair) {
		//fprintf(stderr,"Pair\n");
		//the "Both Sides" option is selected
		//send a similar instruction to the complimentary motor if one exists
		//get MotorPair record for this motor

		//first get mac
		tc=GetFTControl("ddMotorPCBNum");
		if (tc!=0) {
			lmac=Get_MAC_Connection_By_PCBName(tc->text); //get remote MAC_Connection
			if (lmac!=0) {
				//if this motor has a paired motor, get the paired Motor NameNum, then use that to get the Motor num (on PCB)
				MotorNameNum=GetMotorNameNumByPCBNum(lmac->pcb.Num,MotorNum);

				if (RStatus.MotorInfo[MotorNameNum].Pair[0]!=0) { //there is a paired motor
					MotorNameNum2= GetMotorNameNum(RStatus.MotorInfo[MotorNameNum].Pair);
					MotorNum2=RStatus.MotorInfo[MotorNameNum2].Num;
					//see if motor cw and ccw directions are opposite
					if ((RStatus.MotorInfo[MotorNameNum].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MotorNameNum2].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)) {
						MotorDir=!MotorDir;
					}
					//motors need to move in opposite directions
					if (PairOpposite) {
						MotorDir=!MotorDir;
					}

//					if (RStatus.MotorInfo[MotorNameNum].flags&MOTOR_INFO_TABLE_MOTOR_PAIR_OPPOSITE) {
//						MotorDir=!MotorDir;
//					}
//					inst[1]=(MotorNum2<<4)|(MotorDir<<3)|MotorStrength;
					inst[1]=(MotorNum2<<4);
					//inst[2]=(MotorDir<<7)|(MotorStrength*ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE/7); //scale to 0-7
					inst[2]=(MotorDir<<7)|MotorStrength; //0-19
					SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);


					//if the paired motor has a complementary motor, turn that motor too
					if (RStatus.MotorInfo[MotorNameNum2].Complement[0]!=0) { //there is a complementary motor
						MotorNameNum2c= GetMotorNameNum(RStatus.MotorInfo[MotorNameNum2].Complement);
						MotorNum2c=RStatus.MotorInfo[MotorNameNum2c].Num;
						//see if motor cw and ccw directions are opposite
						if ((RStatus.MotorInfo[MotorNameNum2].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MotorNameNum2c].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)) {
							MotorDir=!MotorDir;
						}
						//motors need to move in opposite directions
						if (ComplementOpposite) {
							MotorDir=!MotorDir;
						}

						inst[1]=(MotorNum2c<<4);
						inst[2]=(MotorDir<<7)|MotorStrength; //0-19
						SendUserInstructionToMotor(inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
					} else {  //if (RStatus.MotorInfo[MotorNameNum].Complement[0]!=0) { //there is a complementary motor
						//do not print because some motors may not have opposite motors
						//fprintf(stderr,"Could not get MotorPair for PCB:%d MotorNum:%d. Has no opposite motor.\n",lmac->pcb.PCBNum,MotorNum);
					} //else if (RStatus.MotorInfo[MotorNameNum].Complement[0]!=0) { //there is a complementary motor


				} else {  //if (RStatus.MotorInfo[MotorNameNum].Pair[0]!=0) { //there is a paired motor
					//do not print because some motors may not have opposite motors
					//fprintf(stderr,"Could not get MotorPair for PCB:%d MotorNum:%d. Has no opposite motor.\n",lmac->pcb.PCBNum,MotorNum);
				} //else if (RStatus.MotorInfo[MotorNameNum].Pair[0]!=0) { //there is a paired motor




			} else {
				//this can mean that there is no opposite motor (as is the case for the neck and head motors)
				//fprintf(stderr,"Could not get MAC_Connection for PCB Name: %s\n",tc->text);
			} //if (lmac!=0) {
			//return(0);
		} //if (tc!=0) {



	} //if (Pair) {


	return(1);
} //int SendUserSettingsInstruction(void)



//Thread_TurnMotor, which runs every 100us will then execute a single TurnMotor inst (if one is available), and move the pointer forward to the next position in the motor instruction queue
#if USE_RT_PREEMPT
int Thread_TurnMotor(void) 
{

	struct timespec t;
	struct sched_param param;
	int keep_on_running;
	MAC_Connection *lmac;
	unsigned char *inst;

	fprintf(stderr,"Starting TurnMotor thread\n");

	keep_on_running=1;

	while(!(RStatus.flags&ROBOT_STATUS_TASK_THREAD_RUNNING));  //wait for flag to be set

	//MaxThrust=ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE-5; //current I presume MaxThrust does not change (20-5=16) and is the same for all EthMotors
	//in my experience if a motor cannot move an accelerometer with 10/20 (3/7) then more thrust will not matter.

	param.sched_priority = RT_PRIORITY;
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		fprintf(stderr, "Error: In Thread_TurnMotor() sched_setscheduler failed.\n");
	}

	//Lock memory
	/*
	if (mlockall(MCL_CURRENT_MCL_FUTURE) == -1) {
	fprintf(stderr, "Error: In Thread_TurnMotor() mlockall failed.\n");
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
		t.tv_nsec += TurnMotorTimerInterval; //100us
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}

		//set keep_on_running to 0 if you want to exit
		if (!(RStatus.flags&ROBOT_STATUS_TURN_MOTOR_THREAD_RUNNING)) {
			keep_on_running = 0;
		} //


		//if the queue start and end are different, then there is at least one TurnMOtor instruction to send
		if (RStatus.TurnMotorEnd != RStatus.TurnMotorStart) {
			//send a TurnMotor instruction
			lmac=RStatus.TurnMotorInst[RStatus.TurnMotorStart].mac;
			//fprintf(stderr,"TurnMotor lmac=%x PCB=%sStart=%d, inst=%x\n",lmac,lmac->pcb.Name,RStatus.TurnMotorStart,RStatus.TurnMotorInst[RStatus.TurnMotorStart]);
			if (!SendInstructionToMotor(lmac,RStatus.TurnMotorInst[RStatus.TurnMotorStart].inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION)) {
				if (!lmac) {
					fprintf(stderr,"SendInstructionToMotor error: lmac=0 TurnMotorStart=%d\n",RStatus.TurnMotorStart);
				} else {					
					inst=(unsigned char *)RStatus.TurnMotorInst[RStatus.TurnMotorStart].inst;
					fprintf(stderr,"SendInstructionToMotor error: mac=%p %s inst: %x%x%x%x%x TurnMotorStart=%d\n",lmac,lmac->pcb.Name,inst[0],*(inst+1),*(inst+2),*(inst+3),*(inst+4),RStatus.TurnMotorStart);
//					fprintf(stderr,"SendInstructionToMotor error: mac=%p TurnMotor %s TurnMotorStart=%d\n",lmac,lmac->pcb.Name,RStatus.TurnMotorStart);
				} //if (!lmac)
			} else { //if (!SendInstructionToMotor(
				//inst=(unsigned char *)RStatus.TurnMotorInst[RStatus.TurnMotorStart].inst;
				//fprintf(stderr,"TurnMotor mac=%p %s inst: %x%x%x%x%x TurnMotorStart=%d\n",lmac,lmac->pcb.Name,*inst,*(inst+1),*(inst+2),*(inst+3),*(inst+4),RStatus.TurnMotorStart);
				//fprintf(stderr,"TurnMotor %s TurnMotorStart=%d\n",lmac->pcb.Name,RStatus.TurnMotorStart);
			} //if (!SendInstructionToMotor(
			//and increment the start of the queue, TurnMotorStart

			if (RStatus.TurnMotorStart+1>=TURN_MOTOR_NUM_INSTS) {
				RStatus.TurnMotorStart=0; //go to start of queue
			} else {
				RStatus.TurnMotorStart++;
			} 
		} //if (RStatus.TurnMotorEnd != RStatus.TurnMotorStart) {
		
	} //while (keep_on_running) {

	return(1);
} //int Thread_TurnMotor(void) 
#endif //#if USE_RT_PREEMPT




//for example: TurnMotor(0,MOTOR_COUNTER_CLOCKWISE,MOTOR_SPEED_3_7,100); //100 ms pulse 
//todo: probably direction should be included in speed +=cw, -=ccw
//0=clockwise, 1=counterclockwise
//int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Direction,int Speed,int Duration)
//Queue=0 run now (currently not used anywhere in the Robot app code), queue=1, add to Motor instruction queue for TurnMotorThread to send
int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Speed,int Duration,int Queue)
{
	unsigned char inst[512];
	int Direction;//,MotorNameNum;

	if (!lmac) {
		fprintf(stderr,"Error: TurnMotor called with MAC_Connection==0 MotorNum=%d Thrust=%d Duration=%d Queue=%d\n",MotorNum,Speed,Duration,Queue);
		return(0);
	}


	//sending 0 stops motor, change any duration that is less than 0 to 0.
	if (Duration<0) {
		Duration=0;
		fprintf(stderr,"TurnMotor called with Duration < 0, set to 0\n");
		return(0);
	}
	Direction=(Speed<0);
	//SendInstructionToRobot(ROBOT_MOTORS_SEND_B24_INST,key);
	inst[0]=ROBOT_MOTORS_SEND_4BYTE_INST;
	inst[1]=(MotorNum<<4);
	inst[2]=(Direction<<7)|abs(Speed);  //Speed is true scale here
	inst[3]=(unsigned char)(Duration&0xff); //duration is sent little endian
	inst[4]=(unsigned char)((Duration&0xff00)>>0x8);

/*
	inst[1]=(MotorNum<<4)|(Direction<<3)|abs(Speed);
	inst[2]=(unsigned char)(Duration&0xff); //duration is sent little endian
	inst[3]=(unsigned char)((Duration&0xff00)>>0x8);
*/

	//set LastThrust in MotorInfo table (needed for inst after MAINTAIN_THRUST flag to know to set initial thrust to last thrust)
	//MotorNameNum=GetMotorNameNumByPCB(lmac->pcb.PCBNum,MotorNum);
	//RStatus.MotorInfo[MotorNameNum].LastThrust=Speed;

#if !USE_RT_PREEMPT
	//in wireshark logs, TurnMotor() calls can go out on the wire to the EthMotors PCB with only 24us delay between them, and that may be too fast for the EthMotors MCU, so make sure that there is at least a 1ms delay between all TurnMotor insts.
//	usleep(ROBOT_MOTORS_DEFAULT_MOTOR_DUTY_CYCLE_CLK*2+10); //25*2+10=60us delay between any two TurnMotor inst (use a touch sensor script that quickly ends the stage to test if this delay is enough. I verified that 100us works for 4 motors, and probably this delay could be lower. One thing to be aware of is that at 25us duty cycle, the EthMotors timer is every 12us, so I would keep this delay above 25us.
//no longer needed for realtime because of TurnMotorThread, only needed for non-realtime now	
#if Linux
	usleep(100); //100us
#endif 

#endif //#if !USE_RT_PREEMPT

	if (Queue) {
		//add motor inst to Motor Instruction Queue for TurnMotorThread to execute
#if USE_RT_PREEMPT
		//add motor instruction to MotorInstructionQueue
		memcpy(RStatus.TurnMotorInst[RStatus.TurnMotorEnd].inst,inst,5);  //add 5 byte instruction
		RStatus.TurnMotorInst[RStatus.TurnMotorEnd].mac=lmac;  //add MAC_Connection
		//lock MotorInstructionQueue LastIndex
//		pthread_mutex_lock(&RStatus.TurnMotorLock);
		//Increment LastIndex

		if (RStatus.TurnMotorEnd+1>=TURN_MOTOR_NUM_INSTS) {
			RStatus.TurnMotorEnd=0;  //reached end of queue, go back to start
		} else {
			RStatus.TurnMotorEnd++;
		} 
		//unlock LastIndex
//		pthread_mutex_unlock(&RStatus.TurnMotorLock);
#endif //#if USE_RT_PREEMPT
		return(1);
	} else {
		if (SendInstructionToMotor(lmac,inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION)) {
			//fprintf(stderr,"%s Motor %d Speed:%d Dur:%dms\n",lmac->pcb.Name,MotorNum,Speed,Duration);
			return(1);
		} else {
			return(0);
		}
	} //if (Queue) {
} //int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Speed,int Duration)
//} //int	TurnMotor(MAC_Connection *lmac,int MotorNum,int Direction,int Speed,int Duration,int Queue)

//sends to the selected PCB in the winMotors window
int SendUserInstructionToMotor(unsigned char *Inst,int numbyte,unsigned int flags)
{
	FTControl *tc;
	MAC_Connection *lmac;

	//get the MAC_Connection
	tc=GetFTControl("txtMotorDestIPAddressStr"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control txtMotorDestIPAddressStr\n");
		return(0);
	}
	lmac=Get_MAC_Connection(tc->text); //get remote MAC_Connection
	if (lmac==0) {
		lmac = Create_Remote_MAC_Connection(RStatus.WiredIPAddressStr, DEFAULT_ROBOT_PORT, tc->text,DEFAULT_ROBOT_PORT );
		//lmac=Get_MAC_Connection_By_IPs(RStatus.WiredIPAddressStr,tc->text); //get remote MAC_Connection
		if (lmac==0) {
			fprintf(stderr,"Could not get MAC_Connection for dest ip: %s\n",tc->text);
			return(0);
		}
	}

	return(SendInstructionToMotor(lmac,Inst,numbyte,flags));
} //int SendUserInstructionToMotor(unsigned char *Inst,int numbyte,unsigned int flags)

//Set the Duty Cycle clock (in us) of EthMotors PCB
int SetEthMotorsDutyCycle(MAC_Connection *lmac,unsigned int DutyCycleClock)
{
	unsigned char inst[512];

	RStatus.MotorDutyCycleClock=DutyCycleClock;  //todo: needed to know if the Motor duty cycle has changed- should get from each individual EthMotors- should be on PCB struct- not RStatus
	inst[0]=ROBOT_MOTORS_SET_MOTOR_DUTY_CYCLE_CLK_IN_USEC;
	//int value is sent Little Endian
	*(unsigned int *)(&inst[1])=DutyCycleClock;
	fprintf(stderr,"Set %s motor duty cycle clock to %d us\n",lmac->pcb.Name,DutyCycleClock);
  return(SendInstructionToMotor(lmac,inst,5,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION));
} //int SetEthMotorsDutyCycle(MAC_Connection *lmac,unsigned int DutyCycleClock


//Checks to see if the source and dest (motor) MAC_Connections exist first
//and if yes, sends the data to the dest
//flags:
//ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION - add the robot source IP address before the instruction (otherwise there should be some other source IP address in the first 4 bytes of the instruction)
//note is very similar to SendInstructionToRobot- the only difference is ConnectGenericIPs vs ConnectMotorIPs
int SendInstructionToMotor(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)
{
	int i;
	char LogStr[256];
	int MotorNum,MotorDir,MotorStrength,MotorDur;


	if (lmac==0) {
		fprintf(stderr,"Error: SendInstructionToMotor() called with lmac==0\n");
		return(0);
	}

	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "SendInstructionToMotor\n");
		fprintf(stderr, "Send from %s to %s\n",lmac->SrcIPAddressStr,lmac->DestIPAddressStr);
		fprintf(stderr, "%d bytes: ",numbyte);
		for(i=0;i<numbyte;i++) {
			fprintf(stderr, "0x%02x ",Inst[i]);
		}
		fprintf(stderr, "\n");
	}


	if (!(RStatus.flags&ROBOT_STATUS_MOTORS_DISABLED)) {

		SendInstructionToEthMotors(lmac,Inst,numbyte,flags); //send the actual instruction to the EthMotors PCB

		//if ModelLog: output EthMotors PCBName, Motor #, Direction+Strength, and Duraion
		if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
			MotorNum=Inst[1]>>4;
			MotorDir=(Inst[2]&0x80)>>7;
			MotorStrength=Inst[2]&0x7f;
			MotorDur=(uint16_t)((Inst[4]<<8) + Inst[3]);

	/*
			MotorDir=(Inst[1]&0x08)>>3;
			MotorStrength=Inst[1]&0x7;
			MotorDur=(uint16_t)((Inst[3]<<8) + Inst[2]);
	*/
	//		sprintf(LogStr,"%s\t%d\t%d\t%d",lmac->pcb.Name,MotorNum,(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);
	//		sprintf(LogStr,"%s\t%s(%d)\t%d\t%d",lmac->pcb.Name,GetMotorName(lmac->pcb.PCBNum,MotorNum),MotorNum,(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);
	//		sprintf(LogStr,"%s\t%s\t%d\t%d\t%d",lmac->pcb.Name,GetMotorName(lmac->pcb.PCBNum,MotorNum),MotorNum,(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);
	//not writing Motor Name to log anymore
	//		sprintf(LogStr,"%s\t%d\t%d\t%d",lmac->pcb.Name,MotorNum,(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);
			//sprintf(LogStr,"%s\t%d\t%d",GetMotorName(lmac->pcb.PCBNum,MotorNum),(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);
			sprintf(LogStr,"%s\t%d\t%d",GetMotorName(lmac->pcb.Num,MotorNum),(-(MotorDir==1)+(MotorDir==0))*MotorStrength,MotorDur);

			LogRobotModelData(LogStr);
		}
	} //if (!(Robot.flags&ROBOT_STATUS_MOTORS_DISABLED)) {


	return(1);
} //int SendInstructionToMotor(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)



int SendInstructionToEthMotors(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)
{
	int i;
	int iResult=0;
	int NumBytes;
	char DataStr[512];
	unsigned char SendInst[ROBOT_MAX_INST_LEN]; 


	if (lmac==0) {
		fprintf(stderr,"Error: SendInstructionToEthMotors() called with lmac==0\n");
		return(0);
	}

	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "SendInstructionToEthMotors\n");
		fprintf(stderr, "Send from %s to %s\n",lmac->SrcIPAddressStr,lmac->DestIPAddressStr);
		fprintf(stderr, "%d bytes: ",numbyte);
		for(i=0;i<numbyte;i++) {
			fprintf(stderr, "0x%02x ",Inst[i]);
		}
		fprintf(stderr, "\n");
	}


	if (!(RStatus.flags&ROBOT_STATUS_MOTORS_DISABLED)) {

		//******
		//Note that raw instructions must already have integer values (like duration, memory address, memory data, etc.) in little endian format
		//****
		//test for connect and connect if not connected
		//ConnectMotorIPs();
			
		//if the instruction originates on this machine, copy the source IP Address to the front of the instruction/packet
		if (flags&ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION) {
			memcpy(SendInst,(unsigned int *)&lmac->SrcAddress.sin_addr.s_addr,4);
			memcpy(SendInst+4,(const char *)Inst,numbyte);
			numbyte+=4;
		} else {
			//otherwise the IP is presumably already at the beginning of the instruction
			memcpy(SendInst,(const char *)Inst,numbyte);
		}

			//iResult = send(lmac->Socket,(const char *)SendInst,numbyte+4,0);
		iResult = sendto(lmac->Socket,(const char *)SendInst,numbyte,0,(struct sockaddr*)&lmac->DestAddress,sizeof(lmac->DestAddress));


	#if Linux
		if (iResult == 0) {
			fprintf(stderr,"send failed with error\n");
	#endif
	#if WIN32
		if (iResult == SOCKET_ERROR) {
			fprintf(stderr,"send failed with error: %d\n", WSAGetLastError());
	#endif

			//CloseRobotSocket();
			return(0);
		}

		if ((RStatus.flags&ROBOT_STATUS_NETWORK_LOG)&&(RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA)) { //log all outgoing network data
			NumBytes=ConvertBytesToString(SendInst,DataStr,numbyte); //convert bytes to string
			LogRobotNetworkData(lmac->SrcIPAddressStr,lmac->DestIPAddressStr,DataStr,NumBytes);		
		} //if (RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA) { //log all incoming data

		if (RStatus.flags&ROBOT_STATUS_INFO) {
			fprintf(stderr,"Bytes Sent: %d\n", iResult);
		}

	} //if (!(Robot.flags&ROBOT_STATUS_MOTORS_DISABLED)) {


	return(1);
} //int SendInstructionToEthMotors(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)


int StopAllMotors(void)
{
	MAC_Connection *lmac;
	char LogStr[256];
	unsigned char Inst[5];

	//Go through each EthMotors and send ROBOT_MOTORS_STOP_ALL_MOTORS inst

	Inst[0]=ROBOT_MOTORS_STOP_ALL_MOTORS;
	//Currently all the macs are gone through searching for Motor PCBs 
	lmac=RStatus.iMAC_Connection;
	while (lmac!=0) {
		if (!strncmp(lmac->Name, "Motor",5)) {	//EthMotors PCB
			SendInstructionToEthMotors(lmac,Inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
			//output EthMotors PCBName, Motor #, Direction+Strength, and Duraion
			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				sprintf(LogStr,"All Motors Stopped on %s",lmac->pcb.Name);
				LogRobotModelData(LogStr);
			} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		} //if (!strncmp(lmac->Name, "Motor",5)) {
		lmac=lmac->next;
	} //while	
	return(1);
} //StopAllMotors(void)

void btnwinMotorsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinMotorsExit_Click


void btnMotorLeftArrow_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
FTControl *tc;

	//change direction to counter-clockwise and send
	tc=GetFTControl("ddMotorDir");
	if (tc!=0) {
		tc->value=2;
		DrawFTControl(tc); //redraw control
		//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
		SendUserSettingsInstruction();
	}

}

void btnMotorStop_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
FTControl *tc;

	//change duration of motor to 0 (stop)
	//tc=GetFTControl("ddMotorDuration");
	tc=GetFTControl("ddMotorStrength");
	if (tc!=0) {
		tc->value=1;  //strength=0 (stop)
		DrawFTControl(tc); //redraw control
		//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
		SendUserSettingsInstruction();
	}
}

void btnMotorRightArrow_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
FTControl *tc;

	//change direction to counter-clockwise and send
	tc=GetFTControl("ddMotorDir");
	if (tc!=0) {
		tc->value=1;
		DrawFTControl(tc); //redraw control
		//btnSendInstructionToRobot_Click(twin,tc,tc->x1,tc->y1);
		SendUserSettingsInstruction();
	}
}

int chkComplement_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol!=0) {
		if (!tcontrol->value) {
			Complement=0;
		} else {
			Complement=1;
		}
	} //if (tcontrol!=0) {

return(1);
} //int chkComplement_OnChange(FTWindow *twin,FTControl *tcontrol)


int chkCompOpp_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol!=0) {
		if (!tcontrol->value) {
			ComplementOpposite=0;
		} else {
			ComplementOpposite=1;
		}
	} //if (tcontrol!=0) {

return(1);
} //int chkCompOpp_OnChange(FTWindow *twin,FTControl *tcontrol)


int chkPair_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol!=0) {
		if (!tcontrol->value) {
			Pair=0;
		} else {
			Pair=1;
		}
	} //if (tcontrol!=0) {

return(1);
} //int chkPair_OnChange(FTWindow *twin,FTControl *tcontrol)


int chkPairOpp_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol!=0) {
		if (!tcontrol->value) {
			PairOpposite=0;
		} else {
			PairOpposite=1;
		}
	} //if (tcontrol!=0) {

return(1);
} //int chkPairOpp_OnChange(FTWindow *twin,FTControl *tcontrol)

int chkDisableMotors_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol!=0) {
		if (!tcontrol->value) {
			RStatus.flags&=~ROBOT_STATUS_MOTORS_DISABLED;
		} else {
			RStatus.flags|=ROBOT_STATUS_MOTORS_DISABLED;
		}
	} //if (tcontrol!=0) {

return(1);
} //int chkDisableMotors_OnChange(FTWindow *twin,FTControl *tcontrol)

int winMotors_OnOpen(FTWindow *twin)
{
	FTControl *tcontrol;

	RStatus.MotorDutyCycleClock=ROBOT_MOTORS_DEFAULT_MOTOR_DUTY_CYCLE_CLK;
	RStatus.NumClksInMotorDutyCycle=ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;


	winMotorsFlags=0; //flags for window keyboard shortcuts

	//possibly automatically connect to robot


	//note the MAC label and IP controls must be created by the time this functions is called in order to fill them
	tcontrol=GetFTControl("ddMotorPCBNum");
	if (tcontrol!=0) {
		Populate_EthMotors_Dropdown(tcontrol);  //add recognized EthMotors to dropdown control
	}


	return(1);
} //int winMotors_OnOpen(FTWindow *twin)

int winMotors_OnClose(FTWindow *twin)
{
//	DisconnectFromRobot();

	return(1);
} //int winMotors_OnClose(FTWindow *twin)


