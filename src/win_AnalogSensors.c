//win_AnalogSensors.c
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
#include "win_AnalogSensors.h"
#include "win_Accels.h"
#include "win_Robot.h"
#include "robot_accelmagtouchgps_mcu_instructions.h"

#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  //WIN32


#define DEFAULT_BUFLEN 512

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons

int winAnalogSensors_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winAnalogSensors");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winAnalogSensors");
	strcpy(twin.title,"Analog Sensors");
	strcpy(twin.ititle,"Analog Sensors");
	twin.x=130;
	twin.y=130;
	twin.w=580;
	twin.h=440;
	twin.Keypress=(FTControlfuncwk *)winAnalogSensors_key;
	twin.AddFTControls=(FTControlfuncw *)winAnalogSensors_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winAnalogSensors_OnOpen;
	twin.OnClose=(FTControlfuncw *)winAnalogSensors_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winAnalogSensors_AddFTWindow



int winAnalogSensors_AddFTControls(FTWindow *twin)
{
int fw,fh,i,j;
FTControl *tcontrol,*tcontrol2;



#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winAnalogSensors'\n");

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
sprintf(tcontrol->name,"chkAnalogSensorAll");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->x2=fw*7;
tcontrol->y2=2.5*fh;
sprintf(tcontrol->text,"All");
tcontrol->OnChange=(FTControlfuncwc *)chkAnalogSensorAll_OnChange;
AddFTControl(twin,tcontrol);


//Show incoming touch sensor data 
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkAnalogSensor_ShowData");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw*8 ;
tcontrol->y1 = fh;
tcontrol->x2 = fw*20;
tcontrol->y2 = fh*2.5;
strcpy(tcontrol->text, "Show Data");
tcontrol->OnChange = (FTControlfuncwc *)chkAnalogSensor_ShowData_OnChange;
RStatus.flags|=ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA;  //default to show data when opened
tcontrol->value=1;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//note that the dest address is not needed- because SendInstToAccel is used to send, 
//but is listed for convenience
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAnalogDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*34;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"Dest IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtAnalogDestIPAddressStr");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*43;
tcontrol->y1=fh;
tcontrol->x2=fw*70;
tcontrol->y2=fh*2+fh/2;
//use the dest address from the accel window which should be open already
tcontrol2=GetFTControl("txtAccelDestIPAddressStr");
if (tcontrol2!=0) {
	strcpy(tcontrol->text,tcontrol2->text);
}
//strcpy(tcontrol->text,"192.168.1.2");
//see if a name exists for this PCB yet
//if (lmac!=0) {
		//strcpy(tcontrol->text,"192.168.1.1");
	//strcpy(tcontrol->text,lmac->DestIPAddressStr);
//} 
//tcontrol->OnChange=(FTControlfuncwc *)txtAccelDestIPAddress_OnChange;
AddFTControl(twin,tcontrol);


//newline

//newline
//Start AnalogSensor #0
#define ROWSIZE	27 //width in fw of each column
//newline
for(i=0;i<3;i++) {

	//column headers (3)
	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblHeaderSensorNum%02d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw+fw*ROWSIZE*i;
	tcontrol->y1=fh*3;
	tcontrol->y2=fh*4+fh/2;
	strcpy(tcontrol->text,"Sensor");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblHeaderPerCent%02d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*8+fw*ROWSIZE*i;
	tcontrol->y1=fh*3;
	tcontrol->y2=fh*4+fh/2;
	strcpy(tcontrol->text,"%/Deg");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblHeaderPerCentChange%02d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*15+fw*ROWSIZE*i;
	tcontrol->y1=fh*3;
	tcontrol->y2=fh*4+fh/2;
	strcpy(tcontrol->text,"%Change");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblHeaderAnalogSensorVolt%02d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*23+fw*ROWSIZE*i;
	tcontrol->y1=fh*3;
	tcontrol->y2=fh*4+fh/2;
	strcpy(tcontrol->text,"Volt");
	AddFTControl(twin,tcontrol);

	//touch sensors
	for(j=0;j<5;j++) {
		memset(tcontrol,0,sizeof(FTControl));
		sprintf(tcontrol->name,"chkAnalogSensorNum%02d",i*5+j);
		tcontrol->type=CTCheckBox;
		tcontrol->x1=fw+fw*i*ROWSIZE;
		tcontrol->y1=fh*(j+2)*2+fh;
		tcontrol->x2=tcontrol->x1+fw*3;
		tcontrol->y2=fh*(j+2)*2+2.5*fh;
		sprintf(tcontrol->text,"%02d:",i*5+j);
		AddFTControl(twin,tcontrol);

		memset(tcontrol,0,sizeof(FTControl));
		sprintf(tcontrol->name,"lblAnalogSensorPress%02d",i*5+j);
		tcontrol->type=CTLabel;
		tcontrol->x1=fw*8+fw*i*ROWSIZE;
		tcontrol->y1=fh*(j+2)*2+fh;
		tcontrol->x2=tcontrol->x1+fw*4;
		tcontrol->y2=tcontrol->y1+1.5*fh;
		strcpy(tcontrol->text,"  0%");
		AddFTControl(twin,tcontrol);

		memset(tcontrol,0,sizeof(FTControl));
		sprintf(tcontrol->name,"lblAnalogSensorChange%02d",i*5+j);
		tcontrol->type=CTLabel;
		tcontrol->x1=fw*15+fw*i*ROWSIZE;
		tcontrol->y1=fh*(j+2)*2+fh;
		tcontrol->x2=tcontrol->x1+fw*4;
		tcontrol->y2=tcontrol->y1+1.5*fh;
		strcpy(tcontrol->text,"  0%");
		AddFTControl(twin,tcontrol);

		memset(tcontrol,0,sizeof(FTControl));
		sprintf(tcontrol->name,"lblAnalogSensorVolt%02d",i*5+j);
		tcontrol->type=CTLabel;
		tcontrol->x1=fw*23+fw*i*ROWSIZE;
		tcontrol->y1=fh*(j+2)*2+fh;
		tcontrol->x2=tcontrol->x1+fw*5;
		tcontrol->y2=tcontrol->y1+1.5*fh;
		strcpy(tcontrol->text," - v");
		AddFTControl(twin,tcontrol);

	} //j
} //i

//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblSendInstructionToAnalogSensorRaw");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"Send raw instruction: "); 
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtSendInstructionToAnalogSensorRaw");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*25;
tcontrol->y1=fh*17;
tcontrol->x2=fw*55;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
//tcontrol->color[0]=white;  //bkground
//tcontrol->color[1]=black;  //frame
//tcontrol->textcolor[0]=black;//text
//tcontrol->textcolor[1]=white;//text background
//tcontrol->textcolor[2]=white;//selected text8
//tcontrol->textcolor[3]=blue;//selected text background
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAnalogSensorsMin");
tcontrol->type=CTLabel;
tcontrol->x1=fw*56;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"Min:"); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtAnalogSensorsMin");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*60;
tcontrol->y1=fh*17;
tcontrol->x2=fw*70;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAnalogSensorsMax");
tcontrol->type=CTLabel;
tcontrol->x1=fw*71;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"Max:"); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtAnalogSensorsMax");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*76;
tcontrol->y1=fh*17;
tcontrol->x2=fw*86;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAnalogSensorsMinMaxVolts");
tcontrol->type=CTLabel;
tcontrol->x1=fw*87;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"volts"); 
AddFTControl(twin,tcontrol);

//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGetAnalogSensorData");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*19;
tcontrol->x2=fw*21;
tcontrol->y2=fh*20+fh/2;
strcpy(tcontrol->text,"Data received: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGetAnalogSensorData");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*21;
tcontrol->y1=fh*19;
tcontrol->x2=fw*35;
tcontrol->y2=fh*20+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
AddFTControl(twin,tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsGetValues");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*14;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Get Values");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsGetValues_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsStartPolling");
tcontrol->type=CTButton;
tcontrol->x1=fw*15;
tcontrol->y1=fh*21;
tcontrol->x2=fw*29;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Start Polling");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsStartPolling_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsStopPolling");
tcontrol->type=CTButton;
tcontrol->x1=fw*30;
tcontrol->y1=fh*21;
tcontrol->x2=fw*44;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Stop Polling");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsStopPolling_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsGetMinMax");
tcontrol->type=CTButton;
tcontrol->x1=fw*45;
tcontrol->y1=fh*21;
tcontrol->x2=fw*59;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Get Min+Max");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsGetMinMax_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsSetMinMax");
tcontrol->type=CTButton;
tcontrol->x1=fw*60;
tcontrol->y1=fh*21;
tcontrol->x2=fw*74;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Set Min+Max");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsSetMinMax_Click;
AddFTControl(twin,tcontrol);

//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsStartInterrupt");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*27;
tcontrol->x2=fw*14;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Start Interrupt");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsStartInterrupt_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsStopInterrupt");
tcontrol->type=CTButton;
tcontrol->x1=fw*15;
tcontrol->y1=fh*27;
tcontrol->x2=fw*29;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Stop Interrupt");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsStopInterrupt_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSendInstructionToAnalogSensor");
tcontrol->type=CTButton;
tcontrol->x1=fw*30;
tcontrol->y1=fh*27;
tcontrol->x2=fw*44;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Send Instruction");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSendInstructionToAnalogSensor_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinAnalogSensorsExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*45;
tcontrol->y1=fh*27;
tcontrol->x2=fw*59;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinAnalogSensorsExit_Click;
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


void winAnalogSensors_key(FTWindow *twin,KeySym key)
{

    switch (key)	{
		case XK_Shift_L://-30:
		case XK_Shift_R: //-31:  //ignore shift keys
        break;
      case XK_Escape:	/*esc*/
				if (twin->FocusFTControl==0) {  //esc key and no control has focus
					//PStatus.flags|=PExit;
					CloseFTWindow(twin);
				}	     
        break;
      case XK_KP_1: //0xffb1:  //1 key
      case XK_KP_2: //0xffb2:  //1 key
      case XK_KP_3: //0xffb3:  //1 key
      case XK_KP_4: //0xffb4:  //1 key
      case XK_KP_5: //0xffb5:  //1 key
      case XK_1: //0x31:  //1 key
      case XK_2: //0x32:  //1 key
      case XK_3: //0x33:  //1 key
      case XK_4: //0x34:  //1 key
      case XK_5: //0x35:  //1 key
        break;



      default:
				//fprintf(stderr,"key=%x\n",key);
        break;
      }
} //void winAnalogSensors_key(FTWindow *twin,KeySym key)

int chkAnalogSensorAll_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];


	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				tc->value=tcontrol->value;
			}
	} //for 
	//redraw window
	DrawFTWindow(twin);

	return(1);
}//int chkAnalogSensorAll_OnChange;(FTWindow *twin,FTControl *tcontrol)

//Get Touch Sensors Values
int btnAnalogSensorsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];

	//create a 32-bit mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_VALUES; 
		memcpy(&Inst[1],&mask,4);  //is send little endian
		NumBytes=5; //IP is added later
		SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		return(1);
	}  else {
		FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
		return(0);
	}//if (mask!=0) {

	return(0);
} //int btnAnalogSensorsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Start Polling Touch Sensors
int btnAnalogSensorsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];


	//start polling any checked touch sensors
	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_START_POLLING_ANALOG_SENSORS; 
		memcpy(&Inst[1],&mask,4);  //is send little endian
		NumBytes=5;
		SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	}  else {
		FTMessageBox("You must select a touch sensor first.",FTMB_OK,"Robot -- Info",0);
		return(0);
	}//if (mask!=0) {
		
	return(1);
} //int btnAnalogSensorsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Stop Polling Analog Sensors
int btnAnalogSensorsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];

	//Stop polling analog sensors
	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_POLLING_ANALOG_SENSORS; 
		memcpy(&Inst[1],&mask,4);  //is send little endian
		NumBytes=5;
		SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	} else {
		FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
		return(0);
	}

	return(1);
} //int btnAnalogSensorsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Start Analog Sensors Interrupt
int btnAnalogSensorsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];


	//probably should be mask of only 16 bits 
	//because currently there can only be 15 analog sensors/PCB
	//start polling analog sensors
	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_START_ANALOG_SENSORS_INTERRUPT; 
		memcpy(&Inst[1],&mask,4);  //is send little endian
		NumBytes=5;
		SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	} else {
		FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (mask!=0) {
		
	return(1);
} //int btnAnalogSensorsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Stop Analog Sensors Interrupt
int btnAnalogSensorsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];

	//probably should be mask of only 16 bits 
	//because currently there can only be 15 analog sensors/PCB


	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
		} //for

		if (mask!=0) {
			//Stop polling analog sensors	
			Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_ANALOG_SENSORS_INTERRUPT; 
			memcpy(&Inst[1],&mask,4);  //is send little endian
			NumBytes=5;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		} else {
			FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
			return(0);
		}//if (mask!=0) {

return(1);
} //int btnAnalogSensorsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Get min and max voltage of analog sensors
int btnAnalogSensorsGetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];

	//probably should be mask of only 16 bits 
	//because currently there can only be 15 analog sensors/PCB


	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);  
				}
			}
		} //for

		if (mask!=0) {
			//Get touch sensors	min and max voltages
			Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ANALOG_MINMAX; 
			memcpy(&Inst[1],&mask,4);  //is sent little endian
			NumBytes=5;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		} else {
			FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
			return(0);
		}//if (mask!=0) {

return(1);
} //int btnAnalogSensorsGetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Set min and max voltage of analog sensors
int btnAnalogSensorsSetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned int mask;
	int NumBytes;
	unsigned char Inst[512];
	float minf,maxf;
	uint16_t mini,maxi;

	//probably should be mask of only 16 bits 
	//because currently there can only be 15 analog sensors/PCB


	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAnalogSensorNum%02d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
		} //for

		if (mask!=0) {
			//Set touch sensors	min and max voltages
			Inst[0]=ROBOT_ACCELMAGTOUCH_SET_ANALOG_MINMAX; 
			memcpy(&Inst[1],&mask,4);  //is sent little endian

			//set min and max values (note all checked sensors will have
			//their min and max set to the same min and max values
			tc=GetFTControl("txtAnalogSensorsMin");
			if (tc!=0) {
				//possibly it should be just the hex values, 
				//but the values are converted to fractional volts for the user
				//and that is what the user will probably use to determine min and max.
				minf=atof(tc->text);
				//convert fraction to int
				//pic32mx 10-bit ADC sample, 0x3ff=1023.0  3.3/1023 = 0.003225806
				//mini=(uint16_t)(minf/0.003225806);
				//pic32mz 12-bit ADC sample, 0xfff=4095.0  3.3/4095 = 0.003225806
				mini=(uint16_t)(minf/0.000805664);
				tc=GetFTControl("txtAnalogSensorsMax");
				if (tc!=0) {
					maxf=atof(tc->text);
					//pic32mx 10-bit 
					//maxi=(uint16_t)(maxf/0.003225806);
					//pic32mz 12-bit 
					maxi=(uint16_t)(maxf/0.000805664);

					fprintf(stderr,"Setting analog sensors (mask %04x) with min: %f (%04x), max: %f (%04x)\n",mask,minf,mini,maxf,maxi);

					memcpy(&Inst[5],&mini,2);  //is sent little endian
					memcpy(&Inst[7],&maxi,2);  //is sent little endian

					NumBytes=9;
					SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 

			
				} //tc!=0
			} //tc!=0
		
		} else {
			FTMessageBox("You must select an analog sensor first.",FTMB_OK,"Robot -- Info",0);
			return(0);
		}//if (mask!=0) {

return(1);
} //int btnAnalogSensorsSetMinMax_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

int SetAnalogSensorsMinAndMax(MAC_Connection *lmac,unsigned int Mask,float MinV,float MaxV)
{
	unsigned char Inst[512];
	uint16_t mini,maxi;
	int NumBytes;


	//analog sensor mask is bit0 (0x1)=sensor 0, bit1 (0x2)=sensor 1, bit3 (0x4)=sensor2

	//Set touch sensors	min and max voltages
	Inst[0]=ROBOT_ACCELMAGTOUCH_SET_ANALOG_MINMAX; 
	memcpy(&Inst[1],&Mask,4);  //is sent little endian
	mini=(uint16_t)(MinV/0.000805664);
	maxi=(uint16_t)(MaxV/0.000805664);

	if (maxi>4096) {
		maxi=4096;
	}
	fprintf(stderr,"Setting analog sensors (mask %04x) with min: %3.1f (%04x), max: %3.1f (%04x)\n",Mask,MinV,mini,MaxV,maxi);

	memcpy(&Inst[5],&mini,2);  //is sent little endian
	memcpy(&Inst[7],&maxi,2);  //is sent little endian

	NumBytes=9;
	
	SendInstructionToAccel(lmac,Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
	return(1);
}//int SetAnalogSensorsMinAndMax(MAC_Connection *lmac, unsigned int Mask,float Min,float Max)


void btnSendInstructionToAnalogSensor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	FTControl *tc;
	unsigned char Inst[512];
	int NumBytes;

	//note that the dest address is not needed
	tc=GetFTControl("txtSendInstructionToAnalogSensorRaw");
	if (tc!=0) {
		//NumBytes=ConvertInstructionString(tc->text,Inst);
		NumBytes=ConvertStringToBytes((unsigned char *)tc->text,Inst); //NumBytes=NumChars/2
		if (NumBytes>0) {
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		}
	} //	if (tc!=0) {
}  //void btnSendInstructionToAnalogSensor_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


void btnwinAnalogSensorsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinAnalogSensorsExit_Click

int winAnalogSensors_OnOpen(FTWindow *twin)
{

	return(1);
} //int winAnalogSensors_OnOpen(FTWindow *twin)

int winAnalogSensors_OnClose(FTWindow *twin)
{

	return(1);
} //int winAnalogSensors_OnClose(FTWindow *twin)

int chkAnalogSensor_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol) 
{
	//if (RStatus.flags&ROBOT_STATUS_SHOW_AnalogSensor_DATA) {
	if (tcontrol->value) {
		RStatus.flags|=ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA;
	} else {
		RStatus.flags&=~ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA;
	}
	return(1);
} //int chkAccel_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol) 
