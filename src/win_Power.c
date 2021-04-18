//win_Power.c
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
#include "win_Power.h"
#include "win_Robot.h"
#include "robot_power_mcu_instructions.h"

#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  //WIN32


//#define DEFAULT_MOTOR_BUFLEN 512

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons


int winPower_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winPower");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winPower");
	strcpy(twin.title,"Power");
	strcpy(twin.ititle,"Power");
	twin.x=50;
	twin.y=560;
	twin.w=580;
	twin.h=400;
	twin.AddFTControls=(FTControlfuncw *)winPower_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winPower_OnOpen;
	twin.OnClose=(FTControlfuncw *)winPower_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winPower_AddFTWindow



int winPower_AddFTControls(FTWindow *twin)
{
int fw,fh,i;
FTControl *tcontrol,*tcontrol2;
FTItem *titem;
//MAC_Connection *lmac;

#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winPower'\n");

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
strcpy(tcontrol->name,"lblPowerPCBNum");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"PCB:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"ddPowerPCBNum");
tcontrol->type=CTDropdown;
tcontrol->x1=fw*6;
tcontrol->y1=fh;
tcontrol->x2=fw*31;
tcontrol->y2=fh*2+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)ddPowerPCBNum_OnChange;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerMACStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*32;
tcontrol->y1=fh;
tcontrol->x2=fw*50; //need because would otherwise be too short
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"MAC: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*51;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtPowerDestIPAddressStr");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*55;
tcontrol->y1=fh;
tcontrol->x2=fw*74;
tcontrol->y2=fh*2+fh/2;
AddFTControl(twin,tcontrol);



//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerDesc");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Description");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerVolt");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Voltage");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerCurrent");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Current");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerMaxCurrent");
tcontrol->type=CTLabel;
tcontrol->x1=fw*40;
tcontrol->y1=fh*3;
strcpy(tcontrol->text,"Max Current");
AddFTControl(twin,tcontrol);

//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSource");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*5;
strcpy(tcontrol->text,"Source:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceVolt");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*5;
tcontrol->x2=fw*29;
strcpy(tcontrol->text,"? V");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceCurrent");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*5;
tcontrol->x2=fw*39;
strcpy(tcontrol->text,"? A");
AddFTControl(twin,tcontrol);

//newline 3V
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSource3V");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*7;
strcpy(tcontrol->text,"3.3V out:");
AddFTControl(twin,tcontrol);

/*
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceVolt3V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*7;
tcontrol->x2=fw*29; //need or else field is too short
strcpy(tcontrol->text,"? V");
AddFTControl(twin,tcontrol);
*/

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceCurrent3V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*7;
tcontrol->x2=fw*39;
strcpy(tcontrol->text,"? A");
AddFTControl(twin,tcontrol);

/*
//newline 12V
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSource12V");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*9;
strcpy(tcontrol->text,"12V out:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceVolt12V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*9;
tcontrol->x2=fw*29;
strcpy(tcontrol->text,"? V");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceCurrent12V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*9;
tcontrol->x2=fw*39;
strcpy(tcontrol->text,"? A");
AddFTControl(twin,tcontrol);
*/


//newline 36V
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSource36V");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*11;
strcpy(tcontrol->text,"36V out:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceVolt36V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*11;
tcontrol->x2=fw*29;
strcpy(tcontrol->text,"? V");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceCurrent36V");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*11;
tcontrol->x2=fw*39;
strcpy(tcontrol->text,"? A");
AddFTControl(twin,tcontrol);



//newline Source:
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceWallOrBat");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*13;
strcpy(tcontrol->text,"Power source:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblPowerSourceCharging");
tcontrol->type=CTLabel;
tcontrol->x1=fw*20;
tcontrol->y1=fh*13;
tcontrol->x2=fw*39;
strcpy(tcontrol->text,"Charging Status:");
AddFTControl(twin,tcontrol);




//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblSendInstructionToPowerRaw");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*15;
tcontrol->x2=fw*47; //error: for some reason this needs to be longer than the text box position
tcontrol->y2=fh*16+fh/2;
strcpy(tcontrol->text,"Send raw instruction: (m:4 dir:1 str:4 dur:16)"); //46 chars
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtSendInstructionToPowerRaw");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*48;
tcontrol->y1=fh*15;
tcontrol->x2=fw*60;
tcontrol->y2=fh*16+fh/2;
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
strcpy(tcontrol->name,"lblGetPowerData");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*17;
tcontrol->x2=fw*21;
tcontrol->y2=fh*18+fh/2;
strcpy(tcontrol->text,"Data received: ");
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGetPowerData");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*21;
tcontrol->y1=fh*17;
tcontrol->x2=fw*35;
tcontrol->y2=fh*18+fh/2;
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
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnUpdate");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*19;
tcontrol->x2=fw*19;
tcontrol->y2=fh*24;
strcpy(tcontrol->text,"Update");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnUpdatePower_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnToggle3V");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*25;
tcontrol->x2=fw*19;
tcontrol->y2=fh*30;
strcpy(tcontrol->text,"Turn On 3.3V");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnToggle3V_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnToggle36V");
tcontrol->type=CTButton;
tcontrol->x1=fw*20;
tcontrol->y1=fh*25;
tcontrol->x2=fw*39;
tcontrol->y2=fh*30;
strcpy(tcontrol->text,"Turn On 36V");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnToggle36V_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSendInstructionToPower");
tcontrol->type=CTButton;
tcontrol->x1=fw*40;
tcontrol->y1=fh*25;
tcontrol->x2=fw*59;
tcontrol->y2=fh*30;
strcpy(tcontrol->text,"Send Instruction");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSendInstructionToPower_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinPowerExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*60;
tcontrol->y1=fh*25;
tcontrol->x2=fw*79;
tcontrol->y2=fh*30;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinPowerExit_Click;
//tcontrol->color[0]=ltgray; //button color background no press
//tcontrol->color[1]=dkgray; //button down color (and toggle color)
//tcontrol->color[2]=gray;//mouse over color
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



int Populate_EthPower_Dropdown(FTControl *tc)
{
int NumPowerPCBs;
FTControl *tc2;
FTItem *titem;
MAC_Connection *lmac,*tmac;

//Even though there can only be 1 EthPower PCB 
//this is a dropdown control just for consistency


//Delete all existing dropdown items
DelAllFTItems(tc);
//Go through each MAC_Connection and get all names that exist for this PCB yet
//lmac=Get_MAC_Connection_By_WindowFunction((FTControlfunc *)winPower_AddFTWindow);
NumPowerPCBs=0;
if (tc!=0) {
	titem=(FTItem *)malloc(sizeof(FTItem));
	lmac=RStatus.iMAC_Connection;	
	//for all the MAC_Connections, compare the AddWindowFunction (could compare name to Motor too)
	while(lmac!=0) {
		if (lmac!=0) {
			if (lmac->AddWindowFunction==(FTControlfunc *)winPower_AddFTWindow) {
				//strcpy(tcontrol->text,"192.168.1.1");
				memset(titem,0,sizeof(FTItem));
				sprintf(titem->name,"%s",lmac->pcb.Name);
				//sprintf(titem->name,"%i",lmac->Num);
				//strcpy(titem->name,lmac->Name);
				AddFTItem(tc,titem);
				if (NumPowerPCBs==0) {
					tmac=lmac; //remember the first Motor PCB
				}
				NumPowerPCBs++;
			} //if (lmac->AddWindowFunction==(FTControlfunc *)winPower_AddFTWindow) {
		} 
		lmac=lmac->next;
	}//while(lmac!=0) {
	free(titem);

	if (NumPowerPCBs>0) {
		SelectFTItemByNum(tc->ilist,1,0); //select the first item
		lmac=tmac;
// is done in ddPowerPCBNum_OnChange, but only when there is a change
		//populate Mac address and IP
		tc2=GetFTControl("lblPowerMACStr");
//    fprintf(stderr,"Mac Address: %s\n",lmac->PhysicalAddressStr);
		if (tc2!=0) {		
			sprintf(tc2->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc2);
		}
		tc2=GetFTControl("txtPowerDestIPAddressStr");
		if (tc2!=0) {		
			sprintf(tc2->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc2);
		}

	} //if (NumMotorPCBs>0) {

	//redraw dropdown
	DrawFTControl(tc);
	return(NumPowerPCBs);
} //tc!=0
return(0);
} //int Populate_EthPower_Dropdown(FTControl *tc)


int ddPowerPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol) {
	MAC_Connection *lmac;
	FTControl *tc;
	
	//fprintf(stderr,"OnChange\n");
	//if the user changed Motor PCBs update the MAC and IP
	lmac=Get_MAC_Connection_By_PCBName(tcontrol->text); //get remote MAC_Connection
	if (lmac!=0) {
		//MAC
		tc=GetFTControl("lblPowerMACStr");
		if (tc) {
			sprintf(tc->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc);
		} //tc
		//IP
		tc=GetFTControl("txtPowerDestIPAddressStr");
		if (tc) {
			sprintf(tc->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc);
		} //tc
		return(1);
	} //if (lmac==0) {
	return(0);
} //int ddPowerPCBNum_OnChange(FTWindow *twin,FTControl *tcontrol) {


void btnSendInstructionToPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	FTControl *tc;
	int NumChars,NumBytes;
	unsigned char inst[512];
	int SentRawInstruction;

	//check to see if there is an instruction in the Send raw instruction text box


	SentRawInstruction=0;
	tc=GetFTControl("txtSendInstructionToPowerRaw");

	if (tc!=0) {
		if (strlen(tc->text)>0) { //there is a command there
			SentRawInstruction=1;
			//presumes instruction is in hexadecimal (without 0x)
			NumChars=strlen(tc->text);
			if (NumChars>0) {
				//convert ascii to numbers
				NumBytes=ConvertStringToBytes((unsigned char *)tc->text,inst); //NumBytes=NumChars/2
				//SendInstructionToMotor(inst,NumChars/2,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
				SendUserInstructionToPower(inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
			} //if (NumChars>0) {
		} //	if (strlen(tc->text)>0) { //there is a command there
	} //	if (tc!=0) {

}  //void btnSendInstructionToPower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//sends to the selected PCB in the winPower window
int SendUserInstructionToPower(unsigned char *Inst,int numbyte,unsigned int flags)
{
	FTControl *tc;
	MAC_Connection *lmac;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"SendUserInstructionToPower\n");
	}


	//get the MAC_Connection
	tc=GetFTControl("txtPowerDestIPAddressStr"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control txtPowerDestIPAddressStr\n");
		return(0);
	}

//	fprintf(stderr,"get mac connection %s\n",tc->text);
	lmac=Get_MAC_Connection(tc->text); //get remote MAC_Connection

	if (lmac==0) {
		lmac = Create_Remote_MAC_Connection(RStatus.WiredIPAddressStr, DEFAULT_ROBOT_PORT, tc->text,DEFAULT_ROBOT_PORT );
		if (lmac==0) {
			fprintf(stderr,"Could not get MAC_Connection for dest ip: %s\n",tc->text);
			return(0);
		}
	}

	return(SendInstructionToPower(lmac,Inst,numbyte,flags));
} //int SendUserInstructionToMotor(unsigned char *Inst,int numbyte,unsigned int flags)

//Checks to see if the source and dest (power) MAC_Connections exist first
//and if yes, sends the data to the dest
//flags:
//ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION - add the robot source IP address before the instruction (otherwise there should be some other source IP address in the first 4 bytes of the instruction)
//note is very similar to SendInstructionToRobot- the only difference is ConnectGenericIPs vs ConnectPowerIPs
int SendInstructionToPower(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)
{
	int i;
	int iResult=0;
	unsigned char SendInst[ROBOT_MAX_INST_LEN]; //
	int NumBytes;
	char DataStr[512];//,LogStr[256];

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"SendInstructionToPower\n");
	}

	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "SendInstructionToPower\n");
		fprintf(stderr, "Send from %s to %s\n",lmac->SrcIPAddressStr,lmac->DestIPAddressStr);
		fprintf(stderr, "%d bytes: ",numbyte);
		for(i=0;i<numbyte;i++) {
			fprintf(stderr, "0x%02x ",Inst[i]);
		}
	}


	//******
	//Note that raw instructions must already have integer values (like duration, memory address, memory data, etc.) in little endian format
	//****
			
	//if the instruction originates on this machine, copy the source IP Address
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
} //int SendInstructionToPower(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)


void btnwinPowerExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinPowerExit_Click



int winPower_OnOpen(FTWindow *twin)
{
	FTControl *tcontrol;

//	fprintf(stderr,"winPower_OnOpen\n");

	//note the MAC label and IP controls must be created by the time this functions is called in order to fill them
	tcontrol=GetFTControl("ddPowerPCBNum");
	if (tcontrol!=0) {
		Populate_EthPower_Dropdown(tcontrol);  //add recognized EthPower PCBs to dropdown control
	}


	return(1);
} //int winPower_OnOpen(FTWindow *twin)

int winPower_OnClose(FTWindow *twin)
{
//	fprintf(stderr,"winPower_OnClose\n");
	//If power window closes, currently the program makes sure both 3.3v and 24v are off
  TurnOff3V();
  TurnOff36V();
	return(1);
} //int winPower_OnClose(FTWindow *twin)


int TurnOff3V(void) {
	FTControl *tc;
	unsigned char inst[10];

	//turn off first
	inst[0]=ROBOT_POWER_TURN_OFF_3_3V;
	//SendInstructionToRobot(ROBOT_MOTORS_SEND_B24_INST,key);
	SendUserInstructionToPower(inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);


	//then update the control if it still exists
	tc=GetFTControl("btnToggle3V"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control btnToggle3V\n");
		return(0);
	}

	strcpy(tc->text,"Turn On 3.3V");
	DrawFTControl(tc);

//	fprintf(stderr,"TurnOff3V\n");
	return(1);
} //int TurnOff3V(void) {

int TurnOff36V(void) {
	FTControl *tc;
	unsigned char inst[10];

	//turn off first
	inst[0]=ROBOT_POWER_TURN_OFF_36V;
	//SendInstructionToRobot(ROBOT_MOTORS_SEND_B24_INST,key);
	SendUserInstructionToPower(inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);


	//then update the control if it still exists
	tc=GetFTControl("btnToggle36V"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control btnToggle36V\n");
		return(0);
	}

	strcpy(tc->text,"Turn On 36V");
	DrawFTControl(tc);
	return(1);
} //int TurnOff36V(void) {

void btnToggle3V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {

	FTControl *tc,*tc2;
	//MAC_Connection *lmac;
	unsigned char inst[10];

/*
	//get the MAC_Connection
	tc=GetFTControl("txtPowerDestIPAddressStr"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control txtPowerDestIPAddressStr\n");
		return(0);
	}
	lmac=Get_MAC_Connection(tc->text); //get remote MAC_Connection
	if (lmac==0) {
		lmac = Create_Remote_MAC_Connection(RStatus.WiredIPAddressStr, DEFAULT_ROBOT_PORT, tc->text,DEFAULT_ROBOT_PORT );
		if (lmac==0) {
			fprintf(stderr,"Could not get MAC_Connection for dest ip: %s\n",tc->text);
			return(0);
		}
	}
*/

	//perhaps the current on/off status should be stored in an EthPowerPCB data structure
	//but for now just get it from the control
	tc=GetFTControl("btnToggle3V");
	if (tc==0) {
		fprintf(stderr,"Could not get control btnToggle3V\n");
		return;
	}

	if (!strcmp(tc->text,"Turn On 3.3V")) {
		inst[0]=ROBOT_POWER_TURN_ON_3_3V;
		strcpy(tc->text,"Turn Off 3.3V");
	} else {
		//Only turn off 3V is 36V is not on
		tc2=GetFTControl("btnToggle36V"); 
		if (tc2==0) {
			fprintf(stderr,"Could not get control btnToggle36V\n");
			return;
		}

		if (strcmp(tc2->text,"Turn On 36V")) {
			fprintf(stderr,"36V must be off before turning off 3.3V\n");
			return;
		} 


		inst[0]=ROBOT_POWER_TURN_OFF_3_3V;
		strcpy(tc->text,"Turn On 3.3V");
	}
	DrawFTControl(tc);

	//SendInstructionToRobot(ROBOT_MOTORS_SEND_B24_INST,key);
	SendUserInstructionToPower(inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);

}  //void btnToggle3V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {

void btnToggle36V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	FTControl *tc,*tc2;
	//MAC_Connection *lmac;
	unsigned char inst[10];

	//perhaps the current on/off status should be stored in an EthPowerPCB data structure
	//but for now just get it from the control
	tc=GetFTControl("btnToggle36V"); 
	if (tc==0) {
		fprintf(stderr,"Could not get control btnToggle36V\n");
		return;
	}

	//todo: perhaps use some kind of EthPower structure and flags to store state of 3V and 36V

	if (!strcmp(tc->text,"Turn On 36V")) {
		//Only turn on 36V is 3v is on
		tc2=GetFTControl("btnToggle3V"); 
		if (tc2==0) {
			fprintf(stderr,"Could not get control btnToggle3V\n");
			return;
		}

		if (strcmp(tc2->text,"Turn Off 3.3V")) {
			fprintf(stderr,"3V must be on before turning on 36V\n");
			return;
		} 

		inst[0]=ROBOT_POWER_TURN_ON_36V;
		strcpy(tc->text,"Turn Off 36V");
	} else {
		inst[0]=ROBOT_POWER_TURN_OFF_36V;
		strcpy(tc->text,"Turn On 36V");
	}
	DrawFTControl(tc);

	SendUserInstructionToPower(inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);

}  //void btnToggle36V_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {


void btnUpdatePower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	unsigned char inst[10];

	inst[0]=ROBOT_POWER_GET_ALL;

	SendUserInstructionToPower(inst,1,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);

}  //void btnUpdatePower_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {

