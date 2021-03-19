//win_Robot.c
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
#include <ifaddrs.h>
#endif

#include "freethought.h"
#include "Robot.h"
#include "win_Robot.h"
#include "win_Motors.h"
#include "win_Accels.h"
#include "win_AnalogSensors.h"
#include "win_RobotModel.h"
#include "win_Cameras.h"
#include "win_Tasks.h"
#include "win_Power.h"
#include "win_NewEthPCB.h"
#include "win_Load_Robot.h"
#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#endif  //WIN32
//#include "robot_pic.h"

#if WIN32
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
#endif //WIN32

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons

int winRobot_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("Robot");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"Robot");
	strcpy(twin.title,"Robot");
	strcpy(twin.ititle,"Robot");
	twin.x=50;
	twin.y=50;
	twin.w=570;
	twin.h=470;
	//twin.Keypress=(FTControlfuncwk *)win_Robot_key;
	twin.AddFTControls=(FTControlfuncw *)winRobot_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winRobot_OnOpen;
	twin.OnClose=(FTControlfuncw *)winRobot_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winRobot_AddFTWindow



int winRobot_AddFTControls(FTWindow *twin)
{
	int fw,fh;
	FTControl *tcontrol;

	if (PStatus.flags&PInfo)
		fprintf(stderr,"Adding controls for window 'Robot'\n");

	//get font from window

	fw=twin->fontwidth;
	fh=twin->fontheight;

	tcontrol=(FTControl *)malloc(sizeof(FTControl));

	//newline
	//I'm just leaving this in for now- but I don't think it's going to be used on this window (only on the Motors, Accel, etc. windows)
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblRobotSrcIPAddressStr");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh;
	tcontrol->y2=fh*2+fh/2;
	if (RStatus.flags&ROBOT_STATUS_REMOTE) {
		strcpy(tcontrol->text,"Wired Source IP:");
	} else {
		strcpy(tcontrol->text,"Robot Source IP:");
	}
	AddFTControl(twin,tcontrol);

	//todo: make dropdown with each network interface
	/*
	//Linux: eth0: 192.168.1.1 (mac 12-34-56...) and wlan0: 192.168.0.130 (mac 12-34-56...)
	//Windows: "Local Area Connetion" mac#12-34-56- .. and "Wireless Network Connection"
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"ddRobotSrcIPAddressStr");
	tcontrol->type=CTTextBox;
	tcontrol->x1=fw*18;
	tcontrol->y1=fh;
	tcontrol->x2=fw*33; //15 characterrs
	tcontrol->y2=fh*2+fh/2;
	//todo: fill with wireless IP
	//InitLocalNetworks - fills RStatus with each local network ip, protocol, etc.
	//see getaddrinfo - use to make a dropdown with all available IP addresses
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520%28v=vs.85%29.aspx
	strcpy(tcontrol->text,"192.168.1.1");
	//tcontrol->OnChange=(FTControlfuncwc *)txtRobotSrcIPAddress_OnChange;
	AddFTControl(twin,tcontrol);
	*/
	memset(tcontrol,0,sizeof(FTControl));
	//strcpy(tcontrol->name,"txtRobotSrcIPAddressStr");
	strcpy(tcontrol->name,"ddRobotSrcIPAddressStr");
	tcontrol->type=CTDropdown;//CTTextBox;
	tcontrol->x1=fw*17;
	tcontrol->y1=fh;
	tcontrol->x2=fw*35; //15 characterrs
	tcontrol->y2=fh*2+fh/2;
	//todo: fill with wireless IP
	//InitLocalNetworks - fills RStatus with each local network ip, protocol, etc.
	//see getaddrinfo - use to make a dropdown with all available IP addresses
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520%28v=vs.85%29.aspx
	//strcpy(tcontrol->text,"192.168.1.1");
	//tcontrol->OnChange=(FTControlfuncwc *)txtRobotSrcIPAddress_OnChange;
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblRobotDestIPAddressStr");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*36;
	tcontrol->y1=fh;
	tcontrol->y2=fh*2+fh/2;
	strcpy(tcontrol->text,"Robot Dest IP:");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"txtRobotDestIPAddressStr");
	tcontrol->type=CTTextBox;
	tcontrol->x1=fw*50;
	tcontrol->y1=fh;
	tcontrol->x2=fw*75;
	tcontrol->y2=fh*2+fh/2;
//	strcpy(tcontrol->text,"192.168.1.2");
	//tcontrol->OnChange=(FTControlfuncwc *)txtRobotDestIPAddress_OnChange;
	AddFTControl(twin,tcontrol);

	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblOutsideSrcIPAddressStr");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh*3;
	tcontrol->y2=fh*4+fh/2;
	if (RStatus.flags&ROBOT_STATUS_REMOTE) {
		strcpy(tcontrol->text,"Wireless Src IP:");
	} else {
		strcpy(tcontrol->text,"Outside Src IP:");
	}
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"ddOutsideSrcIPAddressStr");
	tcontrol->type=CTDropdown;
	tcontrol->x1=fw*17;
	tcontrol->y1=fh*3;
	tcontrol->x2=fw*35;
	tcontrol->y2=fh*4+fh/2;
	//todo: fill with wireless IP
	//InitLocalNetworks - fills RStatus with each local network ip, protocol, etc.
	//see getaddrinfo - use to make a dropdown with all available IP addresses
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520%28v=vs.85%29.aspx
	//strcpy(tcontrol->text,"192.168.0.150");
	//tcontrol->OnChange=(FTControlfuncwc *)ddOutsideSrcIPAddressStr_OnChange;
	AddFTControl(twin,tcontrol);

	if (RStatus.flags&ROBOT_STATUS_REMOTE) {
		memset(tcontrol,0,sizeof(FTControl));
		strcpy(tcontrol->name,"chkUseWirelessInterface");
		tcontrol->type=CTCheckBox;
		tcontrol->x1=fw*36;
		tcontrol->y1=fh*3;
		tcontrol->x2=fw*68;
		tcontrol->y2=fh*4+fh/2;
		strcpy(tcontrol->text,"Use Wireless Interface");
		tcontrol->OnChange=(FTControlfuncwc *)chkUseWirelessInterface_OnChange;
		AddFTControl(twin,tcontrol);
	} else { //if (!(RStatus.flags&ROBOT_STATUS_REMOTE)) {
		//if controlling robot remotely this textbox is not used
		memset(tcontrol,0,sizeof(FTControl));
		strcpy(tcontrol->name,"lblOutsideDestIPAddressStr");
		tcontrol->type=CTLabel;
		tcontrol->x1=fw*36;
		tcontrol->y1=fh*3;
		tcontrol->y2=fh*4+fh/2;
		strcpy(tcontrol->text,"Outside Dest IP:");
		AddFTControl(twin,tcontrol);

		memset(tcontrol,0,sizeof(FTControl));
		strcpy(tcontrol->name,"txtOutsideDestIPAddressStr");
		tcontrol->type=CTTextBox;
		tcontrol->x1=fw*53;
		tcontrol->y1=fh*3;
		tcontrol->x2=fw*68;
		tcontrol->y2=fh*4+fh/2;
		//todo: fill with wireless IP
		//InitLocalNetworks - fills RStatus with each local network ip, protocol, etc.
		//see getaddrinfo - use to make a dropdown with all available IP addresses
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520%28v=vs.85%29.aspx
		strcpy(tcontrol->text,"192.168.0.95");
		//tcontrol->OnChange=(FTControlfuncwc *)txtOutsideDestIPAddress_OnChange;
		AddFTControl(twin,tcontrol);
	} //if (!(RStatus.flags&ROBOT_STATUS_REMOTE)) {

	//newline
	//tcontrol=(FTControl *)malloc(sizeof(FTControl));
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblSendInstructionToRobotRaw");

	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh*5;
	tcontrol->y2=fh*6+fh/2;
//	strcpy(tcontrol->text,"Send raw instruction: (m:4 dir:1 str:4 dur:16)"); //46 chars
	strcpy(tcontrol->text,"Send raw instruction:"); 
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"txtSendInstructionToRobotRaw");
	tcontrol->type=CTTextBox;//2; //textbox
	tcontrol->x1=fw*23;
	tcontrol->y1=fh*5;
	tcontrol->x2=fw*33;
	tcontrol->y2=fh*6+fh/2;
	strcpy(tcontrol->text,"");
	//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblGetRobotData");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*34;
	tcontrol->y1=fh*5;
	tcontrol->y2=fh*6+fh/2;
	strcpy(tcontrol->text,"Robot Data recvd:");
//	tcontrol->color[0]=wingray;  //bkground
//	tcontrol->textcolor[0]=black;//black;  //text
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"txtGetRobotData");
	tcontrol->type=CTTextBox;
	tcontrol->x1=fw*52;
	tcontrol->y1=fh*5;
	tcontrol->x2=fw*67;
	tcontrol->y2=fh*6+fh/2;
	strcpy(tcontrol->text,"");
	AddFTControl(twin,tcontrol);

//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblGetOutsideConnectData");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh*7;
	strcpy(tcontrol->text,"Outside Data recvd:");
	//tcontrol->color[0]=wingray;  //bkground
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"txtGetOutsideConnectData");
	tcontrol->type=CTTextBox;//2; //textbox
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*7;
	tcontrol->x2=fw*36;
	tcontrol->y2=fh*8+fh/2;
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
	strcpy(tcontrol->name,"lblPCBCount0");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*37;
	tcontrol->y1=fh*7;
	strcpy(tcontrol->text,"PCBs:");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblPCBCount1");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*43;
	tcontrol->y1=fh*7;
	strcpy(tcontrol->text,"0");
	tcontrol->textcolor[0]=0x0000ff;
	AddFTControl(twin,tcontrol);


	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRobotModelLog");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*14;
	strcpy(tcontrol->text,"Start Model Log");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRobotModelLog_Click;
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRobotNetworkLog");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*14;
	strcpy(tcontrol->text,"Start Net Log");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRobotNetworkLog_Click;
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"chkLogInData");
	tcontrol->type=CTCheckBox;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*56;
	tcontrol->y2=fh*10+fh/2;
	strcpy(tcontrol->text,"Log Incoming"); //Log Data In
	tcontrol->OnChange=(FTControlfuncwc *)chkLogInData_OnChange;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"chkLogOutData");
	tcontrol->type=CTCheckBox;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh*11;
	tcontrol->x2=fw*56;
	tcontrol->y2=fh*12+fh/2;
	strcpy(tcontrol->text,"Log Outgoing"); //Log Data Out
	tcontrol->OnChange=(FTControlfuncwc *)chkLogOutData_OnChange;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRobotModel");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*57;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*72;
	tcontrol->y2=fh*14;
	strcpy(tcontrol->text,"Robot Model");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRobotModel_Click;
	//tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)test_Click;
	AddFTControl(twin,tcontrol);

	//newline
	/*
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnConnectIP");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*14;
	strcpy(tcontrol->text,"Connect IPs");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnConnectIPs_Click;
	tcontrol->color[0]=ltgray; //button color background no press
	tcontrol->color[1]=dkgray; //button down color (and toggle color)
	tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);
	*/

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnSendInstructionToRobot");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*15;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*20;
	strcpy(tcontrol->text,"Send Instruction");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSendInstructionToRobot_Click;
	//tcontrol->color[0]=ltgray; //button color background no press
	//tcontrol->color[1]=dkgray; //button down color (and toggle color)
	//tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnLoadRobot");  //Load a Robot
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*15;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*20;
	strcpy(tcontrol->text,"Load Robot");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnLoadRobot_Click;
	//tcontrol->color[0]=ltgray; //button color background no press
	//tcontrol->color[1]=dkgray; //button down color (and toggle color)
	//tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);

/*
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnListenToOutside");  //was ListenToOutside- but Connect is more accurate since data will be going both in and out
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*9;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*14;
	strcpy(tcontrol->text,"Listen to Outside");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnListenToOutside_Click;
	tcontrol->color[0]=ltgray; //button color background no press
	tcontrol->color[1]=dkgray; //button down color (and toggle color)
	tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);
*/

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnIdentifyEthPCBs");  //send broadcast packet "01" instruction to have all connected Ethernet PCBs identify themselves
	tcontrol->type=CTButton;
	tcontrol->x1=fw*41;
	tcontrol->y1=fh*15;
	tcontrol->x2=fw*60;
	tcontrol->y2=fh*20;
	strcpy(tcontrol->text,"Identify Eth PCBs");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnIdentifyEthPCBs_Click;
	//tcontrol->color[0]=ltgray; //button color background no press
	//tcontrol->color[1]=dkgray; //button down color (and toggle color)
	//tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);


	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnMotors");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*21;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*26;
	strcpy(tcontrol->text,"Motors");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnMotors_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnAccel");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*21;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*26;
	strcpy(tcontrol->text,"Accelerometers");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccels_Click;
	AddFTControl(twin,tcontrol);

	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "btnAnalogSensors");
	tcontrol->type = CTButton;
	tcontrol->x1 = fw*41;
	tcontrol->y1 = fh*21;
	tcontrol->x2 = fw*60;
	tcontrol->y2 = fh*26;
	strcpy(tcontrol->text, "Analog Sensors");
	tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnAnalogSensors_Click;
	AddFTControl(twin, tcontrol);


	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "btnCameras");
	tcontrol->type = CTButton;
	tcontrol->x1 = fw*61;
	tcontrol->y1 = fh*21;
	tcontrol->x2 = fw*80;
	tcontrol->y2 = fh*26;
	strcpy(tcontrol->text, "Cameras");
	tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnCameras_Click;
	AddFTControl(twin, tcontrol);


	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnTasks");
	tcontrol->type=CTButton;
	tcontrol->x1=fw;
	tcontrol->y1=fh*27;
	tcontrol->x2=fw*20;
	tcontrol->y2=fh*32;
	strcpy(tcontrol->text,"Tasks");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnTasks_Click;
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnPower");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*21;
	tcontrol->y1=fh*27;
	tcontrol->x2=fw*40;
	tcontrol->y2=fh*32;
	strcpy(tcontrol->text,"Power");
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnPower_Click;
	AddFTControl(twin,tcontrol);



	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btnRobotExit");
	tcontrol->type=CTButton;
	tcontrol->x1=fw*48;
	tcontrol->y1=fh*27;
	tcontrol->x2=fw*68;
	tcontrol->y2=fh*32;
	strcpy(tcontrol->text,"Exit");  
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnRobotExit_Click;
	//tcontrol->color[0]=ltgray; //button color background no press
	//tcontrol->color[1]=dkgray; //button down color (and toggle color)
	//tcontrol->color[2]=gray;//mouse over color
	AddFTControl(twin,tcontrol);

	GetLocalNetworkInterfaces();

if (tcontrol!=0) {
	free(tcontrol);
}

if (PStatus.flags&PInfo)
  fprintf(stderr,"done with adding controls to outfile.\n");

//#endif
return 1;
} //end winRobot_AddFTControls



//Find a wired and wireless network interface, and their info (name, ip, etc.) on this machine
int GetLocalNetworkInterfaces(void) 
{
	FTControl *tcontrolWired,*tcontrolWireless;
	FTItem *titem;
	char LocalIPAddress[INET_ADDRSTRLEN],WiredIPAddress[INET_ADDRSTRLEN],WirelessIPAddress[INET_ADDRSTRLEN];
	char PhyAddrStr[20],WiredPhysicalAddressStr[20],WirelessPhysicalAddressStr[20];
	unsigned char WiredPhysicalAddress[6],WirelessPhysicalAddress[6];
	char InterfaceName[FTMedStr],WiredInterfaceName[FTMedStr],WirelessInterfaceName[FTMedStr];
	MAC_Connection *lmac;
	int FoundWired,FoundWireless;
#if Linux
	struct ifaddrs *ifaddr, *ifa;
	int family,n;
	void *tempAddrPtr = NULL;
#endif
#if WIN32
  DWORD dwSize = 0;
  DWORD dwRetVal = 0;
  PIP_ADAPTER_ADDRESSES pAddresses = NULL,lAddresses=NULL;
  ULONG outBufLen = 0;
  ULONG Iterations = 0;

  // Set the flags to pass to GetAdaptersAddresses
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
	int NameLen;
#endif


	memset(WiredIPAddress,0,sizeof(WiredIPAddress)); //so textbox isn't filled if not there
	memset(WirelessIPAddress,0,sizeof(WirelessIPAddress)); //so textbox isn't filled if not there
	FoundWired=0;
	FoundWireless=0;

//START populate local IP addresses
//this needs to be after the other controls have been created because currently a listen function uses one of the textboxes
#if Linux

		if (getifaddrs(&ifaddr) == -1) {
			 perror("getifaddrs");
			 //exit(EXIT_FAILURE);
		}

		// Walk through linked list, maintaining head pointer so we can free list later
    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;

			/* Display interface name and family (including symbolic
				form of the latter for the common families) */

/*			fprintf(stderr,"%-8s %s (%d)\n",
						ifa->ifa_name,
						(family == AF_PACKET) ? "AF_PACKET" :
						(family == AF_INET) ? "AF_INET" :
						(family == AF_INET6) ? "AF_INET6" : "???",
						family);
*/


			tempAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			//copy the IP address (from ifa_addr) to LocalIPAddress
			inet_ntop(family,tempAddrPtr,LocalIPAddress,sizeof(struct sockaddr_in));
//			inet_ntop(AF_INET,&DestAddress.sin_addr,DestIPAddressStr,INET_ADDRSTRLEN);
			sprintf(InterfaceName,"%s",ifa->ifa_name);				
			//strncpy(InterfaceName,ifa->ifa_name,4);
			sprintf(PhyAddrStr,"00:00:00:00:00");			
			//For an AF_INET* interface address, display the address
			if (family == AF_INET) {
				//
//				inet_ntop(family,&LocalAddress,LocalIPAddress,sizeof(struct sockaddr_in));
				if (!strncmp(InterfaceName,"enp",3) || !strncmp(InterfaceName,"eth",3) || !strncmp(InterfaceName,"ens",3)  || !strncmp(InterfaceName,"en",2)) {  //todo add to configuration file
				//if (!strncmp(InterfaceName,"eth",3)) {  //todo add to configuration file LINUX_WIRED_NETWORK_INTERFACE_NAME_TO_USE					
					strcpy(WiredInterfaceName,InterfaceName);			
					strcpy(WiredPhysicalAddressStr,PhyAddrStr);			
//								memcpy(WiredPhysicalAddress,lAddresses->PhysicalAddress,6);
					sprintf(WiredIPAddress,"%s",LocalIPAddress);
					FoundWired=1;
					fprintf(stderr,"Found wired interface: %s %s\n",WiredInterfaceName,WiredIPAddress);
				} //eth0
				if (!strncmp(InterfaceName,"wlp",3) || !strncmp(InterfaceName,"wlan",4) || !strncmp(InterfaceName,"wls",3)) {  //todo add to configuration file
				//if (!strncmp(InterfaceName,"wlan",3)) {  //todo add to configuration file LINUX_WIRELESS_NETWORK_INTERFACE_NAME_TO_USE
					strcpy(WirelessInterfaceName,InterfaceName);			
					strcpy(WirelessPhysicalAddressStr,PhyAddrStr);			
//								memcpy(WirelessPhysicalAddress,lAddresses->PhysicalAddress,6);
					strcpy(WirelessIPAddress,LocalIPAddress);			
					FoundWireless=1;
					fprintf(stderr,"Found wireless interface: %s %s\n",WirelessInterfaceName,WirelessIPAddress);
				} //wlan0
			} //if (family == AF_INET) {

			//printf("\t\taddress: %s\n", LocalIPAddress);
			//printf("\t\tname: <%-8s>\n", ifa->ifa_name);
			//printf("\t\tname: %s\n",InterfaceName);

		} //for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {

    freeifaddrs(ifaddr);

#endif //Linux
#if WIN32

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (pAddresses == NULL) {
            printf
                ("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
            exit(1);
        }

        dwRetVal=GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

	//go through each
	lAddresses=pAddresses;
	while(lAddresses!=0) {		
		inet_ntop(AF_INET,&(((struct sockaddr_in*)lAddresses->FirstUnicastAddress->Address.lpSockaddr)->sin_addr),LocalIPAddress,INET_ADDRSTRLEN);
		sprintf(PhyAddrStr,"%02x%02x%02x%02x%02x%02x",lAddresses->PhysicalAddress[0],lAddresses->PhysicalAddress[1],lAddresses->PhysicalAddress[2],lAddresses->PhysicalAddress[3],lAddresses->PhysicalAddress[4],lAddresses->PhysicalAddress[5]);
		//MultiByteToWideChar(CP_UTF8,0,(LPCSTR)InterfaceFriendlyName,-1,(LPWSTR)pAddresses->FriendlyName,FTMedStr);
		NameLen=wcslen(lAddresses->FriendlyName);
		WideCharToMultiByte(CP_UTF8,0,lAddresses->FriendlyName,NameLen,(LPSTR)InterfaceName,sizeof(InterfaceName),NULL,NULL);
		InterfaceName[NameLen]=0; //add termination
		if (!strcmp(InterfaceName,"Local Area Connection")) {  //todo add to configuration file WINDOWS_WIRED_NETWORK_INTERFACE_NAME_TO_USE
			strcpy(WiredInterfaceName,InterfaceName);			
			strcpy(WiredPhysicalAddressStr,PhyAddrStr);			
			memcpy(WiredPhysicalAddress,lAddresses->PhysicalAddress,6);
			strcpy(WiredIPAddress,LocalIPAddress);			
			FoundWired = 1;
		}
		if (!strcmp(InterfaceName,"Wireless Network Connection")) {  //todo add to configuration file WINDOWS_WIRELESS_NETWORK_INTERFACE_NAME_TO_USE
			strcpy(WirelessInterfaceName,InterfaceName);			
			strcpy(WirelessPhysicalAddressStr,PhyAddrStr);			
			memcpy(WirelessPhysicalAddress,lAddresses->PhysicalAddress,6);
			strcpy(WirelessIPAddress,LocalIPAddress);			
			FoundWireless = 1;
		}

		lAddresses=lAddresses->Next;
	} //while
	if (pAddresses!=0) {
		free(pAddresses);
		pAddresses=NULL;
	}
//#endif 
#endif //WIN32

	//Add and populate Itemlist for both Robot Source IP and Outside Source IP
	tcontrolWired=GetFTControl("ddRobotSrcIPAddressStr");
	tcontrolWireless=GetFTControl("ddOutsideSrcIPAddressStr");
	if (tcontrolWired!=0 && tcontrolWireless!=0) {
		titem=(FTItem *)malloc(sizeof(FTItem));
		memset(titem,0,sizeof(FTItem));
		//Wired Interface

		//sprintf(titem->name,"%s IP:%s MAC:%s",InterfaceFriendlyName,LocalIPAddress,PhyAddr);			
		//wcscpy(litem->name_16,fileinfo.cFileName);
		
		if (!FoundWired) {
			fprintf(stderr,"No wired interface found.\n");
	  } else {
	
			//Create a MAC_Connection record for the wired MAC
			//lmac=Get_MAC_Connection(WiredIPAddress); //later add port# too
			lmac=RStatus.WiredInterface;
			strcpy(RStatus.WiredIPAddressStr,WiredIPAddress); //quick pointer to IP address string
			if (lmac==0) {
				//Local source Connection doesn't exist yet so create it
				//todo: if it does exist then go through, copy new src and dest ip, and reconnect?
				lmac=Create_Local_MAC_Connection(WiredIPAddress,0); //0=wired
				if (lmac==0) {
					fprintf(stderr,"Create_Local_MAC_Connection to %s failed\n",WiredIPAddress);
				} else {
					strcpy(lmac->Name,WiredInterfaceName);
					strcpy(lmac->PhysicalAddressStr,WiredPhysicalAddressStr);
					memcpy(lmac->PhysicalAddress,WiredPhysicalAddress,6);
					RStatus.WiredInterface=lmac; //quick pointer to wired interface for sending data
					//strcpy(lmac->SrcIPAddressStr,WiredIPAddress);  //is done in Create_Local_MAC_Connection anyway
					sprintf(titem->name,"%s",WiredIPAddress);
					AddFTItem(tcontrolWired,titem);
					SelectFTItemByNum(tcontrolWired->ilist,1,0);
					//once ROBOT_STATUS_LISTEN_TO_ROBOT is set, the Thread_ListenToRobot will proceed
					//before this RStatus.WiredInterface, and all lmac variables must be set
					RStatus.flags|=ROBOT_STATUS_LISTEN_TO_ROBOT; //thread will start listening now
				} //if (lmac==0) {

			} //if (lmac==0) {

	//send a broadcast instruction "01" to receive the name of any connected ETH PCBs (MOTOR00, ACCEL00, etc)
		SendBroadcastInstruction();
		SendBroadcastInstruction(); //twice because first time PCBs can't resend yet


		} //if (!FoundWired) {


		//Wireless Interface
		if (!FoundWireless) {
			fprintf(stderr,"No wireless interface found.\n");
	  } else {
			memset(titem,0,sizeof(FTItem));
			//Create a MAC_Connection record for the wireless MAC
			//lmac=Get_MAC_Connection(WirelessIPAddress); //later add port# too 0=IsLocal
			strcpy(RStatus.WirelessIPAddressStr,WirelessIPAddress); //quick pointer to IP address string
			lmac=RStatus.WirelessInterface;
			if (lmac==0) {
				//Local source Connection doesn't exist yet so create it
				//todo: if it does exist then go through, copy new src and dest ip, and reconnect?
				lmac=Create_Local_MAC_Connection(WirelessIPAddress,1); //1=wireless
				if (lmac==0) {
					fprintf(stderr,"Create_Local_MAC_Connection to %s failed\n",WirelessIPAddress);
				} else {
					RStatus.WirelessInterface=lmac; //quick pointer to wireless interface for sending data
					strcpy(lmac->Name,WirelessInterfaceName);
					strcpy(lmac->PhysicalAddressStr,WirelessPhysicalAddressStr);
					memcpy(lmac->PhysicalAddress,WirelessPhysicalAddress,6);
					//strcpy(lmac->SrcIPAddressStr,WirelessIPAddress);  //is done in Create_Local_MAC_Connection anyway
					sprintf(titem->name,"%s",WirelessIPAddress);
					AddFTItem(tcontrolWireless,titem);
					SelectFTItemByNum(tcontrolWireless->ilist,1,0);
					//once ROBOT_STATUS_LISTEN_TO_OUTSIDE is set, the Thread_ListenToOutside will proceed
					//before this RStatus.WirelessInterface, and all lmac variables must be set
					RStatus.flags|=ROBOT_STATUS_LISTEN_TO_OUTSIDE; //thread will start listening now
				} //if (lmac==0) {

			} //if (lmac==0) {
		} //if (!FoundWireless) {
	

		free(titem);
		return(1);
	} //if (tcontrolWired!=0 && tcontrolWireless!=0) {
	//END populate local IP addresses
	return(0);
} //int GetLocalNetworkInterfaces(void) 


//Open the winRobotModel window
int btnRobotModel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];

	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	twin2=GetFTWindow("winRobotModel");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winRobotModel_AddFTWindow();
	}
	return(1);
} //int btnRobotModel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int chkUseWirelessInterface_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol->value) {
		RStatus.flags|=ROBOT_STATUS_USE_WIRELESS_INTERFACE;
	} else {
		RStatus.flags&=~ROBOT_STATUS_USE_WIRELESS_INTERFACE;
	}
	return(1);
} //int chkUseWirelessInterface_OnChange(FTWindow *twin,FTControl *tcontrol)

//Start/Stop Robot Model Log
int btnRobotModelLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{

	//determine if Starting or Stopping the log
	if (!strcmp(tcontrol->text,"Start Model Log")) {
//now done in the below Open and Close functions		strcpy(tcontrol->text,"Stop Model Log");
		OpenRobotModelLogFile();
	} else {
//		strcpy(tcontrol->text,"Start Model Log");
		CloseRobotModelLogFile();
	}

	return(1);
} //int btnRobotNetworkLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Start/Stop Robot Network Log
int btnRobotNetworkLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{

	//determine if Starting or Stopping the log
	if (!strcmp(tcontrol->text,"Start Net Log")) {
		strcpy(tcontrol->text,"Stop Net Log");
		OpenRobotNetworkLogFile();
	} else {
		strcpy(tcontrol->text,"Start Net Log");
		CloseRobotNetworkLogFile();
	}

	return(1);
} //int btnRobotNetworkLog_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)




int chkLogInData_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol->value) {
		RStatus.flags|=ROBOT_STATUS_LOG_INCOMING_DATA;
	} else {
		RStatus.flags&=~ROBOT_STATUS_LOG_INCOMING_DATA;
	}
	return(1);
} //int chkLogInData_OnChange(FTWindow *twin,FTControl *tcontrol)

int chkLogOutData_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	if (tcontrol->value) {
		RStatus.flags|=ROBOT_STATUS_LOG_OUTGOING_DATA;
	} else {
		RStatus.flags&=~ROBOT_STATUS_LOG_OUTGOING_DATA;
	}
	return(1);
} //int chkLogOutData_OnChange(FTWindow *twin,FTControl *tcontrol)

//Open the winMotors window
int btnMotors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];


	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	twin2=GetFTWindow("winMotors");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winMotors_AddFTWindow();
	}
	return(1);
} //int btnMotors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Open the winAccels window
int btnAccels_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];


	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	twin2=GetFTWindow("winAccels");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winAccels_AddFTWindow();
	}
	return(1);
} //int btnAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Open the winAnalogSensors window
int btnAnalogSensors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];


	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {


	twin2=GetFTWindow("winAnalogSensors");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winAnalogSensors_AddFTWindow();
	}
	return(1);
} //int btnAnalogSensors_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Open the winCameras window
int btnCameras_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	FTWindow *twin2;
	char tstr[256];

	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	
	twin2 = GetFTWindow("winCameras");
	if (twin2 != 0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2, 0);
	}
	else {
		//open the window
		winCameras_AddFTWindow();
	}
	
	return(1);
} //int btnCameras_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Open the win_LoadRobot window
int btnLoadRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;

	twin2=GetFTWindow("winLoadRobot");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winLoadRobot_AddFTWindow();
	}
	return(1);
} //int btnLoadRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Send a broadcast packet to identify all connected Ethernet PCBs (motors, accelerometers, etc.)
int btnIdentifyEthPCBs_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	DeleteAllExternalPCBConnections();  //delete all external MAC_Connections, other PCB count will be stale
/*
#if Linux
	usleep(1000000); //sleep 1s
#endif
#if WIN32
	Sleep(1000); //wait 1 second
#endif
*/
	SendBroadcastInstruction();
	return(1);
} //int btnIdentifyEthPCBs_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Open the winTasks window
int btnTasks_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];

	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	twin2=GetFTWindow("winTasks");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winTasks_AddFTWindow();
	}
	return(1);
} //int btnTasks_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Open the winPower window
int btnPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;
	char tstr[256];


	if (!(RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED)) {
		sprintf(tstr,"A robot must be loaded first");
		FTMessageBox(tstr,FTMB_OK,"Robot -- Info",0);
		return(0);
	} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

	twin2=GetFTWindow("winPower");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winPower_AddFTWindow();
	}
	return(1);
} //int btnPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Create a MAC_Connection for a (usually wireless) MAC to the outside (not to the robot PCBs)
//This function doesn't connect to any remote MACs
int btnListenToOutside_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTControl *tc;
	MAC_Connection *lmac;

	//See if a local MAC_Connection exists for this MAC and if it has a socket, is binded, and has a listening thread
	//the outside Source IP is usually the wireless MAC IP address 
	//(and different from inside Source which is usually the wired MAC IP address)
	if (!strcmp(tcontrol->text,"Listen to Outside")) {
		lmac=ListenToOutside();
//			FTMessageBox("Already listening to outside",FTMB_OK,"Robot -- Info",0);
//			strcpy(tcontrol->text,"Stop Listen to Outside");		//may be an error so recopy
	} else { //	if (!strcmp(tcontrol->text,"Listen to Outside")) {
		//stop listening

		//see if this connection exists
		tc=GetFTControl("ddOutsideSrcIPAddressStr");
		if (tc==0) {		
			FTMessageBox("Could not find control ddOutsideSrcIPAddressStr",FTMB_OK,"Robot -- Info",0);
			strcpy(tcontrol->text,"Disconnect IPs");		//may be an error so recopy
			return(0);
		}

		lmac=Get_MAC_Connection(tc->text); //later add port# too, 0=IsLocal
		if (lmac==0) {
			FTMessageBox("Not listening yet",FTMB_OK,"Robot -- Info",0);
			//change button text
			strcpy(tcontrol->text,"Listen to Outside");		
			return(0);
		} //if (lmac==0) {

		//DeleteMAC_Connection will disconnect it, and close any open socket
		Delete_MAC_Connection(lmac); //or reuse?

//			FTMessageBox("Already not listening to outside",FTMB_OK,"Robot -- Info",0);
//			strcpy(tcontrol->text,"Listen to Outside");		//may be an error so recopy
	}  //	if (!strcmp(tcontrol->text,"Listen to Outside")) {
	DrawFTControl(tcontrol);
	return(1);
} //int btnListenToOutside_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

MAC_Connection *ListenToOutside(void) {
	MAC_Connection *lmac;
	FTControl *tc,*tcontrol;
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
#endif

	//see if connection already exists
	tc=GetFTControl("ddOutsideSrcIPAddressStr");
	if (tc==0) {
		FTMessageBox("Could not find control ddOutsideSrcIPAddressStr",FTMB_OK,"Robot -- Info",0);
		//strcpy(tc->text,"Disconnect IPs");		//may be an error so recopy
		return(0);
	}
	

	//replace with lmac=Create_MAC_Connection(tcSrcIP->text,tcDestIP->text); or even just Create_NIC(SrcIP,DestIP)
	//tc=GetFTControl("txtOutsideDestIPAddressStr");	
	lmac=Get_MAC_Connection(tc->text); //later add port# too
	if (lmac==0) {
		//MAC_Connection doesn't exist yet so create it
		//todo: if it does exist then go through, copy new src and dest ip, and recreate?

		//fill MAC_Connection details
		lmac=(MAC_Connection *)malloc(sizeof(MAC_Connection));
		if (lmac==0) {
			fprintf(stderr,"malloc of MAC_Connection failed\n");
			return(0);
		}
		memset(lmac,0,sizeof(MAC_Connection));
		lmac->flags|=ROBOT_MAC_CONNECTION_IS_LOCAL;
		strcpy(lmac->SrcIPAddressStr,tc->text);
		lmac->SrcPort=DEFAULT_ROBOT_PORT;
		//probably the destination will be 0.0.0.0 initially
		//and it doesn't matter for just creating and socket and listening (binding)
		//but if one is given store that one in this MAC_Connection for now
		tc=GetFTControl("txtOutsideDestIPAddressStr");	
		strcpy(lmac->DestIPAddressStr,tc->text);
		lmac->DestPort=DEFAULT_ROBOT_PORT;

		lmac->SrcAddress.sin_family=AF_INET;
		lmac->SrcAddress.sin_addr.s_addr=inet_addr(lmac->SrcIPAddressStr); 
		lmac->SrcAddress.sin_port=htons(lmac->SrcPort);

		lmac->DestAddress.sin_family=AF_INET;
		lmac->DestAddress.sin_addr.s_addr=inet_addr(lmac->DestIPAddressStr); 
		lmac->DestAddress.sin_port=htons(lmac->DestPort);

		//Open a socket for this MAC_Connection
		if (!Open_MAC_Socket(lmac)) {
			fprintf(stderr,"Open_MAC_Socket failed\n");
			return(0); 
		}

		//don't connect yet, we will connect if and when we need to send any data back outside

		//add MAC_Connection to the list of other MAC_Connections
		if (!Add_MAC_Connection(lmac)) {
			fprintf(stderr,"Add_MAC_Connection failed\n");
			free(lmac);
			return(0); 
		}	


	} //if (lmac==0) {

	//the ListenToOutside thread calls Get_MAC_Connection() to get the socket, so the MAC_Connection must exist when the thread is created
	//create the receive thread if it doesn't already exist
	if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
#if Linux
		//set thread priority
		pthread_attr_init(&tattr);
		pthread_attr_getschedparam(&tattr,&param);
		param.sched_priority=98;
		pthread_attr_setschedparam(&tattr,&param);
		pthread_create(&lmac->ReceiveThread,&tattr,(void *)Thread_ListenToOutside,NULL);
#endif
#if WIN32
		lmac->ReceiveThread=CreateThread(0,0,Thread_ListenToOutside,0,0,0);
#endif
		lmac->flags|=ROBOT_MAC_CONNECTION_LISTENING;
	} //	if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {

	//change button text
	tcontrol=GetFTControl("btnListenToOutside");
	if (tcontrol!=0) {
		strcpy(tcontrol->text,"Stop Listen Outside");		
	}
	
	return(lmac);	
} //MAC_Connection *ListenToOutside(void) {

void btnSendInstructionToRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	char tkey[5];
	FTControl *tc;
	int NumChars;
	unsigned char inst[512];

	//string is in hex, for example 0d55aa
	//strip off any "0x"
	//check to see if there is an instruction in the Send raw instruction text box

	tc=GetFTControl("txtSendInstructionToRobotRaw");

	if (tc!=0) {

		if (strlen(tc->text)>0) { //there is a command there

			//now sending a raw instruction to the robot includes the PIC instruction (GET_REG, SET_REG, etc.)
			//presumes instruction is in hexadecimal (without 0x)
			NumChars=strlen(tc->text);
			if (NumChars>0) {
				//convert ascii to numbers
				tkey[1]=0;
				for(i=0;i<NumChars/2;i++) {
					tkey[0]=tc->text[i*2];
					inst[i]=strtol(tkey,0,16);
					inst[i]<<=4;
					tkey[0]=tc->text[i*2+1];
					inst[i]|=strtol(tkey,0,16);//atoi(tkey);				
				} //for(i=0;i<NumChars/2;i++) {

				SendInstructionToRobot(inst,NumChars/2,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
			} //if (NumChars>0) {

		} //	if (strlen(tc->text)>0) { //there is a command there

	} //if (tc!=0) {

}  //void btnSendInstructionToRobot_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//
//Checks to see if the source and dest (robot) MAC_Connections exist first
//and if yes, sends the data to the dest
//flags:
//ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION - add the robot source IP address before the instruction (otherwise there should be some other source IP address in the first 4 bytes of the instruction)
//Note that raw instructions must already have integer values (like duration, memory address, memory data, etc.) in little endian format
int SendInstructionToRobot(unsigned char *Inst,int numbyte,unsigned int flags)
{
	int i;
	FTControl *tc;
	int iResult=0;
	unsigned char SendInst[ROBOT_MAX_INST_LEN]; //
	MAC_Connection *lmac;
	int NumBytes;
	char DataStr[512];
	char SrcIP[INET_ADDRSTRLEN],DestIP[INET_ADDRSTRLEN];


	//fprintf(stderr,"SendInstructionToRobot\n");

	//get the destination IP
	tc=GetFTControl("txtRobotDestIPAddressStr"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control txtRobotDestIPAddressStr\n");
		return(0);
	}
	if (strlen(tc->text)==0) {
		fprintf(stderr,"You must put an IP address in the destination IP address textbox.\n");
		return(0);
	} 
	strcpy(DestIP,tc->text); 

	//determine source IP to send to robot
	strcpy(SrcIP,RStatus.WiredInterface->SrcIPAddressStr); //default is wired
	if (RStatus.flags&ROBOT_STATUS_REMOTE) {
		//controlling robot remotely, so determine which interface to send on
		if (RStatus.flags&ROBOT_STATUS_USE_WIRELESS_INTERFACE) {
			if (RStatus.WirelessInterface!=0) {
				strcpy(SrcIP,RStatus.WirelessInterface->SrcIPAddressStr);
			} else {
				fprintf(stderr,"SendInstructionToRobot() called in remote mode, using wireless interface- but no Wireless Interface IP connection\n");
				return(0);
			} //if (RStatus.WirelessInterface!=0) {
		} //if (RStatus.flags&ROBOT_STATUS_USE_WIRELESS_INTERFACE) {
	} //if (RStatus.flags&ROBOT_STATUS_REMOTE) {



	//see if source and dest IPs already have a socket
	lmac = Get_MAC_Connection_By_IPs(SrcIP, DEFAULT_HOST_PORT, DestIP, DEFAULT_ROBOT_PORT);
	if (lmac==0) {
		//create a socket between source and dest IPs
		lmac=Connect2IPs(SrcIP,DestIP);
	}


	//lmac=Get_MAC_Connection(tc->text); //get remote MAC_Connection		
	if (lmac==0) {
		fprintf(stderr,"Could not get MAC_Connection for source %s and dest %s\n",SrcIP,DestIP);
		return(0);
	}


	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "Send from %s to %s\n",lmac->SrcIPAddressStr,lmac->DestIPAddressStr);
		fprintf(stderr, "%d bytes: ",numbyte);
		for(i=0;i<numbyte;i++) {
			fprintf(stderr, "0x%02x ",Inst[i]);
		}
	}


	//if the instruction originates on this machine, copy the source IP Address
	if (flags&ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION) {
		memcpy(SendInst,(unsigned int *)&lmac->SrcAddress.sin_addr.s_addr,4);
		memcpy(SendInst+4,(const char *)Inst,numbyte);
		numbyte+=4;
	} else {
		//otherwise the IP is presumably already at the beginning of the instruction
		memcpy(SendInst,(const char *)Inst,numbyte);
	}
	//iResult = send(lmac->Socket,(const char *)SendInst,numbyte,0);
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

	return(1);
} //int SendInstructionToRobot(unsigned int Inst)

void btnRobotExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{

	PStatus.flags|=PExit;  //exit program - this will close any other open windows

}  //btnRobotExit_Click


int winRobot_OnOpen(FTWindow *twin)
{
	//possibly automatically connect to robot

	//send a broadcast (192.168.

	return(1);
} //int winRobot_OnOpen(FTWindow *twin)

int winRobot_OnClose(FTWindow *twin)
{
//	DisconnectFromRobot();

	//Stop getting accel and touch sensor samples
	//StopAllAccelsAndTouchSensors();


	if (PStatus.flags&PInfo) {
		fprintf(stderr,"winRobot_OnClose\n");
	}
	//stop all threads - is done in CloseRobot()
//	StopAllRobotThreads();//Thread_ListenToRobot


	if (RStatus.flags&ROBOT_STATUS_NETWORK_LOG) {
		CloseRobotNetworkLogFile();
	}
	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		CloseRobotModelLogFile();
	}



	//because other windows (winPower) have OnClose functions
	//that need MAC connections, only do this before
	//this window will be closed and destroyed by freethought

//	FTMessageBox("Close Click",FTMB_OK,"Info");
//DisconnectFromRobot();
	Delete_All_MAC_Connections();  //close all sockets, binds, and connections

//CloseFTWindow(twin); //causes core dump
	
	FreeAllRobotTasks(); //free memory malloc'd for all RobotTasks
	FreeAllRobotScripts(); //free memory malloc'd for all RobotScripts

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"Done winRobot_OnClose()\n");
	}

	return(1);
} //int winRobot_OnClose(FTWindow *twin)


void test_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{

	char Test[256];
	FTControl *tc;

	fprintf(stderr, "Name (location) of PCB on Robot: unknown\n");
	//because the user needs to change between Motor PCBs in the Motor window
	//add a unique generic PCBName
	strcpy(Test, "123456789012");

	//open window so user can choose which Name/location is assocaited with this Eth PCB
	win_NewEthPCB_CreateFTWindow();

	//set mac address
	tc = GetFTControl("lblNewEthPCB_MacNum");
	if (tc != 0) {
		sprintf(tc->text, "%s", Test);
		DrawFTControl(tc);
	} 
} //test_Click
