//win_NewEthPCB.c
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
#include "win_RobotModel.h"  //for RMInfo.iLogFile
#include "win_NewEthPCB.h"

extern ProgramStatus PStatus; //for info messages
extern RobotStatus RStatus;
extern RobotModelInfo RMInfo; //Robot Model Info 

int win_NewEthPCB_CreateFTWindow(void)
{
FTWindow twin,*twin2;
int newx,newy;

twin2=GetFTWindow("win_NewEthPCB");
if (twin2!=0) {
	newx=twin2->x+300;
	newy=twin2->y+20;
} else {
	newx=300;
	newy=100;
}

memset(&twin,0,sizeof(FTWindow));
strcpy(twin.name,"win_NewEthPCB");
strcpy(twin.title,"New Eth PCB detected");
strcpy(twin.ititle,"New Eth PCB detected");
twin.x=newx;
twin.y=newy;
twin.w=350;
twin.h=150;
//twin.Keypress=(FTControlfuncwk *)main_Keypress;
//twin.OnOpen=(FTControlfuncw *)main_OnOpen;
//twin.GotFocus=(FTControlfuncw *)main_GotFocus;
//twin.ButtonClick[0]=(FTControlfuncwcxy *)main_ButtonDownUpOnFTControl;
twin.AddFTControls=(FTControlfuncw *)win_NewEthPCB_AddFTControls;

//change dir to logs folder
#if Linux
chdir(RStatus.ModelLogFolder);
#endif
#if WIN32
_chdir(RStatus.ModelLogFolder);
#endif

//fprintf(stderr,"create window main_CreateFTWindow\n");
CreateFTWindow(&twin);
twin2=GetFTWindow(twin.name);
DrawFTWindow(twin2);

//fprintf(stderr,"end main_CreateFTWindow\n");
return 1;
} //end win_NewEthPCB_CreateFTWindow



int win_NewEthPCB_AddFTControls(FTWindow *twin)
{
	FTControl *tcontrol,*tcontrol2;
	FTItem *titem;
	int fh,fw,i,PCBType;

	if (PStatus.flags&PInfo) {
		fprintf(stderr, "Adding FTControls for window %s\n", twin->name);
	}

	fw=twin->fontwidth;
	fh=twin->fontheight;

	tcontrol=(FTControl *)malloc(sizeof(FTControl));

	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "lblNewEthPCBDetected");
	tcontrol->type = CTLabel;
	tcontrol->x1 = fw;
	tcontrol->y1 = fh;
	strcpy(tcontrol->text, "New Ethernet PCB Detected");
	//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
	//tcontrol->flags=CGrowX1|CGrowX2;
	AddFTControl(twin, tcontrol);

	//newline
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblNewEthPCB_Mac");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh*2.5;
	tcontrol->x2=fw*5;
	strcpy(tcontrol->text, "Mac: ");
	//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
	//tcontrol->flags=CGrowX1|CGrowX2;
	AddFTControl(twin,tcontrol);

	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "lblNewEthPCB_MacNum");
	tcontrol->type = CTLabel;
	tcontrol->x1 = fw*6;
	tcontrol->y1 = fh*2.5;
	tcontrol->x2 = fw*26;
	//copy the MAC address into the Mac: name
	if (RStatus.NewMac!=0) {
		sprintf(tcontrol->text, "%s", RStatus.NewMac->PhysicalAddressStr);
	}
	//sprintf(tcontrol->text, "Mac: ");
	//tcontrol->flags=CScaleX1|CScaleY1|CScaleX2|CScaleY2;
	//tcontrol->flags=CGrowX1|CGrowX2;
	AddFTControl(twin, tcontrol);

	//new line
	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"lblNewEthPCB_SelectLocation");
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->y1=fh*4;
	tcontrol->x2=fw*17;
	//tcontrol->flags=CGrowY1|CGrowY2;
	strcpy(tcontrol->text,"Select Location:");
	AddFTControl(twin,tcontrol);



	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "ddNewEthPCB_Location");
	tcontrol->type = CTDropdown;
	tcontrol->x1 = fw*18;
	tcontrol->y1 = fh*4;
	tcontrol->x2 = fw*48;
	//tcontrol->flags=CGrowY1|CGrowY2;
	//strcpy(tcontrol->text, "Select Location:");
	tcontrol->OnChange = (FTControlfuncwc *)ddNewEthPCB_Location_OnChange;
	AddFTControl(twin, tcontrol);
	tcontrol2 = GetFTControl(tcontrol->name);
	//delete all existing items
	DelAllFTItems(tcontrol2);		
	//add items
	titem = (FTItem *)malloc(sizeof(FTItem));
	memset(titem, 0, sizeof(FTItem));
	//titem.hotkey[0]=0x78;  //x
	//titem.hotkey[1]=0x58;  //X
	//titem.click[0]=(FTControlfuncwcxy *)btnExit_Click;
	//tcontrol.ilist=AddFTItem(tcontrol.ilist,&titem);
	PCBType=0;
	if (RStatus.NewMac!=0) {
		if (!strcmp(RStatus.NewMac->Name,"Accel")) {
			PCBType=1;
		} 
		if (!strcmp(RStatus.NewMac->Name,"Motor")) {
			PCBType=2;
		} 
		if (!strcmp(RStatus.NewMac->Name,"Power")) {
			PCBType=3;
		} 
	} //if (RStatus.NewMac!=0) {

	for (i=0;i<RStatus.NumEthPCBsExpected;i++) {
		//if new PCB name is "ACCEL", "MOTOR", or "POWER" filter the dropdown to just those PCB choices
		//otherwise list all
		switch(PCBType) {
			case 0:
				sprintf(titem->name, "%s",RStatus.EthPCBInfo[i].Name);
				AddFTItem(tcontrol2, titem);
			break;
			case 1: 
				if (!strncmp(RStatus.EthPCBInfo[i].Name,"ETHACCEL",8)) {
					sprintf(titem->name, "%s",RStatus.EthPCBInfo[i].Name);
					AddFTItem(tcontrol2, titem);
				} 
			break;
			case 2: 
				if (!strncmp(RStatus.EthPCBInfo[i].Name,"ETHMOTOR",8)) {
					sprintf(titem->name, "%s",RStatus.EthPCBInfo[i].Name);
					AddFTItem(tcontrol2, titem);
				} 
			break;
			case 3: 
				if (!strncmp(RStatus.EthPCBInfo[i].Name,"ETHPOWER",8)) {
					sprintf(titem->name, "%s",RStatus.EthPCBInfo[i].Name);
					AddFTItem(tcontrol2, titem);
				} 
			break;
		} //switch(PCBType) {
	} //for i
	free(titem);



	memset(tcontrol, 0, sizeof(FTControl));
	strcpy(tcontrol->name, "btn_NewEthPCB_Cancel");
	tcontrol->type = CTButton;
	strcpy(tcontrol->text, "Cancel");
	tcontrol->x1 = fw * 31;
	tcontrol->y1 = fh * 8;
	tcontrol->x2 = fw * 39;
	tcontrol->y2 = fh * 8 + 32;
	tcontrol->flags = CGrowY1 | CGrowY2;
	tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btn_NewEthPCB_Cancel_Click;
	AddFTControl(twin, tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	strcpy(tcontrol->name,"btn_NewEthPCB_OK");
	tcontrol->type=CTButton;
	strcpy(tcontrol->text,"OK");
	tcontrol->x1=fw*40;
	tcontrol->y1=fh*8;
	tcontrol->x2=fw*48;
	tcontrol->y2=fh*8+32;
	tcontrol->flags=CGrowY1|CGrowY2;
	tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btn_NewEthPCB_OK_Click;
	AddFTControl(twin,tcontrol);



	if (tcontrol!=0) {
		free(tcontrol);
	}

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"Done adding FTControls for window %s\n",twin->name);
	}

	//#endif
	return(1);
} //end win_NewEthPCB_AddFTControls


void btn_NewEthPCB_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
CloseFTWindow(twin);
}

//load the log and store the pointer in the global Robot structure
unsigned int btn_NewEthPCB_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)
{
	FTControl *tcName,*tcMac;
	unsigned char MACInBytes[6];
	int PCBNameNum;



	tcMac = GetFTControl("lblNewEthPCB_MacNum");
	tcName = GetFTControl("ddNewEthPCB_Location");
	if (tcMac != 0 && tcName!=0) {

		//get index into ROBOT_PCB_NAMES array (ex: ROBOT_PCB_ETHMOTORS_LOWER_BODY)
		PCBNameNum=GetPCBNameNum(tcName->text);

		ConvertStringToBytesNum((unsigned char *)tcMac->text, MACInBytes, 6);
		//copy MAC address to PCB table
		//memcpy(RStatus.pcb[PCBNameNum].MAC, MACInBytes, 6);
		memcpy(RStatus.EthPCBInfo[PCBNameNum].MAC, MACInBytes, 6);  //user assigned this new mac to an existing known EthPCB
		//copy the PCBName name to the PCB table too
		//strcpy(RStatus.pcb[PCBNameNum].PCBName, ROBOT_PCB_NAMES[PCBNameNum]); //"ETHMOTORS_LOWER_BODY");
		//name is unchanged  strcpy(RStatus.PCBInfo[PCBNameNum]->Name, ROBOT_PCB_NAMES[PCBNameNum]); //"ETHMOTORS_LOWER_BODY");
		//copy PCB number to the PCB table
		//num is unchanged RStatus.pcb[PCBNameNum].Num = PCBNameNum;

		//depending on which kind of PCB, update the conf file
		
		if (!strncmp(RStatus.EthPCBInfo[PCBNameNum].Kind,"Accel",5)) {
			UpdateEthConfig(RStatus.EthPCBInfo[PCBNameNum].Name,"Accel"); // update mac in EthAccels.conf
		} 
		if (!strncmp(RStatus.EthPCBInfo[PCBNameNum].Kind,"Motor",5)) {
			UpdateEthConfig(RStatus.EthPCBInfo[PCBNameNum].Name,"Motor"); // update mac in EthMotors.conf
		} 
		if (!strncmp(RStatus.EthPCBInfo[PCBNameNum].Kind,"Power",5)) {
			UpdateEthConfig(RStatus.EthPCBInfo[PCBNameNum].Name,"Power"); // update mac in EthPower.conf
		} 

		//Save the Config file so the new change can be loaded next time
//		SaveRobotAppConfigFile(); // but this way allows user to put in comments

		RStatus.NewMac=0; //done with New PCB

		//send Broadcast again to run through the code that recognizes the new Eth PCB
		SendBroadcastInstruction();

	} //if (tcMac != 0 && tcName!=0) {


	//close this window
	CloseFTWindow(twin);

	return(1);
} //void btn_NewEthPCB_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y)


int ddNewEthPCB_Location_OnChange(FTWindow *twin, FTControl *tFTControl)
{
	FTControl *tc;

	tc = GetFTControl("lblNewEthPCB_MacNum");
	if (tc != 0) {
		fprintf(stderr, "User chose to associate mac %s with %s\n", tc->text, tFTControl->text);
	} //if (tc != 0) {
	
	return(1);
} //int ddNewEthPCB_Location_OnChange(FTWindow *twin, FTControl *tFTControl)
