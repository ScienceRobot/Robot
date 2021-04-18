//win_GPS.c
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
#include "win_GPS.h"
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

int winGPS_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winGPS");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winGPS");
	strcpy(twin.title,"GPS");
	strcpy(twin.ititle,"GPS");
	twin.x=130;
	twin.y=130;
	twin.w=580;
	twin.h=440;
//	twin.Keypress=(FTControlfuncwk *)winGPS_key;
	twin.AddFTControls=(FTControlfuncw *)winGPS_AddFTControls;
//	twin.OnOpen=(FTControlfuncw *)winGPS_OnOpen;
//	twin.OnClose=(FTControlfuncw *)winGPS_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winGPS_AddFTWindow



int winGPS_AddFTControls(FTWindow *twin)
{
int fw,fh;//,i,j;
FTControl *tcontrol,*tcontrol2;



#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winGPS'\n");

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
//note that the dest address is not needed- because SendInstToAccel is used to send, 
//but is listed for convenience
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*34;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"Dest IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGPSDestIPAddressStr");
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

//Latitude N/S
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLatitude");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*3;
tcontrol->y2=fh*4+fh/2;
strcpy(tcontrol->text,"Latitude: "); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLatitudeDeg");
tcontrol->type=CTLabel;
tcontrol->x1=fw*13;
tcontrol->y1=fh*3;
tcontrol->x2=fw*16;
tcontrol->y2=fh*4+fh/2;
strcpy(tcontrol->text,"0.0"); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLatitudeMin");
tcontrol->type=CTLabel;
tcontrol->x1=fw*17;
tcontrol->y1=fh*3;
tcontrol->x2=fw*21;
tcontrol->y2=fh*4+fh/2;
strcpy(tcontrol->text,""); 
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLatitudeSec");
tcontrol->type=CTLabel;
tcontrol->x1=fw*22;
tcontrol->y1=fh*3;
tcontrol->x2=fw*37;
tcontrol->y2=fh*4+fh/2;
strcpy(tcontrol->text,""); 
AddFTControl(twin,tcontrol);

//Longitude E/W
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLongitude");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,"Longitude: "); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLongitudeDeg");
tcontrol->type=CTLabel;
tcontrol->x1=fw*12;
tcontrol->y1=fh*5;
tcontrol->x2=fw*16;
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,"0.0"); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLongitudeMin");
tcontrol->type=CTLabel;
tcontrol->x1=fw*17;
tcontrol->y1=fh*5;
tcontrol->x2=fw*20;
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,""); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSLongitudeSec");
tcontrol->type=CTLabel;
tcontrol->x1=fw*21;
tcontrol->y1=fh*5;
tcontrol->x2=fw*37;
tcontrol->y2=fh*6+fh/2;
strcpy(tcontrol->text,""); 
AddFTControl(twin,tcontrol);

//Altitude Units
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSAltitude");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*7;
tcontrol->y2=fh*8+fh/2;
strcpy(tcontrol->text,"Altitude: "); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSAltitudeValue");
tcontrol->type=CTLabel;
tcontrol->x1=fw*12;
tcontrol->y1=fh*7;
tcontrol->x2=fw*18;
tcontrol->y2=fh*8+fh/2;
strcpy(tcontrol->text,"0.0"); 
AddFTControl(twin,tcontrol);


//checkbox Send all GPS data
//Send latitude, longitude and altitude
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkSendAllGPSData");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw;
tcontrol->y1=fh*9;
tcontrol->x2=fw*20;
tcontrol->y2=fh*10+fh/2;
strcpy(tcontrol->text,"Send All GPS Data"); 
tcontrol->OnChange=(FTControlfuncwc *)chkSendAllGPSData_OnChange;
AddFTControl(twin,tcontrol);



//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblSendInstructionToGPSRaw");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*12;
tcontrol->y2=fh*13+fh/2;
strcpy(tcontrol->text,"Send raw instruction: "); 
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtSendInstructionToGPSRaw");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*25;
tcontrol->y1=fh*12;
tcontrol->x2=fw*55;
tcontrol->y2=fh*13+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
AddFTControl(twin,tcontrol);


//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGPSData");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*14;
tcontrol->x2=fw*16;
tcontrol->y2=fh*15+fh/2;
strcpy(tcontrol->text,"Data received:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGPSData");
tcontrol->type=CTTextArea;
tcontrol->x1=fw*17;
tcontrol->y1=fh*14;
tcontrol->x2=fw*75;
tcontrol->y2=fh*20+fh/2;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
AddFTControl(twin,tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGPSGetData");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*19;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Get GPS Data");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGPSGetData_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGPSStopData");
tcontrol->type=CTButton;
tcontrol->x1=fw*20;
tcontrol->y1=fh*21;
tcontrol->x2=fw*39;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Stop GPS Data");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGPSStopData_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGPSStaticNav");
tcontrol->type=CTButton;
tcontrol->x1=fw*40;
tcontrol->y1=fh*21;
tcontrol->x2=fw*59;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Disable Static Navigation");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGPSStaticNav_Click;
AddFTControl(twin,tcontrol);


//newline
/*
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGPSSendData");
tcontrol->type=CTButton;
tcontrol->x1=fw*40;
tcontrol->y1=fh*27;
tcontrol->x2=fw*59;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Send Data");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGPSSendData_Click;
AddFTControl(twin,tcontrol);
*/

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinGPSExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*60;
tcontrol->y1=fh*27;
tcontrol->x2=fw*79;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinGPSExit_Click;
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


//Get GPS Data
int btnGPSGetData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int NumBytes;
	unsigned char Inst[512];
	FTControl *tc;

//clear GPS textbox
	tc=GetFTControl("txtGPSData");
	if (tc!=0) {
		tc->text[0]=0;
		DrawFTControl(tc);
	}

	NumBytes=5; //IP is added later
	Inst[0]=ROBOT_ACCELMAGTOUCH_GET_GPS_DATA;
	SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 

	return(1);
} //int btnGPSGetData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Stop GPS Data
int btnGPSStopData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int NumBytes;
	unsigned char Inst[512];

	NumBytes=5; //IP is added later
	Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_GPS_DATA;
	SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 

	return(1);
} //int btnGPSStopData_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


void btnSendInstructionToGPS_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	FTControl *tc;
	unsigned char Inst[512];
	int NumBytes;

	//note that the dest address is not needed
	tc=GetFTControl("txtSendInstructionToGPSRaw");
	if (tc!=0) {
		//NumBytes=ConvertInstructionString(tc->text,Inst);
		NumBytes=ConvertStringToBytes((unsigned char *)tc->text,Inst); //NumBytes=NumChars/2
		if (NumBytes>0) {
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		}
	} //	if (tc!=0) {
}  //void btnSendInstructionToGPS_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {


int btnGPSStaticNav_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int NumBytes;
	unsigned char Inst[512];

	NumBytes=5; //IP is added later
	if (strcmp(tcontrol->text,"Disable Static Navigation")) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_ENABLE_STATIC_NAVIGATION;
		strcpy(tcontrol->text,"Disable Static Navigation");
	} else {
		Inst[0]=ROBOT_ACCELMAGTOUCH_DISABLE_STATIC_NAVIGATION;
		strcpy(tcontrol->text,"Enable Static Navigation");
	}
	SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	DrawFTControl(tcontrol);

	return(1);
}//int btnGPSStaticNav_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int chkSendAllGPSData_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	int NumBytes;
	unsigned char Inst[512];

	NumBytes=5; //IP is added later
	if (tcontrol->value) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_SET_SEND_ALL_GPS_DATA;
	} else {
		Inst[0]=ROBOT_ACCELMAGTOUCH_UNSET_SEND_ALL_GPS_DATA;
	}
	SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 

	return(1);
}//int chkSendAllGPSData_OnChange;(FTWindow *twin,FTControl *tcontrol)

void btnwinGPSExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinGPSExit_Click

/*
int winGPS_OnOpen(FTWindow *twin)
{

	return(1);
} //int winGPS_OnOpen(FTWindow *twin)

int winGPS_OnClose(FTWindow *twin)
{

	return(1);
} //int winGPS_OnClose(FTWindow *twin)
*/
