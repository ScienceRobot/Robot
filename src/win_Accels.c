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
#include "win_Accels.h"
#include "win_AnalogSensors.h"
#include "win_GPS.h"
#include "win_Robot.h"
#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  //WIN32
#include "robot_accelmagtouchgps_mcu_instructions.h"
#include "Free3D.h" //for PI


#define DEFAULT_BUFLEN 512

#define MAX_NUM_OF_ETHACCELS 100  //todo: remove this constant and use dynamic memory allocation

extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons
//extern const char *ROBOT_PCB_NAMES[NUM_ROBOT_PCB_NAMES];
//extern CalibrationFile CaliFile[MAX_NUM_OF_ETHACCELS];


AccelWindowStatus AStatus; //Accelerometer Window Status


int winAccels_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winAccels");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winAccels");
	strcpy(twin.title,"Accelerometers");
	strcpy(twin.ititle,"Accelerometers");
	if (PStatus.rw < 1200 + 640) {
		twin.x = PStatus.rw-640;
		if (twin.x < 0) {
			twin.x = 0;
		}
	} else {
		twin.x = 1200;
	}
	twin.y=50;
	twin.w = 640;
	twin.h=460;
	twin.Keypress=(FTControlfuncwk *)winAccels_key;
	twin.AddFTControls=(FTControlfuncw *)winAccels_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winAccels_OnOpen;
	twin.OnClose=(FTControlfuncw *)winAccels_OnClose;

	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winAccels_AddFTWindow



int winAccels_AddFTControls(FTWindow *twin)
{
int fw,fh;
FTControl *tcontrol;
int i,fonty;
MAC_Connection *lmac;

#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winAccels'\n");

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
strcpy(tcontrol->name,"lblAccelPCBName");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"PCB:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
//strcpy(tcontrol->name,"txtAccelPCBName");
strcpy(tcontrol->name,"ddAccelPCBName");
tcontrol->type=CTDropdown;
tcontrol->x1=fw*6;
tcontrol->y1=fh;
tcontrol->x2=fw*31;
tcontrol->y2=fh*2+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)ddAccelPCBName_OnChange;
AddFTControl(twin,tcontrol);

lmac=Get_MAC_Connection_By_PCBName(tcontrol->text);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAccelMACStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*32;
tcontrol->y1=fh;
tcontrol->x2=fw*50;
tcontrol->y2=fh*2+fh/2;
if (lmac!=0) {		
	sprintf(tcontrol->text,"MAC: %s",lmac->PhysicalAddressStr);
} else {
	sprintf(tcontrol->text,"MAC: ");
} 
AddFTControl(twin,tcontrol);



memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblAccelDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*51;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtAccelDestIPAddressStr");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*55;
tcontrol->y1=fh;
tcontrol->x2=fw*75;
tcontrol->y2=fh*2+fh/2;
//strcpy(tcontrol->text,"192.168.1.2");
//see if a name exists for this PCB yet
if (lmac!=0) {
		//strcpy(tcontrol->text,"192.168.1.1");
	strcpy(tcontrol->text,lmac->DestIPAddressStr);
} 
//tcontrol->OnChange=(FTControlfuncwc *)txtAccelDestIPAddress_OnChange;
AddFTControl(twin,tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
sprintf(tcontrol->name,"chkAccelAll");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw;
tcontrol->y1=fh*3;
tcontrol->x2=fw*10;
tcontrol->y2=fh*4.5;
sprintf(tcontrol->text,"All");
tcontrol->OnChange=(FTControlfuncwc *)chkAccelAll_OnChange;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
sprintf(tcontrol->name,"lblInterval");
tcontrol->type=CTLabel;
tcontrol->x1=fw*11;
tcontrol->y1=fh*3;
tcontrol->y2=fh*4.5;
sprintf(tcontrol->text,"Interval:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
sprintf(tcontrol->name,"txtInterval");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*21;
tcontrol->y1=fh*3;
tcontrol->x2=fw*26;
tcontrol->y2=fh*4.5;
sprintf(tcontrol->text,"%d",ACCEL_TIMER_INTERVAL_IN_MS); //todo: get from selected EthAccelTouch PCB, add to Robot.config?
//tcontrol->OnChange=(FTControlfuncwc *)txtInterval_OnChange;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
sprintf(tcontrol->name,"lblIntervalms");
tcontrol->type=CTLabel;
tcontrol->x1=fw*27;
tcontrol->y1=fh*3;
tcontrol->y2=fh*4.5;
sprintf(tcontrol->text,"ms");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSetAccelTimerInterval");
tcontrol->type=CTButton;
tcontrol->x1=fw*31;
tcontrol->y1=fh*3;
tcontrol->x2=fw*45;
tcontrol->y2=fh*4.5;
strcpy(tcontrol->text,"Set Interval");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSetAccelTimerInterval_Click;
AddFTControl(twin,tcontrol);


//Show incoming accel data alternates: real-time, update data, display data
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkAccel_ShowData");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw*46;
tcontrol->y1 = fh*3;
tcontrol->x2 = fw*57;
tcontrol->y2 = fh*4.5;
strcpy(tcontrol->text, "Show Data");
tcontrol->OnChange = (FTControlfuncwc *)chkAccel_ShowData_OnChange;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
if (RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) {
	tcontrol->value=1;
}
AddFTControl(twin, tcontrol);

//use calibration offsets on accels, gyros, and magnetometers
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkAccel_UseCalibration");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw * 59;
tcontrol->y1 = fh * 3;
tcontrol->x2 = fw * 69;
tcontrol->y2 = fh*4.5;
strcpy(tcontrol->text, "Use Calib");
if (RStatus.flags&ROBOT_STATUS_USE_CALIBRATION) {
	tcontrol->value=1;
}
tcontrol->OnChange = (FTControlfuncwc *)chkAccel_UseCalibration_OnChange;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);

//use calibration offsets on accels, gyros, and magnetometers
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkAccel_AccelsOnly");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw * 71;
tcontrol->y1 = fh * 3;
tcontrol->x2 = fw * 81;
tcontrol->y2 = fh*4.5;
strcpy(tcontrol->text, "Accel Only");
if (RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY) {
	tcontrol->value=1;
}
tcontrol->OnChange = (FTControlfuncwc *)chkAccel_Accelerometers_Only_OnChange;
//tcontrol->flags |= CGrowX1 | CGrowY1 | CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);


//newline
//Individual Accelerometers
for(i=0;i<3;i++) {
	fonty=fh*i*4;
	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"chkAccelNum%d",i);
	tcontrol->type=CTCheckBox;
	tcontrol->x1=fw;
	tcontrol->x2=fw*13;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	sprintf(tcontrol->text,"Accel %d:",i);
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAccelx%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*14;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	strcpy(tcontrol->text,"x: 0.0g");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAccely%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*25;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	strcpy(tcontrol->text,"y: 0.0g");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAccelz%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*35;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	strcpy(tcontrol->text,"z: 0.0g");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAccelThresh%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*45;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	strcpy(tcontrol->text,"Threshold:");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"txtAccelThresh%d",i);
	tcontrol->type=CTTextBox;
	tcontrol->x1=fw*56;
	tcontrol->x2=fw*64;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	//can also be read using: ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_INTERRUPT_THRESHOLD
	sprintf(tcontrol->text,"%0.3f",(float)DEFAULT_ACCEL_THRESHOLD/1000.0);
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAccelThreshg%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*65;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	strcpy(tcontrol->text,"g");
	AddFTControl(twin,tcontrol);

	sprintf(tcontrol->name,"chkAccelRelative%d",i);
	tcontrol->type=CTCheckBox;
	tcontrol->value=1;  //default interrupt is relative so select- maybe this should be absolute and then unchecked by default
	tcontrol->x1=fw*67;
	tcontrol->x2=fw*70;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	sprintf(tcontrol->text,"Relative");
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblAcceltime%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*71;
	tcontrol->y1=fh*5+fonty;
	tcontrol->y2=fh*6.5+fonty;
	//strcpy(tcontrol->text,"");
	AddFTControl(twin,tcontrol);


//add magentic checkbox and x,y,z

	memset(tcontrol,0,sizeof(FTControl));
//	sprintf(tcontrol->name,"chkMagNum%d",i);
	sprintf(tcontrol->name,"lblMagNum%d",i);
//	tcontrol->type=CTCheckBox;
	tcontrol->type=CTLabel;
	tcontrol->x1=fw;
	tcontrol->x2=fw*8;
	tcontrol->y1=fh*7+fonty;
	tcontrol->y2=fh*8.5+fonty;
#if USE_FXOS8700CQ
	sprintf(tcontrol->text,"   Mag %d:",i);
#endif
#if USE_MPU6050
	sprintf(tcontrol->text,"   Gyro %d:",i);
#endif
	AddFTControl(twin,tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblMagx%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*14;
	tcontrol->y1=fh*7+fonty;
	tcontrol->y2=fh*8.5+fonty;
#if USE_FXOS8700CQ
	strcpy(tcontrol->text,"x: 0.0uT");
#endif
#if USE_MPU6050
#if Linux
	strcpy(tcontrol->text,"x: 0.0\xB0/s");
#endif //Linux
#if WIN32
	strcpy(tcontrol->text, "x: 0.0°/s");
#endif //WIN32
#endif //MPU6050
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblMagy%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*25;
	tcontrol->y1=fh*7+fonty;
	tcontrol->y2=fh*8.5+fonty;
#if USE_FXOS8700CQ
	strcpy(tcontrol->text,"y: 0.0uT");
#endif //FXOS8700CQ
#if USE_MPU6050
#if Linux
	strcpy(tcontrol->text,"y: 0.0\xB0/s");
#endif //Linux
#if WIN32
	strcpy(tcontrol->text, "y: 0.0°/s");
#endif //WIN32#endif
#endif //MPU6050
	AddFTControl(twin,tcontrol);

	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblMagz%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*35;
	tcontrol->y1=fh*7+fonty;
	tcontrol->y2=fh*8.5+fonty;
#if USE_FXOS8700CQ
	strcpy(tcontrol->text,"z: 0.0uT");
#endif
#if USE_MPU6050
#if Linux
	strcpy(tcontrol->text,"z: 0.0\xB0/s");
#endif //Linux
#if WIN32
	strcpy(tcontrol->text, "z: 0.0°/s");
#endif //WIN32
#endif //MPU6050
	AddFTControl(twin,tcontrol);

	memset(tcontrol, 0, sizeof(FTControl));
	sprintf(tcontrol->name, "lblOrientation%d", i);
	tcontrol->type = CTLabel;
	tcontrol->x1 = fw * 45;
	tcontrol->y1 = fh * 7 + fonty;
	tcontrol->x2 = fw * 57;
	tcontrol->y2 = fh*8.5 + fonty;
#if Linux
	strcpy(tcontrol->text, "Orientation:");
#endif //Linux
#if WIN32
	strcpy(tcontrol->text, "Orientation:");
#endif //WIN32
	AddFTControl(twin, tcontrol);


	memset(tcontrol,0,sizeof(FTControl));
	sprintf(tcontrol->name,"lblDegrees%d",i);
	tcontrol->type=CTLabel;
	tcontrol->x1=fw*58;
	tcontrol->y1=fh*7+fonty;
	tcontrol->x2=fw*80;
	tcontrol->y2=fh*8.5+fonty;
#if Linux
	strcpy(tcontrol->text,"\xB0");
#endif //Linux
#if WIN32
	strcpy(tcontrol->text, "°");
#endif //WIN32
	AddFTControl(twin,tcontrol);

} //for i
//End individual acclerometers


//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblSendInstructionToAccelRaw");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"Send inst:"); 
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtSendInstructionToAccelRaw");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*12;
tcontrol->y1=fh*17;
tcontrol->x2=fw*29;
tcontrol->y2=fh*18.5;
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
strcpy(tcontrol->name,"lblNumCalibSamples");
tcontrol->type=CTLabel;
tcontrol->x1=fw*30;
tcontrol->y1=fh*17;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"Num Calib Samples:"); 
//tcontrol->color[0]=wingray;  //bkground
//tcontrol->textcolor[0]=black;//black;  //text
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtNumCalibSamples");
tcontrol->type=CTTextBox;//number of samples to get for calibration
tcontrol->x1=fw*49;
tcontrol->y1=fh*17;
tcontrol->x2=fw*55;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"100");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkCalibAccels");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*57;
tcontrol->y1=fh*17;
tcontrol->x2=fw*66;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"Accels");
tcontrol->value=1;
//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnCalibrateAccels_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkCalibGyros");
tcontrol->type=CTCheckBox;
tcontrol->x1=fw*67;
tcontrol->y1=fh*17;
tcontrol->x2=fw*75;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"Gyros");
tcontrol->value=1;
//tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnCalibrateGyros_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnCalibrateAccelGyro");
tcontrol->type=CTButton;
tcontrol->x1=fw*77;
tcontrol->y1=fh*17;
tcontrol->x2=fw*88;
tcontrol->y2=fh*18.5;
strcpy(tcontrol->text,"Calibrate");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnCalibrateAccelGyro_Click;
AddFTControl(twin,tcontrol);


//newline
//tcontrol=(FTControl *)malloc(sizeof(FTControl));
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblGetAccelData");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh*19;
tcontrol->x2=fw*21;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"Data received: ");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtGetAccelData");
tcontrol->type=CTTextBox;//2; //textbox
tcontrol->x1=fw*21;
tcontrol->y1=fh*19;
tcontrol->x2=fw*35;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"");
//tcontrol->Keypress=(FTControlfuncwck *)txtOutFile_Keypress;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMagOffsetX");
tcontrol->type=CTLabel;
tcontrol->x1=fw*36;
tcontrol->y1=fh*19;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"Mag Offset x:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtMagOffsetX");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*50;
tcontrol->y1=fh*19;
tcontrol->x2=fw*55;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMagOffsetY");
tcontrol->type=CTLabel;
tcontrol->x1=fw*56;
tcontrol->y1=fh*19;
strcpy(tcontrol->text,"y:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtMagOffsetY");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*59;
tcontrol->y1=fh*19;
tcontrol->x2=fw*64;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMagOffsetZ");
tcontrol->type=CTLabel;
tcontrol->x1=fw*65;
tcontrol->y1=fh*19;
strcpy(tcontrol->text,"z:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtMagOffsetZ");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*68;
tcontrol->y1=fh*19;
tcontrol->x2=fw*73;
tcontrol->y2=fh*20.5;
strcpy(tcontrol->text,"");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblMagOffsetuT");
tcontrol->type=CTLabel;
tcontrol->x1=fw*74;
tcontrol->y1=fh*19;
strcpy(tcontrol->text,"uT");
AddFTControl(twin,tcontrol);


memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnCalibrateAllAccelGyro");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 77;
tcontrol->y1 = fh * 19;
tcontrol->x2 = fw * 91;
tcontrol->y2 = fh*20.5;
strcpy(tcontrol->text, "CalibrateAll");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnCalibrateAllAccelGyro_Click;
AddFTControl(twin, tcontrol);


//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAccelGetValues");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*21;
tcontrol->x2=fw*15;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Get Value");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccelsGetValues_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAccelsStartPolling");
tcontrol->type=CTButton;
tcontrol->x1=fw*16;
tcontrol->y1=fh*21;
tcontrol->x2=fw*30;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Start Polling");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccelsStartPolling_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAccelsStopPolling");
tcontrol->type=CTButton;
tcontrol->x1=fw*31;
tcontrol->y1=fh*21;
tcontrol->x2=fw*45;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Stop Polling");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccelsStopPolling_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAnalogSensorsWin");
tcontrol->type=CTButton;
tcontrol->x1=fw*46;
tcontrol->y1=fh*21;
tcontrol->x2=fw*60;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Analog Sensors");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAnalogSensorsWin_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGetMagOffset");
tcontrol->type=CTButton;
tcontrol->x1=fw*61;
tcontrol->y1=fh*21;
tcontrol->x2=fw*75;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Get Mag Offset");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGetMagOffset_Click;
AddFTControl(twin,tcontrol);

/*
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAutoCalibrate");
tcontrol->type=CTButton;
tcontrol->x1=fw*76;
tcontrol->y1=fh*21;
tcontrol->x2=fw*90;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Disable Auto Calibration");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAutoCalibrate_Click;
AddFTControl(twin,tcontrol);
*/
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnResetAccel");
tcontrol->type=CTButton;
tcontrol->x1=fw*76;
tcontrol->y1=fh*21;
tcontrol->x2=fw*90;
tcontrol->y2=fh*26;
strcpy(tcontrol->text,"Reset Accel");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnResetAccel_Click;
AddFTControl(twin,tcontrol);



//newline
memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAccelStartInterrupt");
tcontrol->type=CTButton;
tcontrol->x1=fw;
tcontrol->y1=fh*27;
tcontrol->x2=fw*15;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Start Interrupt");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccelsStartInterrupt_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnAccelStopInterrupt");
tcontrol->type=CTButton;
tcontrol->x1=fw*16;
tcontrol->y1=fh*27;
tcontrol->x2=fw*30;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Stop Interrupt");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnAccelsStopInterrupt_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSendInstructionToAccel");
tcontrol->type=CTButton;
tcontrol->x1=fw*31;
tcontrol->y1=fh*27;
tcontrol->x2=fw*45;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Send Instruction");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSendInstructionToAccel_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinGPS");
tcontrol->type=CTButton;
tcontrol->x1=fw*46;
tcontrol->y1=fh*27;
tcontrol->x2=fw*60;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"GPS");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGPSWin_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnSetMagOffset");
tcontrol->type=CTButton;
tcontrol->x1=fw*61;
tcontrol->y1=fh*27;
tcontrol->x2=fw*75;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Set Mag Offset");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnSetMagOffset_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinAccelsExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*76;
tcontrol->y1=fh*27;
tcontrol->x2=fw*90;
tcontrol->y2=fh*32;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinAccelsExit_Click;
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


void winAccels_key(FTWindow *twin,KeySym key)
{
FTControl *tc;

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

				//change Accel to #bof keypress if no control is selected
			if (twin->FocusFTControl==0) {
				tc=GetFTControl("cmbAccelNum");
				if (tc!=0) {
					tc->value=key&0xf;
					DrawFTControl(tc); //redraw control
				}
			} //if (twin->FocusFTControl==0) {
        break;



      default:
				//fprintf(stderr,"key=%x\n",key);
        break;
      }
} //void winAccels_key(FTWindow *twin,KeySym key)

int FillAccelItemList(void)
{
	FTControl *tc;
	FTItem *titem;
	MAC_Connection *lmac;
	int NumAccelPCBs;

	NumAccelPCBs=0;
	tc=GetFTControl("ddAccelPCBName");
	if (tc!=0) {
		//delete all current items
		DelAllFTItems(tc);
		titem=(FTItem *)malloc(sizeof(FTItem));
		lmac=RStatus.iMAC_Connection;	
		while(lmac!=0) {
			if (lmac!=0) {
//				if (lmac->AddWindowFunction==(FTControlfunc *)winAccels_AddFTWindow) {
					if (!strncmp(lmac->pcb.Name,"ETHACCEL",8) || !strncmp(lmac->pcb.Name,"GAMTP",5)) {
						//strcpy(tcontrol->text,"192.168.1.1");
						memset(titem,0,sizeof(FTItem));
						sprintf(titem->name,"%s",lmac->pcb.Name);
						//sprintf(titem->name,"%i",lmac->Num);
						//strcpy(titem->name,lmac->Name);
						AddFTItem(tc,titem);  //sometimes crashes in debugger
						//if (NumAccelPCBs==0) {
							//tmac=lmac; //remember the first Accel PCB
						//}
						NumAccelPCBs++;
				} //if (!strncmp(lmac->Name,"ACCEL")) {
			} 
			lmac=lmac->next;
		}//while(lmac!=0) {
		free(titem);
		if (NumAccelPCBs>0) {
			SelectFTItemByNum(tc->ilist,1,0); //select the first item
			//lmac=tmac;
		}
		DrawFTControl(tc); //redraw the control 


		//set initial PCB, MAC, and IP and redraw
/*		tc=GetFTControl("ddAccelPCBName");
		if (tc!=0) {
			DrawFTControl(tc);
			 
		}
*/

		lmac=Get_MAC_Connection_By_PCBName(tc->text);
		if (lmac!=0) { 
			tc=GetFTControl("lblAccelMACStr");
			if (tc!=0) {
				sprintf(tc->text,"MAC: %s",lmac->PhysicalAddressStr);
				DrawFTControl(tc);
			}

			tc=GetFTControl("txtAccelDestIPAddressStr");
			if (tc!=0) {
				strcpy(tc->text,lmac->DestIPAddressStr);
				DrawFTControl(tc);
			} 
		} //if (lmac!=0) 

	} //tc!=0

	return(NumAccelPCBs);
} //int FillAccelItemList(void)


int ddAccelPCBName_OnChange(FTWindow *twin,FTControl *tcontrol) {
	MAC_Connection *lmac;
	FTControl *tc;
	
	//fprintf(stderr,"OnChange\n");
	//if the user changed Accel PCBs update the MAC and IP
	lmac=Get_MAC_Connection_By_PCBName(tcontrol->text); //get remote MAC_Connection
	if (lmac!=0) {
		//MAC
		tc=GetFTControl("lblAccelMACStr");
		if (tc) {
			sprintf(tc->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc);
		} //tc
		//IP
		tc=GetFTControl("txtAccelDestIPAddressStr");
		if (tc) {
			sprintf(tc->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc);
		} //tc

		//in addition, change the Analog Sensor window IP if created
		tc=GetFTControl("txtTouchDestIPAddressStr");
		if (tc) {
			sprintf(tc->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc);
		} //tc

		return(1);
	} //if (lmac==0) {
	return(0);
} //int ddAccelPCBName_OnChange(FTWindow *twin,FTControl *tcontrol) {




int btnSetAccelTimerInterval_Click(FTWindow *twin,FTControl *tcontrol,int x, int y) 
{
	return(SetUserEthAccelTimerInterval());
}//int btnSetAccelTimerInterval_Click(FTWindow *twin,FTControl *tcontrol,int x, int y) {


//user changed the polling interrupt timer (in ms)
int txtInterval_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	uint16_t TimerInMS;
	MAC_Connection *lmac;
	FTControl *tc;

	tc=GetFTControl("ddAccelPCBName");
	if (tc!=0) {
		lmac=Get_MAC_Connection_By_PCBName(tc->text);
		if (lmac!=0) {
			TimerInMS=atoi(tcontrol->text);
			return(SetEthAccelTimerInterval(lmac,TimerInMS));
		} //if (lmac!=0) {
	} //if (tc!=0) {
	return(0);
}//int txtInterval_OnChange(FTWindow *twin,FTControl *tcontrol)


int SetUserEthAccelTimerInterval(void)
{
	uint16_t TimerInMS;
	MAC_Connection *lmac;
	FTControl *tc;

	tc=GetFTControl("ddAccelPCBName");
	if (tc!=0) {
		lmac=Get_MAC_Connection_By_PCBName(tc->text);
		if (lmac!=0) {
			tc=GetFTControl("txtInterval");
			if (tc!=0) {
				TimerInMS=atoi(tc->text);
				return(SetEthAccelTimerInterval(lmac,TimerInMS));
			} //if (tc!=0) {
		} //if (lmac!=0) {
	} //if (tc!=0) {
	return(0);
}//int SetUserEthAccelTimerInterval(void)



//Set the EthAccelTouch PCB Timer interval (the time between timer interrupts)
int SetEthAccelTimerInterval(MAC_Connection *lmac,uint16_t TimerInMS)
{
	int NumBytes;
	unsigned char Inst[512];

	//send the robot an instruction to change the timer period
	Inst[0]=ROBOT_ACCELMAGTOUCH_SET_TIMER_INTERVAL_IN_MSEC; 
	memcpy(&Inst[1],&TimerInMS,2);  //is send little endian
	NumBytes=3;  //IP 4 bytes are added later
	return(SendInstructionToAccel(lmac,Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION)); 

}//int SetEthAccelTimerInterval(MAC_Connection *lmac,uint16_t TimerInMS)


int chkAccelAll_OnChange(FTWindow *twin,FTControl *tcontrol)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];


	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAccelNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				tc->value=tcontrol->value;
			}
/*
			sprintf(tstr,"chkMagNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				tc->value=tcontrol->value;
			}
*/

	} //for 
	//redraw window
	DrawFTWindow(twin);

	return(1);
}//int chkAccelAll_OnChange(FTWindow *twin,FTControl *tcontrol)

//Get Accelerometers Values
int btnAccelsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	unsigned short int mask;
	int NumBytes;
	unsigned char Inst[512];

	//create a 16-bit mask depending on the checkbox selected
	//so getting a sample from accel 0, 1 and 2 = 0x0007
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAccelNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES; 
		memcpy(&Inst[1],&mask,2);  //is send little endian
		NumBytes=3;  //IP 4 bytes are added later
		SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		return(1);
	} else {
		FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
	} //if (mask!=0) {

	return(0);
} //int btnAccelsGetValues_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

#if 0 
//clear the ACCELEROMETER_GOT_SAMPLE flag for selected accelerometers
int ClearAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask)  
{
	int i;

	//clear the "got result" flag for any masked accelerometers
	for(i=0;i<3;i++) {
		if (AccelMask&(1<<i)) {
			leth->Accel[i].flags&=~ACCELEROMETER_GOT_SAMPLE; //clear got result
		} //
	}  //for i

	return(1);
} //int ClearAccelerometersSamples(EthAccels_PCB *leth,uint16_t AccelMask)  

//RequestAccelerometerSamples- send a request to get a single sample
int RequestAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask)  
{
	int NumBytes;
	unsigned char Inst[512];

	//first clear any already existing sample flags
	ClearAccelerometerSamples(leth,AccelMask);
	
	//Send a Get Sample instruction to the EthAccelTouch PCB
	Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES; 
	memcpy(&Inst[1],&AccelMask,2);  //is send little endian
	NumBytes=3;  //IP 4 bytes are added later
	SendInstructionToAccel(leth->mac,Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);

	return(1);
}//int RequestAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask)  


//Wait to receive the accel sample data sent by the EthAccelTouch PCB
int WaitForAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask)  
{
	int Failed,i;

	//note that it's important to wait for the last flag (Accel[2]) to get set before processing data
	//otherwise the program thinks that there is no data from those accelerometers 
	while(!(leth->Accel[2].flags&ACCELEROMETER_GOT_SAMPLE)); //wait for the result- only 1 GotResult flag indicates a return packet since all results are sent together in one packet for each EthAccelTouch PCB
	//memcpy(Sample,&RStatus.Accel[AccelNum].Sample,sizeof(AccelSample));	

	Failed=0;
	//verify that all masked Accelerometers got a sample, otherwise return 0
	for(i=0;i<3;i++) {
		if (AccelMask&(1<<i)) {
			if (!(leth->Accel[i].flags&ACCELEROMETER_GOT_SAMPLE)) {
				Failed=1; //didn't get a sample for this accelerometer
				fprintf(stderr,"Fail: %s Accel %d\n",leth->mac->pcb.Name,i);
			}
		} //
	} //for i

	if (Failed) {
		return(0);
	} else {
		return(1);
	}


} //int WaitForAccelerometerSamples(EthAccels_PCB *leth,uint16_t AccelMask)  
#endif

/*

//GetAccelerometerSample - get a single sample from a single accelerometer
int GetAccelerometerSample(int AccelNum,AccelSample *Sample) 
{
	unsigned short int mask;

	int NumBytes;
	unsigned char Inst[512];

	
	Inst[0]=ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES; 
	mask=1<<AccelNum;
	memcpy(&Inst[1],&mask,2);  //is send little endian
	NumBytes=3;  //IP 4 bytes are added later
	RStatus.Accel[AccelNum].flags&=~ACCELEROMETER_GOT_SAMPLE; //clear got result
	SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	while(!(RStatus.Accel[AccelNum].flags&ACCELEROMETER_GOT_SAMPLE)); //wait for the result
	memcpy(Sample,&RStatus.Accel[AccelNum].Sample,sizeof(AccelSample));	
	return(1);
} //int GetAccelerometerSample(AccelNum,&Sample)) 
*/


//Start Polling Analog Sensors
int btnAccelsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	MAC_Connection *lmac;

	//start polling any checked accelerometers
	//create a mask depending on the checkbox selected
	mask=0;
	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAccelNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		//get the currently selected EthAccelPCB
		tc=GetFTControl("ddAccelPCBName");
		if (tc!=0) {		
			lmac=Get_MAC_Connection_By_PCBName(tc->text);
			if (lmac!=0) {

				StartPollingAccelerometers(lmac,mask);
			} else {
				fprintf(stderr,"Could not get MAC_Connection for '%s'\n",tc->text);
			} 
		} //if (tc!=0) 
	} else {
		FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
	} //if (mask!=0) {
		
	return(1);
} //int btnAccelsStartPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Stop Polling Accelerometers
int btnAccelsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	MAC_Connection *lmac;

	//Stop polling accelerometers
	//create a mask depending on the checkbox selected
	mask=0;

	for(i=0;i<15;i++) {
			sprintf(tstr,"chkAccelNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					mask|=(1<<i);
				}
			}
	} //for

	if (mask!=0) {
		//get the currently selected EthAccelPCB

		tc=GetFTControl("ddAccelPCBName");
		if (tc!=0) {		
			lmac=Get_MAC_Connection_By_PCBName(tc->text);
			if (lmac!=0) {
				StopPollingAccelerometers(lmac,mask);
			} else {
				fprintf(stderr,"Could not get MAC_Connection for '%s'\n",tc->text);
			}
		} //if (tc!=0)
	} else {
		FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
	}
	return(1);
} //int btnAccelsStopPolling_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

int	StartPollingAccelerometers(MAC_Connection *lmac,uint16_t mask)
{
	unsigned char Inst[512];
	int NumBytes;


	if (lmac!=0 && mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_START_POLLING_ACCELEROMETER; 
		memcpy(&Inst[1],&mask,2);  //is send little endian
		NumBytes=3;
		SendInstructionToAccel(lmac,Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	} else {
		if (lmac==0) {
			fprintf(stderr,"StartPollingAccelerometers with MAC_Connection=0\n");
		} 
		if (mask==0) {
			fprintf(stderr,"StartPollingAccelerometers with mask=0\n");
		} 

	} //if (lmac!=0 && mask!=0) {
	return(1);
} //int	StartPollingAccelerometers(MAC_Connection *lmac,uint16_t mask)

int	StopPollingAccelerometers(MAC_Connection *lmac,uint16_t mask)
{
	unsigned char Inst[512];
	int NumBytes;

	if (lmac!=0 && mask!=0) {
		Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_POLLING_ACCELEROMETER; 
		memcpy(&Inst[1],&mask,2);  //is send little endian
		NumBytes=3;
		SendInstructionToAccel(lmac,Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
	} //if (lmac!=0 && mask!=0) {
	return(1);
} //int	StopPollingAccelerometers(MAC_Connection *lmac,uint16_t mask)


//Start Acclerometers Interrupts
int btnAccelsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i,FoundAccel;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t Threshold;
	int NumBytes;
	unsigned char Inst[512];
	uint16_t mask;

	//start accelerometers interrupt

	//initially starting an interrupt was done with a mask, 
	//but because the interrupt could be based on an absolute or relative acceleration
	//and because the threshold could be sent too when activating accelerometer interrupts (multiple 16-bit thresholds would need to be sent)
	//sending a UDP packet for each accelerometers seems to be a better approach in addition to being more user friendly and readable

	//update: I went back (in firmware) to reading an Accel mask to select accelerometers, because it's more consistent-removes the guesswork when sending a command to the EthAccels PCB. But here I am masking one accel at a time to make sure the individual relative/absolute, and individual thresholds can be set correctly.


	Inst[0]=ROBOT_ACCELMAGTOUCH_START_ACCELEROMETER_INTERRUPT; //same for all accelerometers

	//go through each Accel checkbox and send an instruction for each
	FoundAccel=0;
	for(i=0;i<15;i++) {
			NumBytes=0;
			sprintf(tstr,"chkAccelNum%d",i);
			tc=GetFTControl(tstr);
			if (tc!=0) {
				if (tc->value) {
					//enable this accelerometer
					FoundAccel=1;
					//Inst[1]=i;  //accel number
					mask=(1<<i);
					memcpy(&Inst[1],&mask,2);  //accel number mask
					//Inst[1]=mask&0xff;
					//Inst[2]=(mask&0xff00)>>8;
					NumBytes=3;//2; - mask is 16-bit

					//see if relative acceleration interrupt is checked
					sprintf(tstr,"chkAccelRelative%d",i);
					tc=GetFTControl(tstr);
					if (tc!=0) {
						if(tc->value) {
							Inst[3]=1;//[2]
						} else {
							Inst[3]=0;//[2]
						}
						NumBytes++;
					} //if (tc!=0) {

					//set the acceleration threshold too if not blank
					sprintf(tstr,"txtAccelThresh%d",i);
					tc=GetFTControl(tstr);
					if (tc!=0) {
						if(strlen(tc->text)!=0) {
							//there is a Threshold value (in g)
							Threshold=(uint16_t)(atof(tc->text)*1000); //convert to 16-bit int, and mg
							memcpy(&Inst[4],&Threshold,2);  //is send little endian
							NumBytes+=2;
						} //if(strlen(tc->text)!=0) {
					}		
				SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
				}//if (tc->value) {
			}//if (tc!=0) {
	} //for

	if (!FoundAccel) {
		FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		return(0);
	}

	return(1);
} //int btnAccelsStartInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Stop Accelerometers Interrupt
int btnAccelsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	int NumBytes;
	unsigned char Inst[512];


		//create a mask depending on the checkbox selected
		mask=0;
		for(i=0;i<15;i++) {
				sprintf(tstr,"chkAccelNum%d",i);
				tc=GetFTControl(tstr);
				if (tc!=0) {
					if (tc->value) {
						mask|=(1<<i);
					}
				}
		} //for

		if (mask!=0) {
			//Stop accelerometers interrupt
			Inst[0]=ROBOT_ACCELMAGTOUCH_STOP_ACCELEROMETER_INTERRUPT; 
			memcpy(&Inst[1],&mask,2);  //is send little endian
			NumBytes=3;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		} else {
			FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		}//if (mask!=0) {

return(1);
} //int btnAccelsStopInterrupt_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Get magnetic "hard iron" offset
int btnGetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	int NumBytes;
	unsigned char Inst[512];


		//create a mask depending on the checkbox selected
		mask=0;
		for(i=0;i<15;i++) {
				sprintf(tstr,"chkAccelNum%d",i);
				tc=GetFTControl(tstr);
				if (tc!=0) {
					if (tc->value) {
						mask|=(1<<i);
					}
				}
		} //for

		if (mask!=0) {
			//get hard iron offset
			Inst[0]=ROBOT_ACCELMAGTOUCH_GET_HARD_IRON_OFFSET; 
			memcpy(&Inst[1],&mask,2);  //is sent little endian
			NumBytes=3;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		} else {
			FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		}//if (mask!=0) {

return(1);
} //int btnGetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Set magnetic "hard iron" offset
int btnSetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	int NumBytes;
	unsigned char Inst[512];
	int16_t offset;


		//create a mask depending on the checkbox selected
		mask=0;
		for(i=0;i<15;i++) {
				sprintf(tstr,"chkAccelNum%d",i);
				tc=GetFTControl(tstr);
				if (tc!=0) {
					if (tc->value) {
						mask|=(1<<i);
					}
				}
		} //for

		if (mask!=0) {
			//Set hard iron offset
			Inst[0]=ROBOT_ACCELMAGTOUCH_SET_HARD_IRON_OFFSET; 
			memcpy(&Inst[1],&mask,2);  //is sent little endian
			NumBytes=3;
		
			//now add the offsets
			tc=GetFTControl("txtMagOffsetX");
			if (tc!=0 && strlen(tc->text)>0) {

				//convert uT into a signed 16-bit int
//from datasheet: "...For example, to correct for an offset of 60 μT, a value representing 60 μT (= 60 μT / 0.1 μT/LSB = 600 LSB), which is 0x0258 in 15-bit two's complement representation, should be written to the offset registers following a left-shift for proper alignment (0x0258<<1 = 0x04b0). ..."
				//to convert from uT to 0.1uT/LSB is *10 to left-shift is *2 so result= *20
				offset=((int16_t)(atof(tc->text)*20));//<<1;
				//memcpy(&Inst[3],&offset,2);
				//memcpy(&Inst[3],(uint8_t *)&offset,2);
				Inst[3]=(offset&0xff00)>>8;
				Inst[4]=offset&0xff;
				NumBytes+=2;
				tc=GetFTControl("txtMagOffsetY");
				if (tc!=0 && strlen(tc->text)>0) {
					offset=((int16_t)(atof(tc->text)*20));//<<1;
					//memcpy(&Inst[5],&offset,2);
					//memcpy(&Inst[5],(uint8_t *)&offset,2);
					Inst[5]=(offset&0xff00)>>8;
					Inst[6]=offset&0xff;
					NumBytes+=2;
					tc=GetFTControl("txtMagOffsetZ");
					if (tc!=0 && strlen(tc->text)>0) {
						offset=((int16_t)(atof(tc->text)*20));//<<1;
						//memcpy(&Inst[7],&offset,2);
						//memcpy(&Inst[7],(uint8_t *)&offset,2);
						Inst[7]=(offset&0xff00)>>8;
						Inst[8]=offset&0xff;
						NumBytes+=2;
					} //if (tc!=0 && strlen(tc->text)>0) {
				} //if (tc!=0 && strlen(tc->text)>0) {
			} //if (tc!=0 && strlen(tc->text)>0) {
			


			if (NumBytes>3) {
				SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
			} else {
				fprintf(stderr,"You need to fill the Mag Offset x,y,z fields\n");
				return(0);
			}
		} else {
			FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		}//if (mask!=0) {

return(1);
} //int btnSetMagOffset_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


uint16_t GetAccelMask(void)
{
	uint16_t mask;
	unsigned char tstr[64];
	FTControl *tc;
	int i;

	//create a mask depending on the checkbox selected
	mask = 0;
	for (i = 0; i < 15; i++) {
		sprintf(tstr, "chkAccelNum%d", i);
		tc = GetFTControl(tstr);
		if (tc != 0) {
			if (tc->value) {
				mask |= (1 << i);
			}
		}
	} //for

	return(mask);
} //uint32_t GetAccelMask(void)

//enable/disable accelerometer+magnometer autocalibration
int btnAutoCalibrate_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	int NumBytes;
	unsigned char Inst[512];


		//create a mask depending on the checkbox selected
		mask=0;
		for(i=0;i<15;i++) {
				sprintf(tstr,"chkAccelNum%d",i);
				tc=GetFTControl(tstr);
				if (tc!=0) {
					if (tc->value) {
						mask|=(1<<i);
					}
				}
		} //for

		if (mask!=0) {
			//Stop accelerometers interrupt

			tc=GetFTControl("btnAutoCalibrate");			
			if (!tc) {
				fprintf(stderr,"Could not get control btnAutoCalibrate\n");
				return(0);
			}
			if (!strcmp(tc->text,"Enable AutoCalibration")) {
				strcpy(tc->text,"Disable AutoCalibration");				
				Inst[0]=ROBOT_ACCELMAGTOUCH_ENABLE_ACCELMAG_AUTOCALIBRATION; 
			} else {
				strcpy(tc->text,"Enable AutoCalibration");
				Inst[0]=ROBOT_ACCELMAGTOUCH_DISABLE_ACCELMAG_AUTOCALIBRATION; 
			}
			memcpy(&Inst[1],&mask,2);  //is send little endian
			NumBytes=3;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 

			DrawFTControl(tc); //refresh control text
		} else {
			FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		}//if (mask!=0) {

return(1);
} //int btnAutoCalibrate_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//enable/disable accelerometer+magnometer autocalibration
int btnResetAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	int i;
	FTControl *tc;
	char tstr[FTMedStr];
	uint16_t mask;
	int NumBytes;
	unsigned char Inst[512];


		//create a mask depending on the checkbox selected
		mask=0;
		for(i=0;i<15;i++) {
				sprintf(tstr,"chkAccelNum%d",i);
				tc=GetFTControl(tstr);
				if (tc!=0) {
					if (tc->value) {
						mask|=(1<<i);
					}
				}
		} //for

		if (mask!=0) {
			Inst[0]=ROBOT_ACCELMAGTOUCH_RESET_ACCELEROMETER;
			memcpy(&Inst[1],&mask,2);  //is send little endian
			NumBytes=3;
			SendUserInstructionToAccel(Inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION); 
		} else {
			FTMessageBox("You must select an accelerometer first.",FTMB_OK,"Robot -- Info",0);
		}//if (mask!=0) {

return(1);
} //int btnResetAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//Open the winAnalogSensors window
int btnAnalogSensorsWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;

	twin2=GetFTWindow("winAnalogSensors");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winAnalogSensors_AddFTWindow();
	}
	return(1);
} //int btnAnalogSensorsWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//Open the winGPS window
int btnGPSWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
	FTWindow *twin2;

	twin2=GetFTWindow("winGPS");
	if (twin2!=0) { //window already open
		//bring window to focus
		FT_SetFocus(twin2,0);
	} else {
		//open the window
		winGPS_AddFTWindow();
	}
	return(1);
} //int btnGPSWin_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


void btnSendInstructionToAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y) {
	//int i;
	//char tkey[5];
	FTControl *tc;
	int NumChars,NumBytes;
	unsigned char inst[512];

	//check to see if there is an instruction in the Send raw instruction text box

	tc=GetFTControl("txtSendInstructionToAccelRaw");

	if (tc!=0) {
		if (strlen(tc->text)>0) { //there is a command there
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
				SendUserInstructionToAccel(inst,NumChars/2,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);
*/
				SendUserInstructionToAccel(inst,NumBytes,ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION);			
			}//if (NumChars>0) {
		} else {
				FTMessageBox("Enter an instruction in the Instruction textbox.",FTMB_OK,"Robot -- Info",0);
		}//	if (strlen(tc->text)>0) { //there is a command there
	} //	if (tc!=0) {

}  //void btnSendInstructionToAccel_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


//SendUserInstructionToAccel - use the Robot Accelerometers Dest IP as the EthAccelTouch PCB to send to
int SendUserInstructionToAccel(unsigned char *Inst,int numbyte,unsigned int flags)
{
	MAC_Connection *lmac;
	FTControl *tc;

	//******
	//Note that raw instructions must already have integer values (like duration, memory address, memory data, etc.) in little endian format
	//****
	//test for connect and connect if not connected
	//ConnectAccelIPs();
			
	//get the MAC_Connection
	tc=GetFTControl("txtAccelDestIPAddressStr"); //get remote connection IP
	if (tc==0) {
		fprintf(stderr,"Could not get control txtAccelDestIPAddressStr\n");
		return(0);
	}
	lmac=Get_MAC_Connection(tc->text); //get remote MAC_Connection, 1=IsRemote			
	if (lmac==0) {
		fprintf(stderr,"Could not get MAC_Connection for dest ip: %s\n",tc->text);
		return(0);
	}

	return(SendInstructionToAccel(lmac,Inst,numbyte,flags));

} //int SendUserInstructionToAccel(unsigned char *Inst, int numbyte, unsigned int flags);


//Checks to see if the source and dest (Accel) MAC_Connections exist first
//and if yes, sends the data to the dest
//flags:
//ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION - add the robot source IP address before the instruction (otherwise there should be some other source IP address in the first 4 bytes of the instruction)
//note is very similar to SendInstructionToRobot- the only difference is ConnectGenericIPs vs ConnectAccelIPs
int SendInstructionToAccel(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)
{
	int i;
	int iResult=0;
	unsigned char SendInst[ROBOT_MAX_INST_LEN]; //
	int NumBytes;
	char DataStr[512];

	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "SendInstructionToAccel\n");
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
		//iResult = send(lmac->Socket,(const char *)SendInst,numbyte+4,0);
	} else {
		//otherwise the IP is presumably already at the beginning of the instruction
		memcpy(SendInst,(const char *)Inst,numbyte);
//		iResult = send(lmac->Socket,(const char *)SendInst,numbyte,0);
	}
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
} //int SendInstructionToAccel(MAC_Connection *lmac,unsigned char *Inst,int numbyte,unsigned int flags)

void btnwinAccelsExit_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
{
//	FTMessageBox("Close Click",FTMB_OK,"Info");

CloseFTWindow(twin); 

}  //btnwinAccelsExit_Click

int winAccels_OnOpen(FTWindow *twin)
{
	FTControl *tc;
	MAC_Connection *lmac;


	lmac=0;
	//Go through each MAC_Connection and get all names that exist for this PCB yet
	//lmac=Get_MAC_Connection_By_WindowFunction((FTControlfunc *)winAccels_AddFTWindow);
	//add all known EthAccelTouch PCBs to the item list
	FillAccelItemList();
	


	return(1);
} //int winAccels_OnOpen(FTWindow *twin)

int winAccels_OnClose(FTWindow *twin)
{

	return(1);
} //int winAccels_OnClose(FTWindow *twin)

int chkAccel_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol) 
{
	//if (RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) {
	if (tcontrol->value) {
		RStatus.flags|=ROBOT_STATUS_SHOW_ACCEL_DATA;
	} else {
		RStatus.flags&=~ROBOT_STATUS_SHOW_ACCEL_DATA;
	}
	return(1);
} //int chkAccel_ShowData_OnChange(FTWindow *twin,FTControl *tcontrol) 

int chkAccel_UseCalibration_OnChange(FTWindow *twin, FTControl *tcontrol)
{
	//if (RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) {
	if (tcontrol->value) {
		RStatus.flags |= ROBOT_STATUS_USE_CALIBRATION;
	}
	else {
		RStatus.flags &= ~ROBOT_STATUS_USE_CALIBRATION;
	}
	return(1);
} //int chkAccel_UseCalibration_OnChange(FTWindow *twin,FTControl *tcontrol) 



int chkAccel_Accelerometers_Only_OnChange(FTWindow *twin, FTControl *tcontrol)
{
	if (tcontrol->value) {
		RStatus.flags |= ROBOT_STATUS_ACCELEROMETERS_ONLY;
	}
	else {
		RStatus.flags &= ~ROBOT_STATUS_ACCELEROMETERS_ONLY;
	}
	return(1);
} //int chkAccel_Accelerometers_Only_OnChange(FTWindow *twin, FTControl *tcontrol)


//void btnCalibrateAccels_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
//{
//	CalibrateAccels(0); //0=accels
//} //void btnCalibrateAccels_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)

//void btnCalibrateGyros_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)
//{
//	CalibrateAccels(1); //1=gyros
//} //void btnCalibrateGyros_Click(FTWindow *twin,FTControl *tcontrol,int x,int y)


int btnCalibrateAllAccelGyro_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
//	int PCBNum[NUM_ETHACCEL_PCBS],i,NumCalibSamples,AccelAndOrGyro;
	int i,NumCalibSamples,AccelAndOrGyro;
	FTControl *tc;

/*
	for (i = 0; i < NUM_ETHACCEL_PCBS; i++) {
		PCBNum[i] = i+1; //set PCBNum (ROBOT_PCB_ETHACCELS_LEFT_LEG, etc.)
		AStatus.PCBNum[i]=i+1;  //for thread, above can be eventually removed
	}//
*/

	AStatus.flags|=ACCEL_WINDOWS_STATUS_ALL_ETHACCELS;

	AccelAndOrGyro = 0;
	tc = GetFTControl("chkCalibAccels");
	if (tc != 0 && tc->value) {
		AccelAndOrGyro = 1;
	}
	tc = GetFTControl("chkCalibGyros");
	if (tc != 0 && tc->value) {
		AccelAndOrGyro |= 2;
	}

	//for now also calibrate touch sensors
	AccelAndOrGyro |= 4;


	tc = GetFTControl("txtNumCalibSamples");
	if (tc != 0 && atoi(tc->text)>0) {
		NumCalibSamples = atoi(tc->text);
	}
	else {
		NumCalibSamples = 100;
	}


	//CalibrateAccelGyro(NUM_ETHACCEL_PCBS, PCBNum, 7, AccelAndOrGyro, NumCalibSamples);
  //AStatus.NumPCBs=NUM_ETHACCEL_PCBS;
	AStatus.AccelMask=7;
	AStatus.AccelAndOrGyro=AccelAndOrGyro;
	AStatus.NumCalibSamples=NumCalibSamples;

	//to do - destroy or wait for thread if already exists
	CreateAutoCalibrateThread();

	RStatus.flags|=ROBOT_STATUS_CALIBRATING;
	while(RStatus.flags&ROBOT_STATUS_CALIBRATING); //wait for thread to finish
	//Terminate autocalibration (real-time) thread
	StopAutoCalibrateThread();


	AStatus.flags&=~ACCEL_WINDOWS_STATUS_ALL_ETHACCELS;  //clear flag

	return(1);
} //int btnCalibrateAllAccelGyro_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)


int btnCalibrateAccelGyro_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	int AccelAndOrGyro,NumCalibSamples,PCBNum;
	uint16_t AccelMask;
	FTControl *tc;

	//determine which EthAccel PCB is selected
	tc=GetFTControl("ddAccelPCBName");
	if (tc != 0) {
		//get the PCBNum (we could just get the PCB and then would have the PCBNum, but there is a function to get the PCBNum)
		PCBNum = Get_AccelsPCBNum_By_PCBName(tc->text);
		if (PCBNum==-1) {
			fprintf(stderr, "Error: Could not find PCBNum for EthAccel PCB with name '%s'\n", tc->text);
			return(0);
		}
		AccelAndOrGyro = 0;
		tc = GetFTControl("chkCalibAccels");
		if (tc != 0 && tc->value) {
			AccelAndOrGyro = 1;
		}
		tc = GetFTControl("chkCalibGyros");
		if (tc != 0 && tc->value) {
			AccelAndOrGyro |= 2;
		}
		tc = GetFTControl("txtNumCalibSamples");
		if (tc != 0 && atoi(tc->text) > 0) {
			NumCalibSamples = atoi(tc->text);
		}
		else {
			NumCalibSamples = 100;
		}

		//determine which Accel+Gyro(s)
		AccelMask = GetAccelMask();

		if (AccelMask == 0) {
			FTMessageBox("You must select one or more accelerometers first.", FTMB_OK, "Robot -- Info", 0);
			return(0);
		}
//		CalibrateAccelGyro(1, PCBNum, AccelMask, AccelAndOrGyro, NumCalibSamples);
		//AStatus.NumPCBs=NUM_ETHACCEL_PCBS;
		AStatus.PCBNum=PCBNum;  //Is AccelsPCBNum
		AStatus.AccelMask=7;
		AStatus.AccelAndOrGyro=AccelAndOrGyro;
		AStatus.NumCalibSamples=NumCalibSamples;

		//to do - destroy or wait for thread if already exists
		CreateAutoCalibrateThread();


		RStatus.flags|=ROBOT_STATUS_CALIBRATING;
		while(RStatus.flags&ROBOT_STATUS_CALIBRATING); //wait for thread to finish
		//Terminate autocalibration (real-time) thread
		StopAutoCalibrateThread();


		return(1);
	} //if (tc!=0) {
	return(0);
} //void btnCalibrateAccelGyro_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)


//Calibate samples in a real-time thread 

//Create the RobotTasks thread- the thread that process all Robot tasks (currently: 1:run scripts, 2:balance)
int CreateAutoCalibrateThread(void)
{
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
	int HighestPriority;
#endif


		//create AutoCalibrate thread 
#if Linux
		//get highest priority possible
		HighestPriority=sched_get_priority_max(SCHED_FIFO);
		//fprintf(stderr,"Highest Priority=%d\n",HighestPriority); //=99 in Linux
		//set thread priority
		pthread_attr_init(&tattr);
		pthread_attr_getschedparam(&tattr,&param);
		param.sched_priority=HighestPriority-1;//98;
		pthread_attr_setschedparam(&tattr,&param);
		pthread_create(&RStatus.AutoCalibrateAccelGyroThread,&tattr,(void *)Thread_AutoCalibrateAccelGyro,NULL);
#endif
#if WIN32
		RStatus.AutoCalibrateAccelGyroThread=CreateThread(0,0,Thread_AutoCalibrateAccelGyro,0,0,0);
#endif

		RStatus.flags|=ROBOT_STATUS_AUTOCALIBRATE_THREAD_RUNNING;

	return(1);
} //int CreateAutoCalibrateThread(void)


int StopAutoCalibrateThread(void)
{
	int Failed,iResult;

	Failed=0;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"StopAutoCalinbrateThread\n");
	}


	RStatus.flags&=~ROBOT_STATUS_AUTOCALIBRATE_THREAD_RUNNING;  //exits while in thread
#if Linux
	usleep(100000); //0.5s for thread to exit while fully
	iResult=pthread_cancel(RStatus.AutoCalibrateAccelGyroThread);
	if (iResult<0) {
		Failed=1;
	}
		//wait for thread to terminal cleanly
		if (iResult < 0) {
	    //handle_error_en(iResult,"pthread_cancel");
			fprintf(stderr,"pthread_cancel failed on AutoCalibrateAccelGyroThread\n");
		} 
		/* Join with thread to see what its exit status was */
		iResult = pthread_join(RStatus.AutoCalibrateAccelGyroThread, 0);
		if (iResult != 0) {
	    //handle_error_en(iResults, "pthread_join");
			fprintf(stderr,"pthread_join failed on AutoCalibrateAccelGyroThread\n");
		} 
#endif
#if WIN32
	Sleep(100); //wait 100ms for thread while to exit cleanly
	if (TerminateThread(RStatus.AutoCalibrateAccelGyroThread,0)<0) {
		Failed=1;
	}
	Sleep(100); //for now wait 100ms todo: wait until thread is terminated
#endif


	return(!Failed);
	//return(1);
} //int StopAutoCalinbrateThread(void)



//TODO: make into a thread, and then add RT_PREEMPT real-time- because some times many more samples are being read than needed
//Calibrate Accelerometers and Gyroscopes
//NumPCs: number of EthAccelPCBs with accels/gyros being calibrated
//*PCBNum: pointer to an array of PCB Names (ROBOT_PCB_ETHACCELS_LEFT_LEG, etc.)
//AccelMask: 000-111b (0-7) depending on which Accels/Gyros to calibrate 001=1, 011=1&2, etc. - possibly make array- currently if more than one EthAccel, all accels/gyros are calibrated. It is easy 
//  for a user just to calibrate individual accel/gyros on different PCBs one at a time if that is desired/necessary.
//AccelGyroFlag: 0-3 01=calibrate accel only, 10=gyro only, 11=both accel and gyro 
//NumCalibSamples: number of samples to use in calibration
//int CalibrateAccelGyro(int NumPCBs, int *PCBNum, uint16_t AccelMask, int AccelAndOrGyro, int NumCalibSamples)
#if Linux
int Thread_AutoCalibrateAccelGyro(void)
#endif
#if WIN32
DWORD WINAPI Thread_AutoCalibrateAccelGyro(LPVOID lpParam)
#endif
{
//	RobotScript *lscript;
//	char ScriptName[255], *ScriptData;
	//int ScriptLen, 
	int i, j, k, timeout, GotNewTimeStamp, CalibratedPCB;
	//RobotTask *ltask;
	Accels_PCB *leth[MAX_NUM_OF_ETHACCELS];
	int SampleCount[MAX_NUM_OF_ETHACCELS],TotalSampleCount,NoSampleCount,SampleComplete;
	uint32_t CurTimeStamp,LastTimeStamp[MAX_NUM_OF_ETHACCELS];
	AccelSample SampleA[MAX_NUM_OF_ETHACCELS][3], SampleG[MAX_NUM_OF_ETHACCELS][3], AvgSamp;
	FILE *fptrA, *fptrG;
	char FilenameA[512], FilenameG[512], tstr[256];
	Accelerometer *lAccel;
	//FTControl *tc;
	MAC_Connection *lmac;
	int NumPCBs;
	int PCBNum[MAX_NUM_OF_ETHACCELS],EAPCBNum[MAX_NUM_OF_ETHACCELS],EAPN;
	uint16_t AccelMask;
	int AccelAndOrGyro;
	int NumCalibSamples;
	FTControl *tc;

#if USE_RT_PREEMPT
	struct timespec t;
	struct sched_param param;
#endif


	if (AStatus.flags&ACCEL_WINDOWS_STATUS_ALL_ETHACCELS) {
		//calibrate accels on all EthAccels
		//go through EthAccels PCB dropdown to determine how many EthAccels PCBs there are
		tc=GetFTControl("ddAccelPCBName");
		if (tc!=0) {
			//NumPCBs=tc->NumItems;  //get the number of active EthAccels PCBs
			//NumPCBs=GetCurrentEthAccelsPCBCount();  //perhaps better to order EthAccels by PCB NameNum
			NumPCBs=RStatus.NumEthAccelsPCBsExpected;
		} else {
			fprintf(stderr,"Error: Failed to get ddAccelPCBName control\n");
			return(0);
		}
	} else { //if (AStatus.flags&ACCEL_WINDOWS_STATUS_ALL_ETHACCELS) {
		NumPCBs=1;
		PCBNum[0]=AStatus.PCBNum;  //Is EthAccels PCBNum- Important to note that PCBNum[0] may != 0
	} //else if (AStatus.flags&ACCEL_WINDOWS_STATUS_ALL_ETHACCELS) {


	AccelMask=AStatus.AccelMask;
	AccelAndOrGyro=AStatus.AccelAndOrGyro;
	NumCalibSamples=AStatus.NumCalibSamples;

	if (AccelAndOrGyro & 1) {
		fprintf(stderr, "\n\nTo calibrate accelerometers make sure all acclerometers are being polled, and that all robot body segments are as close to vertical as possible (except the feet which should be as close to horizontal as possible)- except for any accelerometers/body segments that the user has specified pitch or roll angles for in the Robot.conf file.\n\n");
	}
	if (AccelAndOrGyro & 2) {
		fprintf(stderr, "\n\nTo calibrate gyroscopes make sure all gyroscopes are being polled, and that none are moving.\n\n");
	}

	//now go through each accel
	//skip first 100, then add next 1000 samples 

	
	for(i=0;i<NumPCBs;i++) {  //for each EthAccel PCB
		if (AStatus.flags&ACCEL_WINDOWS_STATUS_ALL_ETHACCELS) {
			//calibrate accels on all Accels PCBs
			PCBNum[i]=RStatus.AccelsPCBInfo[i]->PCBNum;
		} //if (AStatus.flags&ACCEL_WINDOWS_STATUS_ALL_ETHACCELS) {
		leth[i] = Get_AccelsPCB_By_PCBNum(PCBNum[i]);
		if (!leth[i]) {
			fprintf(stderr,"Error: CalibrateAccelGyro(): Could not find EthAccel PCB with number=%d\n",PCBNum[i]);
			return(0);
		}
		//Record the AccelsPCBNum (index into RStatus.AccelsPCBInfo[])
		EAPCBNum[i]=leth[i]->AccelsPCBNum;
	} //for(i=0;i<NumPCBs;i++) { 	



	for(i=0;i<NumPCBs;i++) {  //for each EthAccel PCB
		LastTimeStamp[i]=0;
		SampleCount[i]=0;
		for(j=0;j<3;j++) {  //for each Accel 0,1, 2
			for(k=0;k<3;k++) {  //for each dimension (X, Y, Z)
				SampleA[i][j].i[k]=0.0;
				SampleG[i][j].i[k]=0.0;
			} //for k
		}  //for j
	} //for i



	switch(AccelAndOrGyro) {
		case 1:
			sprintf(tstr, "Starting to calibrate Accelerometers...\n");
		break;
		case 2:
			sprintf(tstr, "Starting to calibrate Gyroscopes...\n");
		break;
		case 3:
			sprintf(tstr, "Starting to calibrate Accelerometers and Gyroscopes...\n");
		break;
		case 7:
			sprintf(tstr, "Starting to calibrate Accelerometers and Gyroscopes (and setting Analog Sensors Max and Min)...\n");
		break;

	}  //switch(AccelAndOrGyro)


	TotalSampleCount=NumCalibSamples+100;
	fprintf(stderr,"NumSamples to get: %d\n",NumCalibSamples);

	fprintf(stderr,"%s",tstr);

	RStatus.flags|=ROBOT_STATUS_CALIBRATING; //so samples are not rounded to +-1.000
	timeout = 0;
	for (i = 0; i < NumPCBs; i++) {  //for each EthAccel PCB
		LastTimeStamp[i] = GetTimeInMS();
	}


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
#endif //#if USE_RT_PREEMPT



//		while (keep_on_running) {
	while(!timeout) {

#if USE_RT_PREEMPT
		//wait until next timer interval
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		//calculate next time to run
		t.tv_nsec += TasksTimerInterval;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}

		//set keep_on_running to 0 if you want to exit
//		if (!(RStatus.flags&ROBOT_STATUS_AUTOCALIBRATE_THREAD_RUNNING)) {
//			keep_on_running = 0;
//		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
#endif //USE_RT_PREEMPT


	
		CurTimeStamp = GetTimeInMS();


		for(i=0;i<NumPCBs;i++) { //for each PCB
			//for each Accel in mask
			//determine if TimeStamp is new
			//note that for a single PCB, not all accels may have a timestamp. If only 1 or 2 accels are selected, the firmware will noly send samples from those accels. 
			//For convenience, and for now, since I scan all accels at once, and do not want to scan one PCB at a time, so just verify that at least one accel per PCB has a new timestamp
			GotNewTimeStamp = 0;
			for (j = 0; j < 3; j++) { //for each accel
				if ((1<<j) & AccelMask) {  //only for selected accels
					if (LastTimeStamp[i] != leth[i]->Accel[j].TimeStamp) {
						GotNewTimeStamp = 1;
						LastTimeStamp[i] = leth[i]->Accel[j].TimeStamp; //update timestamp
					}
				} //if ((1<<j) & AccelMask) {  //only for selected accels
			} //for j


			if (GotNewTimeStamp) { //new accel sample				
				if (SampleCount[i]>100) { //only add up after 100 timestamps/samples
					for(j=0;j<3;j++) { //for each accel
						//if ((int)pow(2.0,(double)j) & AccelMask) {  //only for selected accels
						if ((1<<j) & AccelMask) {  //only for selected accels
							for (k = 0; k < 3; k++) { //x,y,z
								switch (AccelAndOrGyro) {
								case 1:  //accels only
									SampleA[i][j].i[k] += leth[i]->Accel[j].ASample.i[k];
									break;
								case 2:  //gyros only
									SampleG[i][j].i[k] += leth[i]->Accel[j].GSample.i[k];
									break;
								case 3:  //accels and gyros
								case 7:  //accels and gyros and touch sensors
									SampleA[i][j].i[k] += leth[i]->Accel[j].ASample.i[k];
									SampleG[i][j].i[k] += leth[i]->Accel[j].GSample.i[k];
									break;
								} //switch
							} //for k
						} //if ((1<<j) & AccelMask) {
					} //for j
				} //if (SampleCount[i]>100) {
				SampleCount[i]++;					
				if (i==0 && SampleCount[0]>100 && SampleCount[0]%100==0) { 
					fprintf(stderr,"Samples: %d\n",SampleCount[0]-100);
				} 
			} //if (GotNewTimeStamp) {//new accel sample
		} //for i

		//end if all accels have the correct number of samples
		NoSampleCount = 0;
		SampleComplete = 0;
		for (i = 0; i < NumPCBs; i++) {  //for each EthAccel PCB  
		
			if (SampleCount[i] >= TotalSampleCount) {
				SampleComplete++;
			}
			if (SampleComplete == NumPCBs) {  //if all SampleCounts >= TotalSampleCount
				timeout = 1;  //done sampling, exit while loop
				fprintf(stderr, "Calibration completed successfully.\n");
			}

			//timeout if any EthAccels have not returned samples in 1 second (note that curently timestamp is connected to accel not EthAccel PCB since one accel may not be connected or respond)
			//note that when sampling all accels, some accels are not connected and so will not return samples, but we still want to sample the others
			//so above if timestamp for accel[0] has not changed, accel[1] and then accel[2] are examined
			//1 second with no sample
			if (CurTimeStamp > LastTimeStamp[i] + 1000) {
				NoSampleCount++;
				//NoSampleCount=i;
			}
//			if (NoSampleCount == NumPCBs) {  //if all PCBs have not returned a sample in 1 second
			if (NoSampleCount==2) {  //at least one accel PCB has not returned a sample in 1 second
				timeout = 1;  //done sampling, exit while loop
				fprintf(stderr, "Calibration timed out: no samples from any accel on PCB %s in 1 second.\n", RStatus.EthPCBInfo[PCBNum[i]].Name);
			}

		} //for i

	} //while(!timeout)


	//*********************
	//Done reading in samples, now determine average and write into .conf files
	//*********************

	//print results to terminal and to accel.conf and/or gyro.conf file(s)

	//Note that the currently the calibration values of all EthAccels are output

	//Accelerometers
 	if (AccelAndOrGyro&1) {
#if Linux
		sprintf(FilenameA, "%s/accel.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(FilenameA, "%s\\accel.conf", RStatus.CurrentRobotFolder);
#endif

		//sprintf(FilenameA, "%saccel.conf", PStatus.ProjectPath);
		fptrA = fopen(FilenameA, "wb"); //currently presumes config is in current working directory
		if (fptrA == 0) {
			fprintf(stderr, "Error trying to open file %s for writing\n", FilenameA);
		}
		fprintf(stderr,"\n\n%s:\n",FilenameA);

		for(i=0;i<RStatus.NumEthAccelsPCBsExpected;i++) { //for each pcb
			EAPN=EAPCBNum[i];

			//fprintf(stderr,"%s\n",ROBOT_PCB_NAMES[i+1]);
			fprintf(stderr,"%s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
			if (fptrA != 0) {
				//fprintf(fptrA,"%s\n", ROBOT_PCB_NAMES[i+1]);
				fprintf(fptrA,"%s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
			}

			//if this PCB did not have an accel that was calibrated, just print the accel.conf values
			CalibratedPCB=0;
			for(j=0;j<NumPCBs;j++) {
				if (j==EAPN) {
					CalibratedPCB=1;  //needs to be calibrated
				} //if (i+1==PCBNum[j]) {
			} //for j=0


			if (CalibratedPCB==0) {
				for(j=0;j<3;j++) { //for each accel on a single pcb
					//fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,RStatus.AccelsPCBInfo[EAPN].AOffset[j][0],RStatus.AccelsPCBInfo[EAPN].AOffset[j][1], RStatus.AccelsPCBInfo[EAPN].AOffset[j][2]);
					fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->AOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][1], RStatus.AccelsPCBInfo[EAPN]->AOffset[j][2]);

					if (fptrA != 0) {
						fprintf(fptrA, "%d: %03.3f %03.3f %03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->AOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][1], RStatus.AccelsPCBInfo[EAPN]->AOffset[j][2]);
					}
				} //for j
			} else { //CalibratedPCB==0
				//PCB has at least 1 accel that was calibrated

				for(j=0;j<3;j++) { //for each accel on a single pcb

					//because not all accelerometers on a single EthAccels PCB may be getting calibrated, 
					//only output new values for those accels/gyros being calibrated, for the rest use the 
					//calibration file values read in InitRobot().
					if ((1<<j) & AccelMask) { //2^j only for selected accels


						//note that some samplecounts may not be exactly 1100- for example if accel[0] on EthAccel[0] reaches 1100 the while ends, the accel[0] on EthAccel[1] and [2] will have 1099 samples. So use SampleCount[i]-100
						//offset is subtracted from sample, so if z=1.07, zoffset=0.07, so sample-=0.07
						lAccel = &leth[i]->Accel[j];
						//accel - determine offset values needed to make accel x,y,z = 0.0,0.0,1.0 respectively
										//       unless the user has specified a pitch (x) and/or roll (z)
						AvgSamp.x=SampleA[i][j].x / (SampleCount[i] - 100.0);
						AvgSamp.y=SampleA[i][j].y / (SampleCount[i] - 100.0);
						AvgSamp.z=SampleA[i][j].z / (SampleCount[i] - 100.0);
						if (RStatus.AccelInfo[lAccel->NameNum].Calib.x!=0.0 || RStatus.AccelInfo[lAccel->NameNum].Calib.z!=0.0) {
							lAccel->ACali[0] = AvgSamp.x-(RStatus.AccelInfo[lAccel->NameNum].Calib.z*PI*AvgSamp.z/180.0);  //accel x should = (Roll*PI/180)*z
							lAccel->ACali[1] = AvgSamp.y+(RStatus.AccelInfo[lAccel->NameNum].Calib.x*PI*AvgSamp.z/180.0);  //accel y should = -(Pitch*PI/180)*z
							lAccel->ACali[2] = 0.0;// Z is not calibrated since user gave pitch and roll
						} else {
							lAccel->ACali[0] = AvgSamp.x;  //accel x should = 0.0
							lAccel->ACali[1] = AvgSamp.y;  //accel y should = 0.0
							lAccel->ACali[2] = AvgSamp.z-1.0;  //accel z should = 1.0 
						} //if (RStatus.AccelInfo[lAccel->NameNum].Calib.x!=0.0 ...
						fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,lAccel->ACali[0], lAccel->ACali[1], lAccel->ACali[2]);

						if (fptrA != 0) {
							fprintf(fptrA, "%d: %03.3f %03.3f %03.3f\n", j, lAccel->ACali[0], lAccel->ACali[1], lAccel->ACali[2]);
						}

						//update the earlier loaded values- in case different accels are updated later
						RStatus.AccelsPCBInfo[EAPN]->AOffset[j][0]=lAccel->ACali[0];
						RStatus.AccelsPCBInfo[EAPN]->AOffset[j][1]=lAccel->ACali[1];
						RStatus.AccelsPCBInfo[EAPN]->AOffset[j][2]=lAccel->ACali[2];


					} else { //if ((1<<j) & AccelMask) { //2^j only for selected accels selected for calibration
						//Accel was not selected for calibration so just print out the calibration values read in InitRobot().
						fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->AOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][2]);

						if (fptrA != 0) {
							fprintf(fptrA, "%d: %03.3f %03.3f %03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->AOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->AOffset[j][2]);
						}
					} //if ((1<<j) & AccelMask) { //2^j only for selected accels
				} //for j
			} //if (CalibratedPCB==0) {
		} //for i

		if (fptrA != 0) {
			fclose(fptrA);
		}
	} //if (AccelAndOrGyro&1) {



	//Gyroscopes
 	if (AccelAndOrGyro&2) {
//		sprintf(FilenameG, "%sgyro.conf", PStatus.ProjectPath);
#if Linux
		sprintf(FilenameG, "%s/gyro.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(FilenameG, "%s\\gyro.conf", RStatus.CurrentRobotFolder);
#endif
		fptrG = fopen(FilenameG, "wb"); //currently presumes config is in current working directory
		if (fptrG == 0) {
			fprintf(stderr, "Error trying to open file %s for writing\n", FilenameG);
		}
		fprintf(stderr,"\n\n%s:\n",FilenameG);


		for(i=0;i<RStatus.NumAccelsPCBsExpected;i++) {
			EAPN=EAPCBNum[i];
			fprintf(stderr,"%s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
			if (fptrG != 0) {
				fprintf(fptrG,"%s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
			}

			//if this PCB did not have an accel that was calibrated, just print the accel.conf values
			CalibratedPCB=0;
			for(j=0;j<NumPCBs;j++) {
				if (i+1==PCBNum[j]) {
					CalibratedPCB=1;
				} //if (i==PCBNum[j]) {
			} //for j=0


			if (CalibratedPCB==0) {
				for(j=0;j<3;j++) { //for each gyro on a single pcb
					fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->GOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][2]);

					if (fptrG != 0) {
						fprintf(fptrG, "%d: %03.3f %03.3f %03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->GOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][2]);
					}
				} //for j
			} else { //CalibratedPCB==0
				//PCB has at least 1 accel that was calibrated


				for(j=0;j<3;j++) { //for each accel on a single pcb

					//because not all accelerometers on a single EthAccels PCB may be getting calibrated, 
					//only output new values for those accels/gyros being calibrated, for the rest use the 
					//calibration file values read in InitRobot().

					if ((1<<j) & AccelMask) { //2^j only for selected accels

						//note that some samplecounts may not be exactly 1100- for example if accel[0] on EthAccel[0] reaches 1100 the while ends, the accel[0] on EthAccel[1] and [2] will have 1099 samples. So use SampleCount[i]-100
						//offset is subtracted from sample, so if z=1.07, zoffset=0.07, so sample-=0.07
						lAccel = &leth[i]->Accel[j];
						//gyro - determine offset values needed to make gyro x,y,z all = 0.0
						lAccel->GCali[0] = SampleG[i][j].x / (SampleCount[i] - 100.0);
						lAccel->GCali[1] = SampleG[i][j].y / (SampleCount[i] - 100.0);
						lAccel->GCali[2] = SampleG[i][j].z / (SampleCount[i] - 100.0);
						fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,lAccel->GCali[0], lAccel->GCali[1], lAccel->GCali[2]);

						if (fptrG != 0) {
							fprintf(fptrG, "%d: %03.3f %03.3f %03.3f\n", j, lAccel->GCali[0], lAccel->GCali[1], lAccel->GCali[2]);
						}
						//update the RStatus.AccelsPCBInfo calibration values- in case different accels are updated later
						RStatus.AccelsPCBInfo[EAPN]->GOffset[j][0]=lAccel->GCali[0];
						RStatus.AccelsPCBInfo[EAPN]->GOffset[j][1]=lAccel->GCali[1];
						RStatus.AccelsPCBInfo[EAPN]->GOffset[j][2]=lAccel->GCali[2];

					} else { //if ((1<<j) & AccelMask) { //2^j only for selected accels
						//Accel/Gyro was not selected for calibration so just print out the calibration values read in InitRobot().
						fprintf(stderr,"%d: x=%03.3f y=%03.3f z=%03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->GOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][2]);

						if (fptrG != 0) {
							fprintf(fptrG, "%d: %03.3f %03.3f %03.3f\n",j,RStatus.AccelsPCBInfo[EAPN]->GOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][1],RStatus.AccelsPCBInfo[EAPN]->GOffset[j][2]);
						}
					} //if ((1<<j) & AccelMask) { //2^j only for selected accels
				} //for j
			} //if (CalibratedPCB==0) { PCB had one or more gyros that were calibrated
		} //for i

		if (fptrG != 0) {
			fclose(fptrG);
		}
	} //if (AccelAndOrGyro&2) {


	//set Analog Sensor Max and Min
	if (AccelAndOrGyro&4) {
		//for each EthAccel
		for(i=0;i<NumPCBs;i++) {
			EAPN=EAPCBNum[i];


			//get the MAC_Connection this Detected PCB is part of 
			lmac=Get_MAC_Connection_By_PCBNum(EAPN);
			//lmac=Get_MAC_Connection_By_PCBNum(CaliFile[i].PCBNum);
			//lmac=leth[i]->mac;
			
			if (!lmac) {
				//fprintf(stderr,"Error: Could not get MAC_Connection for PCB %s\n",ROBOT_PCB_NAMES[CaliFile[i].PCBNum]);
				fprintf(stderr,"Error: Could not get MAC_Connection for PCB %s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
			} 
			for (j=0;j<RStatus.AccelsPCBInfo[EAPN]->NumAnalogSensors;j++) { //for each touch sensor (start at 0)
				if (lmac) {
					SetAnalogSensorsMinAndMax(lmac,(unsigned int)1<<j,RStatus.AccelsPCBInfo[EAPN]->AnOffset[j][0],RStatus.AccelsPCBInfo[EAPN]->AnOffset[j][1]);		
				} //if (lmac) {
			} //for j
			fprintf(stderr,"Set Analog Sensors Min and Max for PCB %s\n",RStatus.AccelsPCBInfo[EAPN]->Name);
		} //for i
	} //if (AccelAndOrGyro&4) {


	RStatus.flags&=~ROBOT_STATUS_CALIBRATING; //so samples are again rounded to +-1.000

	return(1);
}  //int Thread_AutoCalibrateAccelGyro(void);
//int CalibrateAccelGyro(int NumPCBs, int *PCBNum,uint16_t AccelMask,int AccelAndOrGyro,int NumCalibSamples)



