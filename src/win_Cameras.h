//win_Cameras.h
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
#ifndef _WIN_CAMERAS_H
#define _WIN_CAMERAS_H
int winCameras_AddFTWindow(void);
int winCameras_AddFTControls(FTWindow *twin);
int PopulateCameraDropdowns(void);
void winCameras_key(FTWindow *twin,KeySym key);
int ddCameraName_OnChange(FTWindow *twin,FTControl *tcontrol);
//void btnConnectToCamera_Click(FTWindow *twin,FTControl *tcontrol,int x,int y);
int btnwinCamerasExit_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int winCameras_OnOpen(FTWindow *twin);
int winCameras_OnClose(FTWindow *twin);
int btnIdentifyCameras_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnGetImageFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnGetVideoFromCamera_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
MAC_Connection *InitiateCameraConnection(void);
int StartCameraConnection(MAC_Connection *lmac);
int StopCameraVideo(void);
int btnCameraUpArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnCameraLeftArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnCameraDownArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int btnCameraRightArrow_Click(FTWindow *twin, FTControl *tcontrol, int x, int y);
int MoveCameraMotor(int Inst1, int Inst2, int Delay);
int chkCameraShowImage_OnChange(FTWindow *twin, FTControl *tcontrol);
int UpdateShowImageFlag(MAC_Connection *lmac);
int chkCameraSaveImage_OnChange(FTWindow *twin, FTControl *tcontrol);
int UpdateSaveImageFlag(MAC_Connection *lmac);
int txtCameraSaveImagePath_LostFocus(FTWindow *twin, FTControl *tcontrol);
int UpdateSaveImagePath(MAC_Connection *lmac);
int chkCameraGetImageOnInterval_OnChange(FTWindow *twin, FTControl *tcontrol);
int GetImageTimerFunc(FTWindow *twin, FTControl *tcontrol);
int KeepConnectionAlive(FTWindow *twin, FTControl *tcontrol);
//camera instructions:

//establish connection with camera:
//"GET /check_user.cgi?loginuse=admin&loginpas=password&user=admin&pwd=password&"

//make camera send single image
//?

//make camera send streaming images
//"GET /livestream.cgi?streamid=10&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//move camera motors - 
//before get:
//f1 d0 00 75  //STUN channel number f1d0 length 0x75 (117)
//varies:
//(2) d1 00 - some kind of instruction
//(2) GET instruction number, increments for each new GET sent (big endian)
//(4) 01 0a 00 00 (unknown)
//(4) length of data-12 byte header, in little endian? 0x75-0x69=0xc
//d1 00 00 20 01 0a 00 00 68 00 00 00
//d1 00 00 21 01 0a 00 00 68 00 00 00
//d1 00 00 22 01 0a 00 00 69 00 00 00



//two commands are required to move the camera one step
//camera responds with:
//f1 d1 00 06
//d1 00 (?)
//00 01 (received ok?)
//00 2d (GET instruction number being confirmed)
//up
//"GET /decoder_control.cgi?command=0&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=1&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//up+right
//"GET /decoder_control.cgi?command=91&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=1&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//right
//"GET /decoder_control.cgi?command=6&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=7&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//right+down
//"GET /decoder_control.cgi?command=93&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=1&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//down
//"GET /decoder_control.cgi?command=2&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=3&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//down+left
//"GET /decoder_control.cgi?command=92&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=1&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//left
//"GET /decoder_control.cgi?command=4&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=5&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//left+up
//"GET /decoder_control.cgi?command=90&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"
//"GET /decoder_control.cgi?command=1&onestep=0&loginuse=admin&loginpas=password&user=admin&pwd=password&"

//get camera parameters: 
//"GET /get_params.cgi?loginuse=admin&loginpas=password&user=admin&pwd=password&
//returns result=0 and a variable=value, variable list in text


//param 0 = resolution - should be sent only after getting the end of a jpg image
//"GET /camera_control.cgi?param=0&value=1&loginuse=..."
//value=1:320x240 2:640x480 3:1280x720

//"GET /camera_control.cgi?param=6&value=30&loginuse=..."

//get audio 
//"GET /audiostream.cgi?streamid=16&loginuse=..."
//is followed by
//GET /livestream.cgi?streamid=16&loginuse..."



//other STUN commands:
//f1 e0 00 00 (ping?)
//f1 e1 00 00 (confirm f1 e0)
//f1 f0 00 00 -end conenction/stop streaming images (confirmed with f1 f0 +18 00s)

#endif //_WIN_CAMERAS_H