//win_Cameras.c
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
#include "win_Cameras.h"
#include "win_Robot.h"
//#include "robot_Camera_mcu_instructions.h"

#include <errno.h>
#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif  //WIN32




extern RobotStatus RStatus;
extern ProgramStatus PStatus;
extern int *Ly1,*Ly2;  //levels for buttons

int winCameras_AddFTWindow(void)
{
FTWindow twin,*twin2;

twin2=GetFTWindow("winCameras");
if (twin2==0) {
	memset(&twin,0,sizeof(FTWindow));
	strcpy(twin.name,"winCameras");
	strcpy(twin.title,"Cameras");
	strcpy(twin.ititle,"Cameras");
	twin.x=100;
	twin.y=100;
	twin.w = ROBOT_CAMERA_WIDTH + 1000;// 320;
	twin.h = ROBOT_CAMERA_HEIGHT+130;
	twin.Keypress=(FTControlfuncwk *)winCameras_key;
	twin.AddFTControls=(FTControlfuncw *)winCameras_AddFTControls;
	twin.OnOpen=(FTControlfuncw *)winCameras_OnOpen;
	twin.OnClose=(FTControlfuncw *)winCameras_OnClose;
	//twin.OnTimer = (FTControlfuncw *)KeepConnectionAlive;

	//twin.timer = 100; //every 100ms see if camera is still connected and send an f1e0 if not- actually this tells the camera to send another image and so the camera restarts
	CreateFTWindow(&twin);
	twin2=GetFTWindow(twin.name);
	DrawFTWindow(twin2);
		
  //fprintf(stderr,"b4 free %p %p\n",twin->ititle,twin);
  return 1;
  }
return 0;
}  //end winCameras_AddFTWindow



int winCameras_AddFTControls(FTWindow *twin)
{
int fw,fh;
FTControl *tcontrol;

#if WIN32
DWORD dwWaitResult;
#endif

if (PStatus.flags&PInfo)
  fprintf(stderr,"Adding controls for window 'winCameras'\n");

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
strcpy(tcontrol->name,"lblCameraName");
tcontrol->type=CTLabel;
tcontrol->x1=fw;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"Camera:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
//strcpy(tcontrol->name,"txtCameraPCBName");
strcpy(tcontrol->name,"ddCameraName");
tcontrol->type=CTDropdown;
tcontrol->x1=fw*9;
tcontrol->y1=fh;
tcontrol->x2=fw*34;
tcontrol->y2=fh*2+fh/2;
tcontrol->OnChange=(FTControlfuncwc *)ddCameraName_OnChange;
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblCameraMACStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*35;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text, "MAC:");
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"lblCameraDestIPAddressStr");
tcontrol->type=CTLabel;
tcontrol->x1=fw*53;
tcontrol->y1=fh;
tcontrol->y2=fh*2+fh/2;
strcpy(tcontrol->text,"IP:");
AddFTControl(twin,tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"txtCameraDestIPAddressStr");
tcontrol->type=CTTextBox;
tcontrol->x1=fw*57;
tcontrol->y1=fh;
tcontrol->x2=fw*77;
tcontrol->y2=fh*2+fh/2;
//strcpy(tcontrol->text,"192.168.1.2");
//see if a name exists for this PCB yet
//tcontrol->OnChange=(FTControlfuncwc *)txtCameraDestIPAddress_OnChange;
AddFTControl(twin,tcontrol);

PopulateCameraDropdowns(); //fill dropdown with any recognized camera connections


memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblCameraUsername");
tcontrol->type = CTLabel;
tcontrol->x1 = fw * 78;
tcontrol->y1 = fh;
tcontrol->y2 = fh * 2 + fh / 2;
strcpy(tcontrol->text, "User:");
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtCameraUsername");
tcontrol->type = CTTextBox;
tcontrol->x1 = fw * 84;
tcontrol->y1 = fh;
tcontrol->x2 = fw * 94;
tcontrol->y2 = fh * 2 + fh / 2;
//tcontrol->OnChange=(FTControlfuncwc *)txtCameraUsername_OnChange;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblCameraPassword");
tcontrol->type = CTLabel;
tcontrol->x1 = fw * 95;
tcontrol->y1 = fh;
tcontrol->y2 = fh * 2 + fh / 2;
strcpy(tcontrol->text, "Password:");
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtCameraPassword");
tcontrol->type = CTTextBox;
tcontrol->x1 = fw * 105;
tcontrol->y1 = fh;
tcontrol->x2 = fw * 115;
tcontrol->y2 = fh * 2 + fh / 2;
//tcontrol->OnChange=(FTControlfuncwc *)txtCameraPassword_OnChange;
AddFTControl(twin, tcontrol);


//newline
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "imgCamera00");
tcontrol->type = CTImage;
tcontrol->x1 = fw;
tcontrol->y1 = fh*3;
tcontrol->x2 = fw+ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 3 + ROBOT_CAMERA_HEIGHT; 
//this will malloc the space needed for the image
#if Linux
LoadImageFileToFTControl("images/black_640x480.bmp", tcontrol, FT_SCALE_IMAGE, 0);
//LoadImageFileToFTControl("images/black_256x320.bmp",tcontrol,FT_SCALE_IMAGE,0);
#endif
#if WIN32
LoadImageFileToFTControl("images\\black_640x480.bmp", tcontrol, FT_SCALE_IMAGE, 0);
#endif
//tcontrol->flags |= CGrowX2 | CGrowY2;
AddFTControl(twin, tcontrol);



//newline
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnIdentifyCameras");
tcontrol->type = CTButton;
tcontrol->x1 = fw;
tcontrol->y1 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh;
tcontrol->x2 = fw * 13;
tcontrol->y2 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh * 5;
strcpy(tcontrol->text, "Identify   Cameras");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnIdentifyCameras_Click;
AddFTControl(twin, tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnGetImageFromCamera");
tcontrol->type=CTButton;
tcontrol->x1 = fw*14;
tcontrol->y1 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh;
tcontrol->x2=fw*24;
tcontrol->y2 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh*5;
strcpy(tcontrol->text,"Get Image");
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnGetImageFromCamera_Click;
AddFTControl(twin,tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnGetVideoFromCamera");
tcontrol->type = CTButton;
tcontrol->x1 = fw * 25;
tcontrol->y1 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh;
tcontrol->x2 = fw * 35;
tcontrol->y2 = fh * 3 + ROBOT_CAMERA_HEIGHT + fh * 5;
strcpy(tcontrol->text, "Get Video");
tcontrol->ButtonClick[0] = (FTControlfuncwcxy *)btnGetVideoFromCamera_Click;
AddFTControl(twin, tcontrol);

memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnwinCamerasExit");
tcontrol->type=CTButton;
tcontrol->x1=fw*36;
tcontrol->y1 = fh*3 + ROBOT_CAMERA_HEIGHT + fh;
tcontrol->x2=fw*46;
tcontrol->y2 = fh*3 + ROBOT_CAMERA_HEIGHT + fh*5;
strcpy(tcontrol->text,"Close");  
tcontrol->ButtonClick[0]=(FTControlfuncwcxy *)btnwinCamerasExit_Click;
AddFTControl(twin,tcontrol);



//newline
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnCameraUpArrow");
tcontrol->type = CTButton;
tcontrol->x1 = fw*15 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 3;
tcontrol->x2 = fw * 28 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 6;
//tcontrol->image=FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//tcontrol->image=FTLoadBitmap("images/left_arrow_01.bmp");
strcpy(tcontrol->text,"^");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnCameraUpArrow_Click;
AddFTControl(twin,tcontrol);



memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "btnCameraLeftArrow");
tcontrol->type = CTButton;
tcontrol->x1 = fw*2 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 6;
tcontrol->x2 = fw * 14 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 9;
//tcontrol->image=FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//FTLoadBitmapFileToFTControl(tcontrol,"images/left_arrow_01.bmp",1);
//tcontrol->image=FTLoadBitmap("images/left_arrow_01.bmp");
strcpy(tcontrol->text,"<");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnCameraLeftArrow_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnCameraRightArrow");
tcontrol->type=CTButton;
tcontrol->type = CTButton;
tcontrol->x1 = fw*28 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 6;
tcontrol->x2 = fw*40 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 9;
strcpy(tcontrol->text, ">");
tcontrol->ButtonDownUp[0]=(FTControlfuncwcxy *)btnCameraRightArrow_Click;
AddFTControl(twin,tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"btnCameraDownArrow");
tcontrol->type=CTButton;
tcontrol->x1 = fw * 15 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 9;
tcontrol->x2 = fw * 28 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 12;
strcpy(tcontrol->text, "v");
tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)btnCameraDownArrow_Click;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkCameraShowImage");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw*2 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 13+fh/2;
tcontrol->x2 = fw * 13 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 15;
strcpy(tcontrol->text, "Show image");
tcontrol->value = 1; //checked by default
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
tcontrol->OnChange = (FTControlfuncwc *)chkCameraShowImage_OnChange;
AddFTControl(twin, tcontrol);


memset(tcontrol,0,sizeof(FTControl));
strcpy(tcontrol->name,"chkCameraSaveImage");
tcontrol->type=CTCheckBox;
tcontrol->x1 = fw*2  + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 15+fh/2;
tcontrol->x2 = fw * 13 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 17;
strcpy(tcontrol->text, "Save image to path:");
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
tcontrol->OnChange = (FTControlfuncwc *)chkCameraSaveImage_OnChange;
AddFTControl(twin, tcontrol);

/*
memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblCameraSaveImagePath");
tcontrol->type = CTLabel;
tcontrol->x1 = fw * 15 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 15+fh/2;
tcontrol->y2 = fh * 17;
strcpy(tcontrol->text, "Path:");
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
AddFTControl(twin, tcontrol);
*/

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtCameraSaveImagePath");
tcontrol->type = CTTextBox;
tcontrol->x1 = fw * 25 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 15+fh/2;
tcontrol->x2 = fw * 45 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 17;
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
tcontrol->LostFocus = (FTControlfuncwc *)txtCameraSaveImagePath_LostFocus;
AddFTControl(twin, tcontrol);



memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "chkCameraGetImageOnInterval");
tcontrol->type = CTCheckBox;
tcontrol->x1 = fw*2 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 17 + fh / 2;
tcontrol->x2 = fw * 13 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 19;
strcpy(tcontrol->text, "Get image every:");
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
tcontrol->OnChange = (FTControlfuncwc *)chkCameraGetImageOnInterval_OnChange;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "txtCameraGetImageInterval");
tcontrol->type = CTTextBox;
tcontrol->x1 = fw * 22 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 17 + fh / 2;
tcontrol->x2 = fw * 42 + ROBOT_CAMERA_WIDTH;
tcontrol->y2 = fh * 19;
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
//tcontrol->LostFocus = (FTControlfuncwc *)txtCameraSaveImagePath_LostFocus;
AddFTControl(twin, tcontrol);

memset(tcontrol, 0, sizeof(FTControl));
strcpy(tcontrol->name, "lblCameraGetImageIntervalMS");
tcontrol->type = CTLabel;
tcontrol->x1 = fw * 43 + ROBOT_CAMERA_WIDTH;
tcontrol->y1 = fh * 17+fh/2;
tcontrol->y2 = fh * 19;
strcpy(tcontrol->text, "ms");
//tcontrol->ButtonDownUp[0] = (FTControlfuncwcxy *)chkCameraSaveImage_Click;
AddFTControl(twin, tcontrol);




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


int PopulateCameraDropdowns(void)
{
	FTControl *tcontrol,*tcontrol2;
	FTItem *titem;
	int NumCameras;
	MAC_Connection *lmac,*tmac;

	//Go through each MAC_Connection and get all names that exist for this PCB yet
	//lmac=Get_MAC_Connection_By_WindowFunction((FTControlfunc *)winCameras_AddFTWindow);
	NumCameras = 0;
	tcontrol = GetFTControl("ddCameraName");
	if (tcontrol != 0) {
		titem = (FTItem *)malloc(sizeof(FTItem));
		lmac = RStatus.iMAC_Connection;
		//for all the MAC_Connections, compare the AddWindowFunction (could compare name to Camera too)
		while (lmac != 0) {
			if (lmac != 0) {
				//if (lmac->AddWindowFunction==(FTControlfunc *)winCameras_AddFTWindow) {
				if (lmac->flags&ROBOT_MAC_CONNECTION_IS_CAMERA) { //this connection is a camera
					//strcpy(tcontrol->text,"192.168.1.1");
					memset(titem, 0, sizeof(FTItem));
					sprintf(titem->name, "%s", lmac->cam.SerialNumber);
					//sprintf(titem->name,"%i",lmac->Num);
					//strcpy(titem->name,lmac->Name);
					AddFTItem(tcontrol, titem);
					if (NumCameras == 0) {
						tmac = lmac; //remember the first Camera PCB
					}
					NumCameras++;
				} //if (lmac->AddWindowFunction==(FTControlfunc *)winCameras_AddFTWindow) {
			}
			lmac = lmac->next;
		}//while(lmac!=0) {
		free(titem);
		if (NumCameras>0) {
			SelectFTItemByNum(tcontrol->ilist, 1, 0); //select the first item
			lmac = tmac;
			if (lmac != 0) {
				tcontrol2 = GetFTControl("lblCameraMACStr");
				if (tcontrol2 != 0) {
					sprintf(tcontrol2->text, "MAC: %s", lmac->PhysicalAddressStr);
					DrawFTControl(tcontrol2);
				}

				tcontrol2 = GetFTControl("txtCameraDestIPAddressStr");
				if (tcontrol2 != 0) {
					//strcpy(tcontrol->text,"192.168.1.1");
					strcpy(tcontrol2->text, lmac->DestIPAddressStr);
					DrawFTControl(tcontrol2);
					}

			} //if (lmac != 0) {

		} //if (NumCameras>0) {
	} //tcontrol!=0
	return(NumCameras);
} //int PopulateCameraDropdowns(void)

void winCameras_key(FTWindow *twin,KeySym key)
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
      case XK_Left: //0xff51:  //left arrow key
#if Linux
      case XK_KP_Left: //0xff96:  //key pad left arrow (no num lock)
#endif
				if (twin->FocusFTControl==0) {  //only if no other controls have focus

				} //				if (twin->FocusFTControl==0) {  //only if no other controls have focus
        break;
      case XK_Right: //0xff52:  //right arrow key
#if Linux
      case XK_KP_Right: //0xff98:  //key pad right arrow (no num lock)
#endif
				if (twin->FocusFTControl==0) {  //only if no other controls have focus
				} //if (twin->FocusFTControl==0) {

        break;
      case XK_Down: //0xff54:  //down arrow key
#if Linux
      case XK_KP_Down: //0xff99:  //key pad down arrow (no num lock)
#endif
      case XK_KP_Begin: //0xff9d:  //key pad middle key (no num lock)
				//change duration of Camera to 0 (stop)
				//tc=GetFTControl("ddCameraDuration");
				if (twin->FocusFTControl==0) {  //only if no other controls have focus
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


			//change Camera to # of keypress if no control is selected
			if (twin->FocusFTControl==0) {
			} //if (twin->FocusFTControl==0) {
    break;
		case XK_A: //A key
#if Linux
		case XK_a: //a key
#endif
			//change Camera to left leg Cameras if no control is selected
			if (twin->FocusFTControl==0) {
			} //if (twin->FocusFTControl==0) {
		break;
		case XK_D: //D key
#if Linux
		case XK_d: //d key
#endif
			//change Camera to right leg Cameras if no control is selected
			if (twin->FocusFTControl==0) {
			} //if (twin->FocusFTControl==0) {
		break;

    default:
			//fprintf(stderr,"key=%x\n",key);
    break;
    }
} //void winCameras_key(FTWindow *twin,KeySym key)

int ddCameraName_OnChange(FTWindow *twin,FTControl *tcontrol) {
	MAC_Connection *lmac;
	FTControl *tc;
	
	//fprintf(stderr,"OnChange\n");
	//if the user changed Camera PCBs update the MAC and IP
	//todo: probably make a Get_MAC_Connection_By_Camera_SN()
	lmac=Get_MAC_Connection_By_Name(tcontrol->text); //get remote MAC_Connection
	//MAC_Connection *Get_MAC_Connection_By_Dest_MAC(char *MACStr);
	if (lmac!=0) {
		//MAC
		tc=GetFTControl("lblCameraMACStr");
		if (tc) {
			sprintf(tc->text,"MAC: %s",lmac->PhysicalAddressStr);
			DrawFTControl(tc);
		} //tc
		//IP
		tc=GetFTControl("txtCameraDestIPAddressStr");
		if (tc) {
			sprintf(tc->text,"%s",lmac->DestIPAddressStr);
			DrawFTControl(tc);
		} //tc
		return(1);
	} //if (lmac==0) {
	return(0);
} //int ddCameraName_OnChange(FTWindow *twin,FTControl *tcontrol) {


//Close Robot Cameras window
int btnwinCamerasExit_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{

	CloseFTWindow(twin);

	return(1);
} //int btnwinCamerasExit_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

int winCameras_OnOpen(FTWindow *twin)
{

	return(1);
} //int winCameras_OnOpen(FTWindow *twin)

int winCameras_OnClose(FTWindow *twin)
{
//	DisconnectFromRobot();

	return(1);
} //int winCameras_OnClose(FTWindow *twin)

//Identify any cameras using a broadcast UDP packet
int btnIdentifyCameras_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	char Inst[512];
	MAC_Connection *wmac, *lmacid;
	int DataLen;
	//640=0x280  480=0x1e0

	//send broadcast to identify all cameras, and create a new MAC_Connection for all new cameras
	
	//we could use a single listen function (and source port) for all robot traffic, or
	//a separate listening function (and port) for getting images from cameras (the other for motors, accelerometers, and touch sensors)
	//because the camera requires a specific dest port (32108)
	//currently this is done over ethernet using a network socket and UDP
	//get the local port used at the start of the program and used by the listening function
	//we need to use the same socket as the MAC_Connection connected to the listening function
	if (RStatus.WiredInterface != 0) {

		wmac = RStatus.WiredInterface;

		/*
		lmac = Create_Remote_MAC_Connection(wmac->SrcIPAddressStr, wmac->SrcPort, "255.255.255.255", 8600); //8600 is asterix surveillance data
		lmac->flags |= ROBOT_MAC_CONNECTION_IS_CAMERA;
		//for now do connect, etc. here
		//ip cam responds to 255.255.255.255 "{"command": "broadcast"}"
		//send broadcast "f1 30 00 00"
		//44 48 01 01 to 255.255.255.255 apparently wakes up camera which replies to 255.255.255.255 with info
		Inst[0] = 0x44;
		Inst[1] = 0x48;
		Inst[2] = 0x01;
		Inst[3] = 0x01;
		DataLen = 4;
		//sprintf(Inst, "{\"command\": \"broadcast\"}");
		//DataLen = strlen(Inst);
		SendDataToMAC(lmac, Inst, DataLen);
		*/

		//todo: check for an existing MAC_Connection and reuse/change dest ip and port
		
		lmacid = Get_MAC_Connection_By_IPs(wmac->SrcIPAddressStr, wmac->SrcPort, "128.200.102.255", 32108);
		if (lmacid == 0) {
			lmacid = Create_Remote_MAC_Connection(wmac->SrcIPAddressStr, wmac->SrcPort, "128.200.102.255", 32108);
		}

		//port 32108 is the port used to initiate contact with the camera
		//this will get the camera IP and establish the src and dest ports for getting images
		DataLen = 4;	
		Inst[0] = 0xf1;
		Inst[1] = 0x30;
		Inst[2] = 0x0;
		Inst[3] = 0x0;
		SendDataToMAC(lmacid, Inst, DataLen);

		//any new cameras will respond with (for example):
		//recv "f1 41 00 14 4e 49 50 00 00 00 00 00 00 00 36 33 56 59 54 46 57 00 00 00" (4+20 bytes) 20 bytes= NIP....... 63VYTFW... (last 5 of camera ID VYTFW)

		//Thread_ListenToRobot will receive these return packets and create a new connection for any unknown camera IP+Port

		//then an image can be obtained by selecting the camera and clicking "Get Image"


		//other communications:
		//send (this time to IP): same above 24 bytes

		//recv same 24 bytes but "f1 42..." (camera keeps sending until response)

		//send "f1 e0 00 00"


		//optional? - send "f1 d0 00 5b d1 00 00 00 01 0a 00 00 4f 00 00 00" (last 4= length of text) + "GET /check_user.cgi?loginuse=admin&loginpas=isnozeus0&user=admin&pwd=isnozeus0&"


	} //if (RStatus.WiredInterface != 0) {
	return(1);
} //int btnIdentifyCameras_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

//Get an image from a camera
int btnGetImageFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	MAC_Connection *lmac;
	FTControl *tc;
	char tstr[512];
	int timeout;
	//640=0x280  480=0x1e0

	//get name+ip of camera, create a new connection, or change any existing connection to port 32108 (to re-establish contact), 
	//set username+pw in mac->cam struct
	lmac = InitiateCameraConnection(); 


	if (lmac != 0) {
		lmac->cam.flags |= ROBOT_CAMERA_INFO_GET_IMAGE;
		lmac->cam.StartImagePointer = lmac->cam.EndImagePointer; //to stop partial image
		//update the show and save image flags and save file path
		UpdateShowImageFlag(lmac);
		UpdateSaveImageFlag(lmac);
		UpdateSaveImagePath(lmac);
		//if the communcation was kept active with e0-e1
		//we need to send a check_user.cgi to start getting the image
		if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {
			StartCameraConnection(lmac); //sends the necessary UDP packet to start image getting
			//then we will need a check_user
			//but we need to wait until the camera responds with which port number to use for the session
			timeout = 0;
			while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) && timeout<ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
				usleep(10000); //sleep 10 ms
#endif
#if WIN32
				Sleep(10); //sleep 10 ms
#endif
				timeout+=10;
			} //while
			if (timeout >= ROBOT_CAMERA_NETWORK_TIMEOUT) {
				lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
				return(0); //retry? or message?
			}
		} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {

		//reauth anyway for now
		//lmac->cam.flags&=~ROBOT_CAMERA_INFO_AUTHENTICATED;
		if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
			sprintf(tstr, "GET /check_user.cgi?loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
			SendCameraGET(lmac, tstr);
			timeout = 0;
			while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED) && timeout<ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
				usleep(10000); //sleep 10 ms
#endif
#if WIN32
				Sleep(10); //sleep 10 ms
#endif
				timeout+=10;
			} //while
			if (timeout >= ROBOT_CAMERA_NETWORK_TIMEOUT) {
				lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
				return(0); //retry? or message?
			}

		} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {

#if 0 
		sprintf(tstr, "GET /livestream.cgi?streamid=10&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);


		//set resolution to 1280x720
		sprintf(tstr, "GET /camera_control?param=0&value=3&loginuse=%s&loginpas=%s&user=%s&pwd=%s", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);
#if Linux
		usleep(100000); //sleep 100 ms
#endif
#if WIN32
		Sleep(100); //sleep 100 ms
#endif

		sprintf(tstr, "GET /camera_control?param=3&value=1&loginuse=%s&loginpas=%s&user=%s&pwd=%s", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);
#if Linux
		usleep(100000); //sleep 100 ms
#endif
#if WIN32
		Sleep(100); //sleep 100 ms
#endif


		sprintf(tstr, "GET /camera_control?param=6&value=5&loginuse=%s&loginpas=%s&user=%s&pwd=%s", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);
#if Linux
		usleep(100000); //sleep 100 ms
#endif
#if WIN32
		Sleep(100); //sleep 100 ms
#endif
#endif
		//sprintf(tstr, "GET /livestream.cgi?streamid=10&rate=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		//sprintf(tstr, "GET /snapshot.cgi?&user=%s&pwd=%s&count=0", lmac->cam.Username, lmac->cam.Password);
		//sprintf(tstr, "GET /snapshot.cgi?count=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		sprintf(tstr, "GET /livestream.cgi?streamid=10&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		//sprintf(tstr, "GET /livestream.cgi?streamid=16&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);

	} //	if (lmac != 0) {

	return(1);

} //int btnGetImageFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

// Get a video (stream jpg images) from a camera
int btnGetVideoFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	MAC_Connection *lmac;
	FTControl *tc;
	char tstr[512];
	int timeout;

	//640=0x280  480=0x1e0


	//get video or stop video
	if (strcmp(tcontrol->text, "Get Video")) {
		//stop video
		StopCameraVideo();

		strcpy(tcontrol->text, "Get Video");
		DrawFTControl(tcontrol);
	}
	else {
		strcpy(tcontrol->text, "Stop Video");
		DrawFTControl(tcontrol);

		//get name+ip of camera, create a new connection, or change any existing connection to port 32108 (to re-establish contact), 
		//set username+pw in mac->cam struct
		lmac = InitiateCameraConnection();

		if (lmac != 0) {

			lmac->cam.flags |= ROBOT_CAMERA_INFO_GET_VIDEO;

			lmac->cam.StartImagePointer = lmac->cam.EndImagePointer; //to stop partial image
			//update the show and save image flags and save file path
			UpdateShowImageFlag(lmac);
			UpdateSaveImageFlag(lmac);
			UpdateSaveImagePath(lmac);
			//if the communcation was kept active with e0-e1
			//we need to send a check_user.cgi to start getting the image
			if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {

				StartCameraConnection(lmac); //sends the necessary UDP packet to start image getting
				//then we will need a check_user
				//but we need to wait until the camera responds with which port number to use for the session
				timeout = 0;
				while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) && timeout < ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
					usleep(1000); //sleep 1 ms
#endif
#if WIN32
					Sleep(1); //sleep 1 ms
#endif
					timeout++;
				} //while
				if (timeout == ROBOT_CAMERA_NETWORK_TIMEOUT) {
					lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
					return(0); //retry? or message?
				}

			} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {

			//always reauth for now
			//lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_CONNECTED|ROBOT_CAMERA_INFO_AUTHENTICATED);
			if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {

				//sprintf(tstr, "GET /check_user.cgi?loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
				sprintf(tstr, "GET /livestream.cgi?streamid=10&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);

				SendCameraGET(lmac, tstr);
				timeout = 0;
				while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED) && timeout < ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
					usleep(1000); //sleep 1 ms
#endif
#if WIN32
					Sleep(1); //sleep 1 ms
#endif
					timeout++;
				} //while
				if (timeout == ROBOT_CAMERA_NETWORK_TIMEOUT) {
					lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
					return(0); //retry? or message?
				}

			} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {

			sprintf(tstr, "GET /livestream.cgi?streamid=10&rate=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
			SendCameraGET(lmac, tstr);

		} //	if (lmac != 0) {
	} //if (strcmp(tcontrol->text, "Get Video")) {
	return(1);
} //int btnGetVideoFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

//currently this only moves the current camera motors
int MoveCameraMotor(int Inst1, int Inst2, int Delay)
{
	MAC_Connection *lmac;
	char tstr[1024];
	int timeout;

	//get name+ip of camera, create a new connection, or change any existing connection to port 32108 (to re-establish contact), 
	//set username+pw in mac->cam struct
	lmac = InitiateCameraConnection();

	if (lmac != 0) {
		//if the communcation was kept active with e0-e1
		//we need to send a check_user.cgi to start getting the image
		if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {
			StartCameraConnection(lmac); //sends the necessary UDP packet to start image getting
			//then we will need a check_user
			//but we need to wait until the camera responds with which port number to use for the session
			timeout = 0;
			while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) && timeout<ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
				usleep(1000); //sleep 1 ms
#endif
#if WIN32
				Sleep(1); //sleep 1 ms
#endif
				timeout++;
			} //while
			if (timeout == ROBOT_CAMERA_NETWORK_TIMEOUT) {
				lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
				return(0); //retry? or message?
			}

		} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {

		if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
				sprintf(tstr, "GET /check_user.cgi?loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
				SendCameraGET(lmac, tstr);
				timeout = 0;
				while (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED) && timeout<ROBOT_CAMERA_NETWORK_TIMEOUT) {
#if Linux
					usleep(1000); //sleep 1 ms
#endif
#if WIN32
					Sleep(1); //sleep 1 ms
#endif
					timeout++;
				} //while
				if (timeout == ROBOT_CAMERA_NETWORK_TIMEOUT) {
					lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
					return(0); //retry? or message?
				}

		} //if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {

		//onestep=1 moves the camera continuously until stopped
		sprintf(tstr, "GET /decoder_control.cgi?command=%d&onestep=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", Inst1, lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);
#if Linux
				usleep(Delay*1000); //sleep Delay ms
#endif
#if WIN32
				Sleep(Delay); //sleep Delay ms
#endif
		//command=1 is stop
		sprintf(tstr, "GET /decoder_control.cgi?command=%d&onestep=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", Inst2, lmac->cam.Username, lmac->cam.Password, lmac->cam.Username, lmac->cam.Password);
		SendCameraGET(lmac, tstr);

	} //	if (lmac != 0) {



	return(1);
} //int MoveCameraMotor( int Inst1, int Inst2,int Delay)

MAC_Connection *InitiateCameraConnection(void)
{
	MAC_Connection *lmac,  *wmac;
	FTControl *tc,*tcip,*tcuser,*tcpw;

	//we could use a single listen function (and source port) for all robot traffic, or
	//a separate listening function (and port) for getting images from cameras (the other for motors, accelerometers, and touch sensors)
	//because the camera requires a specific dest port (32108)
	//currently this is done over ethernet using a network socket and UDP
	//get the local port used at the start of the program and used by the listening function
	//we need to use the same socket as the MAC_Connection connected to the listening function
	if (RStatus.WiredInterface != 0) {

		wmac = RStatus.WiredInterface;

		//get ip of selected camera (is probably not connected, but was identified earlier)
		tc = GetFTControl("ddCameraName");
		if (tc == 0) {
			return(0);
		}
		if (strlen(tc->text) == 0) {
			FTMessageBox("No camera identified\n", FTMB_OK, "Robot - error", 0);
			return(0);
		}

		tcip = GetFTControl("txtCameraDestIPAddressStr");
		if (tcip != 0) {
			tcuser = GetFTControl("txtCameraUsername");
			if (tcuser != 0) {
				if (strlen(tcuser->text) == 0) {
					FTMessageBox("Enter a username\n", FTMB_OK, "Robot - error", 0);
					return(0);
				}//if (strlen(tc->text) == 0) {
			}
			else {
				return(0);

			}//if (tcuser != 0) {

			tcpw = GetFTControl("txtCameraPassword");
			if (tcpw != 0) {
				if (strlen(tcpw->text) == 0) {
					FTMessageBox("Enter a password\n", FTMB_OK, "Robot - error", 0);
					return(0);
				}//if (strlen(tcpw->text) == 0) {
			}
			else {
				return(0);
			}//if (tcpw!=0) {

		} //if (tcip!=0) {


		//check for any existing MAC_Connections and see if already connected
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			//Delete_MAC_Connection(lmac);
			if (lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) {
				//fprintf(stderr, "called InitiateCameraConnection() with camera already connected.\n");
				//the connection may still need the userid and pw since that is not needed to make initial contact with the camera
				if (!(lmac->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
					strcpy(lmac->cam.Username, tcuser->text);
					strcpy(lmac->cam.Password, tcpw->text);
					//calling function will need to check_user.cgi again to authenticate
				} //else already authenticated
				return(lmac);
			}

		}
		else {
			//the first time we are trying to connect to this camera- currently can't happen
			lmac = Create_Remote_MAC_Connection(wmac->SrcIPAddressStr, wmac->SrcPort, tcip->text, 32108);
		} //if (lmac != 0) {



		//camera is not connected (an f1f0 occurred closing the session)
		//we need to re-contact the camera using port 32108

		//change ip (in case the last contact was from .255)- but that is done in the Listen function anyway
		strcpy(lmac->DestIPAddressStr, tcip->text);
		//change the port on this connection
		lmac->DestPort = 32108;
		//lmac->DestAddress = *(struct sockaddr_in *)&RemoteAddr; //this updates port too?
		lmac->DestAddress.sin_addr.s_addr = inet_addr(lmac->DestIPAddressStr);
		lmac->DestAddress.sin_port = htons(lmac->DestPort);

		strcpy(lmac->cam.Username, tcuser->text);
		strcpy(lmac->cam.Password, tcpw->text);

		//start camera connection? otherwise functions need to check to see if camera is connected
		return(lmac);

	} //if (RStatus.WiredInterface != 0) {
	return(0);
} //	MAC_Connection *InitiateCameraConnection(void)

int StartCameraConnection(MAC_Connection *lmac)
{
	char Inst[5];

	//try to initiate contact again (this time not using the .255 broadcast address, but specifically sending to the IP obtained earlier)
	Inst[0] = 0xf1;
	Inst[1] = 0x30;
	Inst[2] = 0x0;
	Inst[3] = 0x0;
	SendDataToMAC(lmac, Inst, 4);
	return(1);
}

int StopCameraVideo(void)
{
	char Inst[10];
	MAC_Connection *lmac;
	FTControl *tc;

	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(0);
	}
	if (strlen(tc->text) == 0) {
		FTMessageBox("No camera identified\n", FTMB_OK, "Robot - error", 0);
		return(0);
	}

	//check for any existing MAC_Connections and see if already connected
	lmac = Get_MAC_Connection_By_Name(tc->text);
	if (lmac != 0) {

		//for now there is no better way to end a session that had a problem
		Inst[0] = 0xf1;
		Inst[1] = 0xf0;
		Inst[2] = 0;
		Inst[3] = 0;
		SendDataToMAC(lmac, Inst,4);
		lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED);
	}

	return(1);
} //int StopCameraVideo(void)

int btnCameraUpArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	MoveCameraMotor(0, 1, 100); //inst=0 inst1 100ms delay between instructions

	return(1);
} //int btnCameraUpArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

int btnCameraLeftArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	MoveCameraMotor(4, 5, 100); //inst=4 inst5 100ms delay between instructions

	return(1);
} //int btnCameraLeftArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

int btnCameraDownArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{
	MoveCameraMotor(2, 3, 100); //inst=2 inst3 100ms delay between instructions
	return(1);
} //int btnCameraDownArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

int btnCameraRightArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)
{

	MoveCameraMotor(6, 7, 100); //inst=6 inst7 100ms delay between instructions

	return(1);
} //int btnCameraRightArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y)

int chkCameraShowImage_OnChange(FTWindow *twin, FTControl *tcontrol)
{
	MAC_Connection *lmac;
	FTControl *tc;

	//find the connection to the current camera if any
	//get ip of selected camera (is probably not connected, but was identified earlier)
	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(1);
	}
	if (strlen(tc->text) != 0){
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			if (tcontrol->value) {
				lmac->cam.flags |= ROBOT_CAMERA_INFO_SHOW_IMAGE;
			}
			else {
				lmac->cam.flags &= ~ROBOT_CAMERA_INFO_SHOW_IMAGE;
			}
		}
	}

	return(1);
} //int chkCameraShowImage_OnChange(FTWindow *twin, FTControl *tcontrol)


//needed because camera mac might not be available when this checkbox is checked
int UpdateShowImageFlag(MAC_Connection *lmac)
{
	FTControl *tc;

	if (lmac == 0) {
		return(0);
	}
	tc = GetFTControl("chkCameraShowImage");
	if (tc == 0) {
		return(0);
	}
	if (tc->value) {
		lmac->cam.flags |= ROBOT_CAMERA_INFO_SHOW_IMAGE;
	}
	else {
		lmac->cam.flags &= ~ROBOT_CAMERA_INFO_SHOW_IMAGE;
	}
	return(1);
} //int UpdateShowImageFlag(MAC_Connection *lmac)

int chkCameraSaveImage_OnChange(FTWindow *twin, FTControl *tcontrol)
{
	MAC_Connection *lmac;
	FTControl *tc;

	//find the connection to the current camera if any
	//get ip of selected camera (is probably not connected, but was identified earlier)
	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(1);
	}
	if (strlen(tc->text) != 0){
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			if (tcontrol->value) {
				lmac->cam.flags |= ROBOT_CAMERA_INFO_SAVE_IMAGE;
			}
			else {
				lmac->cam.flags &= ~ROBOT_CAMERA_INFO_SAVE_IMAGE;
			}
		}
	}

	return(1);
} //int chkCameraSaveImage_OnChange(FTWindow *twin, FTControl *tcontrol)


int UpdateSaveImageFlag(MAC_Connection *lmac)
{
	FTControl *tc;

	if (lmac == 0) {
		return(0);
	}
	tc = GetFTControl("chkCameraSaveImage");
	if (tc == 0) {
		return(0);
	}
	if (tc->value) {
		lmac->cam.flags |= ROBOT_CAMERA_INFO_SAVE_IMAGE;
	}
	else {
		lmac->cam.flags &= ~ROBOT_CAMERA_INFO_SAVE_IMAGE;
	}
	return(1);
} //int UpdateSaveImageFlag(MAC_Connection *lmac)


int txtCameraSaveImagePath_LostFocus(FTWindow *twin, FTControl *tcontrol)
{
	MAC_Connection *lmac;
	FTControl *tc;

	//find the connection to the current camera if any
	//get ip of selected camera (is probably not connected, but was identified earlier)
	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(1);
	}
	if (strlen(tc->text) != 0){
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			UpdateSaveImagePath(lmac);
//			strcpy(lmac->cam.FilePath, tc->text);
		}
	}

	return(1);
} //int txtCameraSaveImagePath_LostFocus(FTWindow *twin, FTControl *tcontrol)


int UpdateSaveImagePath(MAC_Connection *lmac)
{
	FTControl *tc;
	char tstr[512];
	int len,i,j;

	if (lmac == 0) {
		return(0);
	}
	tc = GetFTControl("txtCameraSaveImagePath");
	if (tc == 0) {
		return(0);
	}
	len = strlen(tc->text);
	if (len > 0) {
#if Linux
		if (tc->text[len - 1] != 0x2f) {
			sprintf(lmac->cam.FilePath, "%s/", tc->text); //add last slash
		}
		else {
			strcpy(lmac->cam.FilePath, tc->text); //already has /
		}
#endif
#if WIN32
		//double all back slashes
		i = 0;
		j = 0;
		while(i<len) {
			if (tc->text[i] == 0x5c) {  //found backslash
				tstr[j] = 0x5c;
				tstr[j + 1] = 0x5c;
				if (i==len-1 || tc->text[i + 1] != 0x5c) {
					i++; //skip 1 backslash
				}
				else {
					i += 2; //skip 2 backslashes
				}
				j += 2;
			}//if (tc->text[i] == 0x5c) {
			else {
				//not backslash
				tstr[j] = tc->text[i];
				i++;
				j++;
				if (i == len) { //last character
					//add ending backslashes because last character was not back slash
					tstr[j] = 0x5c;
					tstr[j + 1] = 0x5c;
					j += 2;
				} //if (i == len) {
			}//if (tc->text[i] == 0x5c) {

		} //while
		
		tstr[j] = 0; //terminate string
		strcpy(lmac->cam.FilePath, tstr); 

#endif
		sprintf(tstr, tc->text);
	}
	else {
		strcpy(lmac->cam.FilePath, tc->text); //empty path
	}
	

	return(1);
} //int UpdateSaveImageFlag(MAC_Connection *lmac)



//enable/disable save (and possibly draw, ftp) image at regular interval
int chkCameraGetImageOnInterval_OnChange(FTWindow *twin, FTControl *tcontrol)
{
	MAC_Connection *lmac;
	FTControl *tc,*tcinterval;

	//find the connection to the current camera if any
	//get ip of selected camera (is probably not connected, but was identified earlier)
	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(1);
	}
	if (strlen(tc->text) != 0){
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			if (tcontrol->value) {
				lmac->cam.flags |= ROBOT_CAMERA_INFO_GET_IMAGE_PERIODICALLY;
				//store timer value (in ms)
				tcinterval = GetFTControl("txtCameraGetImageInterval");
				if (tcinterval != 0) {
					if (strlen(tcinterval->text) > 0) {
						tcontrol->timer = atoi(tcinterval->text);
						tcontrol->OnTimer = GetImageTimerFunc;
					}
					else {
						return(0);
					} //if (strlen(tcinterval->text) > 0) {
				}
				else {
					return(0);
				}
				
			}
			else {
				lmac->cam.flags &= ~ROBOT_CAMERA_INFO_GET_IMAGE_PERIODICALLY;
				tcontrol->OnTimer = 0;
				tcontrol->timer = 0;
			}
		} //if (lmac != 0) {
		else {
			//else no connection- for now just reset control to unchecked
			tc->value = 0;
		} //if (lmac != 0) {
	}

	return(1);
} //int chkCameraGetImageOnInterval_OnChange(FTWindow *twin, FTControl *tcontrol)

int GetImageTimerFunc(FTWindow *twin, FTControl *tcontrol)
{
	//Get Image from camera
	btnGetImageFromCamera_Click(twin,tcontrol,0,0);
	return(1);
}

int KeepConnectionAlive(FTWindow *twin, FTControl *tcontrol)
{
	MAC_Connection *lmac;
	FTControl *tc, *tcinterval;
	char Inst[4];

	//find the connection to the current camera if any
	//get ip of selected camera (is probably not connected, but was identified earlier)
	tc = GetFTControl("ddCameraName");
	if (tc == 0) {
		return(1);
	}
	if (strlen(tc->text) != 0){
		lmac = Get_MAC_Connection_By_Name(tc->text);
		if (lmac != 0) {
			if (lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) {
				if (lmac->cam.LastContact < GetTimeInMS() - 200) {
					//send f1e0
					Inst[0] = 0xf1;
					Inst[1] = 0xe0;
					Inst[2] = 0;
					Inst[3] = 0;
					SendDataToMAC(lmac, Inst, 4);
				} //if (lmac->cam.LastContact < GetTimeInMS() - 200) {
				if (lmac->cam.LastContact < GetTimeInMS() - 1000) {
					//haven't heard from camera in 1 second, disconnect
					//send f1f0?
					Inst[0] = 0xf1;
					Inst[1] = 0xf0;
					Inst[2] = 0;
					Inst[3] = 0;
					SendDataToMAC(lmac, Inst, 4);
					lmac->cam.flags &= ~(ROBOT_CAMERA_INFO_CONNECTED | ROBOT_CAMERA_INFO_AUTHENTICATED);
				} //if (lmac->cam.LastContact < GetTimeInMS() - 1000) {
			} //if (lmac->cam.flags&ROBOT_CAMERA_INFO_CONNECTED) {
		} //if (lmac != 0) {
	} //if (strlen(tc->text) != 0){
	
	return(1);
} //int KeepConnectionAlive(FTWindow *twin, FTControl *tcontrol)
