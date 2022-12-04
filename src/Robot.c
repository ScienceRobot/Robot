// Robot.c Main Robot code
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

#if WIN32
#include "stdafx.h"
#endif

#include "freethought.h"
#include "Robot.h"
#include "win_Robot.h"
#include "win_Motors.h"
#include "win_Accels.h"
#include "win_AnalogSensors.h"
#include "win_RobotModel.h"
#include "win_Cameras.h"
#include "win_Power.h"
#include "win_GPS.h"
#include "win_NewEthPCB.h"
#include "robot_accelmagtouchgps_mcu_instructions.h"
#include "robot_motor_mcu_instructions.h"

#if Linux
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>  //sys/socket.h does not have SO_REUSEADDR
#include <sys/select.h>
#include <sys/stat.h>  //for stat() to determine if folder/file exists
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h> //for gethostbyname
#endif

#include "time.h"  //for time_t and struct tm, time() and localtime(), for year, mon, day, hour, etc.
#if Linux
#include "sys/time.h"  //for timeval, gettimeofday() detailed time (us)
#endif
#include <string.h> //for strcmp
#include "Free3D.h" //for POINT3D

#if USE_RTAI
#include <rtai_lxrt.h>
#endif //USE_RTAI




//Global variables
extern ProgramStatus PStatus;
extern F3D_Status F3DStatus;
extern RobotModelInfo RMInfo; //Robot Model Info
#if 0  
//from RobotConfig.c:
extern const char *ROBOT_PCB_NAMES[NUM_ROBOT_PCB_NAMES];
extern const char *ROBOT_MOTOR_NAMES[NUM_MOTOR_NAMES];
extern const char *ROBOT_MOTOR_NAMES_PCB[NUM_MOTOR_NAMES];
extern const char *ROBOT_MOTOR_READABLE_NAMES[NUM_MOTOR_NAMES];
extern const char *ROBOT_PCB_ETHACCEL_NAMES[NUM_ETHACCEL_PCBS];
extern const char *ROBOT_ACCEL_NAMES[NUM_ACCEL_NAMES];
extern const char *ROBOT_ACCEL_NAMES_PCB[NUM_ACCEL_NAMES];
extern const char *ROBOT_TOUCH_SENSOR_NAMES[NUM_TOUCH_SENSOR_NAMES];
extern const char *ROBOT_TOUCH_SENSOR_NAMES_PCB[NUM_TOUCH_SENSOR_NAMES];
//end from RobotConfig.c
#endif
RobotStatus RStatus;


#define LOWPASS_FACTOR 0.3  //low pass filter factor for current accelerometer sample
#define LOWPASS_ONE_MINUS_FACTOR 0.7  //last sample gets*ONE_MINUS one minus low pass filter factor for accelerometer samples
//CalibrationFile CaliFile[NUM_ETHACCEL_PCBS]; //calibration data read from accel.conf, gyro.conf, and/or mag.conf (move into RStatus?)


//run the Free thought test program
#if Linux
int main(int argc, char *argv[])  //argument count and values
#endif
#if WIN32
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
#endif
{ 
	int recog;
	MAC_Connection *lmac;   
	uint32_t Rflags; //to preserve robot flags
	FTWindow *twin;
#if WIN32
#define MAX_ARGS 10
	int argc;
	char *argv[MAX_ARGS];
	char *token;
	HWND hWin;
#endif //WIN32
#if USE_RTAI
	RT_TASK *Main_RT_Task;
	static RTIME sampling_interval;
	int hard_timer_running;
	char filename[256];
#endif


#if WIN32
	//convert lpCmdLine to argv
	argc = 0;
	token = strtok((char *)lpCmdLine,(char *)_T(" "));

	while (token)
	{
	argv[argc++] = token;
	token = strtok((char *)NULL,(char *)_T(" "));
	}
#endif

	memset(&PStatus,0,sizeof(ProgramStatus));

#if WIN32
	PStatus.hInstance=hInstance;
#endif

	recog=0;
	if (argc>1) {
	//todo get dash or double dash and then switch on all letter options

	//look for debug flag
#if Linux
		if (strcasecmp(argv[1],"-h")==0 || strcasecmp(argv[1],"--help")==0) {
#endif
#if WIN32
		if (_stricmp(argv[1],"-h")==0 || _stricmp(argv[1],"--help")==0) {
#endif
		  printhelp(); //print FreeThought help for now
		  return 0;
		}
#if Linux
		if (strcasecmp(argv[1],"-i")==0 || strcasecmp(argv[1],"--info")==0) {
#endif
#if WIN32
		if (_stricmp(argv[1],"-i")==0 || _stricmp(argv[1],"--info")==0) {
#endif
	//    PStatus.flags|=PInfo;
			RStatus.flags|=ROBOT_STATUS_INFO;
		  fprintf(stderr,"\nShow Robot Info Mode on\n");
		  recog=1;
		} 
#if Linux
		if (strcasecmp(argv[1],"-fi")==0 || strcasecmp(argv[1],"--finfo")==0) {
#endif
#if WIN32
		if (_stricmp(argv[1],"-fi")==0 || _stricmp(argv[1],"--finfo")==0) {
#endif
		  PStatus.flags|=PInfo;
		  fprintf(stderr,"\nShow FreeThought Info Mode on\n");
		  recog=1;
		} 

#if Linux
		if (strcasecmp(argv[1],"-r")==0 || strcasecmp(argv[1],"--remote")==0) {
#endif
#if WIN32
		if (_stricmp(argv[1],"-r")==0 || _stricmp(argv[1],"--remote")==0) {
#endif
			//PStatus.flags|=PInfo;
			RStatus.flags|=ROBOT_STATUS_REMOTE;
			fprintf(stderr,"Controlling robot remotely\n");
			recog=1;
		} 


		if (recog==0) {
		  fprintf(stderr,"\nUnrecognized argument: %s\n",argv[1]);
		  printhelp();
		  return 0;
		}
	} //if (argc>1) {





	getcwd(PStatus.ProjectPath,FTMedStr);
	strcat(PStatus.ProjectPath,"/");
	fprintf(stderr,"ProjectPath=%s\n",PStatus.ProjectPath);



	//maybe should just call OpenWindow("main")
	PStatus.FirstWindow=(FTControlfunc *)winRobot_AddFTWindow;



	//fprintf(stderr,"after firstwin\n");
	InitFreeThought();


#if USE_RTAI
	//initialize RTAI

	//load modules
	//sprintf(filename, "%sload_rt", PStatus.ProjectPath);
	//system(filename);

	if (!(Main_RT_Task = rt_task_init_schmod(nam2num("MNTSK"), 0, 0, 0, SCHED_FIFO, 0xF))) {
		printf("Error: RTAI cannot init main task\n");
	}
	if ((hard_timer_running = rt_is_hard_timer_running())) 
	{
	//printf("Skip hard real_timer setting...\n");
	//sampling_interval = nano2count(TICK_TIME);
	}
	else 
	{
	printf("Starting real time timer...\n");
	rt_set_oneshot_mode();
	start_rt_timer(0);
	}
	//sampling_interval = nano2count(TICK_TIME);
#endif //USE_RTAI


	Rflags=RStatus.flags;
	InitRobot();  //needed because clears RStatus- and some args set RStatus vars
	RStatus.flags|=Rflags; //add command line flags if any to existing flags from InitRobot

	//fprintf(stderr,"after InitFreeThought\n");
	//GetTime();  //update PStatus time variables utim (us) and ctim (h:m:s)
	//PStatus.lastexpose=localtime(&ttime);  //is mallocing?

	//  FT_GetInput();

	//Create a thread that will process Robot tasks (current tasks are: 1:run scripts, 2:balance)
	CreateRobotTasksThread();

#if USE_RT_PREEMPT
	//create a thread that will send any queued TurnMotor instructions one at a time every 100us
	CreateTurnMotorThread();
#endif //USE_RT_PREEMPT

	//fprintf(stderr,"after FT_Get_Input\n");
	//call universal (global) function to make firstwindow
	(*PStatus.FirstWindow) ();
	//alternatively:
	//main_CreateFTWindow();
	//or

	//InitFTI();

	//fprintf(stderr,"after FirstWindow\n");

#if WIN32
	hWin=PStatus.iwindow->hwindow;
#endif

	//PStatus.exitprogram=0;
	while((PStatus.flags&PExit)==0)
	{

	//go through all connections and see if we need to open any new windows
		lmac=RStatus.iMAC_Connection;
		//this may need a lock- because some macs are being deleted elsewhere
		while(lmac!=0) {
		if (lmac->flags&ROBOT_MAC_CONNECTION_OPEN_WINDOW) {
			lmac->flags&=~ROBOT_MAC_CONNECTION_OPEN_WINDOW;  //clear flag
			(*lmac->AddWindowFunction) (); //create and open the window
			//temp fix for now: if winAnalogSensors is opening, and winAccel is not open, open that as well (for GAMTP PCB)
			if (lmac->AddWindowFunction==winAnalogSensors_AddFTWindow) {
				twin = GetFTWindow("winAccels");
				if (twin == 0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
					lmac->AddWindowFunction = (FTControlfunc *)winAccels_AddFTWindow;
					(*lmac->AddWindowFunction) (); //create and open the window
				} //if (twin == 0) {
			} //if (!strcmp(lmac->Name,"winAnanlog")) 
		} //if (RStatus.flags&ROBOT_OPEN_WINDOW) {
		lmac=lmac->next;
		} //while(lmac!=0) {

		//GetTime();  //update PStatus time variables utim (us) and ctim (h:m:s)
		FT_GetInput();
		} //while((PStatus.flags&PExit)==0)
	//fprintf(stderr,"0 ");

	CloseRobot();

#if USE_RTAI
	rt_task_delete(Main_RT_Task);
#endif

	//fprintf(stderr,"before CloseX\n ");
	CloseFreeThought();


	return(1);
}  //end main()

//Initialize the Robot
int InitRobot(void)
{
	char filename[512],tstr[2048];
	int i,j;
	//MotorAngleAssociation lma;
	//int MotorOppDir,MotorDirCCW;


	memset(&RStatus,0,sizeof(RobotStatus));
	//memset(RStatusAccel,0,sizeof(Accelerometer)*MAX_NUM_ACCELEROMETERS);

	//set initial flags
	RStatus.flags|=ROBOT_STATUS_SHOW_ACCEL_DATA;

	//read in RobotApp.conf - the Robot app configuration
	//to connect each PCB MAC address to the correct Robot body part (Left Leg, Right Arm, etc.)
	//also sets motor orientation flag (MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)
	if (!LoadRobotAppConfigFile()) {
		fprintf(stderr,"Failed to load RobotApp.conf file\n");
	};


	//If there is a default robot read in all of its configuration files
	if (strlen(RStatus.DefaultRobot)>0) {
		//there is a default robot to load
#if Linux
		sprintf(tstr,"%s/%s",RStatus.RobotsFolder,RStatus.DefaultRobot);
#endif
#if WIN32
		sprintf(tstr,"%s\\%s",RStatus.RobotsFolder,RStatus.DefaultRobot);
#endif
		LoadRobot(tstr,RStatus.DefaultRobot);
	} 	



	//Fill the Motor Info table - for a fast way to determine get motor name for the logs from motor (port) number
	//this is because the port number a motor is connected to can change and this allows the code to remain the same.
	//note that the index in MotorInfo[index] is not the same as the motor number.
	
	//Set Maximum thrust for each motor
	//set all to 5 or 6/20 (equiv of 2/7 is 5.7/20)
	//for(i=0;i<NUM_MOTOR_NAMES;i++) {
		//RStatus.MotorInfo[i].MaxThrustAG=7;
		//RStatus.MotorInfo[i].MaxThrustWG=7;
	//}	
	//add exceptions  8/20 (equiv 3/7 is 8.57)  mostly feet only push upper body when thrust is at least 3/7 (unless near X=0)
	//currently against of with gravity is determined just by |Segment Angle|<|TargetAngle|=segment is moving with gravity assist
//4/7=0.57 = 11.42/20

#if 0 
	//Set Accels that have to have their Y and -Z exchanged because they are vertical
	RStatus.AccelInfo[ACCEL_NAME_LEFT_LOWER_LEG].flags|=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
	RStatus.AccelInfo[ACCEL_NAME_LEFT_UPPER_LEG].flags|=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
	RStatus.AccelInfo[ACCEL_NAME_RIGHT_LOWER_LEG].flags|=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
	RStatus.AccelInfo[ACCEL_NAME_RIGHT_UPPER_LEG].flags|=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
	RStatus.AccelInfo[ACCEL_NAME_HIP].flags|=ACCEL_INFO_TABLE_SWAP_MINUS_Y_AND_MINUS_Z;
	RStatus.AccelInfo[ACCEL_NAME_TORSO].flags|=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;

	//These MaxThrusts are designed for leaning and slow motions, for stepping, script should specify MAX_THRUST=10 (or whatever the higher thrust needed should be). Otherwise too large a velocity develops during balancing motions.

	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].MaxThrustAG=14;//12;//9-12
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].MaxThrustWG=3;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].MaxThrustAG=5;//14;//12;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].MaxThrustWG=5;//14;//12;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].MaxThrustAG=12;//9-12
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].MaxThrustWG=6;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].MaxThrustAG=9;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].MaxThrustWG=4;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].MaxThrustAG=6;//4;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].MaxThrustWG=6;//4;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].MaxThrustAG=5;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].MaxThrustWG=5;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].MaxThrustAG=12;  //9-12
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].MaxThrustWG=3;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].MaxThrustAG=5;//12;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].MaxThrustWG=5;//12;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].MaxThrustAG=12; //9-12
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].MaxThrustWG=6;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].MaxThrustAG=9;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].MaxThrustWG=4;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].MaxThrustAG=6;//4;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].MaxThrustWG=6;//4;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].MaxThrustAG=4;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].MaxThrustWG=4;

	RStatus.MotorInfo[MOTOR_NAME_TORSO].MaxThrustAG=9;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].MaxThrustWG=3;
	RStatus.MotorInfo[MOTOR_NAME_NECK].MaxThrustAG=9;
	RStatus.MotorInfo[MOTOR_NAME_NECK].MaxThrustWG=3;
	RStatus.MotorInfo[MOTOR_NAME_HEAD].MaxThrustAG=12;//5; //4 has enough thrust to move from/to any angle
	RStatus.MotorInfo[MOTOR_NAME_HEAD].MaxThrustWG=3;




	//add Motor Thrusts (force) in g-m/s^2 (milliNewtons) currently estimated manually
	//HEAD MOTOR
	RStatus.MotorInfo[MOTOR_NAME_HEAD].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_HEAD].Thrust[1]=177;
	RStatus.MotorInfo[MOTOR_NAME_HEAD].Thrust[2]=333;
	RStatus.MotorInfo[MOTOR_NAME_HEAD].Thrust[3]=409;
	for(i=4;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_HEAD].Thrust[i]=500; //Estimated maximum thrust force for this motor
	} //for i


	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[1]=200;//orig: 82;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[2]=400;//orig: 169;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[3]=581;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[4]=629;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[5]=774;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[6]=1065;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[7]=1283;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[8]=1283;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[9]=1283;
	for(i=10;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[i]=1290; //Estimated maximum thrust force for this motor
	} //for i
	for(i=0;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) { //right is same as left
		RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].Thrust[i]=RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Thrust[i];
	} //for i

	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[1]=47;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[2]=194;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[3]=544;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[4]=837;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[5]=1004;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[6]=1130;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[7]=1269;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[8]=1395;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[9]=1395;
	for(i=10;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[i]=1395; //Estimated maximum thrust force for this motor
	} //for i
	for(i=0;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) { //right is same as left
		RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].Thrust[i]=RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Thrust[i];
	} //for i

	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[1]=0;//43;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[2]=20;//264;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[3]=200;//533;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[4]=400;//838;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[5]=600;//1067;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[6]=800;//1372;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[7]=1000;//1626;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[8]=1200;//1803;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[9]=1400;//1956;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[10]=1600;//2083;
	for(i=11;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[i]=1800;//2083; //Estimated maximum thrust force for this motor
	} //for i
	for(i=0;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) { //right is same as left
		RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].Thrust[i]=RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Thrust[i];
	} //for i


//at 12 degrees weight 8195 * sin(12) = 1704g /2=852g
//maxthrust=12 (changed from 40rpm to 14rpm estimated about 2x thrust)
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[1]=150;//75;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[2]=200;//100;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[3]=250;//125;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[4]=300;//150;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[5]=300;//150;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[6]=300;//150;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[7]=860;//430;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[8]=860;//430;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[9]=1000;//500;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[10]=1000;//500;
	for(i=11;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[i]=1200+100*(i-11);//600+100*(i-11); //Estimated maximum thrust force for this motor
	} //for i
	for(i=0;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) { //right is same as left
		RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].Thrust[i]=RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Thrust[i];
	} //for i

	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[1]=10;//180;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[2]=20;//360;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[3]=30;//495;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[4]=40;//500;//550;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[5]=50;//500;//750;  //need to reach MaxThrust to hold lower leg Z for step
	for(i=6;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[i]=60+(i-6)*10;//500;//850+100*(i-6); //Estimated maximum thrust force for this motor
	} //for i
	for(i=0;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) { //right is same as left
		RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].Thrust[i]=RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Thrust[i];
	} //for i


	//TORSO
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[0]=0;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[1]=10;//180;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[2]=20;//360;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[3]=30;//495;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[4]=40;//500;//550;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[5]=50;//500;//750;  //need to reach MaxThrust to hold lower leg Z for step
	for(i=6;i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE;i++) {
		RStatus.MotorInfo[MOTOR_NAME_TORSO].Thrust[i]=60+(i-6)*10;//500;//850+100*(i-6); //Estimated maximum thrust force for this motor
	} //for i


/*
	//Set Accelerometer+Gyroscope device maximum speeds (in Degrees/s)
	//set all to 30deg/s, then change any that need more
	for(i=0;i<NUM_ACCEL_NAMES;i++) {
		for(j=0;j<3;j++) {
			RStatus.AccelInfo[i].MaxSpeed[j]=30;
		} 
	}	
	RStatus.AccelInfo[ACCEL_NAME_HEAD].MaxSpeed[0]=50;  //head can nod (pitch X) up to +/- 50 deg/s
	RStatus.AccelInfo[ACCEL_NAME_HEAD].MaxSpeed[1]=50;  //head can turn (yaw Y) up to +/- 50 deg/s
*/

	//Copy Motor names
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_FOOT]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_FOOT]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].Num=MOTOR_LEFT_FOOT;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_ANKLE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_ANKLE]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].Num=MOTOR_LEFT_ANKLE;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_KNEE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_KNEE]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].Num=MOTOR_LEFT_KNEE;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_LEG]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_LEG]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].Num=MOTOR_LEFT_LEG;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_SIDE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_SIDE]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].Num=MOTOR_LEFT_SIDE;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_HIP]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_HIP]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].Num=MOTOR_LEFT_HIP;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_FOOT]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_FOOT]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].Num=MOTOR_RIGHT_FOOT;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_ANKLE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_ANKLE]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].Num=MOTOR_RIGHT_ANKLE;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_KNEE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_KNEE]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].Num=MOTOR_RIGHT_KNEE;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_LEG]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_LEG]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].Num=MOTOR_RIGHT_LEG;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_SIDE]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_SIDE]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].Num=MOTOR_RIGHT_SIDE;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_HIP]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_HIP]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].Num=MOTOR_RIGHT_HIP;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;

	strcpy(RStatus.MotorInfo[MOTOR_NAME_TORSO].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_TORSO]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_TORSO].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_TORSO]);
	RStatus.MotorInfo[MOTOR_NAME_TORSO].Num=MOTOR_TORSO;
	RStatus.MotorInfo[MOTOR_NAME_TORSO].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;

	strcpy(RStatus.MotorInfo[MOTOR_NAME_NECK].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_NECK]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_NECK].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_NECK]);
	RStatus.MotorInfo[MOTOR_NAME_NECK].Num=MOTOR_NECK;
	RStatus.MotorInfo[MOTOR_NAME_NECK].PCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_HEAD].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_HEAD]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_HEAD].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_HEAD]);
	RStatus.MotorInfo[MOTOR_NAME_HEAD].Num=MOTOR_HEAD;
	RStatus.MotorInfo[MOTOR_NAME_HEAD].PCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
#if 0 
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_SHOULDER].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_LEFT_SHOULDER]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_LEFT_SHOULDER].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_LEFT_SHOULDER]);
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SHOULDER].Num=MOTOR_LEFT_SHOULDER;
	RStatus.MotorInfo[MOTOR_NAME_LEFT_SHOULDER].PCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_SHOULDER].Name,ROBOT_MOTOR_NAMES[MOTOR_NAME_RIGHT_SHOULDER]);
	strcpy(RStatus.MotorInfo[MOTOR_NAME_RIGHT_SHOULDER].ReadableName,ROBOT_MOTOR_READABLE_NAMES[MOTOR_NAME_RIGHT_SHOULDER]);
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SHOULDER].Num=MOTOR_RIGHT_SHOULDER;
	RStatus.MotorInfo[MOTOR_NAME_RIGHT_SHOULDER].PCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
#endif

#endif
	
	//Add motor info (motor and opposite motor) - needs to be done after loading RobotApp.conf to determine motor orientations 
	//todo: could be simplified to just AddMotorPair(MOTOR_NAME_LEFT_FOOT,MOTOR_NAME_RIGHT_FOOT,MotorOppDir
#if 0 
	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,MOTOR_RIGHT_FOOT,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,MOTOR_LEFT_FOOT,MotorOppDir);

	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,MOTOR_LEFT_ANKLE,MotorOppDir);

	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,MOTOR_RIGHT_KNEE,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,MOTOR_LEFT_KNEE,MotorOppDir);

	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,MOTOR_LEFT_LEG,MotorOppDir);

	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,MOTOR_LEFT_SIDE,MotorOppDir);

	MotorOppDir=((RStatus.MotorInfo[MOTOR_NAME_LEFT_HIP].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=(RStatus.MotorInfo[MOTOR_NAME_RIGHT_HIP].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW));
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_HIP,MOTOR_RIGHT_HIP,MotorOppDir);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_HIP,MOTOR_LEFT_HIP,MotorOppDir);
#endif

/*
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,MOTOR_RIGHT_FOOT,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,MOTOR_RIGHT_KNEE,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_HIP,MOTOR_RIGHT_HIP,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,MOTOR_LEFT_FOOT,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,MOTOR_LEFT_ANKLE,1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,MOTOR_LEFT_KNEE,-1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,MOTOR_LEFT_LEG,1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,MOTOR_LEFT_SIDE,1);
	AddMotorPair(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_HIP,MOTOR_LEFT_HIP,-1);
*/
//	AddMotorPair(ROBOT_PCB_ETHMOTORS_UPPER_BODY,MOTOR_LEFT_SHOULDER,MOTOR_RIGHT_SHOULDER,-1);
//	AddMotorPair(ROBOT_PCB_ETHMOTORS_UPPER_BODY,MOTOR_RIGHT_SHOULDER,MOTOR_LEFT_SHOULDER,-1);



#if 0 

	//Associate motors and accelerometer angles
	//X=pitch, positive is up from robot perspective
	//Y=yaw, positive is right from robot perspective
	//Z=roll, positive is clockwise from robot perspective
	//left side
	//foot motor with (foot) accel0x
	//int AssociateMotorAndAngle(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz,int IncAngDir);


	//LEFT SIDE
	//=========================

//LEFT FOOT MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);
	//foot motor with (foot) accel0x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_FOOT;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=0;
	lma.xyz=ACCEL_ANGLE_X;	
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=705;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MotionThreshold=1.0;//1.0;
	lma.WaitAndSeeDelay=10; //10 samples (100ms) WaitAndSeeDelay after TargetAngle (or IsMoving==0) reached
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;
	lma.MaxAccel=0.5;
	lma.ThrustVelocityRatio=1.0;  //1dps=thrust 1, 2dps=thrust 2, etc. 
	lma.flags=0;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,ROBOT_PCB_ETHACCELS_LEFT_LEG,0,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,705); 

	//foot motor with (lower leg) accel1x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_FOOT;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=8195;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5; //need higher velocity because velocity can quickly reverse 
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,ROBOT_PCB_ETHACCELS_LEFT_LEG,1,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,8195); 

//foot motor with (hip) accel0x -falling forward is to increase torso pitch (X)
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_LEFT_FOOT;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	}
	else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 8195;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);

	//foot motor with (torso) accel1x -falling forward is to increase torso pitch (X)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(&lma);	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_FOOT,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,8195);


//LEFT ANKLE
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_LEFT_ANKLE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);
	//lma.MotionThreshold=1.0;  //was 0.01 for robot00, for ankles, because they are only 5rpm, InRange and NotMoving cause Thrust=0, but that stops the motor from pushing against a step, so for now setting the motion threshold to 0.01 (usually 2 degrees per second)
	//ankle motor with (foot) accel0z
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_ANKLE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=0;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=760;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5; 
	lma.flags=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,ROBOT_PCB_ETHACCELS_LEFT_LEG,0,ACCEL_ANGLE_Z,MOTOR_CLOCKWISE,760);

	//ankle motor with (lower leg) accel1z
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_ANKLE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=8140;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5; 
	AssociateMotorAndAngle(&lma);
	//ankle motor with (lower right leg) accel1z
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	AssociateMotorAndAngle(&lma);
	//ankle motor with (upper right leg) accel2z
	lma.AccelNum=2;
	AssociateMotorAndAngle(&lma);
	//ankle motor with (upper left leg) accel2z
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	AssociateMotorAndAngle(&lma);

	//ankle motor with (hip) accel0z
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_LEFT_ANKLE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 8140;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 2;
	lma.MaxVelocity = 5;
	//	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	lma.flags = MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);

	//ankle motor with (torso) accel1z
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_ANKLE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_Z,MOTOR_COUNTER_CLOCKWISE,8140);


//LEFT KNEE MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_LEFT_KNEE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);
	//knee motor with (lower leg) accel1x - knee only needs strength/speed=1
	//lma.MotionThreshold=2.0;  
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_KNEE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=1395;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;  //knee moving lower leg should probably only be when foot is off the ground 
	lma.flags= NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,ROBOT_PCB_ETHACCELS_LEFT_LEG,1,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,1395);

	//knee motor with (upper leg) accel2x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_KNEE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=5080;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=4; 
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,ROBOT_PCB_ETHACCELS_LEFT_LEG,2,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,5080);

//knee motor with (hip) accel0x
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_LEFT_KNEE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);

	//knee motor with (torso) accel1x 
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(&lma);	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,5080);


	//knee motor with (hip) accel0z
	lma.AccelNum=0;
	lma.xyz = ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.flags = 0;
	AssociateMotorAndAngle(&lma);


	//knee motor with (torso) accel1z
	lma.AccelNum=1;
	lma.flags = 0;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(&lma);	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_KNEE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,5080);


//LEFT LEG MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_LEFT_LEG].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);
	//upper leg motor with (upper leg) accel2x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_LEG;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=2420;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=50;  //for lifting leg 
	lma.flags=0;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,ROBOT_PCB_ETHACCELS_LEFT_LEG,2,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,2420); 


//leg motor with (hip) accel0x
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	}	else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}	lma.Weight = 3935;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);

	//leg motor with (torso) accel1x
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_LEG,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,3935); 


	//left leg motor with right leg accel2x (needed for constant thrust step)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
//MotorDirCCW is 1 (CCW)
/*	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
*/
	lma.Weight=3935;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);


//LEFT SIDE MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_LEFT_SIDE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);
	//left side motor with left upper leg (accel2z)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_LEFT_SIDE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=2540;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5; 
	lma.flags=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);

	//left side motor with left lower leg (accel1z)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);

	//left side motor with right upper leg (accel2z)
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG; 
	lma.AccelNum=2;
	AssociateMotorAndAngle(&lma);

	//left side motor with right lower leg (accel1z)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);

//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,ROBOT_PCB_ETHACCELS_LEFT_LEG,2,ACCEL_ANGLE_Z,MOTOR_COUNTER_CLOCKWISE,2540); 

	//left side motor with (hip) accel0z
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_LEFT_SIDE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_Z;
	if (MotorDirCCW) { 
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {  
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 4055;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	//lma.flags|=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	lma.flags = MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity;
	AssociateMotorAndAngle(&lma);

	//left side motor with (torso) accel1z
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_Z,MOTOR_CLOCKWISE,4055); 

	//todo: LEFT HIP MOTOR  


	//===============
	//RIGHT SIDE
	//====================

	//RIGHT FOOT
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_RIGHT_FOOT].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//foot with (foot) accel0x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_FOOT;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=0;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=705;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;
	lma.flags=0; 
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,ROBOT_PCB_ETHACCELS_RIGHT_LEG,0,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,705); 

	//foot with (lower leg) accel1x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_FOOT;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=8195;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,ROBOT_PCB_ETHACCELS_RIGHT_LEG,1,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,8195); 

	//foot with (hip) accel0x
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_RIGHT_FOOT;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 8195;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);
	//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,8195);

	//foot with (torso) accel1x
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_FOOT,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,8195);


	//RIGHT ANKLE
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_RIGHT_ANKLE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//lma.MotionThreshold=1.0; 
	//ankle with (foot) accel0z
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_ANKLE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=0;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=760;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5; 
	lma.flags=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,ROBOT_PCB_ETHACCELS_RIGHT_LEG,0,ACCEL_ANGLE_Z,MOTOR_CLOCKWISE,760); 

	//ankle with (lower leg) accel1z
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_ANKLE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=8140;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5;
	lma.flags=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);
	//ankle motor with (lower left leg) accel1z
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	AssociateMotorAndAngle(&lma);
	//ankle with (upper left leg) accel1z
	lma.AccelNum=2;
	AssociateMotorAndAngle(&lma);
	//ankle motor with (upper right leg) accel1z
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	AssociateMotorAndAngle(&lma);

	//ankle motor with (hip) accel0z
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_RIGHT_ANKLE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	}
	else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 8140;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 2;
	lma.MaxVelocity = 5;
	//	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	//lma.flags=0; 
	lma.flags = MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity
	AssociateMotorAndAngle(&lma);

	//ankle motor with (torso) accel1z
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_ANKLE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_Z,MOTOR_COUNTER_CLOCKWISE,8140); 


	//RIGHT KNEE MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_RIGHT_KNEE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//lma.MotionThreshold=2.0; //restore to default (2 degrees per second) 
	//knee with (lower leg) accel1x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_KNEE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=1;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=1395;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5; 
	lma.flags= NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,ROBOT_PCB_ETHACCELS_RIGHT_LEG,1,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,1395); 

	//knee with (upper leg) accel2x
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_KNEE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight=5080;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=4;
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,ROBOT_PCB_ETHACCELS_RIGHT_LEG,2,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,5080); 

	//knee with (hip) accel0x
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_RIGHT_KNEE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	AssociateMotorAndAngle(&lma);

	//knee with (torso) accel1x
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,5080); 

	//knee motor with (hip) accel0z
	lma.AccelNum=0;
	lma.xyz = ACCEL_ANGLE_Z;
	lma.flags = 0;
	AssociateMotorAndAngle(&lma);

	//knee motor with (torso) accel1z
	lma.AccelNum=1;
	lma.flags = 0;
	AssociateMotorAndAngle(&lma);


	//RIGHT LEG MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_RIGHT_LEG].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//right leg motor with upper right leg (accel2x)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_LEG;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=2420;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=50; //for lifting leg 
	lma.flags=0;
	AssociateMotorAndAngle(&lma);

//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,ROBOT_PCB_ETHACCELS_RIGHT_LEG,2,ACCEL_ANGLE_X,MOTOR_CLOCKWISE,2420);

	//right leg motor with Hip (accel0x)
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	}
	else {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 3935;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	lma.flags |= NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);


	//right leg motor with Torso (accel1x)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,3935);


	//right leg motor with left leg accel2x (needed for constant thrust step)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_X;
	//MotorDirCCW is 0 cw
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}
	lma.Weight=3935;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=1; 
	lma.MaxVelocity=5;
	lma.flags=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);


	//RIGHT SIDE MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_RIGHT_SIDE].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//right side motor with right upper leg (accel2z)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum=MOTOR_RIGHT_SIDE;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;
	lma.AccelNum=2;
	lma.xyz=ACCEL_ANGLE_Z;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}	
	lma.Weight=2540;
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=2; 
	lma.MaxVelocity=5; 
	lma.flags=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY;
	AssociateMotorAndAngle(&lma);

	//right side motor with right lower leg (accel1z)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);

	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG; 
	lma.AccelNum=2;
	AssociateMotorAndAngle(&lma);

	//right side motor with left lower leg (accel1z)
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);


//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,ROBOT_PCB_ETHACCELS_RIGHT_LEG,2,ACCEL_ANGLE_Z,MOTOR_COUNTER_CLOCKWISE,2540); 

	//right side motor with (hip) accel0z
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;
	lma.MotorNum = MOTOR_RIGHT_SIDE;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0;
	lma.xyz = ACCEL_ANGLE_Z;
	if (MotorDirCCW) {  
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {  
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}
	lma.Weight = 4055;
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 5;
	//lma.flags|=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range
	lma.flags = MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY; //ignore gravity;
	AssociateMotorAndAngle(&lma);


	//right side motor with (torso) accel1z
	lma.AccelNum=1;
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,ROBOT_PCB_ETHACCELS_TORSO,0,ACCEL_ANGLE_Z,MOTOR_CLOCKWISE,4055); 


	//================
	//UPPER BODY
	//=================

	//TORSO MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_TORSO].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//torso motor with hip accel0x
	lma.EMPCBNum = ROBOT_PCB_ETHMOTORS_LOWER_BODY;  //***NOTE: CURRENTLY ON LOWER BODY
	lma.MotorNum = MOTOR_TORSO;
	lma.EAPCBNum = ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum = 0; //Torso Accel num
	lma.xyz = ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir = MOTOR_COUNTER_CLOCKWISE;
	}
	else {
		lma.IncAngDir = MOTOR_CLOCKWISE;
	}
	lma.Weight = 3000;  //guess
	lma.Mass = lma.Weight / 9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity = 1;
	lma.MaxVelocity = 20;
	lma.flags = NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range ;
	AssociateMotorAndAngle(&lma);


	//torso motor with torso accel0x
	lma.AccelNum=1; //Torso Accel num
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	}	
	AssociateMotorAndAngle(&lma);


	//NECK MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_NECK].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//neck motor with torso accel1y (in head) (note that this is yaw)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
	lma.MotorNum=MOTOR_NECK;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum=2; //Head accel
	lma.xyz=ACCEL_ANGLE_Y;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}	
	lma.Weight=0;  //0g weight, but there is friction
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MinVelocity=50; 
	lma.MaxVelocity=500;
	lma.flags=0; 
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_UPPER_BODY,MOTOR_NECK,ROBOT_PCB_ETHACCELS_TORSO,1,ACCEL_ANGLE_Y,MOTOR_COUNTER_CLOCKWISE,0); //0g weight, but there is friction

	//HEAD MOTOR
	MotorDirCCW=((RStatus.MotorInfo[MOTOR_NAME_HEAD].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW)!=0);

	//head motor with torso accel1x (in head)
	lma.EMPCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
	lma.MotorNum=MOTOR_HEAD;
	lma.EAPCBNum=ROBOT_PCB_ETHACCELS_TORSO;
	lma.AccelNum=2;  //head accel
	lma.xyz=ACCEL_ANGLE_X;
	if (MotorDirCCW) {
		lma.IncAngDir=MOTOR_COUNTER_CLOCKWISE;
	} else {
		lma.IncAngDir=MOTOR_CLOCKWISE;
	}	
	lma.Weight=435;  //0g weight, but there is friction
	lma.Mass=lma.Weight/9.8;  //9.8m/s^2 is accel due to gravity
	lma.MotionThreshold=1.0;
	lma.WaitAndSeeDelay=20; //20 samples (200ms) WaitAndSeeDelay after TargetAngle (or IsMoving==0) reached
	lma.MinVelocity=50;
	lma.MaxVelocity=500;
	lma.flags|=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE; //no thrust when moving against gravity when in range 
	AssociateMotorAndAngle(&lma);
//	AssociateMotorAndAngle(ROBOT_PCB_ETHMOTORS_UPPER_BODY,MOTOR_HEAD,ROBOT_PCB_ETHACCELS_TORSO,1,ACCEL_ANGLE_X,MOTOR_COUNTER_CLOCKWISE,435);
	
#endif 


	InitFree3D(); //initialize Free3D

	InitSineTable(); //initialize the Sine lookup table

	return(1);
} //int InitRobot(void)


//Initialize sine look up table, only uses 0 to PI/2 (90 degrees) 
int InitSineTable(void)
{
	int i;
	
	for(i=0;i<NUM_SINE_TABLE_ENTRIES;i++) {
		RStatus.SineTable[i]=sin(((float)i/(float)NUM_SINE_TABLE_ENTRIES)*PI_DIV_2);
	} //for i

	return(1);

} //int InitSineTable(void)


//sine look up table (only uses -PI/2 to PI/2)
float sinlu(float Angle) 
{
	unsigned int Index;

	if (Angle>PI_DIV_2 || Angle<-PI_DIV_2) {
		fprintf(stderr,"Error: In sinlu(), Angle %03.8f outside of -PI/2 (%03.8f) to PI/2 (%03.8f) range.\n",Angle,-PI_DIV_2,PI_DIV_2);
		return(0.0);
	}

	//for now just give back nearest value, later interpolate
	Index=(int)(fabs(Angle)*(float)NUM_SINE_TABLE_ENTRIES*INV_PI_DIV_2);
	if (Index>=NUM_SINE_TABLE_ENTRIES) {
		Index=NUM_SINE_TABLE_ENTRIES-1;
	} 
	if (Angle<0) {
		return(-RStatus.SineTable[Index]);
	} else {
		return(RStatus.SineTable[Index]);
	}
} //float sinlu(float Angle)

//cosine lookup table  (only uses -PI/2 to PI/2)
float coslu(float Angle) 
{
	return(sinlu(PI_DIV_2-fabs(Angle)));
} //float coslu(float Angle)


//Close Robot and free any allocated memory
int CloseRobot(void)
{
	if (PStatus.flags&PInfo) {
		fprintf(stderr,"CloseRobot()\n");
	} 

	StopAllRobotThreads();

//	FreeMotorPairList();

//	FreeMotorAngleAssociationList();

	UnloadRobot();  //free all currently loaded robot config info

	CloseFree3D();

	//unload any loaded scripts
	if (PStatus.flags&PInfo) {
		fprintf(stderr,"Done CloseRobot()\n");
	}

return(1);
} //int CloseRobot(void)

int StopAllRobotThreads(void) 
{

	StopRobotTasksThread(); //destroy the RobotTasks thread
#if USE_RT_PREEMPT
	StopTurnMotorThread(); //destroy the TurnMotor thread
#endif
	//Close_MAC_Socket() - Closes Robot Listening Threads

	return(1);
} //int StopAllRobotThreads(void) 


//Delete log files older than NumDays days
int DeleteOldLogs(int NumDays,char *RobotFolder)
{
#if Linux
	DIR *ldir;
#endif
#if WIN32
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
#endif
	char tstr[2048],tdir[2048];
	struct dirent *ep;
	struct stat attrib;
	//char timestamp[64];
	//uint64_t CurTS,FTS,SevenDaysInMS;
	int SevenDaysInSec;
	time_t curtime;
	int i;

	if (NumDays<=0) {
		fprintf(stderr,"Number of days to delete logs after (%d) must be >0.\n",NumDays);
	}

//	fprintf(stderr,"DeleteOldLogs()\n");
	fprintf(stderr,"Deleting logs in %s folder older than %d days...\n",RobotFolder,NumDays);

	//make 3 passes to delete 0:Model Logs, 1: Future Logs, 2: Motion Logs
	i=0;
	while(i<3) {

		switch(i) {
			case 0:
#if Linux
				sprintf(tdir,"%s/logs/model//",RobotFolder);
#endif
#if WIN32
				sprintf(tdir,"%s\\logs\\model\\",RobotFolder);
#endif
			break;
			case 1:
#if Linux
				sprintf(tdir,"%s/logs/future/",RobotFolder);
#endif
#if WIN32
				sprintf(tdir,"%s\\logs\\future\\",RobotFolder);
#endif
			break;
			case 2:
#if Linux
				sprintf(tdir,"%s/logs/motion/",RobotFolder);
#endif
#if WIN32
				sprintf(tdir,"%s\\logs\\motion\\",RobotFolder);
#endif
			break;
		} //switch(i)

		curtime = time(NULL);
		SevenDaysInSec = NumDays * 24 * 3600;
		//SevenDaysInMS=7*24*3600*1000;
#if Linux
		ldir=opendir(tdir);
		if (ldir!=0) {			
			//read in file info
			while((ep=readdir(ldir))) { 
	//			fprintf(stderr,"%s\n",ep->d_name);
				//get file creation time in number of seconds since 00:00:00 on 1/1/1970
				sprintf(tstr,"%s%s",tdir,ep->d_name);
				if (stat(tstr,&attrib) == 0 ) {
					//use time stamp in file name
					//strncpy(timestamp,&ep->d_name[6],19);
					//timestamp[20]=0;
					//fprintf(stderr,"%s\n",timestamp);
					//CurTS=GetTimeInMS();
					//FTS=ConvertTimeStampToMS(timestamp);
					//if ((CurTS-SevenDaysInMS) > FTS) 
					if (attrib.st_mode&S_IFREG) { //is regular file
					//if ( S_ISREG(attrib.st_mode)) { //is regular file
						//fprintf(stderr,"%s %llu %llu\n",tstr,(curtime-SevenDaysInSec),attrib.st_mtime);
						if ((curtime-SevenDaysInSec)>attrib.st_mtime) {
							//fprintf(stderr,"%s\n",ep->d_name);
							unlink(tstr); //delete file
						} 
					} //if ( S_ISREG(attrib.st_mode)) { 
				} else { //if (stat(tstr,&attrib)) {

					fprintf(stderr,"stat() failed on %s\n",tstr);
				} //	if (stat(tstr,&attrib)) {
			} //while


			closedir(ldir);
		} else { //	if (ldir!=0) {
			fprintf(stderr,"Unable to open %s\n",tdir);
		} //	if (ldir!=0) {
#endif //Linux

#if WIN32
		if ((hFind = FindFirstFile(tdir, &FindFileData)) != INVALID_HANDLE_VALUE) {
			do {
				//get file creation time in number of seconds since 00:00:00 on 1/1/1970
				sprintf(tstr, "%s%s", tdir, FindFileData.cFileName);

				if (stat(tstr, &attrib) == 0) {
					if (attrib.st_mode&S_IFREG) { //is regular file
												   //fprintf(stderr,"%s %llu %llu\n",tstr,(curtime-SevenDaysInSec),attrib.st_mtime);
						if ((curtime - SevenDaysInSec)>attrib.st_mtime) {
							//fprintf(stderr,"%s\n",ep->d_name);
							unlink(tstr); //delete file
						}
					} //if (attrib.st_mode&S_IFREG)) { 					
				}
				else { //if (stat(tstr,&attrib)) {
					fprintf(stderr, "stat() failed on %s\n", tstr);
				} //	if (stat(tstr,&attrib)) {


			} while (FindNextFile(hFind, &FindFileData));
			FindClose(hFind);
		} // if ((hFind = FindFirstFile ...

#endif 



		i++;  //go to next log
	} //while(i)
	return(1);

} //int DeleteOldLogs(int NumDays)

int LoadCalibrationFileAccel(char *filename) {
	return(LoadCalibrationFile(filename,0));  //0=Accel 
} //int LoadCalibrationFileAccel(char *filename) {

int LoadCalibrationFileAnalog(char *filename) {
	return(LoadCalibrationFile(filename,1));  //1=Analog
} //int LoadCalibrationFileAccel(char *filename) {

int LoadCalibrationFileGyro(char *filename) {
	return(LoadCalibrationFile(filename,2)); //2=Gyro
} //int LoadCalibrationFileGyro(char *filename) {

int LoadCalibrationFileMag(char *filename) {
	return(LoadCalibrationFile(filename,3)); //3=Mag
} //int LoadCalibrationFileMag(char *filename) {



int LoadCalibrationFile(char *filename,int ATGM) {
	FILE *fptr;
	char tstr[FTMedStr];
//	char Section[NUM_ETHACCEL_PCBS][FTMedStr];
	char tline[FTMedStr];
	int match; //CurSection
	int bSize,result;
	int64_t fSize;
	char *buf,*param;//,*holdptr;
	int NumLines,CurLine,i,j,k,CurChar;
	//unsigned char MACInBytes[6]; 
	struct stat st;

	//load accel.conf gyro.conf or analog.conf

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"LoadCalibrationFile\n");
	}


	result = stat(filename, &st);
	if (result==0) {
		//file exists

		//set use calibration flag
		RStatus.flags |= ROBOT_STATUS_USE_CALIBRATION;

		//load accel,gyro, or magnetometer calibration offsets

		fptr = fopen(filename, "rb");
		if (fptr != 0) {
			fprintf(stderr,"Loading calibration file '%s'\n",filename);
			//read in accel offsets
			//read in complete file
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
			buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
			if (buf == NULL) {
				sprintf(tstr,"Allocating memory for config file failed.\n");
				FTMessageBox(tstr,FTMB_OK,"Error",0);
				return(0);
			}

			//copy the file into the buffer:
			result=fread(buf,1,fSize,fptr);
			if (result != fSize) {
				sprintf(tstr,"Error reading config file %s\n",filename);
				FTMessageBox(tstr,FTMB_OK,"Error",0);
				fclose(fptr);
				return(0);
			}

			fclose(fptr);
			buf[fSize]=0; //close string of buffer - so strlen will give correct size
			bSize=fSize;


			//set each expected section text
			//for(i=0;i<NUM_ETHACCEL_PCBS;i++) {
			//	strcpy(Section[i],ROBOT_PCB_ETHACCEL_NAMES[i]);
			//} 

			//go through each line and each word (separated by spaces or a comma)

			//determine total number of lines in order to allocate space for a pointer to each for strtok
			NumLines=0;
			i=0;
			while(i<bSize) {
				if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
					NumLines++;
				}
				i++;
			}  //while(i<bSize)

			if (PStatus.flags&PInfo) {
				fprintf(stderr,"parse config file\n");
			}

			CurLine=0;
			CurChar=0;
			i=0;
			while(CurLine<NumLines) { 
				//get a line from the file text
				Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
				CurChar+=strlen(tline);
				CurLine++;
				if (PStatus.flags&PInfo) {
					fprintf(stderr,"tline=%s\n",tline);
				}

				//fprintf(stderr,"tline=%s strlen=%d NumEthAccelsPCBsExpected=%d\n",tline,strlen(tline),RStatus.NumEthAccelsPCBsExpected);

				//todo trim any initial spaces
				//get EthAccel PCB Name
				param=strtok(tline,"\n"); 
				//fprintf(stderr,"%s %d i=%d\n",param,param,i);
				//see if this word matches any known section
				match=0;

				while(param!=0 && match==0 && i<RStatus.NumAccelsPCBsExpected) {


#if Linux
//					if (!strcasecmp(param,Section[i])) {
					if (!strcasecmp(param,RStatus.AccelsPCBInfo[i]->Name)) {

#endif
#if WIN32
					//if (!_stricmp(param,Section[i])) {
					if (!strcasecmp(param,RStatus.AccelsPCBInfo[i]->Name)) {
#endif
						//found a match
						match=1;

            //fprintf(stderr,"Found %s\n",param);
						//Set CaliFile PCBNum						
						//CaliFile[i].PCBNum=GetPCBNameNum(ROBOT_PCB_ETHACCEL_NAMES[i]);

						//todo: read in Analog Sensors differently- read until no more, otherwise 15 need to be listed
						//read in each of 3 accels with 3 offsets each
						for(j=0;j<3+12*(ATGM==1);j++) {  //currently 15 analog sensors active on each accel
							Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);  //get accel line, ex: 0: x=1.02 y=-1.24 z=-1.20
							CurChar+=strlen(tline);
							CurLine++;
							param=strtok(tline," ");  //skip #:
							for(k=0;k<3-(ATGM==1);k++) {  //for x,y,z (analog = min,max)
								param=strtok(NULL," \n");
								//fprintf(stderr,"%f ",atof(param));
 								switch(ATGM) {
									case 0: //accel
										//CaliFile[i].AOffset[j][k]=atof(param);
										RStatus.AccelsPCBInfo[i]->AOffset[j][k]=atof(param);
										break;
									case 1: //analog
										//CaliFile[i].AnOffset[j][k]=atof(param);
										RStatus.AccelsPCBInfo[i]->AnOffset[j][k]=atof(param);
										//fprintf(stderr,"%f ",CaliFile[i].AnOffset[j][k]);
										break;
									case 2: //gyro
										//CaliFile[i].GOffset[j][k]=atof(param);
										RStatus.AccelsPCBInfo[i]->GOffset[j][k]=atof(param);
										//fprintf(stderr,"%f ",CaliFile[i].GOffset[j][k]);
										break;
									case 3: //magnetometer
										//CaliFile[i].MOffset[j][k]=atof(param);
										RStatus.AccelsPCBInfo[i]->MOffset[j][k]=atof(param);
										break;
								} //switch
							} //for k						
						}  //for j
					} //if (!_stricmp
					i++;  //advance to next section
				} //while(match==0
        if (!match && param!=0 && i<RStatus.NumAccelsPCBsExpected) {
					if (param==0) {
						sprintf(tstr,"Error on line %d in config file %s",CurLine,filename);
						FTMessageBox(tstr,FTMB_OK,"Error",0);
						i=RStatus.NumAccelsPCBsExpected;  //exit while loop		
					}
					if (param[0]!=35) {//ignore any line starting with # 
						sprintf(tstr,"Unknown Accels PCB '%s' on line %d in config file %s",param,CurLine,filename);
						FTMessageBox(tstr,FTMB_OK,"Error",0);
						i=RStatus.NumAccelsPCBsExpected;  //exit while loop
						//CurLine=NumLines; //exit outer each-line loop
					} //if (param[0]!=35) {//ignore any line starting with #
				} //if (!match) {
			} //while(CurLine<NumLInes) 

			free(buf);

			if (PStatus.flags&PInfo) {
				fprintf(stderr,"end LoadCalibrationFile\n");
			}
		return(1);

		} //if (fptr!=0) {
	} //if (result) {
	return(0);
} //int LoadCalibrationFile(char *filename,int AGM) {

//Load a Robot and all its associated configuration files
int LoadRobot(char *RobotPath, char *RobotName)
{
	FILE *fptr;
	char tstr[FTMedStr];
	char Section[NUM_ROBOT_CONFIG_SECTIONS][FTMedStr];
	char SectionEM[NUM_ETHMOTORS_CONFIG_SECTIONS][FTMedStr];
	char SectionEA[NUM_ETHACCELS_CONFIG_SECTIONS][FTMedStr];
	char SectionEP[NUM_ETHACCELS_CONFIG_SECTIONS][FTMedStr];
	char SectionGAMTP[NUM_GAMTP_CONFIG_SECTIONS][FTMedStr];
	char SectionMA[NUM_MOTOR_ANGLE_CONFIG_SECTIONS][FTMedStr];
	char tline[FTMedStr],filename[FTMedStr],tline2[FTMedStr];
	int match; 
	int bSize,result;
	int64_t fSize;
	char *buf,*param;
	int NumLines,CurLine,i,CurChar;
	int PCBNameNum;
	int CurEthMotorsPCBNum,MotorNum,MotorNameNum,pass,ignore;
	int CurEthAccelsPCBNum,AccelNum,AccelNameNum,AnalogSensorNum,AnalogSensorNameNum;
	int CurEthPowerPCBNum;
	int CurGAMTPPCBNum;
	int CurMotorAngle,CurEntry,NumAccelsOnPCB;
	int GAMTPNumMotors,GAMTPNumAccels,GAMTPNumAnalogSensors;
	unsigned char MACInBytes[6]; 
	float DefaultMotionThreshold,DefaultMinVelocity,DefaultMaxVelocity,DefaultMaxAccel,DefaultTVR;
	MotorsPCBInfoTable *MotorsPCBInfo;
	AccelsPCBInfoTable *AccelsPCBInfo;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"LoadRobot\n");
	}

	fprintf(stderr,"Loading Robot %s\n",RobotName);

	memcpy(RStatus.CurrentRobotName,RobotName,strlen(RobotName));
	RStatus.CurrentRobotName[strlen(RobotName)]=0;

	//first load User Robot File (ex: TwoLeg.Robot)
#if Linux
	sprintf(filename,"%s/%s.Robot",RobotPath,RobotName);
#endif
#if WIN32
	sprintf(filename,"%s\\%s.Robot",RobotPath,RobotName);
#endif
	

	fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
	if (fptr!=0) {
		//read in the entire file

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
		buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
		if (buf == NULL) {
			sprintf(tstr,"Allocating memory for config file failed.\n");
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			return(0);
		}

		//copy the file into the buffer:
		result=fread(buf,1,fSize,fptr);
		if (result != fSize) {
			sprintf(tstr,"Error reading config file %s\n",filename);
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			fclose(fptr);
			return(0);
		}

		fclose(fptr);
		buf[fSize]=0; //close string of buffer - so strlen will give correct size
		bSize=fSize;


		//set each expected section text
		strcpy(Section[ROBOT_CONFIG_HOME_FOLDER],"HOME_FOLDER");
		strcpy(Section[ROBOT_CONFIG_READABLE_NAME],"READABLE_NAME");
		strcpy(Section[ROBOT_CONFIG_NUM_PCBS],"NUM_PCBS");
		strcpy(Section[ROBOT_CONFIG_NUM_ETHMOTORS_PCBS],"NUM_ETHMOTORS_PCBS");
		strcpy(Section[ROBOT_CONFIG_NUM_ETHACCELS_PCBS],"NUM_ETHACCELS_PCBS");
		strcpy(Section[ROBOT_CONFIG_NUM_ETHPOWER_PCBS],"NUM_ETHPOWER_PCBS");
		strcpy(Section[ROBOT_CONFIG_NUM_GAMTP_PCBS],"NUM_GAMTP_PCBS");
		strcpy(Section[ROBOT_CONFIG_3D_MODEL_NAME],"3D_MODEL_NAME");
		strcpy(Section[ROBOT_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS],"DELETE_LOGS_OLDER_THAN_DAYS");

		//go through each line and each word (separated by spaces or a comma)
		//determine total number of lines in order to allocate space for a pointer to each for strtok
		NumLines=0;
		i=0;
		while(i<bSize) {
			if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
				NumLines++;
			}
			i++;
		}  //while(i<bSize)

		if (PStatus.flags&PInfo) {
			fprintf(stderr,"parse config file\n");
		}

		CurLine=0;
		CurChar=0;
		PCBNameNum=-1;
		while(CurLine<NumLines) { 
			//get a line from the file text
			Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
			CurChar+=strlen(tline);
			CurLine++;
			if (PStatus.flags&PInfo) {
				fprintf(stderr,"tline=%s\n",tline);
			}

			//todo trim any initial spaces
			//get a word on this line
			param=strtok(tline,"="); 
			//see if this word matches any known section
			match=0;
			i=0;
			while(param!=0 && match==0 && i<NUM_ROBOT_CONFIG_SECTIONS) {
#if Linux
				if (!strcasecmp(param,Section[i])) {
#endif
#if WIN32
				if (!_stricmp(param,Section[i])) {
#endif
					//found a match
					match=1;
					//read in data for this section
					switch(i) {
						case ROBOT_CONFIG_HOME_FOLDER:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									memcpy(RStatus.CurrentRobotFolder,param,strlen(param));
									//copy log folder and model log folder
#if Linux
									sprintf(RStatus.CurrentRobotLogFolder,"%s/logs",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotModelLogFolder,"%s/logs/model",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotFutureLogFolder,"%s/logs/future",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotScriptFolder,"%s/scripts",RStatus.CurrentRobotFolder);
#endif
#if WIN32
									sprintf(RStatus.CurrentRobotLogFolder,"%s\\logs",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotModelLogFolder,"%s\\logs\\model",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotFutureLogFolder,"%s\\logs\\future",RStatus.CurrentRobotFolder);
									sprintf(RStatus.CurrentRobotScriptFolder,"%s\\logs\\scripts",RStatus.CurrentRobotFolder);
#endif
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_CONFIG_READABLE_NAME:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									memcpy(RStatus.RobotReadableName,param,strlen(param));
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;					
						case ROBOT_CONFIG_NUM_PCBS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									RStatus.NumEthPCBsExpected=atoi(param);
									//allocate the memory for the EthPCBInfo table
									RStatus.EthPCBInfo=malloc(sizeof(EthPCBInfoTable)*atoi(param));
									if (RStatus.EthPCBInfo==0) {
										fprintf(stderr,"Error: Malloc of EthPCBInfo table failed.\n");
									} else {
										memset(RStatus.EthPCBInfo,0,sizeof(EthPCBInfoTable)*atoi(param));
									} 
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;			
						case ROBOT_CONFIG_NUM_ETHMOTORS_PCBS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									RStatus.NumEthMotorsPCBsExpected=atoi(param);
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;	
						case ROBOT_CONFIG_NUM_ETHACCELS_PCBS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									RStatus.NumEthAccelsPCBsExpected=atoi(param);
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;	
						case ROBOT_CONFIG_NUM_ETHPOWER_PCBS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									RStatus.NumEthPowerPCBsExpected=atoi(param);
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;	
						case ROBOT_CONFIG_NUM_GAMTP_PCBS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									RStatus.NumGAMTPPCBsExpected=atoi(param);
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;	
						case ROBOT_CONFIG_3D_MODEL_NAME:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a name given
									memcpy(RStatus.CurrentRobotModelFile,param,strlen(param));
									RStatus.CurrentRobotModelFile[strlen(param)]=0;
									i=NUM_ROBOT_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;					
						case ROBOT_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								//delete any log files older than given number of days
								DeleteOldLogs(atoi(param),RStatus.CurrentRobotFolder);
								//RStatus.CurrentRobotDeleteLogsOlderThanDays=atoi(param);
							} //if (param!=0) {

						break;
						default:  //most likely will never be reached
							if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} 
						break;
					} //switch(i)
				} //if (!_stricmp
				i++;  //advance to next section
			} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {



			if (!match) {
				if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
					sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
					//CurLine=NumLines; //exit outer each-line loop
				} //if (param[0]!=35) {//ignore any line starting with #
			} //if (!match) {

		} //while(CurLine<NumLInes) 

		free(buf);


		//common to GAMTP and EthMotors:
		RStatus.NumMotorsExpected=0;
		MotorNum=-1; //number of motor on PCB
		MotorNameNum=-1; //cumulative motor number
		GAMTPNumMotors=0;//number of motors found in GAMTPs.conf


		RStatus.NumEthAccelsPCBsExpected=0;
		RStatus.NumAccelsExpected=0;
		CurEthAccelsPCBNum=-1;
		AccelNum=-1; //number of accel (not necessarily number of accel port on PCB)
		AccelNameNum=-1; //cumulative accel number of all accels on robot
		AnalogSensorNum=-1;  //number of AnalogSensor (not necessarily number of analog sensor port on PCB)
		AnalogSensorNameNum=-1; //cumulative analog sensor number of all analog sensors on robot
		GAMTPNumAccels=0; //number of accels found in GAMTPs.conf
		GAMTPNumAnalogSensors=0; //number of analog sensors found in GAMTPs.conf

		//now allocate all the memory needed for the currently loaded robot data structures


		//**************************LOAD GAMTPs SECTION
		//Load GAMTPs.conf if any exist
		if (RStatus.NumGAMTPPCBsExpected>0) {
			//Make 2 passes, 1: Determine total number of PCBs and motors, accels, and analog sensors in order to allocate all memory first

#if Linux
			sprintf(filename,"%s/%s/GAMTPs.conf",RStatus.RobotsFolder,RobotName);
#endif
#if WIN32
			sprintf(filename,"%s\\%s\\GAMTPs.conf",RStatus.RobotsFolder,RobotName);
#endif
	

			fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
			if (fptr!=0) {

				fprintf(stderr,"Loading GAMTPs for robot %s\n",RobotName);

				//read in the entire file

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


				//set each expected section text
				strcpy(SectionGAMTP[GAMTP_CONFIG_NUM_GAMTP_PCBS],"NUM_GAMTP_PCBS");
				strcpy(SectionGAMTP[GAMTP_CONFIG_PCB_NUM],"PCB_NUM");
				strcpy(SectionGAMTP[GAMTP_CONFIG_PCB_NAME],"PCB_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_PCB_MAC],"PCB_MAC");
				//Motors
				strcpy(SectionGAMTP[GAMTP_CONFIG_NUM_MOTORS],"NUM_MOTORS");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_NUM],"MOTOR_NUM");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_NAME],"MOTOR_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_READABLE_NAME],"MOTOR_READABLE_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_ORIENTATION],"MOTOR_ORIENTATION");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_MAX_THRUST_AG],"MOTOR_MAX_THRUST_AG");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_MAX_THRUST_WG],"MOTOR_MAX_THRUST_WG");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_PAIR],"MOTOR_PAIR");
				strcpy(SectionGAMTP[GAMTP_CONFIG_MOTOR_COMPLEMENT],"MOTOR_COMPLEMENT");
				//Accels
				strcpy(SectionGAMTP[GAMTP_CONFIG_NUM_ACCELS],"NUM_ACCELS");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_NUM],"ACCEL_NUM");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_NAME],"ACCEL_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_READABLE_NAME],"ACCEL_READABLE_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_ORIENTATION],"ACCEL_ORIENTATION");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_CALIB_PITCH],"ACCEL_CALIB_PITCH");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_CALIB_YAW],"ACCEL_CALIB_YAW");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_CALIB_ROLL],"ACCEL_CALIB_ROLL");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ACCEL_3DMODEL_AXIS],"ACCEL_3DMODEL_AXIS");			
				//Analog Sensors
				strcpy(SectionGAMTP[GAMTP_CONFIG_NUM_ANALOG_SENSORS],"NUM_ANALOG_SENSORS");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ANALOG_SENSOR_NUM],"ANALOG_SENSOR_NUM");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ANALOG_SENSOR_KIND],"ANALOG_SENSOR_KIND");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ANALOG_SENSOR_NAME],"ANALOG_SENSOR_NAME");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ANALOG_SENSOR_MINV_ANGLE],"ANALOG_SENSOR_MINV_ANGLE");
				strcpy(SectionGAMTP[GAMTP_CONFIG_ANALOG_SENSOR_MAXV_ANGLE],"ANALOG_SENSOR_MAXV_ANGLE");


				// allocate memory to contain the whole file:
				buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
				if (buf == NULL) {
					sprintf(tstr,"Allocating memory for config file failed.\n");
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					return(0);
				}

				//copy the file into the buffer:
				result=fread(buf,1,fSize,fptr);
				if (result != fSize) {
					sprintf(tstr,"Error reading config file %s\n",filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					fclose(fptr);
					return(0);
				}

				fclose(fptr);
				buf[fSize]=0; //close string of buffer - so strlen will give correct size
				bSize=fSize;



				//go through each line and each word (separated by spaces or a comma)
				//determine total number of lines in order to allocate space for a pointer to each for strtok
				NumLines=0;
				i=0;
				while(i<bSize) {
					if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
						NumLines++;
					}
					i++;
				}  //while(i<bSize)

				ignore=0;
				RStatus.NumGAMTPPCBsExpected=0;
				CurGAMTPPCBNum=-1;
				for(pass=0;pass<2;pass++) {
	 
					//pass 0 just gets number of GAMTP PCBs and motors and allocates MotorInfo table, pass 1 allocates and fills individual motors in the MotorInfo table

					if (PStatus.flags&PInfo) {
						fprintf(stderr,"parse config file pass %d\n",pass+1);
					}

					CurLine=0;
					CurChar=0;
					while(CurLine<NumLines) { 

						//get a line from the file text
						Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
						CurChar+=strlen(tline);
						CurLine++;
						if (PStatus.flags&PInfo) {
							fprintf(stderr,"tline=%s\n",tline);
						}

						//todo trim any initial spaces
						if (tline[0]==35) {  //skip lines that start with #
							param=0;
							//fprintf(stderr,"comment\n");
						} else {
							//get a word on this line
							param=strtok(tline,"=\n");
						}
						//fprintf(stderr,"%s\n",param);

						//see if this word matches any known section
						match=0;
						i=0;
						while(param!=0 && match==0 && i<NUM_GAMTP_CONFIG_SECTIONS) {
#if Linux
							if (!strcasecmp(param,SectionGAMTP[i])) {
#endif
#if WIN32
							if (!_stricmp(param,SectionGAMTP[i])) {
#endif

								//found a match
								match=1;
								//read in data for this section
								switch(i) {
									case GAMTP_CONFIG_NUM_GAMTP_PCBS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass) {
											if (strlen(param)>0) { //there is a number given
												RStatus.NumGAMTPPCBsExpected=atoi(param); //copy
												RStatus.NumAccelsPCBsExpected=RStatus.NumGAMTPPCBsExpected;  //same AccelsPCB code is used for both EthAccels and GAMTP PCBs

												//allocate memory for GAMTPPCBInfoTable
												RStatus.GAMTPPCBInfo=(GAMTPPCBInfoTable *)malloc(sizeof(GAMTPPCBInfoTable)*atoi(param));
												if (RStatus.GAMTPPCBInfo==0) {
													fprintf(stderr,"Error: Failed to allocate memory for GAMTPPCBInfo table.\n");
												} else {
													memset(RStatus.GAMTPPCBInfo,0,sizeof(GAMTPPCBInfoTable)*atoi(param));
												} 	
												
												//allocate memory for array of pointers to AccelsPCBInfoTable (in EthMotors and GAMTP PCBs)
												RStatus.AccelsPCBInfo=(AccelsPCBInfoTable **)malloc(sizeof(AccelsPCBInfoTable *)*atoi(param));
												if (RStatus.AccelsPCBInfo==0) {
													fprintf(stderr,"Error: Failed to allocate memory for AccelsPCBInfo table.\n");
												} else {
													memset(RStatus.AccelsPCBInfo,0,sizeof(AccelsPCBInfoTable *)*atoi(param));
												} 	
												
												
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_PCB_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass) {
											if (strlen(param)>0) { //there is a number given
												if (CurGAMTPPCBNum+1>=RStatus.NumGAMTPPCBsExpected) {
													fprintf(stderr,"Ignoring GAMTP PCB_NUM %d because NUM_GAMTP_PCBS=%d\n",atoi(param),RStatus.NumGAMTPPCBsExpected);
													ignore=1;
												} else {
													//fprintf(stderr,"reseting ignore NumGAMTPPCBsExpected=%d PCB_NUM=%d\n",RStatus.NumGAMTPPCBsExpected,atoi(param));
													ignore=0; //reset ignore in case there are valid GAMTPPCB entries after GAMTPPCB with number too high
													PCBNameNum++; //increment total PCB count
													CurGAMTPPCBNum++;//just increment, ignore user's PCB number //=atoi(param);
													RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].PCBNum=PCBNameNum;   //record PCB NameNum
													strcpy(RStatus.EthPCBInfo[PCBNameNum].Kind,"GAMTP");  //copy PCB kind
													//shorthand to MotorPCBInfo
													MotorsPCBInfo=&RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].motors;
													MotorNum=-1;  //reset motor number 
													//shorthand to AccelsPCBInfo
													AccelsPCBInfo=&RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].accels;
													AccelsPCBInfo->Name=RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name;//copy pointer to PCB name
													AccelsPCBInfo->PCBNum=PCBNameNum;//copy PCBNum to AccelsPCBInfo
													AccelNum=-1;  //reset accel number
													AnalogSensorNum=-1; //and reset analog sensor number 
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_PCB_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name,param,strlen(param)); //copy 

												//copy the pointer to the PCB Name to the EthPCBInfo structure
												RStatus.EthPCBInfo[PCBNameNum].Name=RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name;

											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_PCB_MAC:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//Convert ascii to bytes
												ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
												//copy MAC address to PCB table
												memcpy(RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].MAC,MACInBytes,6);
												//copy address of MAC address to PCBInfo table too
												RStatus.EthPCBInfo[PCBNameNum].MAC=RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].MAC;

											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									
									case GAMTP_CONFIG_NUM_MOTORS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all motors to find total number of motors in robot
														RStatus.NumMotorsExpected+=atoi(param);
													} else { //if (!pass) {
														fprintf(stderr,"GAMTP PCB %d (%s) has %d motors\n",CurGAMTPPCBNum,RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name,atoi(param));
														MotorsPCBInfo->NumMotors=atoi(param);
														//allocate memory for MotorInfoTable within GAMTPPCB (list of pointers into the quick RStatus.MotorInfo table)
														MotorsPCBInfo->MotorInfo=(MotorInfoTable **)malloc(sizeof(MotorInfoTable *)*atoi(param));
														if (MotorsPCBInfo->MotorInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for GAMTPPCBInfo MotorInfo table of pointers.\n");
														} //if (RStatus.GAMTPPCBInfo[...
													} //if (!pass) {
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//***Important: Note that MOTOR_NUM given is a port number on the GAMTP and so the local variable MotorNum may be different. So for example NumMotors=3 but Motor num=8 could happen because the motor uses GAMTP port 8.
												MotorNum++;
												MotorNameNum++;

												RStatus.MotorInfo[MotorNameNum].Num=atoi(param);
												MotorsPCBInfo->MotorInfo[MotorNum]=&RStatus.MotorInfo[MotorNameNum];

												//also set PCBNum in MotorInfo
												RStatus.MotorInfo[MotorNameNum].PCBNum=PCBNameNum;

												//Set MaxThrust to default
												RStatus.MotorInfo[MotorNameNum].MaxThrustAG=DEFAULT_MOTOR_MAX_THRUST_AG;
												RStatus.MotorInfo[MotorNameNum].MaxThrustWG=DEFAULT_MOTOR_MAX_THRUST_WG;

												//fprintf(stderr,"Found Motor %d\n",MotorNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.MotorInfo[MotorNameNum].Name,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_READABLE_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.MotorInfo[MotorNameNum].ReadableName,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_ORIENTATION:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												if (!strcmp(param,"CCW")) {
													RStatus.MotorInfo[MotorNameNum].flags=MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW;
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_MAX_THRUST_AG:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorInfo[MotorNameNum].MaxThrustAG=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_MAX_THRUST_WG:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorInfo[MotorNameNum].MaxThrustWG=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_PAIR:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.MotorInfo[MotorNameNum].Pair,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_MOTOR_COMPLEMENT:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.MotorInfo[MotorNameNum].Complement,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_NUM_ACCELS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all accels to find total number of accels in robot
														RStatus.NumAccelsExpected+=atoi(param);
													} else { //if (!pass) {
														NumAccelsOnPCB=atoi(param);
														fprintf(stderr,"GAMTP PCB %d (%s) has %d accels\n",CurGAMTPPCBNum,RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name,NumAccelsOnPCB);
														AccelsPCBInfo->NumAccels=NumAccelsOnPCB;
														//allocate memory for AccelInfoTable within GAMTPPCB (list of pointers into the quick RStatus.AccelInfo table) - **Note if more accels are found than NUMACCELS- ignore extra accels (otherwise need to realloc)
														AccelsPCBInfo->AccelInfo=(AccelInfoTable **)malloc(sizeof(AccelInfoTable *)*NumAccelsOnPCB); 
														if (AccelsPCBInfo->AccelInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for GAMTPPCBInfo AccelInfo table of pointers.\n");
														} else {
															//store pointer to the AccelsPCB group in array of AccelsPCBs
															RStatus.AccelsPCBInfo[CurGAMTPPCBNum]=AccelsPCBInfo;
														} 															
													} //else if (!pass) {
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//Important: If more accels are listed than NUM_ACCELS currently ignore the extra accel, otherwise need to realloc
												AccelNum++;
												if (AccelNum>NumAccelsOnPCB) {
													fprintf(stderr,"Warning: Ignoring extra accelerometer on line %d\n",CurLine);
												} else {  //if (AccelNum>NumAccelsOnPCB) {

													//***Important: Note that ACCEL_NUM given is a port number on the GAMTP and so the local variable AccelNum may be different. So for example NumAccels=3 but Accel num=8 could happen because the motor uses GAMTP port 8.
													//AccelNum++;
													AccelNameNum++;
													RStatus.AccelInfo[AccelNameNum].Num=atoi(param);
													AccelsPCBInfo->AccelInfo[AccelNum]=&RStatus.AccelInfo[AccelNameNum];

													//also set PCBNum in AccelInfo
													RStatus.AccelInfo[AccelNameNum].PCBNum=PCBNameNum;

													//clear calibration floating point values
		//											fprintf(stderr,"%d ",AccelNameNum);
		/* realy mysterious this crashes a malloc later on
													RStatus.AccelInfo[AccelNameNum].Calib.i[0]=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.i[1]=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.y=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.z=0.0;
		*/

													//Add this accel number to the GAMTP PCB AccelMask
													AccelsPCBInfo->AccelMask|=(1<<atoi(param));
												} //if (AccelNum>NumAccelsOnPCB) {

												//fprintf(stderr,"Found Accel %d\n",AccelNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.AccelInfo[AccelNameNum].Name,param,strlen(param));
												RStatus.AccelInfo[AccelNameNum].Name[strlen(param)]=0;
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_READABLE_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.AccelInfo[AccelNameNum].ReadableName,param,strlen(param));
												RStatus.AccelInfo[AccelNameNum].ReadableName[strlen(param)]=0;
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_ORIENTATION:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												if (!strcmp(param,"SWAP_Y_AND_MINUS_Z")) {
													RStatus.AccelInfo[AccelNameNum].flags=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
												} 
												if (!strcmp(param,"SWAP_MINUS_Y_AND_MINUS_Z")) {
													RStatus.AccelInfo[AccelNameNum].flags=ACCEL_INFO_TABLE_SWAP_MINUS_Y_AND_MINUS_Z;
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_CALIB_PITCH:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[0]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_CALIB_YAW:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[1]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_CALIB_ROLL:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[2]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ACCEL_3DMODEL_AXIS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].ModelAxis=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_NUM_ANALOG_SENSORS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !ignore) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all motors to find total number of motors in robot
														RStatus.NumAnalogSensorsExpected+=atoi(param);
													} else { //if (!pass)
														fprintf(stderr,"GAMTP PCB %d (%s) has %d analog sensors\n",CurGAMTPPCBNum,RStatus.GAMTPPCBInfo[CurGAMTPPCBNum].Name,atoi(param));
														AccelsPCBInfo->NumAnalogSensors=atoi(param);
														//allocate memory for AccelInfoTable within GAMTPPCB (list of pointers into the quick RStatus.AccelInfo table)
														AccelsPCBInfo->AnalogSensorInfo=(AnalogSensorInfoTable **)malloc(sizeof(AnalogSensorInfoTable *)*atoi(param));
														if (AccelsPCBInfo->AnalogSensorInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for GAMTPPCBInfo AnalogSensorInfo table of pointers.\n");
														} 
													} //else if (!pass)
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ANALOG_SENSOR_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//***Important: Note that ANALOG_NUM given is a port number on the GAMTP and so the local variable AccelNum may be different. So for example AnalogSensor[0] might have .num=3 because it is located on GAMTP port 3.
												AnalogSensorNum++;
												AnalogSensorNameNum++;

												RStatus.AnalogSensorInfo[AnalogSensorNameNum].Num=atoi(param);
												AccelsPCBInfo->AnalogSensorInfo[AnalogSensorNum]=&RStatus.AnalogSensorInfo[AnalogSensorNameNum];

												//also set PCBNum in AnalogInfo
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].PCBNum=PCBNameNum;

												//Add this analog sensor number to the GAMTP PCB AnalogSensorMask
												AccelsPCBInfo->AnalogSensorMask|=(1<<atoi(param));


												//fprintf(stderr,"Found Analog Sensor num=%d\n",atoi(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ANALOG_SENSOR_KIND:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a value given
												//***Important: Note that ANALOG_NUM given is a port number on the GAMTP and so the local variable AccelNum may be different. So for example AnalogSensor[0] might have .num=3 because it is located on GAMTP port 3.
                        						if (!strncmp(param,"TOUCH",5)) {
													RStatus.AnalogSensorInfo[AnalogSensorNameNum].flags|=ANALOG_SENSOR_TOUCH_SENSOR;
                        						}
                        						if (!strncmp(param,"POT",3)) {
													RStatus.AnalogSensorInfo[AnalogSensorNameNum].flags|=ANALOG_SENSOR_POTENTIOMETER;
                        						}
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ANALOG_SENSOR_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.AnalogSensorInfo[AnalogSensorNameNum].Name,param,strlen(param));
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].Name[strlen(param)]=0;
												fprintf(stderr,"Found Analog Sensor: %s (NameNum %d)\n",param,AnalogSensorNameNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ANALOG_SENSOR_MINV_ANGLE:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].MinVAngle=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									case GAMTP_CONFIG_ANALOG_SENSOR_MAXV_ANGLE:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].MaxVAngle=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
									break;
									default:  //most likely will never be reached
										if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
											sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
											//CurLine=NumLines; //exit outer each-line loop
										} 
									break;
								} //switch(i)
							} //if (!_stricmp
							i++;  //advance to next section
						} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {




						if (!match) {
							if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_GAMTP_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} //if (param[0]!=35) {//ignore any line starting with #
						} //if (!match) {


					} //while(CurLine<NumLines) 
	
					//Done with pass 0, malloc MotorInfo table for total number of motors
					//TODO: this is a problem because EthMotors may add more motors
					if (!pass) {
						//Motors:
						if (RStatus.NumMotorsExpected>0) {
							RStatus.MotorInfo=(MotorInfoTable *)malloc(sizeof(MotorInfoTable)*RStatus.NumMotorsExpected);
							if (RStatus.MotorInfo==0) {
								fprintf(stderr,"Error: Failed to allocate MotorInfoTable\n");
							} else {
								memset(RStatus.MotorInfo,0,sizeof(MotorInfoTable)*RStatus.NumMotorsExpected);
							}
						} else {  //if (RStatus.NumMotorsExpected>0) {
							fprintf(stderr,"NUM_MOTORS found in GAMTPs.conf is 0, ignoring rest of GAMTPs.conf info\n");
							ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumMotorsExpected>0) {
						//Accels:
						if (RStatus.NumAccelsExpected>0) {
							RStatus.AccelInfo=(AccelInfoTable *)malloc(sizeof(AccelInfoTable)*RStatus.NumAccelsExpected);
							if (RStatus.AccelInfo==0) {
								fprintf(stderr,"Error: Failed to allocate AccelInfoTable\n");
							} else {
								memset(RStatus.AccelInfo,0,sizeof(AccelInfoTable)*RStatus.NumAccelsExpected);
							} 
						} else {  //if (RStatus.NumAccelsExpected>0) {
							fprintf(stderr,"NUM_ACCELS found in EthAccels.conf is 0, ignoring rest of accels in EthAccels.conf info\n");
							ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumAccelsExpected>0) {
						//Analog Sensors:
						if (RStatus.NumAnalogSensorsExpected>0) {
							RStatus.AnalogSensorInfo=(AnalogSensorInfoTable *)malloc(sizeof(AnalogSensorInfoTable)*RStatus.NumAnalogSensorsExpected);
							if (RStatus.AnalogSensorInfo==0) {
								fprintf(stderr,"Error: Failed to allocate AnalogSensorInfoTable\n");
							} else {
								memset(RStatus.AnalogSensorInfo,0,sizeof(AnalogSensorInfoTable)*RStatus.NumAnalogSensorsExpected);
							} 
						} else {  //if (RStatus.NumAnalogSensorsExpected>0) {
							//fprintf(stderr,"NUM_ANALOG_SENSORS found in EthAccels.conf is 0, ignoring rest of EthAccels.conf info\n");
							//ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumAccelsExpected>0) {


					} //if (!pass) {

				} //for(pass=0;pass<2;pass++) {

				free(buf);


			} else { //if (fptr!=0) {
				fprintf(stderr,"Could not open file %s\n",filename);
				return(0);
			} //else if (fptr!=0) {
		} //if (RStatus.NumGAMTPPCBsExpected>0) {
		//**************************END LOAD GAMTP SECTION



		//**************************LOAD ETHMOTORS SECTION
		//Load EthMotors.conf if any exist
		if (RStatus.NumEthMotorsPCBsExpected>0) {
			//Make 2 passes, 1: Determine total number of PCBs and motors in order to allocate all memory first

#if Linux
			sprintf(filename,"%s/%s/EthMotors.conf",RStatus.RobotsFolder,RobotName);
#endif
#if WIN32
			sprintf(filename,"%s\\%s\\EthMotors.conf",RStatus.RobotsFolder,RobotName);
#endif
	

			fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
			if (fptr!=0) {

				fprintf(stderr,"Loading EthMotors for robot %s\n",RobotName);

				//read in the entire file

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


				//set each expected section text
				strcpy(SectionEM[ETHMOTORS_CONFIG_NUM_ETHMOTORS_PCBS],"NUM_ETHMOTORS_PCBS");
				strcpy(SectionEM[ETHMOTORS_CONFIG_PCB_NUM],"PCB_NUM");
				strcpy(SectionEM[ETHMOTORS_CONFIG_PCB_NAME],"PCB_NAME");
				strcpy(SectionEM[ETHMOTORS_CONFIG_PCB_MAC],"PCB_MAC");
				strcpy(SectionEM[ETHMOTORS_CONFIG_NUM_MOTORS],"NUM_MOTORS");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_NUM],"MOTOR_NUM");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_NAME],"MOTOR_NAME");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_READABLE_NAME],"MOTOR_READABLE_NAME");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_ORIENTATION],"MOTOR_ORIENTATION");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_AG],"MOTOR_MAX_THRUST_AG");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_WG],"MOTOR_MAX_THRUST_WG");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_PAIR],"MOTOR_PAIR");
				strcpy(SectionEM[ETHMOTORS_CONFIG_MOTOR_COMPLEMENT],"MOTOR_COMPLEMENT");

				// allocate memory to contain the whole file:
				buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
				if (buf == NULL) {
					sprintf(tstr,"Allocating memory for config file failed.\n");
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					return(0);
				}

				//copy the file into the buffer:
				result=fread(buf,1,fSize,fptr);
				if (result != fSize) {
					sprintf(tstr,"Error reading config file %s\n",filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					fclose(fptr);
					return(0);
				}

				fclose(fptr);
				buf[fSize]=0; //close string of buffer - so strlen will give correct size
				bSize=fSize;



				//go through each line and each word (separated by spaces or a comma)
				//determine total number of lines in order to allocate space for a pointer to each for strtok
				NumLines=0;
				i=0;
				while(i<bSize) {
					if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
						NumLines++;
					}
					i++;
				}  //while(i<bSize)

				ignore=0;
				RStatus.NumEthMotorsPCBsExpected=0;				
				CurEthMotorsPCBNum=-1;
				GAMTPNumMotors=RStatus.NumMotorsExpected; //may have found motors in GAMTPs.conf
				for(pass=0;pass<2;pass++) {
	 
					//pass 0 just gets number of EthMotors PCBs and motors and allocates MotorInfo table, pass 1 allocates and fills individual motors in the MotorInfo table

					if (PStatus.flags&PInfo) {
						fprintf(stderr,"parse config file pass %d\n",pass+1);
					}

					CurLine=0;
					CurChar=0;
					while(CurLine<NumLines) { 

						//get a line from the file text
						Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
						CurChar+=strlen(tline);
						CurLine++;
						if (PStatus.flags&PInfo) {
							fprintf(stderr,"tline=%s\n",tline);
						}

						//todo trim any initial spaces
						if (tline[0]==35) {  //skip lines that start with #
							param=0;
							//fprintf(stderr,"comment\n");
						} else {
							//get a word on this line
							param=strtok(tline,"=\n");
						}
						//fprintf(stderr,"%s\n",param);

						//see if this word matches any known section
						match=0;
						i=0;
						while(param!=0 && match==0 && i<NUM_ETHMOTORS_CONFIG_SECTIONS) {
#if Linux
							if (!strcasecmp(param,SectionEM[i])) {
#endif
#if WIN32
							if (!_stricmp(param,SectionEM[i])) {
#endif

								//found a match
								match=1;
								//read in data for this section
								switch(i) {
									case ETHMOTORS_CONFIG_NUM_ETHMOTORS_PCBS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass) {
											if (strlen(param)>0) { //there is a number given
												RStatus.NumEthMotorsPCBsExpected=atoi(param); //copy
												//allocate memory for EthMotorsPCBInfoTable
												RStatus.EthMotorsPCBInfo=(EthMotorsPCBInfoTable *)malloc(sizeof(EthMotorsPCBInfoTable)*atoi(param));
												if (RStatus.EthMotorsPCBInfo==0) {
													fprintf(stderr,"Error: Failed to allocate memory for EthMotorsPCBInfo table.\n");
												} else {
													memset(RStatus.EthMotorsPCBInfo,0,sizeof(EthMotorsPCBInfoTable)*atoi(param));
												} 	
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_PCB_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass) {
											if (strlen(param)>0) { //there is a number given
												if (CurEthMotorsPCBNum+1>=RStatus.NumEthMotorsPCBsExpected) {
													fprintf(stderr,"Ignoring EthMotors PCB_NUM %d because NUM_ETHMOTORS_PCBS=%d\n",atoi(param),RStatus.NumEthMotorsPCBsExpected);
													ignore=1;
												} else {
													//fprintf(stderr,"reseting ignore NumEthMotorsPCBsExpected=%d PCB_NUM=%d\n",RStatus.NumEthMotorsPCBsExpected,atoi(param));
													ignore=0; //reset ignore in case there are valid EthMotorsPCB entries after EthMotorsPCB with number too high
													PCBNameNum++; //increment total PCB count
													CurEthMotorsPCBNum++;//just increment, ignore user's PCB number //=atoi(param);
													RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].PCBNum=PCBNameNum;   //record PCB NameNum
													strcpy(RStatus.EthPCBInfo[PCBNameNum].Kind,"Motor");  //copy PCB kind
													//shorthand to MotorPCBInfo
													MotorsPCBInfo=&RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].motors;
													MotorNum=-1;  //reset motor number 
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_PCB_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].Name,param,strlen(param)); //copy 

												//copy the address to the EthPCBInfo structure
												RStatus.EthPCBInfo[PCBNameNum].Name=RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].Name;

											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_PCB_MAC:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//Convert ascii to bytes
												ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
												//copy MAC address to PCB table
												memcpy(RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].MAC,MACInBytes,6);
												//copy address of MAC address to PCBInfo table too
												RStatus.EthPCBInfo[PCBNameNum].MAC=RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].MAC;

											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_NUM_MOTORS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all motors to find total number of motors in robot
														RStatus.NumMotorsExpected+=atoi(param);
													} else { //if (!pass) {
														fprintf(stderr,"EthMotors PCB %d (%s) has %d motors\n",CurEthMotorsPCBNum,RStatus.EthMotorsPCBInfo[CurEthMotorsPCBNum].Name,atoi(param));
														MotorsPCBInfo->NumMotors=atoi(param);
														//allocate memory for MotorInfoTable within EthMotorsPCB (list of pointers into the quick RStatus.MotorInfo table)
														MotorsPCBInfo->MotorInfo=(MotorInfoTable **)malloc(sizeof(MotorInfoTable *)*atoi(param));
														if (MotorsPCBInfo->MotorInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for EthMotorsPCBInfo MotorInfo table of pointers.\n");
														} //if (RStatus.EthMotorsPCBInfo[...
													} //if (!pass) {
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//***Important: Note that MOTOR_NUM given is a port number on the EthMotors and so the local variable MotorNum may be different. So for example NumMotors=3 but Motor num=8 could happen because the motor uses EthMotors port 8.
												MotorNum++;
												MotorNameNum++;

												RStatus.MotorInfo[MotorNameNum].Num=atoi(param);
												MotorsPCBInfo->MotorInfo[MotorNum]=&RStatus.MotorInfo[MotorNameNum];

												//also set PCBNum in MotorInfo
												RStatus.MotorInfo[MotorNameNum].PCBNum=PCBNameNum;

												//Set MaxThrust to default
												RStatus.MotorInfo[MotorNameNum].MaxThrustAG=DEFAULT_MOTOR_MAX_THRUST_AG;
												RStatus.MotorInfo[MotorNameNum].MaxThrustWG=DEFAULT_MOTOR_MAX_THRUST_WG;

												//fprintf(stderr,"Found Motor %d\n",MotorNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.MotorInfo[MotorNameNum].Name,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_READABLE_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.MotorInfo[MotorNameNum].ReadableName,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_ORIENTATION:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												if (!strcmp(param,"CCW")) {
													RStatus.MotorInfo[MotorNameNum].flags=MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW;
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_AG:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorInfo[MotorNameNum].MaxThrustAG=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_WG:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorInfo[MotorNameNum].MaxThrustWG=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_PAIR:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.MotorInfo[MotorNameNum].Pair,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHMOTORS_CONFIG_MOTOR_COMPLEMENT:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.MotorInfo[MotorNameNum].Complement,param,strlen(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
									break;
									default:  //most likely will never be reached
										if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
											sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
											//CurLine=NumLines; //exit outer each-line loop
										} 
									break;
								} //switch(i)
							} //if (!_stricmp
							i++;  //advance to next section
						} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {


						if (!match) {
							if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} //if (param[0]!=35) {//ignore any line starting with #
						} //if (!match) {


					} //while(CurLine<NumLines) 
	
					//Done with pass 0, malloc MotorInfo table for total number of motors
					if (!pass) {
						if (RStatus.NumMotorsExpected>GAMTPNumMotors) { //more motors found after loading any GAMTPs.conf
							if (GAMTPNumMotors>0) {  //there were motors in GAMTPs.conf
								RStatus.MotorInfo=(MotorInfoTable *)realloc(RStatus.MotorInfo,sizeof(MotorInfoTable)*RStatus.NumMotorsExpected);
								if (RStatus.MotorInfo==0) {
									fprintf(stderr,"Error: Failed to reallocate MotorInfoTable\n");
								} else {
									memset((unsigned char *)RStatus.MotorInfo+sizeof(MotorInfoTable)*GAMTPNumMotors,0,sizeof(MotorInfoTable)*(RStatus.NumMotorsExpected-GAMTPNumMotors));
								}
							} else {	
								//no Motors found in GAMTP.conf, just malloc MotorInfoTable
								RStatus.MotorInfo=(MotorInfoTable *)malloc(sizeof(MotorInfoTable)*RStatus.NumMotorsExpected);
								if (RStatus.MotorInfo==0) {
									fprintf(stderr,"Error: Failed to allocate MotorInfoTable\n");
								} else {
									memset(RStatus.MotorInfo,0,sizeof(MotorInfoTable)*RStatus.NumMotorsExpected);
								}
							} //GAMTPNumMotors>0
						} else {  //if (RStatus.NumMotorsExpected>0) {
							fprintf(stderr,"NUM_MOTORS found in EthMotors.conf is 0, ignoring rest of EthMotors.conf info\n");
							ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumMotorsExpected>0) {
					} //if (!pass && RStatus.NumMotorsExpected>0) {

				} //for(pass=0;pass<2;pass++) {

				free(buf);


			} else { //if (fptr!=0) {
				fprintf(stderr,"Could not open file %s\n",filename);
				return(0);
			} //else if (fptr!=0) {
		} //if (RStatus.NumEthMotorsPCBsExpected>0) {
		//**************************END LOAD ETHMOTORS SECTION


		//**************************LOAD ETHACCELS SECTION
		//Load EthAccels.conf if one or more expected
		if (RStatus.NumEthAccelsPCBsExpected>0) {

			//Make 2 passes, 1: Determine total number of PCBs and motors in order to allocate all memory first

			//first load Robot config (ex: TwoLeg.conf)
	#if Linux
			sprintf(filename,"%s/%s/EthAccels.conf",RStatus.RobotsFolder,RobotName);
	#endif
	#if WIN32
			sprintf(filename,"%s\\%s\\EthAccels.conf",RStatus.RobotsFolder,RobotName);
	#endif
	

			fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
			if (fptr!=0) {

				fprintf(stderr,"Loading EthAccels for robot %s\n",RobotName);

				//read in the entire file

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


				//set each expected section text
				strcpy(SectionEA[ETHACCELS_CONFIG_NUM_ETHACCELS_PCBS],"NUM_ETHACCELS_PCBS");
				strcpy(SectionEA[ETHACCELS_CONFIG_PCB_NUM],"PCB_NUM");
				strcpy(SectionEA[ETHACCELS_CONFIG_PCB_NAME],"PCB_NAME");
				strcpy(SectionEA[ETHACCELS_CONFIG_PCB_MAC],"PCB_MAC");
				strcpy(SectionEA[ETHACCELS_CONFIG_NUM_ACCELS],"NUM_ACCELS");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_NUM],"ACCEL_NUM");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_NAME],"ACCEL_NAME");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_READABLE_NAME],"ACCEL_READABLE_NAME");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_ORIENTATION],"ACCEL_ORIENTATION");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_CALIB_PITCH],"ACCEL_CALIB_PITCH");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_CALIB_YAW],"ACCEL_CALIB_YAW");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_CALIB_ROLL],"ACCEL_CALIB_ROLL");
				strcpy(SectionEA[ETHACCELS_CONFIG_ACCEL_3DMODEL_AXIS],"ACCEL_3DMODEL_AXIS");			
				strcpy(SectionEA[ETHACCELS_CONFIG_NUM_ANALOG_SENSORS],"NUM_ANALOG_SENSORS");
				strcpy(SectionEA[ETHACCELS_CONFIG_ANALOG_SENSOR_NUM],"ANALOG_SENSOR_NUM");
				strcpy(SectionEA[ETHACCELS_CONFIG_ANALOG_SENSOR_KIND],"ANALOG_SENSOR_KIND");
				strcpy(SectionEA[ETHACCELS_CONFIG_ANALOG_SENSOR_NAME],"ANALOG_SENSOR_NAME");
				strcpy(SectionEA[ETHACCELS_CONFIG_ANALOG_SENSOR_MINV_ANGLE],"ANALOG_SENSOR_MINV_ANGLE");
				strcpy(SectionEA[ETHACCELS_CONFIG_ANALOG_SENSOR_MAXV_ANGLE],"ANALOG_SENSOR_MAXV_ANGLE");

				// allocate memory to contain the whole file:
				buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
				if (buf == NULL) {
					sprintf(tstr,"Allocating memory for config file failed.\n");
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					return(0);
				}

				//copy the file into the buffer:
				result=fread(buf,1,fSize,fptr);
				if (result != fSize) {
					sprintf(tstr,"Error reading config file %s\n",filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					fclose(fptr);
					return(0);
				}

				fclose(fptr);
				buf[fSize]=0; //close string of buffer - so strlen will give correct size
				bSize=fSize;



				//go through each line and each word (separated by spaces or a comma)
				//determine total number of lines in order to allocate space for a pointer to each for strtok
				NumLines=0;
				i=0;
				while(i<bSize) {
					if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
						NumLines++;
					}
					i++;
				}  //while(i<bSize)

				ignore=0;
				RStatus.NumEthAccelsPCBsExpected=0;
				//RStatus.NumAccelsExpected=0;
				CurEthAccelsPCBNum=-1;
				AccelNum=-1; //number of accel (not necessarily number of accel port on PCB)
				AccelNameNum=-1; //cumulative accel number of all accels on robot
				AnalogSensorNum=-1;  //number of AnalogSensor (not necessarily number of analog sensor port on PCB)
				AnalogSensorNameNum=-1; //cumulative analog sensor number of all analog sensors on robot
				GAMTPNumAccels=RStatus.NumAccelsExpected; //may have found accels in GAMTPs.conf
				GAMTPNumAnalogSensors=RStatus.NumAnalogSensorsExpected; //may have found analog sensors in GAMTPs.conf
				for(pass=0;pass<2;pass++) {
	 
					//pass 0 just gets number of EthAccels PCBs and accels and allocates AccelInfo table, pass 1 allocates and fills individual accels in the AccelInfo table

					if (PStatus.flags&PInfo) {
						fprintf(stderr,"parse config file pass %d\n",pass+1);
					}

					CurLine=0;
					CurChar=0;
					while(CurLine<NumLines) { 

						//get a line from the file text
						Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
						CurChar+=strlen(tline);
						CurLine++;
						if (PStatus.flags&PInfo) {
							fprintf(stderr,"tline=%s",tline);
						}

						if (tline[0]==35) {  //skip lines that start with #
							param=0;
							//fprintf(stderr,"comment\n");
						} else {
							//get a word on this line
							param=strtok(tline,"=\n");
						}
//						fprintf(stderr,"%s\n",param);


#if 0 
						//skip any initial spaces and ignore any comments (#)
						if (tline[0]==35 || tline[0]==10 || tline[0]==13) {
							//skip line
							param=0;
						} else { //if (tline[0]==35 || tline[0]==10 || tline[0]==13) {
							i=0;
							while (tline[i]==	32) { //skip spaces
								i++;
							} 					
						} // if (tline[0]==35 || tline[0]==10 || tline[0]==13) {
#endif	
					//see if this word matches any known section
						match=0;
						i=0;
						while(param!=0 && match==0 && i<NUM_ETHACCELS_CONFIG_SECTIONS) {
	#if Linux
							if (!strcasecmp(param,SectionEA[i])) {
	#endif
	#if WIN32
							if (!_stricmp(param,SectionEA[i])) {
	#endif

								//found a match
								match=1;
								//read in data for this section
								switch(i) {
									case ETHACCELS_CONFIG_NUM_ETHACCELS_PCBS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass) {
											if (strlen(param)>0) { //there is a number given
												RStatus.NumEthAccelsPCBsExpected=atoi(param); //copy
												RStatus.NumAccelsPCBsExpected+=RStatus.NumEthAccelsPCBsExpected;  //same AccelsPCB code is used for both EthAccels and GAMTP PCBs
												//allocate memory for EthAccelsPCBInfoTable
												RStatus.EthAccelsPCBInfo=(EthAccelsPCBInfoTable *)malloc(sizeof(EthAccelsPCBInfoTable)*atoi(param));
												if (RStatus.EthAccelsPCBInfo==0) {
													fprintf(stderr,"Error: Failed to allocate memory for EthAccelsPCBInfo table.\n");
												} else {
													memset(RStatus.EthAccelsPCBInfo,0,sizeof(EthAccelsPCBInfoTable)*atoi(param));
												} 	

												//allocate (or reallocate) memory for array of pointers to AccelsPCBInfoTable (in EthAccels and GAMTP PCBs)
												if (RStatus.AccelsPCBInfo!=0) { //need to reallocate there was already a GAMTP PCB
													RStatus.AccelsPCBInfo=(AccelsPCBInfoTable *)realloc(RStatus.AccelsPCBInfo,sizeof(AccelsPCBInfoTable *)*(atoi(param)+RStatus.NumGAMTPPCBsExpected));
													if (RStatus.AccelsPCBInfo==0) {
														fprintf(stderr,"Error: Failed to reallocate memory for AccelsPCBInfo table.\n");
													} else {
														memset((unsigned char *)RStatus.AccelsPCBInfo+sizeof(AccelsPCBInfoTable *)*RStatus.NumGAMTPPCBsExpected,0,sizeof(AccelsPCBInfoTable *)*atoi(param));
													} 	
												} else {
													RStatus.AccelsPCBInfo=(AccelsPCBInfoTable **)malloc(sizeof(AccelsPCBInfoTable *)*atoi(param));
													if (RStatus.AccelsPCBInfo==0) {
														fprintf(stderr,"Error: Failed to allocate memory for AccelsPCBInfo table.\n");
													} else {
														memset(RStatus.AccelsPCBInfo,0,sizeof(AccelsPCBInfoTable *)*atoi(param));
													} 	
												} 

											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_PCB_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass) {
											if (strlen(param)>0) { //there is a number given
												if (CurEthAccelsPCBNum+1>=RStatus.NumEthAccelsPCBsExpected) {
													fprintf(stderr,"Ignoring EthAccels PCB_NUM %d because NUM_ETHACCELS_PCBS=%d\n",atoi(param),RStatus.NumEthAccelsPCBsExpected);
													ignore=1;
												} else {
													//fprintf(stderr,"reseting ignore NumEthAccelsPCBsExpected=%d PCB_NUM=%d\n",RStatus.NumEthAccelsPCBsExpected,atoi(param));
													ignore=0; //reset ignore in case there are valid EthAccelsPCB entries after EthAccelsPCB with number too high
													PCBNameNum++; //increment total PCB count
													CurEthAccelsPCBNum++;//just increment, ignore user's PCB number //=atoi(param);
													RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].PCBNum=PCBNameNum;  //record PCB NameNum
													strcpy(RStatus.EthPCBInfo[PCBNameNum].Kind,"Accel");  //copy PCB kind

													//shorthand to AccelsPCBInfo
													AccelsPCBInfo=&RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].accels;
													AccelsPCBInfo->Name=RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].Name;//copy pointer to PCB name
													AccelsPCBInfo->PCBNum=PCBNameNum;//copy PCBNum to AccelsPCBInfo

													AccelNum=-1;  //reset accel number
													AnalogSensorNum=-1; //and reset analog sensor number 
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_PCB_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].Name,param,strlen(param)); //copy 
												//copy the address to the EthPCBInfo structure
												RStatus.EthPCBInfo[PCBNameNum].Name=RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].Name;
												//clear Accel and Analog Sensor masks
												AccelsPCBInfo->AccelMask=0; 
												AccelsPCBInfo->AnalogSensorMask=0; 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_PCB_MAC:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//Convert ascii to bytes
												ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
												//copy MAC address to PCB table
												memcpy(RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].MAC,MACInBytes,6);
												//copy the address to the EthPCBInfo structure
												RStatus.EthPCBInfo[PCBNameNum].MAC=RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].MAC;
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_NUM_ACCELS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all accels to find total number of accels in robot
														RStatus.NumAccelsExpected+=atoi(param);
													} else { //if (!pass) {
														NumAccelsOnPCB=atoi(param);
														fprintf(stderr,"EthAccels PCB %d (%s) has %d accels\n",CurEthAccelsPCBNum,RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].Name,NumAccelsOnPCB);
														AccelsPCBInfo->NumAccels=NumAccelsOnPCB;
														//allocate memory for AccelInfoTable within EthAccelsPCB (list of pointers into the quick RStatus.AccelInfo table) - **Note if more accels are found than NUMACCELS- ignore extra accels (otherwise need to realloc)
														AccelsPCBInfo->AccelInfo=(AccelInfoTable **)malloc(sizeof(AccelInfoTable *)*NumAccelsOnPCB); 
														if (AccelsPCBInfo->AccelInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for EthAccelsPCBInfo AccelInfo table of pointers.\n");
														}  else {
															//store pointer to the AccelsPCB group in array of AccelsPCBs
															RStatus.AccelsPCBInfo[CurEthAccelsPCBNum+RStatus.NumGAMTPPCBsExpected]=AccelsPCBInfo;
														} 			
													} //else if (!pass) {
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//Important: If more accels are listed than NUM_ACCELS currently ignore the extra accel, otherwise need to realloc
												AccelNum++;
												if (AccelNum>NumAccelsOnPCB) {
													fprintf(stderr,"Warning: Ignoring extra accelerometer on line %d\n",CurLine);
												} else {  //if (AccelNum>NumAccelsOnPCB) {

													//***Important: Note that ACCEL_NUM given is a port number on the EthAccels and so the local variable AccelNum may be different. So for example NumAccels=3 but Accel num=8 could happen because the motor uses EthAccels port 8.
													//AccelNum++;
													AccelNameNum++;
													RStatus.AccelInfo[AccelNameNum].Num=atoi(param);
													AccelsPCBInfo->AccelInfo[AccelNum]=&RStatus.AccelInfo[AccelNameNum];

													//also set PCBNum in AccelInfo
													RStatus.AccelInfo[AccelNameNum].PCBNum=PCBNameNum;

													//clear calibration floating point values
		//											fprintf(stderr,"%d ",AccelNameNum);
		/* realy mysterious this crashes a malloc later on
													RStatus.AccelInfo[AccelNameNum].Calib.i[0]=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.i[1]=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.y=0.0;
													RStatus.AccelInfo[AccelNameNum].Calib.z=0.0;
		*/

													//Add this accel number to the EthAccels PCB AccelMask
													AccelsPCBInfo->AccelMask|=(1<<atoi(param));
												} //if (AccelNum>NumAccelsOnPCB) {

												//fprintf(stderr,"Found Accel %d\n",AccelNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.AccelInfo[AccelNameNum].Name,param,strlen(param));
												RStatus.AccelInfo[AccelNameNum].Name[strlen(param)]=0;
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_READABLE_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												memcpy(RStatus.AccelInfo[AccelNameNum].ReadableName,param,strlen(param));
												RStatus.AccelInfo[AccelNameNum].ReadableName[strlen(param)]=0;
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_ORIENTATION:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												if (!strcmp(param,"SWAP_Y_AND_MINUS_Z")) {
													RStatus.AccelInfo[AccelNameNum].flags=ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z;
												} 
												if (!strcmp(param,"SWAP_MINUS_Y_AND_MINUS_Z")) {
													RStatus.AccelInfo[AccelNameNum].flags=ACCEL_INFO_TABLE_SWAP_MINUS_Y_AND_MINUS_Z;
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_CALIB_PITCH:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[0]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_CALIB_YAW:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[1]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_CALIB_ROLL:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].Calib.i[2]=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ACCEL_3DMODEL_AXIS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore && (AccelNum<NumAccelsOnPCB)) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AccelInfo[AccelNameNum].ModelAxis=atoi(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_NUM_ANALOG_SENSORS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !ignore) {
											if (strlen(param)>0) { //there is a number given
												if (!ignore) {
													if (!pass) {  //pass0 - add up all motors to find total number of motors in robot
														RStatus.NumAnalogSensorsExpected+=atoi(param);
													} else { //if (!pass)
														fprintf(stderr,"EthAccels PCB %d (%s) has %d analog sensors\n",CurEthAccelsPCBNum,RStatus.EthAccelsPCBInfo[CurEthAccelsPCBNum].Name,atoi(param));
														AccelsPCBInfo->NumAnalogSensors=atoi(param);
														//allocate memory for AccelInfoTable within EthAccelsPCB (list of pointers into the quick RStatus.AccelInfo table)
														AccelsPCBInfo->AnalogSensorInfo=(AnalogSensorInfoTable **)malloc(sizeof(AnalogSensorInfoTable *)*atoi(param));
														if (AccelsPCBInfo->AnalogSensorInfo==0) {
															fprintf(stderr,"Error: Failed to allocate memory for EthAccelsPCBInfo AnalogSensorInfo table of pointers.\n");
														} 
													} //else if (!pass)
												} //if (!ignore)
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ANALOG_SENSOR_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												//***Important: Note that ANALOG_NUM given is a port number on the EthAccels and so the local variable AccelNum may be different. So for example AnalogSensor[0] might have .num=3 because it is located on EthAccels port 3.
												AnalogSensorNum++;
												AnalogSensorNameNum++;

												RStatus.AnalogSensorInfo[AnalogSensorNameNum].Num=atoi(param);
												AccelsPCBInfo->AnalogSensorInfo[AnalogSensorNum]=&RStatus.AnalogSensorInfo[AnalogSensorNameNum];

												//also set PCBNum in AnalogInfo
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].PCBNum=PCBNameNum;

												//Add this analog sensor number to the EthAccels PCB AnalogSensorMask
												AccelsPCBInfo->AnalogSensorMask|=(1<<atoi(param));


												//fprintf(stderr,"Found Analog Sensor num=%d\n",atoi(param));
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ANALOG_SENSOR_KIND:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a value given
												//***Important: Note that ANALOG_NUM given is a port number on the EthAccels and so the local variable AccelNum may be different. So for example AnalogSensor[0] might have .num=3 because it is located on EthAccels port 3.
                        if (!strncmp(param,"TOUCH",5)) {
												  RStatus.AnalogSensorInfo[AnalogSensorNameNum].flags|=ANALOG_SENSOR_TOUCH_SENSOR;
                        }
                        if (!strncmp(param,"POT",3)) {
												  RStatus.AnalogSensorInfo[AnalogSensorNameNum].flags|=ANALOG_SENSOR_POTENTIOMETER;
                        }
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ANALOG_SENSOR_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												memcpy(RStatus.AnalogSensorInfo[AnalogSensorNameNum].Name,param,strlen(param));
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].Name[strlen(param)]=0;
												fprintf(stderr,"Found Analog Sensor: %s (NameNum %d)\n",param,AnalogSensorNameNum);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ANALOG_SENSOR_MINV_ANGLE:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].MinVAngle=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									case ETHACCELS_CONFIG_ANALOG_SENSOR_MAXV_ANGLE:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.AnalogSensorInfo[AnalogSensorNameNum].MaxVAngle=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
									break;
									default:  //most likely will never be reached
										if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
											sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
											//CurLine=NumLines; //exit outer each-line loop
										} 
									break;
								} //switch(i)
							} //if (!_stricmp
							i++;  //advance to next section
						} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {


						if (!match) {
							if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_ETHACCELS_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} //if (param[0]!=35) {//ignore any line starting with #
						} //if (!match) {


					} //while(CurLine<NumLines) 
	
					//Done with pass 0, malloc AccelInfo table for total number of motors
					if (!pass) {
						if (RStatus.NumAccelsExpected>GAMTPNumAccels) { //more accels found after loading any GAMTPs.conf
							if (GAMTPNumAccels>0) {  //there were accels in GAMTPs.conf
								RStatus.AccelInfo=(AccelInfoTable *)realloc(RStatus.AccelInfo,sizeof(AccelInfoTable)*RStatus.NumAccelsExpected);
								if (RStatus.AccelInfo==0) {
									fprintf(stderr,"Error: Failed to reallocate AccelInfoTable\n");
								} else {
									memset((unsigned char *)RStatus.AccelInfo+sizeof(AccelInfoTable)*GAMTPNumAccels,0,sizeof(AccelInfoTable)*(RStatus.NumAccelsExpected-GAMTPNumAccels));
								}
							} else {	
								//no Accels found in GAMTP.conf, just malloc AccelInfoTable
								RStatus.AccelInfo=(AccelInfoTable *)malloc(sizeof(AccelInfoTable)*RStatus.NumAccelsExpected);
								if (RStatus.AccelInfo==0) {
									fprintf(stderr,"Error: Failed to allocate AccelInfoTable\n");
								} else {
									memset(RStatus.AccelInfo,0,sizeof(AccelInfoTable)*RStatus.NumAccelsExpected);
								} 
							} //if (GAMTPNumAccels>0) {
						} else {  //if (RStatus.NumAccelsExpected>0) {
							fprintf(stderr,"NUM_ACCELS found in EthAccels.conf is 0, ignoring rest of accels in EthAccels.conf info\n");
							ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumAccelsExpected>0) {


						if (RStatus.NumAnalogSensorsExpected>GAMTPNumAnalogSensors) { //more Analog Sensors found after loading any GAMTPs.conf
							if (GAMTPNumAnalogSensors>0) {  //there were Analog Sensors in GAMTPs.conf
								RStatus.AnalogSensorInfo=(AnalogSensorInfoTable *)realloc(RStatus.AnalogSensorInfo,sizeof(AnalogSensorInfoTable)*RStatus.NumAnalogSensorsExpected);
								if (RStatus.AnalogSensorInfo==0) {
									fprintf(stderr,"Error: Failed to reallocate AnalogSensorInfoTable\n");
								} else {
									memset((unsigned char *)RStatus.AnalogSensorInfo+sizeof(AnalogSensorInfoTable)*GAMTPNumAnalogSensors,0,sizeof(AnalogSensorInfoTable)*(RStatus.NumAnalogSensorsExpected-GAMTPNumAnalogSensors));
								}
							} else {	
								//no Analog Sensors found in GAMTP.conf, just malloc AccelInfoTable
								RStatus.AnalogSensorInfo=(AnalogSensorInfoTable *)malloc(sizeof(AnalogSensorInfoTable)*RStatus.NumAnalogSensorsExpected);
								if (RStatus.AnalogSensorInfo==0) {
									fprintf(stderr,"Error: Failed to allocate AnalogSensorInfoTable\n");
								} else {
									memset(RStatus.AnalogSensorInfo,0,sizeof(AnalogSensorInfoTable)*RStatus.NumAnalogSensorsExpected);
								} 
							} //if (GAMTPNumAnalogSensors>0) {
						} else {  //if (RStatus.NumAnalogSensorsExpected>0) {
							//fprintf(stderr,"NUM_ANALOG_SENSORS found in EthAccels.conf is 0, ignoring rest of EthAccels.conf info\n");
							//ignore=1; //ignore reading in any motor info
						} //if (RStatus.NumAccelsExpected>0) {
					} //if (!pass) {

				} //for(pass=0;pass<2;pass++) {

				free(buf);


			} else { //if (fptr!=0) {
				fprintf(stderr,"Could not open file %s\n",filename);
				return(0);
			} //else if (fptr!=0) {
		} //} //if (RStatus.NumEthAccelsPCBsExpected>0) {

		//**************************END LOAD ETHACCELS SECTION



		//**************************LOAD ETHPOWER CONFIGURATION
		//Load EthPower.conf

		if (RStatus.NumEthPowerPCBsExpected>0) {

	#if Linux
			sprintf(filename,"%s/%s/EthPower.conf",RStatus.RobotsFolder,RobotName);
	#endif
	#if WIN32
			sprintf(filename,"%s\\%s\\EthPower.conf",RStatus.RobotsFolder,RobotName);
	#endif
	

			fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
			if (fptr!=0) {
				fprintf(stderr,"Loading EthPower.conf for robot %s\n",RobotName);

				//read in the entire file

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


				//set each expected section text
				strcpy(SectionEP[ETHPOWER_CONFIG_NUM_ETHPOWER_PCBS],"NUM_ETHPOWER_PCBS");
				strcpy(SectionEP[ETHPOWER_CONFIG_PCB_NUM],"PCB_NUM");
				strcpy(SectionEP[ETHPOWER_CONFIG_PCB_NAME],"PCB_NAME");
				strcpy(SectionEP[ETHPOWER_CONFIG_PCB_MAC],"PCB_MAC");


				// allocate memory to contain the whole file:
				buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
				if (buf == NULL) {
					sprintf(tstr,"Allocating memory for config file failed.\n");
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					return(0);
				}

				//copy the file into the buffer:
				result=fread(buf,1,fSize,fptr);
				if (result != fSize) {
					sprintf(tstr,"Error reading config file %s\n",filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					fclose(fptr);
					return(0);
				}

				fclose(fptr);
				buf[fSize]=0; //close string of buffer - so strlen will give correct size
				bSize=fSize;



				//go through each line and each word (separated by spaces or a comma)
				//determine total number of lines in order to allocate space for a pointer to each for strtok
				NumLines=0;
				i=0;
				while(i<bSize) {
					if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
						NumLines++;
					}
					i++;
				}  //while(i<bSize)


				ignore=0;
				CurEthPowerPCBNum=-1;

				if (PStatus.flags&PInfo) {
					fprintf(stderr,"parse config file pass %d\n",pass+1);
				}

				CurLine=0;
				CurChar=0;
				while(CurLine<NumLines) { 

					//get a line from the file text
					Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
					CurChar+=strlen(tline);
					CurLine++;
					if (PStatus.flags&PInfo) {
						fprintf(stderr,"tline=%s\n",tline);
					}

					//todo trim any initial spaces
					if (tline[0]==35) {  //skip lines that start with #
						param=0;
						//fprintf(stderr,"comment\n");
					} else {
						//get a word on this line
						param=strtok(tline,"=\n");
					}

					//see if this word matches any known section
					match=0;
					i=0;
					while(param!=0 && match==0 && i<NUM_ETHPOWER_CONFIG_SECTIONS) {
	#if Linux
						if (!strcasecmp(param,SectionEP[i])) {
	#endif
	#if WIN32
						if (!_stricmp(param,SectionEP[i])) {
	#endif

							//found a match
							match=1;
							//read in data for this section
							switch(i) {
								case ETHPOWER_CONFIG_NUM_ETHPOWER_PCBS:
									param=strtok(NULL,"#\n"); //get next token
									if (param!=0) {
										if (strlen(param)>0) { //there is a number given
											if (PCBNameNum+1>=RStatus.NumEthPCBsExpected) {
												//Too many PCBs in .conf files- alternative- just increase number of PCBs- but would then need to realloc memory
												fprintf(stderr,"Ignoring EthPower PCBs because number of PCBs in .conf files (%d) >= NUM_PCBs in %s.Robot (%d)\n",PCBNameNum+1,RStatus.CurrentRobotName,RStatus.NumEthPCBsExpected);
												ignore=1;
											} else {

												RStatus.NumEthPowerPCBsExpected=atoi(param);
												//allocate memory for EthPowerPCBInfoTable
												RStatus.EthPowerPCBInfo=(EthPowerPCBInfoTable *)malloc(sizeof(EthPowerPCBInfoTable)*atoi(param));
												if (RStatus.EthPowerPCBInfo==0) {
													fprintf(stderr,"Error: Failed to allocate memory for EthPowerPCBInfo table.\n");
												} else {
													memset(RStatus.EthPowerPCBInfo,0,sizeof(EthPowerPCBInfoTable)*atoi(param));
												}	
											} //if (CurEthPCBNum+1>=RStatus.NumEthPCBsExpected) {
										} //if (strlen(param)>0) { 
									} //if (param!=0) {
									i=NUM_ETHPOWER_CONFIG_SECTIONS;  //exit while loop
								break;
								case ETHPOWER_CONFIG_PCB_NUM:
									param=strtok(NULL,"#\n"); //get next token
									if (param!=0) {
										if (strlen(param)>0) { //there is a number given
											if (CurEthPowerPCBNum+1>=RStatus.NumEthPowerPCBsExpected) {
												fprintf(stderr,"Ignoring EthPower PCB_NUM %d because NUM_ETHPOWER_PCBS=%d\n",atoi(param),RStatus.NumEthPowerPCBsExpected);
												ignore=1;
											} else {
												//fprintf(stderr,"reseting ignore NumEthAccelsPCBsExpected=%d PCB_NUM=%d\n",RStatus.NumEthAccelsPCBsExpected,atoi(param));
												ignore=0; //reset ignore in case there are valid EthAccelsPCB entries after EthAccelsPCB with number too high
												PCBNameNum++; //increment total PCB count
												CurEthPowerPCBNum++;//just increment, ignore user's PCB number //=atoi(param);
												RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].PCBNum=PCBNameNum;  //record PCB NameNum
												strcpy(RStatus.EthPCBInfo[PCBNameNum].Kind,"Power");  //copy PCB kind

												//AccelNum=-1;  //reset accel number
												//AnalogSensorNum=-1; //and reset analog sensor number 
											} 
										} //if (strlen(param)>0) { 
									} //if (param!=0) {
									i=NUM_ETHPOWER_CONFIG_SECTIONS;  //exit while loop
								break;
								case ETHPOWER_CONFIG_PCB_NAME:
									param=strtok(NULL,"#\n"); //get next token
									if (param!=0 && !ignore) {
										if (strlen(param)>0) { //there is a number given
											memcpy(RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].Name,param,strlen(param)); //copy 
											RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].Name[strlen(param)]=0; 
											//copy the address to the EthPCBInfo structure
											RStatus.EthPCBInfo[PCBNameNum].Name=RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].Name;
										} //if (strlen(param)>0) { 
									} //if (param!=0) {
									i=NUM_ETHPOWER_CONFIG_SECTIONS;  //exit while loop
								break;
								case ETHACCELS_CONFIG_PCB_MAC:
									param=strtok(NULL,"#\n"); //get next token
									if (param!=0 && !ignore) {
										if (strlen(param)>0) { //there is a number given
											//Convert ascii to bytes
											ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
											//copy MAC address to PCB table
											memcpy(RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].MAC,MACInBytes,6);
											//copy the address to the EthPCBInfo structure
											RStatus.EthPCBInfo[PCBNameNum].MAC=RStatus.EthPowerPCBInfo[CurEthPowerPCBNum].MAC;
										} //if (strlen(param)>0) { 
									} //if (param!=0) {
									i=NUM_ETHPOWER_CONFIG_SECTIONS;  //exit while loop
								break;
							} //switch(i)
						} //if (!_stricmp
						i++;  //advance to next section
					} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {



					if (!match) {
						if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
							sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
							FTMessageBox(tstr,FTMB_OK,"Error",0);
							i=NUM_ETHPOWER_CONFIG_SECTIONS;  //exit while loop
							//CurLine=NumLines; //exit outer each-line loop
						} //if (param[0]!=35) {//ignore any line starting with #
					} //if (!match) {


				} //while(CurLine<NumLines) 

				free(buf);


			} else { //if (fptr!=0) {
				fprintf(stderr,"Could not open file %s\n",filename);
				return(0);
			} //else if (fptr!=0) {
		} //if (RStatus.NumEthPowerPCBsExpected>0) {

		//**************************END LOAD ETHPOWER CONFIGURATION




		//**************************LOAD MOTOR-ANGLE ASSOCIATIONS
		//Load MotorAngle.conf


#if Linux
		sprintf(filename,"%s/%s/MotorAngle.conf",RStatus.RobotsFolder,RobotName);
#endif
#if WIN32
		sprintf(filename,"%s\\%s\\MotorAngle.conf",RStatus.RobotsFolder,RobotName);
#endif

		//see if file exists
		if( access( filename,F_OK ) != -1) {

			//Make 2 passes, 1: Determine total number of PCBs and motors in order to allocate all memory first
	
			fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
			if (fptr!=0) {
				fprintf(stderr,"Loading MotorAngle Associations (MotorAngle.conf) for robot %s\n",RobotName);

				//read in the entire file

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

				//set default values
				DefaultMotionThreshold=DEFAULT_MOTION_THRESHOLD;//1.0;
				DefaultMinVelocity=DEFAULT_MIN_VELOCITY;//1.0;
				DefaultMaxVelocity=DEFAULT_MAX_VELOCITY;//5.0;
				DefaultMaxAccel=DEFAULT_MAX_ACCEL;//0.5;
				DefaultTVR=DEFAULT_TVR;//1.0;

				//set each expected section text
				//strcpy(SectionMA[MOTOR_ANGLE_CONFIG_NUM_ASSOCIATIONS],"NUM_ASSOCIATIONS");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_DEFAULT_MOTION_THRESHOLD],"DEFAULT_MOTION_THRESHOLD");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_DEFAULT_MIN_VELOCITY],"DEFAULT_MIN_VELOCITY");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_DEFAULT_MAX_VELOCITY],"DEFAULT_MAX_VELOCITY");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_DEFAULT_MAX_ACCEL],"DEFAULT_MAX_ACCEL");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_DEFAULT_THRUST_VELOCITY_RATIO],"DEFAULT_THRUST_VELOCITY_RATIO");
				//strcpy(SectionMA[MOTOR_ANGLE_CONFIG_ASSOCIATION_NUM],"ASSOCIATION_NUM");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_MOTOR_NAME],"MOTOR_NAME");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_ACCEL_NAME],"ACCEL_NAME");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_ANALOG_SENSOR_NAME],"ANALOG_SENSOR_NAME");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_ANGLE_XYZ],"ANGLE_XYZ");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_INC_ANG_DIR],"INC_ANG_DIR");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_MOTION_THRESHOLD],"MOTION_THRESHOLD");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_MIN_VELOCITY],"MIN_VELOCITY");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_MAX_VELOCITY],"MAX_VELOCITY");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_MAX_ACCEL],"MAX_ACCEL");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_THRUST_VELOCITY_RATIO],"THRUST_VELOCITY_RATIO");
				strcpy(SectionMA[MOTOR_ANGLE_CONFIG_FLAGS],"FLAGS");

				// allocate memory to contain the whole file:
				buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
				if (buf == NULL) {
					sprintf(tstr,"Allocating memory for config file failed.\n");
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					return(0);
				}

				//copy the file into the buffer:
				result=fread(buf,1,fSize,fptr);
				if (result != fSize) {
					sprintf(tstr,"Error reading config file %s\n",filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					fclose(fptr);
					return(0);
				}

				fclose(fptr);
				buf[fSize]=0; //close string of buffer - so strlen will give correct size
				bSize=fSize;



				//go through each line and each word (separated by spaces or a comma)
				//determine total number of lines in order to allocate space for a pointer to each for strtok
				NumLines=0;
				i=0;
				while(i<bSize) {
					if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
						NumLines++;
					}
					i++;
				}  //while(i<bSize)


				ignore=0;
				RStatus.NumMotorAngleAssociations=0;
				CurMotorAngle=-1;
				for(pass=0;pass<2;pass++) {
	 
					//pass 0 just gets number of EthAccels PCBs and accels and allocates AccelInfo table, pass 1 allocates and fills individual accels in the AccelInfo table

					if (PStatus.flags&PInfo) {
						fprintf(stderr,"parse config file pass %d\n",pass+1);
					}



					CurLine=0;
					CurChar=0;
					while(CurLine<NumLines) { 

						//get a line from the file text
						Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
						CurChar+=strlen(tline);
						CurLine++;
						if (PStatus.flags&PInfo) {
							fprintf(stderr,"tline=%s\n",tline);
						}

						//todo trim any initial spaces
						//get a word on this line
						param=strtok(tline,"="); 
						//see if this word matches any known section
						match=0;
						i=0;
						while(param!=0 && match==0 && i<NUM_MOTOR_ANGLE_CONFIG_SECTIONS) {
		#if Linux
							if (!strcasecmp(param,SectionMA[i])) {
		#endif
		#if WIN32
							if (!_stricmp(param,SectionMA[i])) {
		#endif

								//found a match
								match=1;
								//read in data for this section
								switch(i) {
#if 0 
									case MOTOR_ANGLE_CONFIG_NUM_ASSOCIATIONS:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0) {
											if (strlen(param)>0) { //there is a number given
												RStatus.NumMotorAngleAssociations=atoi(param); //copy
												//allocate memory for EthMotorsPCBInfoTable
												RStatus.MotorAngle=(MotorAngleAssociation *)malloc(sizeof(MotorAngleAssociation)*atoi(param));
												if (RStatus.MotorAngle==0) {
													fprintf(stderr,"Error: Failed to allocate memory for MotorAngle Assocation table.\n");
												} else {
													memset(RStatus.MotorAngle,0,sizeof(MotorAngleAssociation)*atoi(param));
												}	
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_ASSOCIATION_NUM:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 ) {
											if (strlen(param)>0) { //there is a number given
												if (atoi(param)>=RStatus.NumMotorAngleAssociations) {
													fprintf(stderr,"Motor-Angle Association %d is >= Total number of Motor-Angle associations given (%d)\n",atoi(param),RStatus.NumMotorAngleAssociations);
													ignore=1;
												} else {
													ignore=0; //reset ignore in case there are valid assocations later on
													CurMotorAngle++;//just increment, ignore user's association number //=atoi(param);
													//Fill in default values for MinVelocity, MaxVelocity, etc. 
													RStatus.MotorAngle[CurMotorAngle].MotionThreshold=DefaultMotionThreshold;
													RStatus.MotorAngle[CurMotorAngle].MinVelocity=DefaultMinVelocity;
													RStatus.MotorAngle[CurMotorAngle].MaxVelocity=DefaultMaxVelocity;
													RStatus.MotorAngle[CurMotorAngle].MaxAccel=DefaultMaxAccel;
													RStatus.MotorAngle[CurMotorAngle].ThrustVelocityRatio=DefaultTVR;
												} //if (atoi(param)>=RStatus.NumMotorAngleAssociations) {
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
#endif
									case MOTOR_ANGLE_CONFIG_MOTOR_NAME:  //MOTOR_NAME defines a new MotorAngle Assocation
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !ignore) {
											if (strlen(param)>0) { //there is a name given
												if (pass==0) { //first pass
													RStatus.NumMotorAngleAssociations++; //increase number of MotorAngleAssocations found
												} else { //pass=1 already allocated MotorAssocation table
													//set default values
													CurMotorAngle++;//just increment, ignore user's association number //=atoi(param);
													//Fill in default values for MinVelocity, MaxVelocity, etc. 
													RStatus.MotorAngle[CurMotorAngle].MotionThreshold=DefaultMotionThreshold;
													RStatus.MotorAngle[CurMotorAngle].MinVelocity=DefaultMinVelocity;
													RStatus.MotorAngle[CurMotorAngle].MaxVelocity=DefaultMaxVelocity;
													RStatus.MotorAngle[CurMotorAngle].MaxAccel=DefaultMaxAccel;
													RStatus.MotorAngle[CurMotorAngle].ThrustVelocityRatio=DefaultTVR;


		//										memcpy(RStatus.MotorAngle[CurMotorAngle].MotorName,param,strlen(param)); //copy 
													RStatus.MotorAngle[CurMotorAngle].MotorNameNum=GetMotorNameNum(param);
													if (RStatus.MotorAngle[CurMotorAngle].MotorNameNum==-1) {
														fprintf(stderr,"No Name number for Motor with name '%s' on line %d - probably wrong motor name.\n",param,CurLine);
													} 
												} //if (pass==0) { //first pass
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_ACCEL_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												//memcpy(RStatus.MotorAngle[CurMotorAngle].AccelName,param,strlen(param)); //copy 
												RStatus.MotorAngle[CurMotorAngle].AccelNameNum=GetAccelNameNum(param);
												if (RStatus.MotorAngle[CurMotorAngle].AccelNameNum==-1) {
													fprintf(stderr,"No Name number for Accelerometer with name '%s' on line %d\n",param,CurLine);
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_ANALOG_SENSOR_NAME:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												//memcpy(RStatus.MotorAngle[CurMotorAngle].AccelName,param,strlen(param)); //copy 
												//RStatus.MotorAngle[CurMotorAngle].AccelNameNum=GetAccelNameNum(param);
                        //set flag indicating that this motor-angle association uses a potentiometer, not an accelerometer 
                        RStatus.MotorAngle[CurMotorAngle].AnalogSensorNameNum=GetAnalogSensorNameNum(param);
												if (RStatus.MotorAngle[CurMotorAngle].AnalogSensorNameNum==-1) {
													fprintf(stderr,"No Name number for Analog Sensor with name '%s' on line %d\n",param,CurLine);
												} 
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_ANGLE_XYZ:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												if (param[0]==88 || param[0]==89 || param[0]==90) {  //X,Y, or Z
													RStatus.MotorAngle[CurMotorAngle].xyz=(int)param[0]-88;	
													//fprintf(stderr,"Param[0]=X Y or Z %d\n",(int)param[0]);
												} else {
													if (param[0]==120 || param[0]==121 || param[0]==122) {  //x,y,or z
														RStatus.MotorAngle[CurMotorAngle].xyz=(int)param[0]-120;	
														//fprintf(stderr,"Param[0]=x y or z %d\n",(int)param[0]);
													} else {
														RStatus.MotorAngle[CurMotorAngle].xyz=atoi(param); //is number 0,1 or 2
													}
												} //if (param=='X' || param=='Y' || param='Z') {
												if (RStatus.MotorAngle[CurMotorAngle].xyz<0 || RStatus.MotorAngle[CurMotorAngle].xyz>2) {
													fprintf(stderr,"Accelerometer dimension needs to be x,y, or z or 0,1, or 2) on line %d of MotorAngle.conf\n",CurLine);
												}
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_INC_ANG_DIR:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a name given
												if (!_stricmp(param,"CCW")) {
													RStatus.MotorAngle[CurMotorAngle].IncAngDir=MOTOR_COUNTER_CLOCKWISE; //-1
												} else {
													RStatus.MotorAngle[CurMotorAngle].IncAngDir=MOTOR_CLOCKWISE; //1
												}
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_DEFAULT_MOTION_THRESHOLD:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass && !ignore) {  //defaults are set on pass 0 
											if (strlen(param)>0) { //there is a number given
												DefaultMotionThreshold=atof(param); //copy
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_DEFAULT_MIN_VELOCITY:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												DefaultMinVelocity=atof(param); //copy
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_DEFAULT_MAX_VELOCITY:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												DefaultMaxVelocity=atof(param); //copy
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_DEFAULT_MAX_ACCEL:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												DefaultMaxAccel=atof(param); //copy
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_DEFAULT_THRUST_VELOCITY_RATIO:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && !pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												DefaultTVR=atof(param); //copy
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_MOTION_THRESHOLD:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorAngle[CurMotorAngle].MotionThreshold=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_MIN_VELOCITY:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorAngle[CurMotorAngle].MinVelocity=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_MAX_VELOCITY:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorAngle[CurMotorAngle].MaxVelocity=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_MAX_ACCEL:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorAngle[CurMotorAngle].MaxAccel=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_THRUST_VELOCITY_RATIO:
										param=strtok(NULL,"#\n"); //get next token
										if (param!=0 && pass && !ignore) {
											if (strlen(param)>0) { //there is a number given
												RStatus.MotorAngle[CurMotorAngle].ThrustVelocityRatio=atof(param);
											} //if (strlen(param)>0) { 
										} //if (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									case MOTOR_ANGLE_CONFIG_FLAGS:
										param=strtok(NULL,"#|\n"); //get next token
										while (param!=0 && pass && !ignore) {
											if (!_stricmp(param,"NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE")) {
												RStatus.MotorAngle[CurMotorAngle].flags|=NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE;
											}
											if (!_stricmp(param,"IGNORE_GRAVITY")) {
												RStatus.MotorAngle[CurMotorAngle].flags|=MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY;
											}
											param=strtok(NULL,"#|\n"); //get next token
										} //while (param!=0) {
										i=NUM_MOTOR_ANGLE_CONFIG_SECTIONS;  //exit while loop
									break;
									default:  //most likely will never be reached
										if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
											sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
											i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
											//CurLine=NumLines; //exit outer each-line loop
										} 
									break;
								} //switch(i)
							} //if (!_stricmp
							i++;  //advance to next section
						} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {


						if (!match) {
							if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_ETHMOTORS_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} //if (param[0]!=35) {//ignore any line starting with #
						} //if (!match) {


					} //while(CurLine<NumLines) 


					//Done with pass 0, malloc MotorAngle table for total number of MotorAngle Associations
					if (!pass) {
						if (RStatus.NumMotorAngleAssociations>0) {
							RStatus.MotorAngle=(MotorAngleAssociation *)malloc(sizeof(MotorAngleAssociation)*RStatus.NumMotorAngleAssociations);
							if (RStatus.MotorAngle==0) {
								fprintf(stderr,"Error: Failed to allocate MotorAngleAssociation Table\n");
							} else {
								memset(RStatus.MotorAngle,0,sizeof(MotorAngleAssociation)*RStatus.NumMotorAngleAssociations);
								fprintf(stderr,"Found %d MotorAngle Assocations in MotorAngle.conf\n",RStatus.NumMotorAngleAssociations);
							} 
						} else {  //if (RStatus.NumMotorAngleAssociations>0) {
							fprintf(stderr,"No Motor Angle Associations found in MotorAngle.conf\n");
							ignore=1; //ignore reading in any motor angle info
						} //if (RStatus.NumAccelsExpected>0) {
					} //if (!pass) {

				} //for(pass=0;pass<2;pass++) {


				free(buf);


			} else { //if (fptr!=0) {
				fprintf(stderr,"Could not open file %s\n",filename);
				return(0);
			} //else if (fptr!=0) {
		} //if( access( filename,F_OK ) != -1) {


		//**************************END LOAD MOTOR-ANGLE ASSOCIATIONS


		//**************************LOAD MODEL CONFIGURATION
		//Load Model.conf if one exists

		if (RStatus.CurrentRobotModelFile[0]!=0) {



			//fprintf(stderr,"Loading MotorAngle Associations (MotorAngle.conf) for robot %s\n",RobotName);

	#if Linux
			sprintf(filename,"%s/%s/Model.conf",RStatus.RobotsFolder,RobotName);
	#endif
	#if WIN32
			sprintf(filename,"%s\\%s\\Model.conf",RStatus.RobotsFolder,RobotName);
	#endif
	

			if(access(filename,F_OK ) != -1) {



				fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
				if (fptr!=0) {
					fprintf(stderr,"Loading Robot Model configuration file (Model.conf) for robot %s\n",RobotName);

					//read in the entire file

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
					buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
					if (buf == NULL) {
						sprintf(tstr,"Allocating memory for config file failed.\n");
						FTMessageBox(tstr,FTMB_OK,"Error",0);
						return(0);
					}

					//copy the file into the buffer:
					result=fread(buf,1,fSize,fptr);
					if (result != fSize) {
						sprintf(tstr,"Error reading config file %s\n",filename);
						FTMessageBox(tstr,FTMB_OK,"Error",0);
						fclose(fptr);
						return(0);
					}

					fclose(fptr);
					buf[fSize]=0; //close string of buffer - so strlen will give correct size
					bSize=fSize;


					RStatus.ModelConf.NumEntries=0;
					//Make two passes pass=0: determine number of entries, rows, and columns
					for(pass=0;pass<2;pass++) {

						//go through each line and each word (separated by spaces or a comma)
						//determine total number of lines in order to allocate space for a pointer to each for strtok
						NumLines=0;
						i=0;
						while(i<bSize) {
							if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
								NumLines++;
							}
							i++;
						}  //while(i<bSize)


						ignore=0;
						if (PStatus.flags&PInfo) {
							fprintf(stderr,"parse config file pass %d\n",pass+1);
						}

						CurLine=0;
						CurChar=0;
						while(CurLine<NumLines) { 

							//get a line from the file text
							Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
							CurChar+=strlen(tline);
							CurLine++;
							if (PStatus.flags&PInfo) {
								fprintf(stderr,"tline=%s\n",tline);
							}

							//todo trim any initial spaces
							//get a word on this line
							param=strtok(tline,"="); 
							//see if this word matches any known section
							match=0;
							i=0;
							if (param!=0) {
								//determine which field is on the current line
								if (!strcmp(param,"COLUMN_WIDTH")) {
									match=1;
									param=strtok(NULL,"#\n"); //get next token
									if (param!=0) {
										RStatus.ModelConf.ColumnWidth=atoi(param);
									} //if (param!=0) {
								} else { //if (!strcmp(param,"COLUMN_WIDTH")) {

									if (param[0]>=48 && param[0]<=57) { //if starts with number, must be regular entry
								
										if (!pass) {
											RStatus.ModelConf.NumEntries++;
										} else { //if (!pass) {
											//get column and row (col,row)
											strcpy(tline2,param); 
											i=GetWordLen(tline2,44); //44=comma
											tline2[i]=0; //terminate string after 1st number
											RStatus.ModelConf.Entry[CurEntry].Column=atoi(tline2);
											RStatus.ModelConf.Entry[CurEntry].Row=atoi(&param[i+1]);
											//get Motor, Accel, or analog sensor name(s)
											param=strtok(NULL,"#\n"); //get next token
											//analog sensors can be divided by |, but for now leave in the string
											if (param==0) {
												fprintf(stderr,"Error on line %d of %s\n",CurLine,filename);
											} else {
												strcpy(RStatus.ModelConf.Entry[CurEntry].Name,param);  //just copy accel, model or analog sensor name
											} 
											CurEntry++;
										} //if (!pass) {
									} else { //if (param[0]>=48 && param[0]<=57) { //if starts with number, must be regular entry
										if (param[0]!=35 && param[0]!=10 && param[0]!=13 && param[0]!=32) {
											sprintf(tstr,"Unrecognized entry '%s' on line %d in config file %s",param,CurLine,filename);
											FTMessageBox(tstr,FTMB_OK,"Error",0);
										} else { //if (param[0]!=35...
											match=1;
										} //if (param[0]!=35...
									} //if (param[0]>=48 && param[0]<=57) { //if starts with number, must be regular entry
								} //else if (!strcmp(param,"COLUMN_WIDTH")) {

							} //if (param!=0) {

						} //while(CurLine<NumLines) 
				
						if (!pass) {
							//allocate memory for Model.conf entries
							if (RStatus.ModelConf.NumEntries>0) {
								RStatus.ModelConf.Entry=(ModelConfTableEntry *)malloc(sizeof(ModelConfTableEntry)*RStatus.ModelConf.NumEntries);
								CurEntry=0;
							} else {
								fprintf(stderr,"No Model.conf entries recognized\n");
							} //if (RStatus.ModeConf.NumEntries>0) {
						} //if (!pass) {

					} //for(pass=0;pass<2;pass++) {

					free(buf);


				} else { //if (fptr!=0) {
					fprintf(stderr,"Could not open file %s\n",filename);
					return(0);
				} //else if (fptr!=0) {
			} //if(access(filename,F_OK ) != -1) {
		} //if (RStatus.CurrentRobotModelFile[0]!=0) {


		//**************************END LOAD MODEL CONFIGURATION FILE


		//**************************LOAD ACCEL, GYRO, MAGNET, ANALOG CALIBRATION OFFSETS
		//LOAD ACCEL GYRO AND MAG CALIBRATION OFFSETS FROM FILE IF THEY EXIST
		//read in accel.conf, gyro.conf, mag.conf if any exist and fill CaliFile[NUM_ETHACCEL_PCBS] entries
	//	memset(CaliFile, 0, sizeof(CalibrationFile)*NUM_ETHACCEL_PCBS);
	
		//check if accel.conf exists in Robot home folder

	//	sprintf(filename, "%saccel.conf", PStatus.ProjectPath);
#if Linux
		sprintf(filename, "%s/accel.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(filename, "%s\\accel.conf", RStatus.CurrentRobotFolder);
#endif
		LoadCalibrationFileAccel(filename);

#if Linux
		sprintf(filename, "%s/analog.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(filename, "%s\\analog.conf", RStatus.CurrentRobotFolder);
#endif
		LoadCalibrationFileAnalog(filename);

		//check if gyro.conf exists in Robot home folder
#if Linux
		sprintf(filename, "%s/gyro.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(filename, "%s\\gyro.conf", RStatus.CurrentRobotFolder);
#endif
		LoadCalibrationFileGyro(filename);

#if Linux
		sprintf(filename, "%s/mag.conf", RStatus.CurrentRobotFolder);
#endif
#if WIN32
		sprintf(filename, "%s\\mag.conf", RStatus.CurrentRobotFolder);
#endif
		LoadCalibrationFileMag(filename);

		//**************************END LOAD ACCEL, GYRO, MAGNET, ANALOG CALIBRATION OFFSETS



		RStatus.flags|=ROBOT_STATUS_USER_ROBOT_LOADED;

		//Run an identication to connect any existing MAC_Connections to the correct motors
		SendBroadcastInstruction();


		if (PStatus.flags&PInfo) {
			fprintf(stderr,"End LoadRobot\n");
		}
		return(1);
	} else { //if (fptr!=0) {
		fprintf(stderr,"Could not open file %s\n",filename);
		return(0);
	} //else if (fptr!=0) {

} //int LoadRobot(char *RobotName)


//Unload Currently loaded Robot and free all reserved memory
int UnloadRobot()
{
	int i;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"UnloadRobot\n");
	}

	//fprintf(stderr,"Unloading currently loaded robot %s\n",RStatus.RobotReadableName);

	//UNLOAD PCBINFO TABLE
	if (RStatus.EthPCBInfo!=0) {
		free(RStatus.EthPCBInfo);
		RStatus.EthPCBInfo=0;
	}

	//UNLOAD ETHMOTORS
	//for each expected EthMotors PCB, free all MotorInfoTables
	for(i=0;i<RStatus.NumEthMotorsPCBsExpected;i++) {
		if (RStatus.EthMotorsPCBInfo[i].motors.MotorInfo!=0) {
			free(RStatus.EthMotorsPCBInfo[i].motors.MotorInfo);
			RStatus.EthMotorsPCBInfo[i].motors.MotorInfo=0;
		}			
	} //for i	

	//free the EthMotorsPCBInfo table
	if (RStatus.EthMotorsPCBInfo!=0) {
		free(RStatus.EthMotorsPCBInfo);
		RStatus.NumEthMotorsPCBsExpected=0;
	} 

	//free the MotorInfo quick lookup pointer table
	if (RStatus.MotorInfo!=0) {
		free(RStatus.MotorInfo);
		RStatus.MotorInfo=0;
	} 

	//UNLOAD ETHACCELS
	//for each expected EthAccels PCB, free all AccelInfo and AnalogSensorInfo memory
	for(i=0;i<RStatus.NumEthAccelsPCBsExpected;i++) {
		if (RStatus.EthAccelsPCBInfo[i].accels.AccelInfo!=0) {
			free(RStatus.EthAccelsPCBInfo[i].accels.AccelInfo);
			RStatus.EthAccelsPCBInfo[i].accels.AccelInfo=0;
		}			
		if (RStatus.EthAccelsPCBInfo[i].accels.AnalogSensorInfo!=0) {
			free(RStatus.EthAccelsPCBInfo[i].accels.AnalogSensorInfo);
			RStatus.EthAccelsPCBInfo[i].accels.AnalogSensorInfo=0;
		}			
	} //for i	

	//UNLOAD GAMTPs
	//for each expected GAMTP PCB, free all AccelInfo and AnalogSensorInfo memory
	for(i=0;i<RStatus.NumGAMTPPCBsExpected;i++) {
		if (RStatus.GAMTPPCBInfo[i].accels.AccelInfo!=0) {
			free(RStatus.GAMTPPCBInfo[i].accels.AccelInfo);
			RStatus.GAMTPPCBInfo[i].accels.AccelInfo=0;
		}			
		if (RStatus.GAMTPPCBInfo[i].accels.AnalogSensorInfo!=0) {
			free(RStatus.GAMTPPCBInfo[i].accels.AnalogSensorInfo);
			RStatus.GAMTPPCBInfo[i].accels.AnalogSensorInfo=0;
		}			
	} //for i	


	//free the EthAccelsPCBInfo table
	if (RStatus.EthAccelsPCBInfo!=0) {
		free(RStatus.EthAccelsPCBInfo);
		RStatus.NumEthAccelsPCBsExpected=0;
	} 

	//free the AccelsPCBInfo table
	if (RStatus.AccelsPCBInfo!=0) {
		free(RStatus.AccelsPCBInfo);
		RStatus.NumAccelsPCBsExpected=0;
	} 

	//free the AccelInfo quick lookup table
	if (RStatus.AccelInfo!=0) {
		free(RStatus.AccelInfo);
		RStatus.AccelInfo=0;
	} 

	//free the AnalogSensorInfo quick lookup table
	if (RStatus.AnalogSensorInfo!=0) {
		free(RStatus.AnalogSensorInfo);
		RStatus.AnalogSensorInfo=0;
	} 

	//UNLOAD ETHPOWER
	//free the EthPowerPCBInfo table
	if (RStatus.EthPowerPCBInfo!=0) {
		free(RStatus.EthPowerPCBInfo);
		RStatus.NumEthPowerPCBsExpected=0;
	} 



	//UNLOAD MOTOR-ANGLE ASSOCIATIONS
	//free the MotorAngle table
	if (RStatus.MotorAngle!=0) {
		free(RStatus.MotorAngle);
		RStatus.MotorAngle=0;
		RStatus.NumMotorAngleAssociations=0;
	} 

	//UNLOAD MODEL.CONF ENTRIES
	if (RStatus.ModelConf.NumEntries>0 && RStatus.ModelConf.Entry!=0) {
		free(RStatus.ModelConf.Entry);
		memset(&RStatus.ModelConf,0,sizeof(ModelConfTable));
	} 


	RStatus.CurrentRobotFolder[0]=0;
	RStatus.CurrentRobotLogFolder[0]=0;
	RStatus.CurrentRobotModelLogFolder[0]=0;
	RStatus.CurrentRobotScriptFolder[0]=0;
	RStatus.CurrentRobotModelFile[0]=0;
	RStatus.CurrentRobotName[0]=0;
	RStatus.RobotReadableName[0]=0;
	RStatus.NumEthPCBsExpected=0;

	RStatus.flags&=~ROBOT_STATUS_USER_ROBOT_LOADED;

	fprintf(stderr,"Unloaded currently loaded robot %s\n",RStatus.RobotReadableName);

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"End UnloadRobot\n");
	}
	return(1);
} //int UnloadRobot()


//Load the Robot app configuration file
int LoadRobotAppConfigFile(void)
{
	FILE *fptr;
	char tstr[FTMedStr];
	char Section[NUM_ROBOT_APP_CONFIG_SECTIONS][FTMedStr];
	char tline[FTMedStr],filename[FTMedStr];
	int match; //CurSection
	int bSize,result;
	int64_t fSize;
	char *buf,*param;//,*holdptr;
	int NumLines,CurLine,i,CurChar,AccelNum,AccelDim;
	unsigned char MACInBytes[6]; 

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"LoadRobotAppConfigFile\n");
	}


	//set some defaults in case they are not in config:
//	getcwd(RStatus.LogFolder,sizeof(RStatus.LogFolder));
	sprintf(RStatus.LogFolder,"%slogs",PStatus.ProjectPath);
#if Linux
	sprintf(RStatus.ModelLogFolder,"%slogs/model",PStatus.ProjectPath);
#endif
#if WIN32
	sprintf(RStatus.ModelLogFolder,"%slogs\\model",PStatus.ProjectPath);
#endif


	//delete any log files older than 7 days
//	DeleteOldLogs();


	sprintf(RStatus.ScriptFolder,"%sscripts",PStatus.ProjectPath);

	sprintf(filename,"%sRobotApp.conf",PStatus.ProjectPath);
	fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
	if (fptr!=0) {
		//read in the entire file

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
		buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
		if (buf == NULL) {
			sprintf(tstr,"Allocating memory for config file failed.\n");
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			return(0);
		}

		//copy the file into the buffer:
		result=fread(buf,1,fSize,fptr);
		if (result != fSize) {
			sprintf(tstr,"Error reading config file %s\n",filename);
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			fclose(fptr);
			return(0);
		}

		fclose(fptr);
		buf[fSize]=0; //close string of buffer - so strlen will give correct size
		bSize=fSize;

#if 0 
		strcpy(Section[ROBOT_APP_CONFIG_ETHMOTORS_LOWER_BODY],"ETHMOTORS_LOWER_BODY");
		strcpy(Section[ROBOT_APP_CONFIG_ETHMOTORS_UPPER_BODY],"ETHMOTORS_UPPER_BODY");
		strcpy(Section[ROBOT_APP_CONFIG_ETHMOTORS_LEFT_HAND], "ETHMOTORS_LEFT_HAND");
		strcpy(Section[ROBOT_APP_CONFIG_ETHMOTORS_RIGHT_HAND], "ETHMOTORS_RIGHT_HAND");
		strcpy(Section[ROBOT_APP_CONFIG_ETHACCELS_LEFT_LEG],"ETHACCELS_LEFT_LEG");
		strcpy(Section[ROBOT_APP_CONFIG_ETHACCELS_RIGHT_LEG],"ETHACCELS_RIGHT_LEG");
		strcpy(Section[ROBOT_APP_CONFIG_ETHACCELS_TORSO],"ETHACCELS_TORSO");
		strcpy(Section[ROBOT_APP_CONFIG_ETHACCELS_LEFT_ARM],"ETHACCELS_LEFT_ARM");
		strcpy(Section[ROBOT_APP_CONFIG_ETHACCELS_RIGHT_ARM],"ETHACCELS_RIGHT_ARM");
		strcpy(Section[ROBOT_APP_CONFIG_ETHPOWER],"ETHPOWER");
#endif
		strcpy(Section[ROBOT_APP_CONFIG_ROBOTS_FOLDER],"ROBOTS_FOLDER");
		strcpy(Section[ROBOT_APP_CONFIG_DEFAULT_ROBOT],"DEFAULT_ROBOT");

		strcpy(Section[ROBOT_APP_CONFIG_LOG_FOLDER],"LOG_FOLDER");
		strcpy(Section[ROBOT_APP_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS],"DELETE_LOGS_OLDER_THAN_DAYS");

#if 0 
		//User sets orientation of motors, "CW" if turning motor CW increases associated angle (X,Y, or Z) of lower body segment
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_FOOT],"MOTOR_LEFT_FOOT");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_ANKLE],"MOTOR_LEFT_ANKLE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_KNEE],"MOTOR_LEFT_KNEE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_LEG],"MOTOR_LEFT_LEG");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_SIDE],"MOTOR_LEFT_SIDE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_LEFT_HIP],"MOTOR_LEFT_HIP");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_TORSO],"MOTOR_TORSO");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_FOOT],"MOTOR_RIGHT_FOOT");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_ANKLE],"MOTOR_RIGHT_ANKLE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_KNEE],"MOTOR_RIGHT_KNEE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_LEG],"MOTOR_RIGHT_LEG");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_SIDE],"MOTOR_RIGHT_SIDE");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_RIGHT_HIP],"MOTOR_RIGHT_HIP");

		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_NECK],"MOTOR_NECK");
		strcpy(Section[ROBOT_APP_CONFIG_MOTOR_HEAD],"MOTOR_HEAD");


		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_PITCH],"CALIB_ACCEL_LEFT_FOOT_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_ROLL],"CALIB_ACCEL_LEFT_FOOT_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_LOWER_LEG_PITCH],"CALIB_ACCEL_LEFT_LOWER_LEG_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_LOWER_LEG_ROLL],"CALIB_ACCEL_LEFT_LOWER_LEG_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_UPPER_LEG_PITCH],"CALIB_ACCEL_LEFT_UPPER_LEG_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_UPPER_LEG_ROLL],"CALIB_ACCEL_LEFT_UPPER_LEG_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_FOOT_PITCH],"CALIB_ACCEL_RIGHT_FOOT_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_FOOT_ROLL],"CALIB_ACCEL_RIGHT_FOOT_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_LOWER_LEG_PITCH],"CALIB_ACCEL_RIGHT_LOWER_LEG_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_LOWER_LEG_ROLL],"CALIB_ACCEL_RIGHT_LOWER_LEG_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_UPPER_LEG_PITCH],"CALIB_ACCEL_RIGHT_UPPER_LEG_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_UPPER_LEG_ROLL],"CALIB_ACCEL_RIGHT_UPPER_LEG_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_WAIST_PITCH],"CALIB_ACCEL_WAIST_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_WAIST_ROLL],"CALIB_ACCEL_WAIST_ROLL");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_TORSO_PITCH],"CALIB_ACCEL_TORSO_PITCH");
		strcpy(Section[ROBOT_APP_CONFIG_CALIB_ACCEL_TORSO_ROLL],"CALIB_ACCEL_TORSO_ROLL");
#endif

		//strcpy(Section[ROBOT_APP_CONFIG_NUM_CALIBRATION_SAMPLES],"NUM_CALIBRATION_SAMPLES");


		//RStatus.NumCalibSamples=300;  //set default number of samples to take for calibration
//		CurSection=0;

		//go through each line and each word (separated by spaces or a comma)

		//determine total number of lines in order to allocate space for a pointer to each for strtok
		NumLines=0;
		i=0;
		while(i<bSize) {
			if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
				NumLines++;
			}
			i++;
		}  //while(i<bSize)

		if (PStatus.flags&PInfo) {
			fprintf(stderr,"parse config file\n");
		}

		CurLine=0;
		CurChar=0;
		while(CurLine<NumLines) { 
			//get a line from the file text
			Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
			CurChar+=strlen(tline);
			CurLine++;
			if (PStatus.flags&PInfo) {
				fprintf(stderr,"tline=%s\n",tline);
			}

			//todo trim any initial spaces
			//get a word on this line
			param=strtok(tline,"="); 
			//see if this word matches any known section
			match=0;
			i=0;
			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {
			#if Linux
				if (!strcasecmp(param,Section[i])) {
			#endif
			#if WIN32
				if (!_stricmp(param,Section[i])) {
			#endif
					//found a match
					match=1;
					//read in data for this section
					switch(i) {
						case ROBOT_APP_CONFIG_ROBOTS_FOLDER:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a folder given
									//copy Robots folder to RStatus
									memcpy(RStatus.RobotsFolder,param,strlen(param));
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_DEFAULT_ROBOT:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a robot name given
									//copy name of Default Robot (Robot folder) to RStatus
									memcpy(RStatus.DefaultRobot,param,strlen(param));
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;

#if 0 
						case ROBOT_APP_CONFIG_ETHMOTORS_LOWER_BODY:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_LOWER_BODY].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_LOWER_BODY].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHMOTORS_LOWER_BODY]); //"ETHMOTORS_LOWER_BODY");
									RStatus.pcb[ROBOT_PCB_ETHMOTORS_LOWER_BODY].PCBNum=ROBOT_PCB_ETHMOTORS_LOWER_BODY;//ROBOT_APP_CONFIG_ETHMOTORS_LOWER_BODY;  note: seems pointless to add the PCBNum to a field that has that num as index, but the PCB struct may be referred to independently and in that case PCBNum might be retrieved without knowing PCBNum.
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_LOWER_BODY].flags|=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_LOWER_BODY].PCBKind=ROBOT_PCB_ETHMOTORS_LOWER_BODY;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHMOTORS_UPPER_BODY:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_UPPER_BODY].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_UPPER_BODY].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHMOTORS_UPPER_BODY]);//"ETHMOTORS_UPPER_BODY");
									RStatus.pcb[ROBOT_PCB_ETHMOTORS_UPPER_BODY].PCBNum=ROBOT_PCB_ETHMOTORS_UPPER_BODY;//ROBOT_APP_CONFIG_ETHMOTORS_UPPER_BODY;
									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_UPPER_BODY].flags|=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_UPPER_BODY].PCBKind=ROBOT_PCB_ETHMOTORS_UPPER_BODY;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHMOTORS_LEFT_HAND:
							param = strtok(NULL, "#\n"); //get next token
							if (param != 0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param, MACInBytes, 6);
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_LEFT_HAND].MAC, MACInBytes, 6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_LEFT_HAND].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHMOTORS_LEFT_HAND]);// "ETHMOTORS_LEFT_HAND");
									RStatus.pcb[ROBOT_PCB_ETHMOTORS_LEFT_HAND].PCBNum=ROBOT_PCB_ETHMOTORS_LEFT_HAND;//ROBOT_APP_CONFIG_ETHMOTORS_LEFT_HAND;
									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_LEFT_HAND].flags |= ROBOT_PCB_ETHMOTORS_LEFT_HAND;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_LEFT_HAND].PCBKind= ROBOT_PCB_ETHMOTORS_LEFT_HAND;
									i = NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
							break;
						case ROBOT_APP_CONFIG_ETHMOTORS_RIGHT_HAND:
							param = strtok(NULL, "#\n"); //get next token
							if (param != 0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param, MACInBytes, 6);
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_RIGHT_HAND].MAC, MACInBytes, 6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHMOTORS_RIGHT_HAND].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHMOTORS_RIGHT_HAND]);// "ETHMOTORS_RIGHT_HAND");
									RStatus.pcb[ROBOT_PCB_ETHMOTORS_RIGHT_HAND].PCBNum=ROBOT_PCB_ETHMOTORS_RIGHT_HAND;//ROBOT_APP_CONFIG_ETHMOTORS_RIGHT_HAND;

									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_RIGHT_HAND].flags |= ROBOT_PCB_ETHMOTORS_RIGHT_HAND;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHMOTORS_RIGHT_HAND].PCBKind = ROBOT_PCB_ETHMOTORS_RIGHT_HAND;
									i = NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
							break;
						case ROBOT_APP_CONFIG_ETHACCELS_LEFT_LEG:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_LEG].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_LEG].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHACCELS_LEFT_LEG]);//"ETHACCELS_LEFT_LEG");
									RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_LEG].PCBNum=ROBOT_PCB_ETHACCELS_LEFT_LEG;//ROBOT_APP_CONFIG_ETHACCELS_LEFT_LEG;

									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_LEFT_LEG].flags |= ROBOT_PCB_ETHACCELS_LEFT_LEG;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_LEFT_LEG].PCBKind = ROBOT_PCB_ETHACCELS_LEFT_LEG;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHACCELS_RIGHT_LEG:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
								if (strlen(param)>0) { //there is a MAC address given
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_LEG].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_LEG].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHACCELS_RIGHT_LEG]);//"ETHACCELS_RIGHT_LEG");
									RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_LEG].PCBNum=ROBOT_PCB_ETHACCELS_RIGHT_LEG;//ROBOT_APP_CONFIG_ETHACCELS_RIGHT_LEG;
									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_RIGHT_LEG].flags |= ROBOT_PCB_ETHACCELS_RIGHT_LEG;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_RIGHT_LEG].PCBKind = ROBOT_PCB_ETHACCELS_RIGHT_LEG;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHACCELS_TORSO:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_TORSO].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_TORSO].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHACCELS_TORSO]);//"ETHACCELS_TORSO");
									RStatus.pcb[ROBOT_PCB_ETHACCELS_TORSO].PCBNum=ROBOT_PCB_ETHACCELS_TORSO;//ROBOT_APP_CONFIG_ETHACCELS_TORSO;
									//set the flag bit to more quickly recognize which PCB this is
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_TORSO].flags |= ROBOT_PCB_ETHACCELS_TORSO;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_TORSO].PCBKind = ROBOT_PCB_ETHACCELS_TORSO;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHACCELS_LEFT_ARM:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_ARM].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_ARM].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHACCELS_LEFT_ARM]);//"ETHACCELS_LEFT_ARM");
									RStatus.pcb[ROBOT_PCB_ETHACCELS_LEFT_ARM].PCBNum=ROBOT_PCB_ETHACCELS_LEFT_ARM;//ROBOT_APP_CONFIG_ETHACCELS_LEFT_ARM;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_LEFT_ARM].PCBKind = ROBOT_PCB_ETHACCELS_LEFT_ARM;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHACCELS_RIGHT_ARM:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
								if (strlen(param)>0) { //there is a MAC address given
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_ARM].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_ARM].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHACCELS_RIGHT_ARM]);//"ETHACCELS_RIGHT_ARM");
									RStatus.pcb[ROBOT_PCB_ETHACCELS_RIGHT_ARM].PCBNum=ROBOT_PCB_ETHACCELS_RIGHT_ARM;//ROBOT_APP_CONFIG_ETHACCELS_RIGHT_ARM;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_RIGHT_ARM].PCBKind = ROBOT_PCB_ETHACCELS_RIGHT_ARM;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_ETHPOWER:
							//ETHPOWER PCB
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is a MAC address given
									//Convert ascii to bytes
									ConvertStringToBytesNum((unsigned char *)param,MACInBytes,6);  
									//copy MAC address to PCB table
									memcpy(RStatus.pcb[ROBOT_PCB_ETHPOWER].MAC,MACInBytes,6);
									//copy the PCBName name to the PCB table too
									strcpy(RStatus.pcb[ROBOT_PCB_ETHPOWER].PCBName,ROBOT_PCB_NAMES[ROBOT_PCB_ETHPOWER]);
									RStatus.pcb[ROBOT_PCB_ETHPOWER].PCBNum=ROBOT_PCB_ETHPOWER;//ROBOT_APP_CONFIG_ETHPOWER;
									//set the flag bit to more quickly recognize which PCB this is
									//unnecessary for EthPower PCB
									//RStatus.pcb[ROBOT_APP_CONFIG_POWER].flags|=ROBOT_PCB_ETHPOWER;
									//RStatus.pcb[ROBOT_APP_CONFIG_ETHACCELS_POWER].PCBKind = ROBOT_PCB_ETHPOWER;
									i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								} //if (strlen(param)>0) { 
							} //if (param!=0) {
						break;
#endif
						case ROBOT_APP_CONFIG_LOG_FOLDER:  //Robot log directory path
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								strcpy(RStatus.LogFolder,param);
							} //if (param!=0) {
						break;
						case ROBOT_APP_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS:  //delete logs older than some number of days
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								//delete any log files older than 7 days
								DeleteOldLogs(atoi(param),PStatus.ProjectPath);
								RStatus.DeleteLogsOlderThanDays=atoi(param);
							} //if (param!=0) {
						break;
#if 0 
					//check for Robot motor orientations- note that the order of these motor orientations in the Robot.h ROBOT_APP_CONFIG_FILE_SECTIONS structure must remain the same as the numbers in the motor name table for this to work correctly. 
						case ROBOT_APP_CONFIG_MOTOR_LEFT_FOOT:
						case ROBOT_APP_CONFIG_MOTOR_LEFT_ANKLE:
						case ROBOT_APP_CONFIG_MOTOR_LEFT_KNEE:
						case ROBOT_APP_CONFIG_MOTOR_LEFT_LEG:
						case ROBOT_APP_CONFIG_MOTOR_LEFT_SIDE:
						case ROBOT_APP_CONFIG_MOTOR_LEFT_HIP:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_FOOT:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_ANKLE:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_KNEE:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_LEG:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_SIDE:
						case ROBOT_APP_CONFIG_MOTOR_RIGHT_HIP:
						case ROBOT_APP_CONFIG_MOTOR_TORSO:
						case ROBOT_APP_CONFIG_MOTOR_NECK:
						case ROBOT_APP_CONFIG_MOTOR_HEAD:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is text given
									if (!_stricmp(param,"CCW")) {
										//RStatus.MotorInfo[MOTOR_NAME_LEFT_FOOT].flags|=MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW;
										RStatus.MotorInfo[i].flags|=MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW;
									}
								} //if (strlen(param)>0) { //there is text given
							} //if (param!=0) {
						break;
					//check for Robot motor orientations- note that the order of these motor orientations in the Robot.h ROBOT_APP_CONFIG_FILE_SECTIONS structure must remain the same as the numbers in the motor name table for this to work correctly. 
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_LOWER_LEG_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_UPPER_LEG_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_FOOT_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_LOWER_LEG_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_UPPER_LEG_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_WAIST_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_TORSO_PITCH:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_LOWER_LEG_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_UPPER_LEG_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_FOOT_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_LOWER_LEG_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_RIGHT_UPPER_LEG_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_WAIST_ROLL:
						case ROBOT_APP_CONFIG_CALIB_ACCEL_TORSO_ROLL:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is text given
									AccelNum=(int)((i-ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_PITCH)/2);
									AccelDim=(i-ROBOT_APP_CONFIG_CALIB_ACCEL_LEFT_FOOT_PITCH)%2;
									if (AccelDim==0) {
										RStatus.AccelInfo[AccelNum].Calib.x=atof(param);
									} else {
										RStatus.AccelInfo[AccelNum].Calib.z=atof(param);
									} //if (AccelDim==0) {
								} //if (strlen(param)>0) { //there is text given
							} //if (param!=0) {
						break;
#endif
/*						case ROBOT_APP_CONFIG_NUM_CALIBRATION_SAMPLES:
							param=strtok(NULL,"#\n"); //get next token
							if (param!=0) {
								if (strlen(param)>0) { //there is text given
									if (atof(param)>0) {
										RStatus.NumCalibSamples=atof(param);
									} //if (atof(param)>0)
								} //if (strlen(param)>0) { //there is text given
							} //if (param!=0) {
						break;*/

						default:  //most likely will never be reached
							if (param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
								sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine+1,filename);
								FTMessageBox(tstr,FTMB_OK,"Error",0);
								i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
								//CurLine=NumLines; //exit outer each-line loop
							} 
						break;
					} //switch(i)
				} //if (!_stricmp
				i++;  //advance to next section
			} //			while(param!=0 && match==0 && i<NUM_ROBOT_APP_CONFIG_SECTIONS) {

			if (!match) {
				if (param!=0 && param[0]!=35 && param[0]!=10 && param[0]!=13) {//ignore any line starting with # 
					sprintf(tstr,"Unknown section '%s' on line %d in config file %s",param,CurLine+1,filename);
					FTMessageBox(tstr,FTMB_OK,"Error",0);
					i=NUM_ROBOT_APP_CONFIG_SECTIONS;  //exit while loop
					//CurLine=NumLines; //exit outer each-line loop
				} //if (param[0]!=35) {//ignore any line starting with #
			} //if (!match) {

		} //while(CurLine<NumLInes) 

		free(buf);

		//now save the default.config file again to get any new settings that have changed
		//since the .config file was last saved
//		SaveMake3DConfigFile(filename);

		if (PStatus.flags&PInfo) {
			fprintf(stderr,"end LoadRobotAppConfigFile\n");
		}

		return(1);
	} else {
		fprintf(stderr,"Could not open file %s\n",filename);
		return(0);
	} //fptr!=0
//fptr!=0

} //int LoadRobotAppConfigFile(void)


//SaveRobotAppConfigFile - Saves all the config settings currently loaded
//note that all the PCB Mac addresses in the config file were loaded at startup in above LoadRobotAppConfigFile() and so are still in memory, 
//even if those PCB are offline (not physically connected) at the time SaveRobotAppConfigFile() is called.
//todo: perhaps only the mac address lines should be replaced to preserve user's comments in the RobotApp.conf file
int SaveRobotAppConfigFile(void)
{
	char filename[FTMedStr],tstr[FTMedStr];
	int PCBNum,i;
	FILE *fptr;


	sprintf(filename, "%sRobotApp.conf", PStatus.ProjectPath);
	//open config file for writing
	fptr = fopen(filename, "wb"); //currently presumes config is in current working directory
	if (fptr != 0) {
		fprintf(fptr, "#Robot configuration file\n");
#if 0 
		for (i = 1; i < NUM_ROBOT_PCB_NAMES; i++) {
			//if (strlen(RStatus.pcb[i].PCBName) != 0 && RStatus.pcb[i].MAC != 0) {
			ConvertBytesToString(RStatus.pcb[i].MAC,tstr,6);
			fprintf(fptr, "%s=%s\n", RStatus.pcb[i].PCBName, tstr);
			//} //if (strlen(RStatus.pcb[i].PCBName) != 0 && RStatus.pcb[i].MAC != 0) {
		} //for i
#endif 

		//write LOG_DIR field
		fprintf(fptr, "LOG_FOLDER=%s\n", RStatus.LogFolder);

		//write ROBOTS_FOLDER field
		fprintf(fptr, "ROBOTS_FOLDER=%s\n", RStatus.RobotsFolder);

		//write DEFAULT_ROBOT field
		fprintf(fptr, "DEFAULT_ROBOT=%s\n", RStatus.DefaultRobot);

		//write DELETE_LOGS_OLDER_THAN_DAYS field
		if (RStatus.DeleteLogsOlderThanDays>0) {
			//user specified that logs be deleted after this many days, so add to config
			fprintf(fptr, "DELETE_LOGS_OLDER_THAN_DAYS=%d\n", RStatus.DeleteLogsOlderThanDays);
		} //if (RStatus.DeleteLogsOlderThanDays>0)

#if 0 
		//write any CCW motor orientations (and CW too- but just for informational purposes)
		for (i=0;i<NUM_ROBOT_APP_CONFIG_MOTOR_SECTIONS;i++) {
			if (RStatus.MotorInfo[i].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW) {
				fprintf(fptr, "%s=CCW\n", RStatus.MotorInfo[i].Name);				
			} else { //if (RStatus.MotorInfo[i].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW) 
				fprintf(fptr, "%s=CW\n", RStatus.MotorInfo[i].Name);
			} //if (RStatus.MotorInfo[i].flags&MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW) 
		} //for i
		
		for (i = 0; i < NUM_ACCEL_NAMES; i++) {
			if (RStatus.AccelInfo[i].Calib.x!=0.0) {
				fprintf(fptr, "CALIB_%s_PITCH=%05.2f\n", ROBOT_ACCEL_NAMES[i], RStatus.AccelInfo[i].Calib.x);
			} 
			if (RStatus.AccelInfo[i].Calib.z != 0.0) {
				fprintf(fptr, "CALIB_%s_ROLL=%05.2f\n", ROBOT_ACCEL_NAMES[i], RStatus.AccelInfo[i].Calib.z);
			}
		} //for i
#endif

		fclose(fptr);
	} else { //if (fptr != 0) {
		fprintf(stderr, "Could not open Robot config file '%s' for writing\n", filename);
		return(0);
	} //if (fptr != 0) {

	return(1);
} //int SaveRobotAppConfigFile(void)


//len is the length of buf
int Robot_GetLineFromText(char *tline,char *buf,int len)
{
	int i;

	i=0;
	while(i<len) {
		if (buf[i]==10) {
			tline[i]=10; //include chr(10)
			tline[i+1]=0; //terminate string  
			return(i+1);
		} else {
			tline[i]=buf[i];
		}
		i++;
	} //while

	return(len);  //no chr(10) in string
} //int Robot_GetLineFromText(char *tline,char *buf)

//len is the length of buf
int Robot_GetLineBackwardsFromText(char *tline,char *buf,int len)
{
	int i;
	int j;
	char tline2[1024];

	i=len-1;  //start at end of buffer
	j=0;
	while(i>=0) {
		if (buf[i]==10) {
			//reverse string
			for(i=0;i<j;i++) {
				tline[i]=tline2[j-i-1];				
			}
			tline[j++]=10; //include chr(10)
			tline[j]=0; //terminate string

			return(j);
		} else {
			tline2[j]=buf[i];
		}
		i--;
		j++;
	} //while


	//reverse string
	for(i=0;i<len;i++) {
		tline[i]=tline2[len-i-1];				
	}
	i=len;
	tline[i++]=10; //include chr(10)
	tline[i]=0; //terminate string

	return(-i);  //minus so caller knows that function reached beginning of buffer, or line is incomplete
} //int Robot_GetLineFromText(char *tline,char *buf)

#if 0 
//add a motor-angle association to the list kept in RStatus.iMotorAngle
//NOTE: that when comparing a relative angle (TurnMotorUntilRelativeAngle), angle1 (not angle2) is used as reference
//so for example with head and torso, head is angle0, torso angle1, head leaning back is 10 deg torso at 0deg, 
//creating diff of 10-0=10, so the same neck motor direction to lower angle1 (head), is the same motor direction
//to lower the relative angle between the two. Moving the head forward to -10 degrees, is consistent with decreasing the 
//resulting relative angle (-10-0=-10). So even if the user specifies the relative accels as torso,head using neck motor-
//the neck motor IncAngDir is opposite to the head (decreasing head X is cw, but ce increases torso X). Another example is
//the left knee motor, cw pushes LUL forward (decreasing angle X/pitch), and so if user specifies LUL,LLL to decrease,
//motor incangdir of cw will decrease LUL and the angle LUL-LLL since 10--10=20, moving forward -10--10=0 even if LLL increases
//-10-0=-10 -10-10=-20 the relative angle only gets lower. If the user specifies LLL,LUL as angle order, to decrease LLL X angle,
//knee motor must turn ccw, and so to decrease the relative angle LLL-LUL, the motor must be turned ccw which decreases LLL and also
//the relative angle LLL-LUL. For example, LUL=10, LLL=-10 relang=-10-10=-20, to decrease angle knee motor is turned ccw,
//resulting in LUL=12,LLL=-12 relang=-12-12=-24 relangle is then less than -20. 
//int AssociateMotorAndAngle(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz,int IncAngDir,float Weight) 
int AssociateMotorAndAngle(MotorAngleAssociation* MAAssociation) 
{
	MotorAngleAssociation *lma;

	//to do: do not add if already added
	
	if (RStatus.iMotorAngle==0) {
			//first record
		RStatus.iMotorAngle=(MotorAngleAssociation *)malloc(sizeof(MotorAngleAssociation));
		memset(RStatus.iMotorAngle,0,sizeof(MotorAngleAssociation));
		lma=RStatus.iMotorAngle;
	} else { 
		//go to the last record and add a new record
		lma=RStatus.iMotorAngle;
		while(lma->next!=0) {
			lma=lma->next;
		} //while
		lma->next=(MotorAngleAssociation *)malloc(sizeof(MotorAngleAssociation));
		lma=lma->next;
		memset(lma,0,sizeof(MotorAngleAssociation));
	} //	if (RStatus.iMotorAngle==0) {OppMotorDir

	//now fill in motor-angle association details (all except next pointer)
	memcpy(lma,MAAssociation,sizeof(MotorAngleAssociation)-sizeof(struct MotorAngleAssociation *));

	return(1);
} //int AssociateMotorAndAngle(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz,int IncAngDir,float Weight) 

//return the MotorAngleAssocation given Motor PCB, Number, Accel PCB, number and dimension
MotorAngleAssociation *GetMotorAngleAssociation(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz) 
{
	MotorAngleAssociation *lma;
	
	//go through MotorAngleAssociation list and find match
	lma=RStatus.iMotorAngle;
	while (lma!=0) {
		if (lma->EMPCBNum==EMPCBNum && lma->MotorNum==MotorNum && lma->EAPCBNum==EAPCBNum && lma->AccelNum==AccelNum && lma->xyz==xyz) {
			return(lma);
		} //if (lma->
		lma=lma->next;
	} //while
	
	return(0);
} //MotorAngleAssociation *GetMotorAngleAssociation(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz)
#endif

//lemi->MotorNameNum[j],leai->AccelNameNum[k],leai->xyz[k]);

//MotorAngleAssociation *GetMotorAngleAssociation(int MotorNameNum,int AccelNameNum,int xyz) 
MotorAngleAssociation *GetMotorAngleAssociation(RobotInstruction *linst, int MotorNum, int AngleNum) 
{
	int i,MotorNameNum,AngleNameNum,xyz,AngleKind;	

	//get MotornameNum and AccelNameNum or AnalogSensorNameNum
	if (!linst) {
		fprintf(stderr,"Error: MotorAngleAssociation called with RobotInstruction=0\n");
		return(0);
	}

	AngleKind=linst->EAInst.AngleKind[AngleNum];

//	fprintf(stderr,"MotorNum=%d AngleNum=%d\n",MotorNum,AngleNum);
	MotorNameNum=linst->EMInst.MotorNameNum[MotorNum];
	switch(AngleKind) {
		case ANGLE_KIND_ACCEL:
			AngleNameNum=linst->EAInst.AccelNameNum[AngleNum];
			break;
		case ANGLE_KIND_ANALOG:
			AngleNameNum=linst->EAInst.AnalogSensorNameNum[AngleNum];
			break;
		default:
			fprintf(stderr,"Error: MotorAngleAssociation()- unknown angle kind MotorNum=%d AngleNum=%d\n",MotorNum,AngleNum);
			return(0);
	} //switch

	xyz=linst->EAInst.xyz[AngleNum];
			
//	fprintf(stderr,"MotorNameNum=%d AngleNameNum=%d xyz=%d\n",MotorNameNum,AngleNameNum,xyz);
	//go through MotorAngleAssociation list and find match
	i=0;
	while (i<RStatus.NumMotorAngleAssociations) {
		switch(AngleKind) {
			case ANGLE_KIND_ACCEL:
				if (RStatus.MotorAngle[i].MotorNameNum==MotorNameNum && RStatus.MotorAngle[i].AccelNameNum==AngleNameNum && RStatus.MotorAngle[i].xyz==xyz) { 
					return(&RStatus.MotorAngle[i]);
				} //if (RStatus.MotorAng...

				break;
			case ANGLE_KIND_ANALOG:
				if (RStatus.MotorAngle[i].MotorNameNum==MotorNameNum && RStatus.MotorAngle[i].AnalogSensorNameNum==AngleNameNum && RStatus.MotorAngle[i].xyz==xyz) { 
					return(&RStatus.MotorAngle[i]);
				} //if (RStatus.MotorAng...
				break;
		} //switch		
		i++;
	} //while
	
	return(0);
} //MotorAngleAssociation *GetMotorAngleAssociation(RobotInstruction *linst, int MotorNum, int AngleNum)  


//return the Motor direction that seeks to increase the assocation accelerometer angle
int GetIncAngDir(int MotorNameNum,int AccelNameNum,int xyz)
{
	int i;	

	//go through MotorAngleAssociation list and find match
	i=0;
	while (i<RStatus.NumMotorAngleAssociations) {
		if (RStatus.MotorAngle[i].MotorNameNum==MotorNameNum && RStatus.MotorAngle[i].AccelNameNum==AccelNameNum && RStatus.MotorAngle[i].xyz==xyz) { 
			return(i);
		} //if (RStatus.MotorAng...
		i++;
	} //while
	
	return(-1);
} //int GetIncAngDir(int MotorNameNum,int AccelNameNum,int xyz)

#if 0 
//return the Motor direction that seeks to increase the assocation accelerometer angle
int GetIncAngDir(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz) 
{
	MotorAngleAssociation *lma;
	int i;	

	//go through MotorAngleAssociation list and find match
	lma=RStatus.MotorAngle;
	while (lma<0) {
		if (lma->EMPCBNum==EMPCBNum && lma->MotorNum==MotorNum && lma->EAPCBNum==EAPCBNum && lma->AccelNum==AccelNum && lma->xyz==xyz) {
			return(lma->IncAngDir);
		} //if (lma->
		lma=lma->next;
	} //while
	
	return(0);
} //int GetIncAngDir(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz) 


int FreeMotorAngleAssociationList(void) 
{
	MotorAngleAssociation *lma,*next;

	lma=RStatus.iMotorAngle;
	while (lma!=0) {
		next=lma->next;
		free(lma);
		lma=next;
	} //if (RStatus.iMotorAngle!=0) {

	return(1);
} //int FreeMotorAngleAssociationList(void) 
#endif

#if 0 
//possibly add MotorName to the MotorPair structure and remove the MotorName array
//add a motor (and opposite pair association) to the Motor Info list kept in RStatus.iMotorPair
//Note that currently not all motors have MotorPair records- those without an opposite motor (like the neck and head) have no MotorPair record.
//OppMotorDir 0=paired motor does not move in opposite direction, 1=paired motor moves in opposite direction
int AddMotorPair(int EMPCBNum,int MotorNum,int OppMotor,int OppMotorDir)
{
	MotorPair *lmi;
	int foundmatch;

	//to do: do not add if already added
	
	if (RStatus.iMotorPair==0) {
			//first record
		RStatus.iMotorPair=(MotorPair *)malloc(sizeof(MotorPair));
		memset(RStatus.iMotorPair,0,sizeof(MotorPair));
		lmi=RStatus.iMotorPair;
	} else { 
		//go to the last record and add a new record
		lmi=RStatus.iMotorPair;
		foundmatch=0;
		while(lmi->next!=0 || foundmatch) {
			//get already existing record if one exists
			if (lmi->EMPCBNum==EMPCBNum && lmi->MotorNum==MotorNum) {
				foundmatch=1;	
			} else {
				lmi=lmi->next;
			}
		} //while
		if (!foundmatch) {  //new record
			lmi->next=(MotorPair *)malloc(sizeof(MotorPair));
			lmi=lmi->next;
			memset(lmi,0,sizeof(MotorPair));
		} //if (!foundmatch) {
	} //	if (RStatus.iMotorPair==0) {

	//now fill in motor-angle association details
	lmi->EMPCBNum=EMPCBNum;
	lmi->MotorNum=MotorNum;
	lmi->OppMotor=OppMotor;
	lmi->OppMotorDir=OppMotorDir;

	return(1);
} //int AddMotorPair(int EMPCBNum,int MotorNum,int OppMotor,int OppMotorDir)

//return the Motor info record based on EthMotors PCBNum and MotorNum
MotorPair *GetMotorPair(int EMPCBNum,int MotorNum)
{
	MotorPair *lmi;
	
	//go through MotorAngleAssociation list and find match
	lmi=RStatus.iMotorPair;
	while (lmi!=0) {
		if (lmi->EMPCBNum==EMPCBNum && lmi->MotorNum==MotorNum) {
			//found match
			return(lmi);
		} //if (lmi->EMPCBNum==EMPCBNum && lmi->MotorNum==MotorNum) {
		lmi=lmi->next;
	} //while
	
	return(0);
} //MotorPair *GetMotorPair(int EMPCBNum,int MotorNum)


int FreeMotorPairList(void) 
{
	MotorPair *lmi,*next;

	lmi=RStatus.iMotorPair;
	while (lmi!=0) {
		next=lmi->next;
		free(lmi);
		lmi=next;
	} //if (RStatus.iMotorPair!=0) {

	return(1);
} //int FreeMotorPairList(void);
#endif


int Open_MAC_Socket(MAC_Connection *lmac)
{
int SocketTrue;


	int iResult;
#if WIN32
	WSADATA wsaData = {0};
#endif
#if Linux
//int one;
#endif


	SocketTrue=1;
	//currently using the IPv4 address family, and UDP protocol 
	// Initialize Winsock
//the PIC program also opens a UDP server socket so that it doesn't need to 
//know the remote IP- so now any IP can send/recv data to/from the robot


	if (lmac==0) {
		fprintf(stderr,"Called Open_MAC_Socket with MAC_Connection==0\n");
		return(0);
	} //lmac==0

#if Linux
	lmac->Socket = 0;
#endif
#if WIN32
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		fprintf(stderr,"WSAStartup failed: %d\n", iResult);
		return(0);
	}
	lmac->Socket = INVALID_SOCKET;
#endif

	//listen will not work with UDP since it is a connectionless protocol- it's a message-passing protocol
	//RAW versus DGRAM socket: RAW=+we can access the MAC -we need to create the IP packet header 
	//see http://en.wikipedia.org/wiki/IPv4#Packet_structure
	//and checksum, in addition to the UDP header info:
	//see http://en.wikipedia.org/wiki/User_Datagram_Protocol#Packet_structure
	//lmac->Socket = socket(AF_PACKET,SOCK_RAW,IPPROTO_UDP);  //raw socket- needed if we want to get the MAC address from a packet
	//since I only need the MAC address of each PCB initially, 
	//I currently am just having each PCB send it in response to the ID (01) command
	lmac->Socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
#if Linux
	if (lmac->Socket == 0) {
		fprintf(stderr,"socket function failed with error\n");
		perror("socket:");
#endif
#if WIN32
	if (lmac->Socket == INVALID_SOCKET) {
		fprintf(stderr,"socket function failed with error = %d\n", WSAGetLastError() );
#endif
		return(0);
	} else {
		if (RStatus.flags&ROBOT_STATUS_INFO) {
			fprintf(stderr,"Socket %d opened\n",lmac->Socket);
		}
	}

#if 0
//	if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && (lmac->flags&ROBOT_MAC_CONNECTION_IS_WIRED)) {
	if (!strcmp(lmac->SrcIPAddressStr,RStatus.WiredIPAddressStr)) {
		//we use the wired (Robot) network interface to send multicast packets (.255) so that all Eth PCBs 
		//can respond with their Name ("Motor00", etc.), so this flags tells the kernel to use this 
		//interface for multicast packets - otherwise (in Windows at least) it will use the wireless interface

		if (setsockopt(lmac->Socket,SOL_SOCKET,SO_BROADCAST,(char *)&lmac->SrcAddress,sizeof(lmac->SrcAddress)) <0) {
			fprintf(stderr,"Failed to set socket SO_BROADCAST\n");
		}
		if (setsockopt(lmac->Socket,IPPROTO_IP,IP_MULTICAST_IF,(char *)&lmac->SrcAddress,sizeof(lmac->SrcAddress)) <0) {
			fprintf(stderr,"Failed to set socket option IP_MULTICAST_IF\n");
		}
		
	}//	if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && (lmac->flags&ROBOT_MAC_CONNECTION_IS_WIRED)) 	
#endif

//allow broadcast for connection with destination IP ending in .255 (because currently, because a broadcast sendto needs it's own socket, the MAC_Connection record is viewed as a remote connection and uses Connect_Remote_MAC)

	if ((lmac->DestAddress.sin_addr.s_addr&0xff000000)==0xff000000 || ((lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && (lmac->flags&ROBOT_MAC_CONNECTION_IS_WIRED))) {
		if (setsockopt(lmac->Socket,SOL_SOCKET,SO_BROADCAST,(char *)&SocketTrue,sizeof(int)) <0) {
			fprintf(stderr,"Failed to set socket option SO_BROADCAST\n");
		} else {
			fprintf(stderr,"set socket option SO_BROADCAST\n");
		}
	}
	
#if Linux
	if (setsockopt(lmac->Socket,SOL_SOCKET,SO_REUSEADDR,(char *)&SocketTrue,sizeof(int)) <0) {
		fprintf(stderr,"Failed to set socket option SO_REUSEADDR\n");
	} else {
			fprintf(stderr,"set socket option SO_REUSEADDR\n");
	}

#endif



	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	
	if (lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) { //if this is a local MAC bind the socket
//	/*Bind the server socket */
//binding connects the socket to an interface (IP and port), otherwise sendto will just pick a port
//update: can't bind the sendto socket, it fails, apparently beause the listen socket already binded with that socket- the PIC just needs to send to port 53510 or the listen needs to listen to all ports  

//http://stackoverflow.com/questions/12540392/udp-how-to-recvfrom-different-ports-at-server-side-with-binded-port :
//"If you want to receive packets on multiple ports, you must open multiple sockets and use something like select() to multiplex the I/O. The Berkeley sockets API does not allow a socket to bind() to more than one address."

	//bind fails on macos without this:
	//tph need to determine why- causes Linux versino to fail memset(&lmac->SrcAddress,0,sizeof(struct sockaddr_in));

	//Microsoft: The bind function is requires on an unconnected socket before subsequent calls to the listen function.
	#if Linux
		iResult=bind(lmac->Socket,(struct sockaddr *)&lmac->SrcAddress,sizeof(struct sockaddr_in));
	#endif
	#if WIN32
		iResult = bind(lmac->Socket,(SOCKADDR *)&lmac->SrcAddress, sizeof(lmac->SrcAddress));
	#endif
	#if Linux
	if (iResult <0) {
			fprintf(stderr,"bind failed: RobotSocket=%d RobotSrcAddress=%s RobotSrcPort=%d\n",lmac->Socket,lmac->SrcIPAddressStr,lmac->SrcPort);
			Close_MAC_Socket(lmac);
	#endif
	#if WIN32
		if (iResult == SOCKET_ERROR) {
			//FTMessageBox("bind failed with error",FTMB_OK,"Error");
			fprintf(stderr,"bind failed with error %u\n", WSAGetLastError());
			Close_MAC_Socket(lmac);
	#endif
			return(0);
		} else {
			//fprintf(stderr,"bind succeeded\n");
			fprintf(stderr,"socket bind succeeded: Socket:%d SrcAddress=%s SrcPort=%d\n",lmac->Socket,lmac->SrcIPAddressStr,lmac->SrcPort);

		}
	} //	if (lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) { //if this is a local MAC bind the socket



	//indicate that the network (send/receive) socket to the robot has been opened
	lmac->flags|=ROBOT_MAC_CONNECTION_SOCKET_OPENED;
	return(1);
} //int Open_MAC_Socket(MAC_Connection *lmac)

int Close_MAC_Socket(MAC_Connection *lmac)
{
	int iResult;

	if (lmac==0) {
		return(0);
	}

	//terminate the listening thread if it is running
	if (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {
		lmac->flags&=~ROBOT_MAC_CONNECTION_LISTENING;
#if Linux
		usleep(100000); //100ms for thread to exit while fully
		iResult=pthread_cancel(lmac->ReceiveThread); 
		//wait for thread to terminal cleanly
		if (iResult != 0) {
	    //handle_error_en(iResult,"pthread_cancel");
			fprintf(stderr,"pthread_cancel failed on %s\n",lmac->Name);
		} 
		/* Join with thread to see what its exit status was */
		iResult = pthread_join(lmac->ReceiveThread, 0);
		if (iResult != 0) {
	    //handle_error_en(iResult, "pthread_join");
			fprintf(stderr,"pthread_join failed on %s\n",lmac->Name);
		} 
#endif
#if WIN32
		Sleep(100); //sleep 100ms to exit while fully
		TerminateThread(lmac->ReceiveThread,0);
		Sleep(100); //sleep 100ms for now- todo: add code to detect that the thread has terminated cleanly
#endif
		
	}

	//if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {
	if ((lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) && (lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL)) {
#if Linux
//shutdown the connection?
#endif
#if WIN32
		// shutdown the connection - doesn't close socket
		//For TCP sockets, a FIN will be sent after all data is sent and acknowledged by the receiver. 
		iResult = shutdown(lmac->Socket,SD_BOTH);  //SD_BOTH disable both send and recv

		if (iResult == SOCKET_ERROR) {
			fprintf(stderr,"shutdown failed with error: %d\n", WSAGetLastError());
			_close(lmac->Socket);

			lmac->flags&=~ROBOT_MAC_CONNECTION_SOCKET_OPENED;

			closesocket(lmac->Socket);
			WSACleanup();
			return(0);
		} //if (iResult == SOCKET_ERROR) {
#endif

		// close the socket
#if Linux
		iResult = close(lmac->Socket);
		if (iResult == -1) {
			fprintf(stderr,"close failed with error\n");
			perror("close: ");
		}
#endif
#if WIN32
		iResult = closesocket(lmac->Socket);
		if (iResult == SOCKET_ERROR) {
			fprintf(stderr,"close failed with error: %d\n", WSAGetLastError());
		}
#endif			
		lmac->flags&=~ROBOT_MAC_CONNECTION_SOCKET_OPENED;
		//lmac->flags&=~ROBOT_MAC_CONNECTION_CONNECTED;
#if WIN32
		WSACleanup();
#endif
		} //if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {
	return(1);

} //int Close_MAC_Socket(MAC_Connection *lmac)



//todo: since not using connect anymore should probably be just Create_MAC_Connection()- just to store the SRC and remote IP+Port
//checks to see if a MAC_Connection already exists
//if no creates and adds the MAC_Connection, a socket, and connect
//futureL possibly add source and dest port#s
//int ConnectToRemoteMAC(MAC_Connection *lmac) {
//MAC_Connection *ConnectToRemoteMAC(char *SourceIP,char *DestIP) {
MAC_Connection *Create_Remote_MAC_Connection(char *SourceIP,int SrcPort,char *DestIP,int DestPort) {
	int iResult;
	MAC_Connection *lmac,*wiredmac;//,*lmacLocal;

	if (SourceIP==0 || DestIP==0) {
		return(0);
	}

	//see if the remote destination MAC_Connection has been created, has a socket, and is connected
	//in theory a remote IP could have multiple local connections
	//todo add Get_MAC_Connection(SourceIP,DestIP,IsRemote)
	lmac=Get_MAC_Connection_By_IPs(SourceIP,SrcPort,DestIP,DestPort); //later add port# too 1=IsRemote
	if (lmac==0) {
		//Remote destination Connection doesn't exist yet so create it
		//todo: if it does exist then go through, copy new src and dest ip, and reconnect?

		//fill MAC_Connection details
		lmac=(MAC_Connection *)malloc(sizeof(MAC_Connection));
		if (lmac==0) {
			fprintf(stderr,"malloc of MAC_Connection failed\n");
			return(0);
		}
		memset(lmac,0,sizeof(MAC_Connection));
		strcpy(lmac->SrcIPAddressStr,SourceIP);
		lmac->SrcPort=SrcPort;
		strcpy(lmac->DestIPAddressStr,DestIP);
		lmac->DestPort=DestPort;

		lmac->SrcAddress.sin_family=AF_INET;
		lmac->SrcAddress.sin_addr.s_addr=inet_addr(lmac->SrcIPAddressStr); 
		lmac->SrcAddress.sin_port=htons(lmac->SrcPort);

		lmac->DestAddress.sin_family=AF_INET;
		lmac->DestAddress.sin_addr.s_addr=inet_addr(lmac->DestIPAddressStr); 
		lmac->DestAddress.sin_port=htons(lmac->DestPort);

		//Open a socket for this MAC_Connection
		if (!(lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED)) {

/*
			//get the local socket from the source
			lmacLocal=Get_MAC_Connection(lmac->SrcIPAddressStr,0); //0=IsLocal
			lmac->Socket=lmacLocal->Socket;
			lmac->flags|=ROBOT_MAC_CONNECTION_SOCKET_OPENED;
*/
			//because currently I use the Listening function for Eth PCBs with more than one port number,
			//if this is a local wired connection don't create a new socket, just use the existing one
			wiredmac = RStatus.WiredInterface;
			if (wiredmac != 0 && wiredmac->SrcPort == lmac->SrcPort && !strcmp(wiredmac->SrcIPAddressStr, lmac->SrcIPAddressStr)) {
				lmac->Socket = wiredmac->Socket;
			} else {
				if (!strcmp(RStatus.WiredIPAddressStr,SourceIP))
					//create a new socket
					if (!(Open_MAC_Socket(lmac))) {
					fprintf(stderr, "Open_MAC_Socket() failed");
					free(lmac);
					return(0);
					}//if (!(Open_MAC_Socket(lmac)) {
			} //if (wiredmac != 0 && wiredmac->SrcPort == lmac->SrcPort && !strcmp(wiredmac->SrcIPAddressStr, lmac->SrcIPAddressStr)) {
			//add this new socket to the list of sockets being listened to
//			FD_SET(lmac->Socket,&lmacLocal->active_fd_set); 
		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED)) {

	
		//now try to connect to the destination
		//from: http://stackoverflow.com/questions/2968008/how-to-bind-connect-multiple-udp-socket
		//"connect(2) on a UDP socket just sets the default destination address of the socket (where the data will be sent if you use write(2) or send(2) on the socket). It has no other effect- you can still send packets to other addresses with sendto(2) or sendmsg(2) and you'll still see packets sent from any address"
#if Linux
		//connect is to send, bind is to receive
		//socket was non-blocking so I used a timeout, but now it blocks since it is called from a thread
		//iResult=connect(lmac->Socket,(struct sockaddr *)&lmac->DestAddress,sizeof(lmac->DestAddress));
		iResult=0; //for now
		if (iResult<0) { //did not connect but could be in progress
			switch(errno) {
			case EACCES:
					fprintf(stderr,"connect error EACCES: write permission sockaddris denied on the socket file.\n");
				break;
				case EPERM:
					fprintf(stderr,"connect error EPERM\n");
				break;
				case EADDRINUSE:
					fprintf(stderr,"connect error EADDRINUSE: local address is already in use.\n");
				break;
				case EAFNOSUPPORT:
					fprintf(stderr,"connect error EAFNOSUPPORT\n");
				break;
				case EAGAIN:
					fprintf(stderr,"connect error EAGAIN: No more free local ports.\n");
				break;
				case EALREADY:
					fprintf(stderr,"connect error EALREADY, socket is non-blocking and a previous connection attempt has not yet completed.\n");
				break;
				case ECONNREFUSED:
					fprintf(stderr,"connect error ECONNREFUSED.\n");
				break;
				case EFAULT:
					fprintf(stderr,"connect error EFAULT.\n");
				break;
				case EINTR:
					fprintf(stderr,"connect error EINTR.\n");
				break;
				case EISCONN:
					fprintf(stderr,"connect error EISCONN: Socket is already connected.\n");
				break;
				case ENETUNREACH:
					fprintf(stderr,"connect error ENETUNREACH: Network is unreachable.\n");
				break;
				case ENOTSOCK:
					fprintf(stderr,"connect error ENOTSOCK: file descriptor is not associated with a socket.\n");
				break;
				case ETIMEDOUT:
					fprintf(stderr,"connect error ETIMEDOUT.\n");
				break;
			}  //switch iResult
		} //iResult<0		

		if (iResult<0) {
			fprintf(stderr,"Connect failed\n");
			free(lmac);
			return(0);
		}  //iResult<0

#endif
#if WIN32
		//iResult = connect(lmac->Socket,(SOCKADDR*)&lmac->DestAddress,sizeof(lmac->DestAddress));
		iResult=0; //for now
#endif
#if Linux
		if (iResult < 0) {
			fprintf(stderr,"connect failed with error\n");
			close(lmac->Socket);
#endif
#if WIN32
		if (iResult == SOCKET_ERROR) {
			//FTMessageBox("connect failed",FTMB_OK,"Error");
			fprintf(stderr,"connect failed with error: %d\n", WSAGetLastError());
			Close_MAC_Socket(lmac);
#endif
			free(lmac);
			return(0);
		}  //if (iResult

		//lmac->flags|=ROBOT_MAC_CONNECTION_CONNECTED; 

		//add MAC_Connection to the list of other MAC_Connections
		if (!Add_MAC_Connection(lmac)) {
			fprintf(stderr,"Add_MAC_Connection failed\n");
			free(lmac);
			return(0); 
		}	

		//done with creating remote MAC_Connection
	//fprintf(stderr,"Connected %s to remote MAC %s\n",SourceIP,DestIP);

	} //if (lmac==0) {



	return(lmac);
}//MAC_Connection *Create_Remote_MAC_Connection(char *SourceIP,int SrcPort,char *DestIP,int DestPort) {



//todo this function is not called anymore
int DisconnectFromRemoteMAC(MAC_Connection *lmac)
{
	int iResult;

	//stop thread that creates network listening socket
	if (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {
#if Linux
		usleep(100000); //100ms for thread to exit while fully
		iResult=pthread_cancel(lmac->ReceiveThread); 
		//wait for thread to terminal cleanly
		if (iResult < 0) {
	    //handle_error_en(iResult,"pthread_cancel");
			fprintf(stderr,"pthread_cancel failed on %s\n",lmac->Name);

		} 
		/* Join with thread to see what its exit status was */
		iResult = pthread_join(lmac->ReceiveThread, 0);
		if (iResult != 0) {
	    //handle_error_en(iResults, "pthread_join");
			fprintf(stderr,"pthread_join failed on %s\n",lmac->Name);
		} 
#endif
#if WIN32
		Sleep(100); //wait 100ms for thread while to end cleanly
		TerminateThread(lmac->ReceiveThread,0);
		Sleep(100); //sleep 100 ms todo: only wait until thread is terminated
#endif
		lmac->flags&=~ROBOT_MAC_CONNECTION_LISTENING;
		fprintf(stderr,"Disconnected from Remote MAC\n");
	}
	//and close socket - done in Delete_MAC_Connection and Delete_All_MAC_Connections
	//Close_MAC_Socket(lmac);

	return(1);
} //int DisconnectFromRemoteMAC(MAC_Connection *lmac)

int Add_MAC_Connection(MAC_Connection *lmac) {
	MAC_Connection *nmac;
		
	if (RStatus.iMAC_Connection==0) {
		//first connection
		lmac->next=0;
		RStatus.iMAC_Connection=lmac;
	} else {		
		nmac=RStatus.iMAC_Connection;
		//todo:see if connection already exists?- check SourceIP and DestIP
		//if yes, disconnect, free, and delete old connection?
		//go to the end of the MAC_Connection list
		while(nmac->next!=0) {
			nmac=nmac->next;
		} //while		
		//add new MAC_Connection to the end of the list
		lmac->next=0;
		nmac->next=lmac;
	} //if (RStatus.iMAC_Connection==0) {
	return(1);
} //int Add_MAC_Connection(MAC_Connection *lmac) {

//later add port# too
//todo: Get_MAC_Connection_By_IP and By_Name
//MAC_Connection *Get_MAC_Connection(char *ip,unsigned int IsRemote) {
MAC_Connection *Get_MAC_Connection(char *ip) {
	MAC_Connection *lmac;

	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		//check for remote match

		if (!(lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && !strcmp(lmac->DestIPAddressStr,ip)) {
			return(lmac);
		}
		//check for local match
		if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && !strcmp(lmac->SrcIPAddressStr,ip)) {
			return(lmac);
		}

	lmac=lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection(char *ip) {

MAC_Connection *Get_MAC_Connection_By_WindowFunction(FTControlfunc *winFunction) 
{
	MAC_Connection *lmac;

	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		//check for a match
		if (lmac->AddWindowFunction==winFunction) {
			return(lmac);
		}
	lmac=lmac->next;
	} //while
	return(0);

} //MAC_Connection *Get_MAC_Connection_By_WindowFunc(FTControlfunc *winMotors_AddFTWindow) 

MAC_Connection *Get_MAC_Connection_By_IPs(char *SrcIP,int SrcPort,char *DestIP,int DestPort) 
{
	MAC_Connection *lmac;

	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		//check for remote match
		if (lmac->SrcPort==SrcPort && lmac->DestPort==DestPort && !strcmp(lmac->SrcIPAddressStr,SrcIP) && !strcmp(lmac->DestIPAddressStr,DestIP)) {
			return(lmac);
		}
	lmac=lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection_By_IPs(char *SrcIP,int SrcPort,char *DestIP,int DestPort) 

//Note that only a connection with a PCB on the Robot will have a (pcb) PCBName
//change to Get_MAC_Connection_By_PCB_Name(char *PCBname) like "ETHMOTORS_UPPER_BODY
MAC_Connection *Get_MAC_Connection_By_PCBName(char *PCBName) 
{
	MAC_Connection *lmac;
	Robot_PCB *lpcb;

	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		lpcb=&lmac->pcb;
		if (lpcb!=0) {
			if (!strcmp(lpcb->Name,PCBName)) {
				return(lmac);
			}
		} //if (lpcb!=0) {
	lmac=lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection_By_PCBName(char *PCBName)


//todo: ex:
MAC_Connection *Get_MAC_Connection_By_Name(char *Name)
{
	MAC_Connection *lmac;

	lmac = RStatus.iMAC_Connection;
	while (lmac != 0) {
		if (!strcmp(lmac->Name, Name)) {
				return(lmac);
		}
		lmac = lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection_By_Name(char *Name)


MAC_Connection *Get_MAC_Connection_By_PCBNum(int num)
{
	MAC_Connection *lmac;

	lmac = RStatus.iMAC_Connection;
	while (lmac != 0) {
		//need to use ROBOT_MAC_CONNECTION_IS_ETHPCB flag because lmac->pcb.Num == 0 for non EthPCB Mac_Connections
		if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) && lmac->pcb.Num==num) {
				return(lmac);
		}
		lmac = lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection_By_PCBNum(char *PCBName)


/*
//Get a MAC_Connection by matching the IP of any camera
MAC_Connection *Get_MAC_Connection_By_Camera_IP(char *CamIP)
{
	MAC_Connection *lmac,*lcam;

	lmac = RStatus.iMAC_Connection;
	while (lmac != 0) {
		if (lmac->cam!=0) { //MAC is a camera
			if (!strcmp(lmac->cam.IPStr, CamIP)) {
				return(lmac);
			}
		} //if (lmac->cam!=0) { //MAC is a camera
		lmac = lmac->next;
	} //while
	return(0);
} //MAC_Connection *Get_MAC_Connection_By_PCBName(char *PCBName)
*/



#if 0 
//EthAccelPCB is now part of lmac.pcb
//Get_EthAccelsPCB - return the pointer to an EthAccelsPCB if found by its associated MAC_Connection address
//EthAccels_PCB *Get_EthAccelsPCB(char *PCBName)
EthAccels_PCB *Get_EthAccelsPCB(MAC_Connection *lmac)
{
	EthAccels_PCB *leth;
	int i;

	//go through mac connections and find 	
	i=0;	
	while(i<RStatus.NumEthAccelsPCBs) {
		leth=&RStatus.EthAccelsPCB[i];
		//check for PCBName (on robot) match
		//if (!strcmp(leth[i]->PCBName,PCBName)) {
		if (leth->mac==lmac) {
			return(leth);
		}
	i++;
	} //while
	return(0);
} //int Get_EthAccelsPCB(MAC_Connection *lmac)
#endif

//Get AccelTouch PCB by PCBName (could be EthAccels or GAMTP PCB)
//EthAccels_PCB *Get_EthAccelsPCB_By_PCBName(char *PCBName)  
Accels_PCB *Get_AccelsPCB_By_PCBName(char *PCBName)  
{
	MAC_Connection *lmac;

	//go through all MAC_Connections and find EthAccelsPCB	
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
			//is an EthPCB
			if (!strcmp(lmac->pcb.Name,PCBName)) {  //presumes user is using a name that begins with ETHACCELS
				return(&lmac->pcb.AccelsPCB);
			} 
		} //if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
		lmac=lmac->next;
	} //while
	return(0);
} //Accels_PCB *Get_AccelsPCB_By_PCBName(char *PCBName)  

//EthAccels_PCB *Get_EthAccelsPCB_By_PCBNum(int num)
Accels_PCB *Get_AccelsPCB_By_PCBNum(int num)
{
	MAC_Connection *lmac;

	//go through all MAC_Connections and find EthAccelsPCB	
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
			//is an EthPCB
			if (lmac->pcb.Num==num) {  //note this will match any pcb num the user sends, not necessarily an EthAccelsPCB PCBNum
				return(&lmac->pcb.AccelsPCB);
			} 
		} //if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
		lmac=lmac->next;
	} //while
	return(0);
} //int Get_EthAccelsPCB_By_PCBNum(int num)


//Get Accels PCB Num by PCBName (for EthAccels or GAMTP PCB)
//***NOTE: THIS IS Accels PCB Num (index into RStatus.EthAccelsPCBInfo[]) not general PCB num (index into RStatus.EthPCBInfo[]). 
//int Get_EthAccelsPCBNum_By_PCBName(char *PCBName)
int Get_AccelsPCBNum_By_PCBName(char *PCBName)
{
	MAC_Connection *lmac;

	//go through all MAC_Connections and find EthAccelsPCB	
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
			//is an EthPCB
			if (!strcmp(lmac->pcb.Name,PCBName)) {  //presumes user is using a name that begins with ETHACCELS
				return(lmac->pcb.AccelsPCB.AccelsPCBNum);
			} 
		} //if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
		lmac=lmac->next;
	} //while
	return(0);
} //EthAccels_PCB *Get_AccelsPCBNum_By_PCBName(char *PCBName)  


int Delete_MAC_Connection(MAC_Connection *lmac) {
	MAC_Connection *nmac,*lastmac;

	if (lmac==0) {
		return(0);
	}

	lastmac=0;
	nmac=RStatus.iMAC_Connection;
	while(nmac!=0) {
		if (nmac==lmac) {
			//found the MAC_Connection
			fprintf(stderr,"Delete MAC_Connection: %s\n",lmac->Name);
			//if (lmac->flags&ROBOT_MAC_CONNECTION_CONNECTED) {	//disconnect if connected
				if (!DisconnectFromRemoteMAC(lmac)) {
					fprintf(stderr,"DeleteMAC_Connection: Failed to disconnect MAC_Connection");
				}
			//} //if (lmac->flags&ROBOT_MAC_CONNECTION_CONNECTED) {	//disconnect if connected

			//close any threads connected to this mac


			if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {	//close socket if opened
				if (!Close_MAC_Socket(lmac)) {
					fprintf(stderr,"DeleteMAC_Connection: Failed to close socket of MAC_Connection");
				}
			} //if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {	//close socket if opened

			if (lmac==RStatus.iMAC_Connection) {
				RStatus.iMAC_Connection=lmac->next;
			} else {
				lastmac->next=lmac->next;
			}
			//fprintf(stderr,"before free %x\n",lmac);
			free(lmac); //free the memory
			//fprintf(stderr,"after free\n");

			return(1);
		}
	lastmac=nmac;
	nmac=nmac->next;
	} //while
	return(1);
	
} //int Delete_MAC_Connection(MAC_Connection *lmac) {


int Delete_All_MAC_Connections(void) {
	MAC_Connection *lmac,*nmac;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"Delete_All_MAC_Connections()\n");
	}


	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
//			if (lmac->flags&ROBOT_MAC_CONNECTION_CONNECTED) {	//disconnect if connected
				if (!DisconnectFromRemoteMAC(lmac)) {
					fprintf(stderr,"DeleteMAC_Connection: Failed to disconnect MAC_Connection");
				}
//			} //if (lmac->flags&ROBOT_MAC_CONNECTION_CONNECTED) {	//disconnect if connected

//			if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {	//close socket if opened
			//close socket if opened - only applies to local MACs
			//if ((lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) &&
				//	(lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL)) {	
				if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {
					if (!Close_MAC_Socket(lmac)) {
						fprintf(stderr,"DeleteMAC_Connection: Failed to close socket of MAC_Connection");
					}
				} //if (lmac->flags&ROBOT_MAC_CONNECTION_SOCKET_OPENED) {	//close socket if opened

			nmac=lmac->next;
			free(lmac); //free the memory
			lmac=nmac;

	} //while
	return(1);
	
} //int Delete_All_MAC_Connections(void) {


//SendDataToMAC presumes a connection already exists
int SendDataToMAC(MAC_Connection *lmac, unsigned char *Inst,int numbyte) {
	int i,NumBytes;
	int iResult;
	char DataStr[512];


	if (lmac==0) {
		fprintf(stderr,"Error: SendDataToMAC called with MAC_Connection==0\n");
		return(0);
	}

	//the IP to send to is the first 4 bytes of the Inst, and is presumably 
	//still connected 
	//(in the future perhaps we should add port to the instruction in case an IP using 2 different ports is communicating)
	if (RStatus.flags&ROBOT_STATUS_INFO) {
		fprintf(stderr, "Send from %s to %s\n",lmac->SrcIPAddressStr,lmac->DestIPAddressStr);
		fprintf(stderr, "%d bytes: ",numbyte);
		for(i=0;i<numbyte;i++) {
			fprintf(stderr, "0x%02x ",Inst[i]);
		}
	} 
	
	//iResult=send(lmac->Socket,(const char *)Inst,numbyte,0);
	iResult=sendto(lmac->Socket,(const char *)Inst,numbyte,0,(struct sockaddr*)&lmac->DestAddress,sizeof(lmac->DestAddress));

#if Linux
	if (iResult == 0) {
		fprintf(stderr,"send failed with error\n");
#endif
#if WIN32
		if (iResult == SOCKET_ERROR) {
			fprintf(stderr,"send failed with error: %d\n", WSAGetLastError());
#endif
			//Close and reopen socket?
			return(0);
		}

		if ((RStatus.flags&ROBOT_STATUS_NETWORK_LOG)&&(RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA)) { //log all outgoing data
			NumBytes=ConvertBytesToString(Inst,DataStr,numbyte); //convert bytes to string
			LogRobotNetworkData(lmac->SrcIPAddressStr,lmac->DestIPAddressStr,DataStr,NumBytes);		
		} //if (RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA) { //log all incoming data

		if (RStatus.flags&ROBOT_STATUS_INFO) {
			fprintf(stderr,"Bytes Sent: %d\n", iResult);
		}
		return(iResult);
} //int SendDataToMAC(MAC_Connection *lmac, unsigned char *Inst,int numbyte)

//Send the "01" instruction on the broadcast address .255 to receive ID packets from all connected ETH PCBs
int SendBroadcastInstruction(void) 
{
	MAC_Connection *lmac,*lmacBC;
	unsigned char SendInst[20],NameInst;
	char DestIPAddressStr[INET_ADDRSTRLEN];
	struct sockaddr_in DestAddress;
	int NumBytes;
	char DataStr[512];

	//only send if running robot locally - should never happen because button should not appear
	if (RStatus.flags&ROBOT_STATUS_REMOTE) {
		return(0); 
	}
	
	lmac=RStatus.WiredInterface;
	if (lmac==0) {
		fprintf(stderr,"SendBroadcastInstruction: Could not find the wired connection.\n");
		//try to establish the wired connection again
		GetLocalNetworkInterfaces(); 
		return(0);
	}
/*
	//get the wired MAC_Connection
	lmac=RStatus.iMAC_Connection;
	FoundMAC=0;
	while(lmac!=0 && !FoundMAC) {
		if ((lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL) && (lmac->flags&ROBOT_MAC_CONNECTION_IS_WIRED)) {
			FoundMAC=1;
		} else {
			lmac=lmac->next;
		}
	} //while
	if (!FoundMAC) {
		return(0);
	}
*/


	//now send the 01 instruction
	memcpy(SendInst,(unsigned int *)&lmac->SrcAddress.sin_addr.s_addr,4);
	NameInst=0x01;
	memcpy(SendInst+4,&NameInst,1);
//	numbyte=5;
	//see if a broadcast MAC_Connection already exists
	memcpy(&DestAddress,&lmac->SrcAddress,sizeof(struct sockaddr_in));
//	DestAddress.sin_addr.S_un.S_un_b.s_b4=0xff; //change to broadcast IP	
	DestAddress.sin_addr.s_addr|=0xff000000; //change to subnet broadcast IP
	//DestAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	inet_ntop(AF_INET,&DestAddress.sin_addr,DestIPAddressStr,INET_ADDRSTRLEN);
	lmacBC=Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr,DEFAULT_HOST_PORT,DestIPAddressStr,DEFAULT_ROBOT_PORT);
	if (lmacBC==0) {
		lmacBC = Create_Remote_MAC_Connection(lmac->SrcIPAddressStr, DEFAULT_HOST_PORT, DestIPAddressStr, DEFAULT_ROBOT_PORT);
	}
	//the dest address should be the broadcast address already
//	return(sendto(lmac2->Socket,(const char *)SendInst,numbyte,0,(struct sockaddr*)&lmac2->DestAddress,sizeof(lmac2->DestAddress)));
	if (SendDataToMAC(lmacBC,SendInst,5)) {
		if ((RStatus.flags&ROBOT_STATUS_NETWORK_LOG)&&(RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA)) { //log all outgoing data
			NumBytes=ConvertBytesToString(SendInst,DataStr,5); //convert bytes to string
			LogRobotNetworkData(lmacBC->SrcIPAddressStr,lmacBC->DestIPAddressStr,DataStr,NumBytes);		
		} //if (RStatus.flags&ROBOT_STATUS_LOG_OUTGOING_DATA) { //log all incoming data
	return(1);
	} else {
		return(0);
	} //if (SendDataToMAC(lmacBC,SendInst,5)) {


} //int SendBroadcastInstruction(void) 

//check to see if a MAC_Connection already exists between a source and dest IP,
//and if not create a new MAC_Connection (socket) between the source and destination IP
MAC_Connection *Connect2IPs(char *SrcIP,char *DestIP)
{
	MAC_Connection *lmac;

		//was: see if the local source MAC_Connection has been created, has a socket, is binded, and has a listening thread
	lmac=Get_MAC_Connection_By_IPs(SrcIP,DEFAULT_HOST_PORT,DestIP,DEFAULT_ROBOT_PORT);
	if (lmac!=0) {
		return(lmac);
	}
	
	//determine if source is wired or wireless
	if (!strcmp(SrcIP,RStatus.WiredIPAddressStr)) {
		lmac = Create_Remote_MAC_Connection(SrcIP, DEFAULT_HOST_PORT, DestIP, DEFAULT_ROBOT_PORT);
		if (lmac==0) {
			fprintf(stderr, "Create_Remote_MAC_Connection(%s:%d,%s:%d) failed\n", SrcIP, DEFAULT_HOST_PORT, DestIP, DEFAULT_ROBOT_PORT);
			return(0);
		} //lmac==0
	} else {
		if (!strcmp(SrcIP,RStatus.WirelessIPAddressStr)) {
			lmac = Create_Remote_MAC_Connection(SrcIP, DEFAULT_HOST_PORT, DestIP,DEFAULT_ROBOT_PORT);
			if (lmac==0) {
				fprintf(stderr, "Create_Remote_MAC_Connection(%s:%d,%s:%d) failed\n", SrcIP, DEFAULT_HOST_PORT, DestIP, DEFAULT_ROBOT_PORT);
				return(0);
			} //lmac==0
		} else { 
				fprintf(stderr,"Unknown source network interface with IP %s\n",SrcIP);
				return(0);
		} //if (!strcmp(SrcIP,RStatus.WirelessIPAddressStr)) {
	} //if (!strcmp(SrcIP,RStatus.WiredIPAddressStr)) {
	return(lmac);
} //int Connect2IPs(void)

//This creates (allocates) a MAC_Connection to a local network interface, 
//then adds the connection to the global list of all connections
MAC_Connection *Create_Local_MAC_Connection(char *ip,int Wireless) 
{
	MAC_Connection *lmac;
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
#endif


	if (ip==0) {
		fprintf(stderr,"Create_Local_MAC_Connection called with ip=0\n");
		return(0);
	}

	if (strlen(ip)==0) {
		fprintf(stderr,"Create_Local_MAC_Connection called with an empty IP address\n");
		return(0);
	}


	//see if MAC_Connection already exists
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		if (!strcmp(lmac->SrcIPAddressStr,ip)) {
			fprintf(stderr,"Create_Local_MAC_Connection: local connection already exists for ip=%s\n",ip);
			return(lmac);
		} 
		lmac=lmac->next;
	} //while

	//fill MAC_Connection details
	lmac=(MAC_Connection *)malloc(sizeof(MAC_Connection));
	if (lmac==0) {
		fprintf(stderr,"malloc of MAC_Connection failed\n");
		return(0);
	}
	memset(lmac,0,sizeof(MAC_Connection));
	lmac->flags|=ROBOT_MAC_CONNECTION_IS_LOCAL;
	if (!Wireless) {
		lmac->flags|=ROBOT_MAC_CONNECTION_IS_WIRED; //so Multicast can be enabled on this socket
	}
	strcpy(lmac->SrcIPAddressStr,ip);
	lmac->SrcPort=DEFAULT_ROBOT_PORT;
//	tc=GetFTControl("txtRobotDestIPAddressStr");			
//	strcpy(lmac->DestIPAddressStr,tc->text);
//	lmac->DestPort=DEFAULT_ROBOT_PORT;

	lmac->SrcAddress.sin_family=AF_INET;
	lmac->SrcAddress.sin_addr.s_addr=inet_addr(lmac->SrcIPAddressStr); 
	lmac->SrcAddress.sin_port=htons(lmac->SrcPort);


	lmac->DestAddress.sin_family=AF_INET;
	//lmac->DestAddress.sin_addr.s_addr=inet_addr(lmac->SrcIPAddressStr); 
	//lmac->DestAddress.sin_port=htons(lmac->DestPort);

	//Open a socket for this MAC_Connection
	if (!Open_MAC_Socket(lmac)) {
		fprintf(stderr,"Open_MAC_Socket failed\n");
		return(0); 
	}			

	//add MAC_Connection to the list of other MAC_Connections
	if (!Add_MAC_Connection(lmac)) {
		fprintf(stderr,"Add_MAC_Connection failed\n");
		free(lmac);
		return(0); 
	}	

	//done with creating basic parts of local MAC_Connection
	//now add listening thread
	//the thread searches for the MAC_Connection to get the socket, so the MAC_Connection needs
	//to be already created by now
	//create the receive thread if it doesn't already exist- 
	//presumably we would never be calling Create_Local_MAC_Connection if we were already listening, 
	//but  there is no harm done in checking for the listening flag
	//note this can only be for a local wired or wireless interface
	if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {

#if Linux
		//set thread priority
		pthread_attr_init(&tattr);
		pthread_attr_getschedparam(&tattr,&param);
		param.sched_priority=98;
		pthread_attr_setschedparam(&tattr,&param);
		
		//pthread_attr_getschedpolicy(&tattr,&param);
		//pthread_attr_setschedpolicy(&tattr,&param);

		if (!Wireless) {
			pthread_create(&lmac->ReceiveThread,&tattr,(void *)Thread_ListenToRobot,NULL);
		} else {
			pthread_create(&lmac->ReceiveThread,&tattr,(void *)Thread_ListenToOutside,NULL);
		}
#endif
#if WIN32
		if (!Wireless) {
			lmac->ReceiveThread=CreateThread(0,0,Thread_ListenToRobot,0,0,0);
			SetThreadPriority(lmac->ReceiveThread, THREAD_PRIORITY_ABOVE_NORMAL);
		} else {
			lmac->ReceiveThread=CreateThread(0,0,Thread_ListenToOutside,0,0,0);
		}
#endif
		lmac->flags|=ROBOT_MAC_CONNECTION_LISTENING;
	} //	if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
	//fprintf(stderr,"Created local MAC_Connection\n");

	return(lmac);

} //MAC_Connection *Create_Local_MAC_Connection(char *ip,int Wireless) 


//create a local source MAC_Connection if it doesn't already exist
//and create a MAC_Connection for the remote MAC if it doesn't already exist
//then connect to the remote MAC if it is not already connected
int ConnectToOutside(char *ip) {
	MAC_Connection *lmac;
	FTControl *tc;
#if Linux
	pthread_attr_t tattr;
	struct sched_param param;
#endif

	//see if the local source MAC_Connection has been created, has a socket, is binded, and has a listening thread
	tc=GetFTControl("ddOutsideSrcIPAddressStr");  //usually wireless MAC (wlan0)
	if (tc!=0) {
		lmac=Get_MAC_Connection(tc->text); //later add port# too
		if (lmac==0) {
			//Local source Connection doesn't exist yet so create it
			//todo: if it does exist then go through, copy new src and dest ip, and reconnect?

			//fill MAC_Connection details
			lmac=malloc(sizeof(MAC_Connection));
			if (lmac==0) {
				fprintf(stderr,"malloc of MAC_Connection failed\n");
				return(0);
			}
			memset(lmac,0,sizeof(MAC_Connection));
			lmac->flags|=ROBOT_MAC_CONNECTION_IS_LOCAL;
			strcpy(lmac->SrcIPAddressStr,tc->text);
			lmac->SrcPort=DEFAULT_ROBOT_PORT;
			//probably should just be clear because there can be many connections to remote MACs
			//and currently, each will have their own MAC_Connection
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

			//add MAC_Connection to the list of other MAC_Connections
			//need to add MAC_Connection before creating listening thread, 
			//because the listening thread looks for the MAC_Connection
			if (!Add_MAC_Connection(lmac)) {
				fprintf(stderr,"Add_MAC_Connection failed\n");
				free(lmac);
				return(0); 
			}	
			
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



			//done with creating local MAC_Connection
			//fprintf(stderr,"Created local MAC_Connection\n");
		} //if (lmac==0) {


		//create the remote MAC_Connection if it doesn't exist already
		if (!Create_Remote_MAC_Connection(lmac->SrcIPAddressStr, DEFAULT_ROBOT_PORT, lmac->DestIPAddressStr, DEFAULT_ROBOT_PORT)) {
			fprintf(stderr, "Create_Remote_MAC_Connection(%s:%d,%s:%d) failed\n", lmac->SrcIPAddressStr, DEFAULT_ROBOT_PORT, lmac->DestIPAddressStr, DEFAULT_ROBOT_PORT);
		} //if (!Connect_To_Remote_MAC(lmac->SrcIPAddressStr,lmac->DestIPAddressStr)) {

					
//			FTMessageBox("Already connected",FTMB_OK,"Robot -- Info",0);
//			strcpy(tcontrol->text,"Disconnect Robot");		//may be an error so recopy
//	fprintf(stderr,"Outside IP %s connected\n",ip);
	return(1);
	} //if (tc!=0) {

	fprintf(stderr,"Error: ConnectToOutside- Could not find control ddOutsideSrcIPAddressStr\n");
	return(0);

}//int ConnectToOutside(char *ip) {


//Thread that listens for data from the outside
#if Linux
int Thread_ListenToOutside(void) 
#endif
#if WIN32
DWORD WINAPI Thread_ListenToOutside(LPVOID lpParam) 
#endif
{
	int recvbuflen = DEFAULT_NETWORK_BUFLEN;
	unsigned char recvbuf[DEFAULT_NETWORK_BUFLEN] = "";
	char DataStr[DEFAULT_NETWORK_BUFLEN*2];
	//FTControl *tc,*tc2,*tcWired;
	int NumBytes,ConvertedData;
	struct sockaddr OutsideClientAddr;
	struct sockaddr_in taddr;
	socklen_t OutsideClientAddrLen;
	char RemoteIP[INET_ADDRSTRLEN];
	char RequestingIP[INET_ADDRSTRLEN];
	MAC_Connection *lmac,*lmac2,*lmacWired;

/*
	tc=GetFTControl("txtGetOutsideConnectData");
	if (tc==0) {
		fprintf(stderr,"Error: Couldn't get FTControl: txtGetOutsideConnectData\n");
		return(0);
	}

	//get the MAC_Connection to get the socket
	tc2=GetFTControl("ddOutsideSrcIPAddressStr");
	if (tc2==0) {
		fprintf(stderr,"Error: Thread_ListenToOutside could not find FTControl ddOutsideSrcIPAddressStr\n");
		return(0);
	}
	lmac=Get_MAC_Connection(tc2->text); //later add port# too
	if (lmac==0) {
		fprintf(stderr,"Error: Thread_ListenToOutside could not find any MAC_Connection for %s\n",tc2->text);
		return(0);
	}

	//get the wired MAC_Connection because we want to ignore those packets
	tcWired=GetFTControl("ddRobotSrcIPAddressStr");
	if (tcWired==0) {
		fprintf(stderr,"Error: Thread_ListenToOutside could not find FTControl ddRobotSrcIPAddressStr\n");
		return(0);
	}
	lmacWired=Get_MAC_Connection(tcWired->text); //later add port# too
	if (lmacWired==0) {
		fprintf(stderr,"Error: Thread_ListenToOutside could not find any MAC_Connection for %s\n",tcWired->text);
		return(0);
	}
*/

	//wait for wireless MAC_Connection to be fully initialized
	while(!(RStatus.flags&ROBOT_STATUS_LISTEN_TO_OUTSIDE));


	lmac=RStatus.WirelessInterface;
	lmacWired=RStatus.WiredInterface;
	//listen for data received from the outside
	// Receive until the peer closes the connection
	//while 1 is ok because the connection is a blocking connection and so recv blocks (waits) until there is received data

	OutsideClientAddrLen=sizeof(OutsideClientAddr); //this is needed so recvfrom doesn't return 0.0.0.0
	memset(&OutsideClientAddr,0,OutsideClientAddrLen); //need?

	fprintf(stderr,"Listening to Outside\n");


//	while (1) {
	while (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {



//	while(RStatus.flags&ROBOT_STATUS_LISTEN_TO_OUTSIDE) {  //replace with this?- currently no option to stop listening
//Linux:	if (listen(RStatus.RobotSocket,10) < 0) {
//		fprintf(stderr,"Failed to listen on server socket\n");
//	} //listen
//note that there is also recvfrom 


		//NumBytes = recv(RStatus.OutsideSocket,recvbuf,recvbuflen, 0);
		NumBytes=recvfrom(lmac->Socket,recvbuf,recvbuflen,0,&OutsideClientAddr,&OutsideClientAddrLen);
		if (NumBytes>0) {

			//we received data from an outside Mac, but we don't have to connect to the remote Mac
			//until the robot sends data back to it

			//inet_ntop is thread-safe
			inet_ntop(AF_INET,&(((struct sockaddr_in*)&OutsideClientAddr)->sin_addr),RemoteIP,INET_ADDRSTRLEN);
			//fprintf(stderr,"%d bytes received from %s\n",NumBytes,RemoteIP);		
			if (RStatus.flags&ROBOT_STATUS_INFO) {
				ConvertBytesToString(recvbuf,DataStr,NumBytes); //convert bytes to string
				ConvertedData=1;
				fprintf(stderr,"%s: sent %d bytes to %s\n%s\n",RemoteIP,NumBytes,lmac->Name,DataStr);
			} 

			ConvertedData=0; //have not converted from bytes to string yet

			if ((RStatus.flags&ROBOT_STATUS_NETWORK_LOG)&&(RStatus.flags&ROBOT_STATUS_LOG_INCOMING_DATA)) { //log all incoming data
					//log needs data in a text string so convert
					if (!ConvertedData) {
						ConvertBytesToString(recvbuf,DataStr,NumBytes); //convert bytes to string
						ConvertedData=1;
					}
					//LogRobotNetworkData(lmac,RemoteIP,tc->text,NumBytes);
					//LogRobotNetworkData(lmac,RemoteIP,DataStr,NumBytes);
					LogRobotNetworkData(RemoteIP,lmac->SrcIPAddressStr,DataStr,NumBytes);
			}	//if (RStatus.flags&ROBOT_STATUS_LOG_INCOMING_DATA) { //log all incoming data

			//we can either create a MAC_Connection for this remote MAC here or before we send data back to it
			//here we get the remote IP from the packet
			//but in sending we get the remote IP from the instruction it sent (presumably)
			//in theory we could just skip sending the IP from remote clients and get it from the packet
			//as we are doing above

			//currently I check for and/or create the connection here, 
			//because I may do away with sending the IP from remote MACs

			//check to see if a connection exists to this remote MAC and if not then create one
			lmac2=Get_MAC_Connection(RemoteIP); //
			if (lmac2==0) {
				//need to create one
				ConnectToOutside(RemoteIP);
			} //if (lmac==0) {

			strcpy(RequestingIP,"");
			if (NumBytes>=4) {
				memcpy(&taddr.sin_addr,recvbuf,4);
				//get the requesting IP in the form of a string:
				inet_ntop(AF_INET,&taddr.sin_addr,RequestingIP,INET_ADDRSTRLEN);
			} //

			//decide if we need to pass this instruction on to the robot
			//don't pass the instruction on, if we are remotely controlling the robot, 
			//or if the source IP is the wired interface (broadcast instructions) or if the 
//			if (!(RStatus.flags&ROBOT_STATUS_USE_WIRELESS_INTERFACE) && strcmp(RemoteIP,lmacWired->SrcIPAddressStr)) {
			if (!(RStatus.flags&ROBOT_STATUS_USE_WIRELESS_INTERFACE) && strcmp(RequestingIP,lmacWired->SrcIPAddressStr) && strcmp(RemoteIP,lmacWired->SrcIPAddressStr)) {
					//Pass the instruction on to the Robot
				SendInstructionToRobot(recvbuf,NumBytes,0);  //0=do not add an IP- the instr already has it
			} 

#if 0 
			//print this data to the remote incoming data textbox			
			//and put result in textbox
			//sprintf(tc->text,"%s",key);
			if (!ConvertedData) {
				ConvertBytesToString(recvbuf,NumBytes,DataStr);		
			} 
			strcpy(tc->text,DataStr);
			DrawFTControl(tc);				
#endif
			

		} else { //if (NumBytes > 0) {
			if (NumBytes == 0) {
				fprintf(stderr,"0 Bytes received - Connection closed\n");
				lmac=Get_MAC_Connection(RemoteIP); 
				if (lmac2!=0) {  //only close the remote connection
					Delete_MAC_Connection(lmac2);
				} 
				//StopListeningToOutside
				//RStatus.flags&=~ROBOT_LISTENING_TO_OUTSIDE;
				//return(0);
			} else { //if (NumBytes == 0) {
#if Linux
				fprintf(stderr,"recv failed with error\n");
#endif
#if WIN32
				fprintf(stderr,"recv failed with error: %d\n", WSAGetLastError());
#endif
				//return(0);
			} //if (NumBytes == 0) {
		} // if (NumBytes > 0) {

//	}  //while(1)
	} //	while (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {

	return(1);
}   //Thread_ListenToOutside

int ConvertBytesToString(unsigned char *ByteStr,char *ResultStr,int NumBytes) 
{
	int i;

	if (NumBytes>0) {
		for(i=0;i<NumBytes;i++) {
			//go through each nibble and convert to ascii
			//0x88 will become "88"
			sprintf(&ResultStr[i*2],"%x",(int)(ByteStr[i]&0xf0));
			sprintf(&ResultStr[i*2+1],"%x",(int)(ByteStr[i]&0xf));
	//				itoa((int)(ByteStr[i]&0xf0),&ResultStr[i*2],16);
	//				itoa((int)(ByteStr[i]&0xf),&ResultStr[i*2+1],16);
		} //for
		ResultStr[NumBytes*2]=0;  //terminal the character string
		return(NumBytes*2);
	} else {
		return(0);
	}
} //int ConvertBytesToString(char *ByteStr,char *ResultStr,int NumBytes) {


//converts text in a textbox to bytes, rename ConvertStringToBytes?
//int ConvertInstructionString(unsigned char *InstStr,unsigned char *Inst)
int ConvertStringToBytes(unsigned char *InstStr,unsigned char *InstBytes)  
{
	int NumChars;
	int i;
	char tkey[5];

	NumChars=strlen((char *)InstStr);
	if (NumChars>0) { //there is a command there
		//presumes instruction is in hexadecimal (without 0x)		
		//convert ascii to numbers
		tkey[1]=0;
		for(i=0;i<NumChars/2;i++) {
			tkey[0]=InstStr[i*2];
			InstBytes[i]=strtol(tkey,0,16);
			InstBytes[i]<<=4;
			tkey[0]=InstStr[i*2+1];
			InstBytes[i]|=strtol(tkey,0,16);//atoi(tkey);				
		} //for(i=0;i<NumChars/2;i++) {
	return(NumChars/2);			
	} //if (NumChars>0) {

	return(0);
} //int ConvertStringToBytes(unsigned char *InstStr,unsigned char *InstBytes)  


//converts Num chars of text in a textbox to bytes, rename ConvertStringToBytes?
int ConvertStringToBytesNum(unsigned char *InstStr,unsigned char *InstBytes, int NumByte)  
{
	int TotalChar;
	int i;
	char tkey[5];

	TotalChar=strlen((char *)InstStr);
	if (TotalChar>0) { //there is a command there
		if (NumByte*2>TotalChar) {
			NumByte=TotalChar/2;
		}

		//presumes instruction is in hexadecimal (without 0x)		
		//convert ascii to numbers
		tkey[1]=0;
		for(i=0;i<NumByte;i++) {
			tkey[0]=InstStr[i*2];
			InstBytes[i]=strtol(tkey,0,16);
			InstBytes[i]<<=4;
			tkey[0]=InstStr[i*2+1];
			InstBytes[i]|=strtol(tkey,0,16);//atoi(tkey);				
		} //for(i=0;i<NumChars/2;i++) {
	return(NumByte);			
	} //if (TotalChar>0) {

	return(0);
} //int ConvertStringToBytesNum(unsigned char *InstStr,unsigned char *InstBytes,int NumByte)  

int GetTimeStamp(char *timestamp)
{
//	time_t ctime; /* calendar time */
	//struct timezone tz;
	struct tm *ltime;
	struct timeval detail_time; //detailed time (us)

	//get timestamp
	gettimeofday(&detail_time, NULL); //get ms  &tz can be NULL
	ltime = localtime(&detail_time.tv_sec);
//	ctime = time(NULL); /* get current cal time */
//	ltime = localtime(&ctime);
	
	//YYYYMMDD-HHMMSS-MMM
	sprintf(timestamp, "%04d%02d%02d_%02d%02d%02d_%03d",
		ltime->tm_year + 1900, ltime->tm_mon + 1, ltime->tm_mday,
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec,
		(int)detail_time.tv_usec / 1000);

	return(1);
} //int GetTimeStamp(char *timestamp)

//Return the current time in milliseconds
uint32_t GetTimeInMS(void)
{
	struct timeval detail_time; //detailed time (us)
	uint64_t milliseconds;
/*
#if USE_RTAI
	int hard_timer_running;
#endif

#if USE_RTAI
	//start rtai time if not started already
	if ((hard_timer_running = rt_is_hard_timer_running())) 
	{
	//printf("Skip hard real_timer setting...\n");
	//sampling_interval = nano2count(TICK_TIME);
	}
	else 
	{
	printf("Starting real time timer...\n");
	rt_set_oneshot_mode();
	start_rt_timer(0);
	}
	return((uint32_t)(rt_get_time_ns()/1000));
#endif
//#if !USE_RTAI
*/
	gettimeofday(&detail_time,NULL); // &tz); //get ms
	milliseconds = detail_time.tv_sec*1000LL + detail_time.tv_usec/1000; 

	return((uint32_t) milliseconds);  //for now only using 32-bit number
//#endif //!USE_RTAI
} //uint32_t GetTimeInMS(void)

int OpenRobotNetworkLogFile(void) 
{
	char	timestamp[512],LogStr[512];
	MAC_Connection *lmac;


	GetTimeStamp(timestamp);

	//close any existing Network Log?

	//open if not already open
	if (!(RStatus.flags&ROBOT_STATUS_NETWORK_LOG)) {
		RStatus.flags|=ROBOT_STATUS_NETWORK_LOG;		
#if Linux
		sprintf(RStatus.RobotNetworkLogFileName,"%s/Network_%s.log",RStatus.LogFolder,timestamp);
#endif
#if WIN32
		sprintf(RStatus.RobotNetworkLogFileName,"%s\\Network_%s.log",RStatus.LogFolder,timestamp);
#endif
		RStatus.fRobotNetworkLog=fopen(RStatus.RobotNetworkLogFileName,"wb");
		if (!RStatus.fRobotNetworkLog) {
			fprintf(stderr,"fopen of log file \"%s\" failed\n",RStatus.RobotNetworkLogFileName);
			return(0);
		} //		if (!RStatus.fRobotNetworkLog) {
		//because there are just IPs in the network log, it helps to associate each IP with a PCB
		//so go through each MAC_Connection and output their IP and PCBName 
		sprintf(LogStr,"IP\tPCBName\n"); 
		lmac=RStatus.iMAC_Connection;
		while(lmac!=0) {
//			if (lmac->pcb!=0) { //Connection is to a PCB
				//sprintf(LogStr,"%s%s\t%s\n",LogStr,lmac->DestIPAddressStr,lmac->EthPCBInfo.Name); //presumes pcb exists
				sprintf(LogStr,"%s%s\t%s\n",LogStr,lmac->DestIPAddressStr,lmac->pcb.Name); //presumes pcb exists
//			}
			lmac=lmac->next;
		} //while(lmac!=0) {
		if (strlen(LogStr)>12) {
			fwrite(LogStr,1,strlen(LogStr),RStatus.fRobotNetworkLog);
		} //if (strlen(LogStr)>12) {

	} //if (!(RStatus.flags&ROBOT_STATUS_NETWORK_LOG)) {
	return(1);
} //int OpenRobotNetworkLogFile(void) 

int CloseRobotNetworkLogFile(void) 
{
	if (PStatus.flags&PInfo) {
		fprintf(stderr,"CloseRobotNetworkLogFile()\n");
	}
	//close if open
	if (RStatus.flags&ROBOT_STATUS_NETWORK_LOG) {
		if (RStatus.fRobotNetworkLog!=0) {
			fclose(RStatus.fRobotNetworkLog);
			RStatus.fRobotNetworkLog=0;
		}
		RStatus.flags&=~ROBOT_STATUS_NETWORK_LOG;		
	}//if (RStatus.flags&ROBOT_STATUS_NETWORK_LOG) {
	return(1);
} //int CloseRobotNetworkLogFile(void) 



//int LogRobotNetworkData(MAC_Connection *lmac,char *RemoteIP,char *DataStr,int NumBytes)
int LogRobotNetworkData(char *FromIP,char *ToIP,char *DataStr,int NumBytes)
{
	char	timestamp[512];


	GetTimeStamp(timestamp); //get timestamp

	if (RStatus.fRobotNetworkLog==0) {
		if (!OpenRobotNetworkLogFile()) {
			return(0);
		}
	} //if (RStatus.fRobotNetworkLog==0) {
	
	//add a record of the received data to the Robot log
	//fprintf(RStatus.fRobotLog,"%s\t%s\t%s\t%s\n",timestamp,lmac->Name,RemoteIP,DataStr);
	fprintf(RStatus.fRobotNetworkLog,"%s\t%s\t%s\t%s\n",timestamp,FromIP,ToIP,DataStr);
	return(1);
} //int LogRobotNetworkData(MAC_Connection *lmac,unsigned char *RemoteIP,unsigned char *DataStr,int NumBytes)

int OpenRobotModelLogFile(void) 
{
// time_t ctime; /* calendar time */
//	struct tm *ltime;
//	struct timeval detail_time; //detailed time (us)
	//char	timestamp[512];
	FTControl *tc;
	struct stat st;
	int result;


	if (PStatus.flags&PInfo) {
		fprintf(stderr,"OpenRobotModelLogFile\n");
	}

	//see if robot log folder exists
	result = stat(RStatus.CurrentRobotLogFolder, &st);
	if (result!=0) {
		//folder does not exist - so create it
		result=mkdir(RStatus.CurrentRobotLogFolder,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		if (result!=0) {
			fprintf(stderr,"could not create robot log folder '%s'\n",RStatus.CurrentRobotLogFolder);
		}
	} //

	//see if robot log/model folder exists
	result = stat(RStatus.CurrentRobotModelLogFolder, &st);
	if (result!=0) {
		//folder does not exist - so create it
		result=mkdir(RStatus.CurrentRobotModelLogFolder,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		if (result!=0) {
			fprintf(stderr,"could not create robot log model folder '%s'\n",RStatus.CurrentRobotModelLogFolder);
		}
	} //

#if 0 
	//get timestamp
	ctime=time(NULL); /* get current cal time */
	ltime=localtime(&ctime);
	gettimeofday(&detail_time,NULL); //get ms
	//YYYYMMDD-HHMMSS-MMM
	sprintf(timestamp,"%04d%02d%02d_%02d%02d%02d_%03d",
		ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday,
		ltime->tm_hour,ltime->tm_min,ltime->tm_sec,
		(int)detail_time.tv_usec/1000);
#endif

	//close any existing Model Log?

	//to do make ROBOT_LOG_DIR path in config and use full path

	//see if logs folder exists, if not create 

	//open if not already open
	if (!(RStatus.flags&ROBOT_STATUS_MODEL_LOG) && strlen(RStatus.RobotModelLogFileName)>0) {
		RStatus.fRobotModelLog=fopen(RStatus.RobotModelLogFileName,"wb");
		if (!RStatus.fRobotModelLog) {
			fprintf(stderr,"fopen of log file \"%s\" failed\n",RStatus.RobotModelLogFileName);
			return(0);
		} //		if (!RStatus.fRobotModelLog) {



		//update button to show that we are now logging
		tc=GetFTControl("btnRobotModelLog");
		if (tc!=0) {
				strcpy(tc->text,"Stop Model Log");
				DrawFTControl(tc);
		} 

	RStatus.flags|=ROBOT_STATUS_MODEL_LOG;	  //do this last, because accel data starts getting logged instantly

	} //if (!(RStatus.flags&ROBOT_STATUS_MODEL_LOG)) 
	return(1);
} //int OpenRobotModelLogFile(void) 

int CloseRobotModelLogFile(void) 
{
	FTControl *tc;
	int lClose;

	if (PStatus.flags&PInfo) {
		fprintf(stderr,"CloseRobotModelLogFile()\n");
	}

	lClose=0;
	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		lClose=1;
		RStatus.flags&=~ROBOT_STATUS_MODEL_LOG;		 //first clear flag because accel samples keep coming in
#if Linux
		usleep(100); //wait for samples to stop coming in
#endif
#if WIN32
		Sleep(100); //wait for samples to stop coming in
#endif
	} 


	//close if open
	if (lClose) {
		if (RStatus.fRobotModelLog!=0) {
			fclose(RStatus.fRobotModelLog);
			RStatus.fRobotModelLog=0;
		}

		//update button to show that we are not logging
		tc=GetFTControl("btnRobotModelLog");
		if (tc!=0) {
				strcpy(tc->text,"Start Model Log");
				DrawFTControl(tc);
		} 

	}//if (lClose) {
	return(1);
} //int CloseRobotModelLogFile(void) 

int LogRobotModelData(char *LogStr)
{
	char	timestamp[512];


	GetTimeStamp(timestamp);//get timestamp

	if (RStatus.fRobotModelLog==0) {
//		if (!OpenRobotModelLogFile()) {
			fprintf(stderr,"Error in LogRobotModelData(): Log file named '%s' handle=0. Log Entry=%s\n",RStatus.RobotModelLogFileName,LogStr);
			return(0);
//		}
	} //if (RStatus.fRobotModelLog==0) {
	
	//add a record of the received data to the Robot log

//this is causing a crash when running a script- fRobotModelLog ==0
	fprintf(RStatus.fRobotModelLog,"%s\t%s\n",timestamp,LogStr);
	return(1);
} //int LogRobotModelData(char *LogStr)


int SendCameraGET(MAC_Connection *lmac, char *GETStr)
{
	unsigned char Inst[256];
	int len;

	Inst[0] = 0xf1;
	Inst[1] = 0xd0;
	Inst[2] = 0x0;
	//Inst[3] = 0x0; //length of entire STUN packet
	Inst[4] = 0xd1;
	Inst[5] = 0x00;
	Inst[6] = lmac->cam.InstNum >> 8; //inst number- 2 bytes big endian
	Inst[7] = lmac->cam.InstNum & 0xff;//
	Inst[8] = 0x01;
	Inst[9] = 0x0a;
	Inst[10] = 0x00;
	Inst[11] = 0x00; //
	//sprintf(tstr, "GET /snapshot.cgi?loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lcam->cam.Username, lcam->cam.Password, lcam->cam.Username, lcam->cam.Password);
	//sprintf(tstr, "GET /livestream.cgi?streamid=10&rate=15&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lcam->cam.Username, lcam->cam.Password, lcam->cam.Username, lcam->cam.Password);

	len = strlen(GETStr);
	Inst[3] = 12 + len;// 0x67; //0x67=103 bytes  livestream
	*(unsigned int *)&Inst[12] = len; //presumes this is 32-bit little endian- which seems kind of unlikely given that the rest of the packet is big endian- 
	//the alternative is that (possibly [10]+) [11]+[12] are little endian, and bytes [13],[14],[15] are something else
	strcpy(&Inst[16], GETStr);
	SendDataToMAC(lmac, Inst, len + 16);
	lmac->cam.InstNum++;  //increase number of instructions sent

	return(1);
} //int SendCameraGET(MAC_Connection *lmac, char *GETStr)



//Thread that listens for data from the robot
#if Linux
int Thread_ListenToRobot(void) 
#endif
#if WIN32
DWORD WINAPI Thread_ListenToRobot(LPVOID lpParam) 
#endif
{
	int recvbuflen = DEFAULT_NETWORK_BUFLEN;
	unsigned char recvbuf[DEFAULT_NETWORK_BUFLEN] = "";
	char DataStr[DEFAULT_NETWORK_BUFLEN*2],Inst[512],tstr[512];
	FTControl *tc,*tcmotor,*tcpower,*tc2;
	int NumBytes,RecognizedSource;
	MAC_Connection *lmac,*lmacLocalToOutside,*lmacRequester,*lmacnew,*lcam;
	struct sockaddr RemoteAddr;
	struct sockaddr_in taddr, TempAddress;
	socklen_t RemoteAddrLen;
	char RemoteIP[INET_ADDRSTRLEN], TempIPAddressStr[INET_ADDRSTRLEN];
	int RemotePort;
	char RequestingIP[INET_ADDRSTRLEN];
//	FTWindow *twin;
	int FromLocalWiredMAC,MAC_Already_Exists;
	int ConvertedData,EthPCBNameNum,AccelNameNum,NumAccels;
	//int EthAccelsPCBNameNum;//,EthMotorsPCBNameNum;
	int AccelsPCBNameNum;
//	EthAccels_PCB *EthAccelTouch;
	Accels_PCB *iAccelsPCB;
	FTWindow *twin;
	int i,j,ImageDataSize,SensorNum;
	FILE *fptr;
	char timestamp[512];
	AnalogSensorInfoTable *AnalogInfo;
	AnalogSensor *lanalog;

	//fd_set read_fd_set;
	//int NewSocket;
	//socklen_t NewSocketSize;
	//struct sockaddr_in RemoteMAC;
#if USE_RT_PREEMPT
#define RT_PRIORITY (49) //PREEMPT_RT uses 50 for kernel tasklets and interrupt handlers by default so use 49
#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
#define ThreadInterval 1000000 //1ms
	struct timespec t;
	struct sched_param param;
	int keep_on_running = 1;

#endif
#if USE_RTAI
	static RT_TASK *RT_Listen_Robot_Task;
	static RTIME ThreadInterval;
	int keep_on_running = 1;
#endif


	memset(DataStr,0,256);

	tc=GetFTControl("txtGetRobotData");
	if (tc==0) {
		fprintf(stderr,"Error: Couldn't get FTControl: txtGetRobotData\n");
		return(0);
	}

/*
	//get the MAC_Connection to get the socket
	tc2=GetFTControl("ddRobotSrcIPAddressStr");
	if (tc2==0) {
		fprintf(stderr,"Error: Thread_ListenToRobot could not find FTControl ddRobotSrcIPAddressStr\n");
		return(0);
	}
	lmac=Get_MAC_Connection(tc2->text); //later add port# too 
	if (lmac==0) {
		fprintf(stderr,"Error: Thread_ListenToRobot could not find any MAC_Connection for %s\n",tc2->text);
		return(0);
	}
*/

	//wait for flag to indicate that the Wired MAC_Connection has been fully initialized and
	//we can proceed with listening
	while(!(RStatus.flags&ROBOT_STATUS_LISTEN_TO_ROBOT)); 

	lmac=RStatus.WiredInterface;

	if (lmac==0) {
		fprintf(stderr,"Thread_ListenToRobot() Found no Wired Network Interface\n");
		return(0);		
	}


	RemoteAddrLen=sizeof(RemoteAddr); //this is needed so recvfrom doesn't return 0.0.0.0
	memset(&RemoteAddr,0,RemoteAddrLen); //need?

	//Listen only works with TCP, not UDP, since UDP is a connectionless protocol, and is instead a message-passing protocol
	//Windows: Listen places a socket in a state in which it is listening for an incoming connection
	//GNU: listen enables the socket to accept connections, thus making it a server socket
//	NumBytes=listen(lmac->Socket,ROBOT_MAX_LENGTH_OF_PENDING_QUEUE_OF_CONNECTIONS);
/*
	iResult=listen(lmac->Socket,SOMAXCONN);
	if (iResult<0) {
#if Linux
		fprintf(stderr,"Error: listen to Robot failed - socket=%d return code %d\n",lmac->Socket,iResult);
		perror("listen");
#endif
#if WIN32
		fprintf(stderr,"Error: listen to Robot failed with error: %ld\n",WSAGetLastError());
#endif
		return(0);
	}
*/
	fprintf(stderr,"Listening to Robot\n");
#if 0 
	//Initialize the set of active sockets
	FD_ZERO(&lmac->active_fd_set); //clear active set
	FD_SET(lmac->Socket,&lmac->active_fd_set); //add the local/server MAC socket to the list of active sockets to monitor
	//this socket will accept new connections 
#endif


#if USE_RT_PREEMPT
	param.sched_priority = RT_PRIORITY;
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		fprintf(stderr, "Error: In Thread_ListenToRobot() sched_setscheduler failed.\n");
	} 

	//Lock memory
	/*
	if (mlockall(MCL_CURRENT_MCL_FUTURE) == -1) {
		fprintf(stderr, "Error: In Thread_ListenToRobot() mlockall failed.\n");
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
		t.tv_nsec += ThreadInterval;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}

		//set keep_on_running to 0 to exit above while
		if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
			keep_on_running = 0;
		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {

#endif //USE_RT_PREEMPT

#if USE_RTAI
  if (!(RT_Listen_Robot_Task = rt_task_init_schmod(nam2num("RTAI_Listen_Robot"), 2, 0, 0, SCHED_FIFO, 1))) {
                printf("Error: RTAI: Cannot init periodic RT_Listen_Robot_Task\n");
        }
	ThreadInterval = nano2count(1000000); //every 1ms (1000000ns)
	rt_task_make_periodic(RT_Listen_Robot_Task, rt_get_time(), ThreadInterval);
	rt_make_hard_real_time();
	while (keep_on_running) {
	//insert your main periodic loop here
    rt_task_wait_period();
		//set keep_on_running to 0 to exit above while
		if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
			keep_on_running=0;
		} //if (!(lmac->flags&ROBOT_MAC_CONNECTION_LISTENING)) {
#endif //USE_RTAI


#if !(USE_RTAI | USE_RT_PREEMPT)
	//listen for data received from the robot
	// Receive until the peer closes the connection
	//while 1 is ok because the connection is a blocking connection and so recv blocks (waits) until there is received data
	//while (RStatus.flags&ROBOT_STATUS_LISTEN_TO_ROBOT) {
	while (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {
#endif
//	while(RStatus.flags&ROBOT_STATUS_LISTEN_TO_ROBOT) ( //replace with this?- currently no option to stop listening
#if 0
		//block until input arrives on one or more active sockets
		FD_ZERO(&read_fd_set); //clear active set
		read_fd_set=lmac->active_fd_set;
		if (select(FD_SETSIZE,&read_fd_set,NULL,NULL,NULL)<0) {
			fprintf(stderr,"Error: Thread_ListenToRobot: select() returned an error.\n");
			return(0);
		} //if (select(FD_SETSIZE,&read_fd_set,NULL,NULL,NULL)<0) {

		//Service all the sockets with input pending
		for(i=0;i<FD_SETSIZE;++i) {
			if (FD_ISSET(i,&read_fd_set)) {  //this socket has input pending
					fprintf(stderr,"Socket %d has input pending\n",i);
/*
					if (i==lmac->Socket) { //Connection request on local server socket
						NewSocketSize=sizeof(RemoteMAC);
						NewSocket=accept(lmac->Socket,(struct sockaddr *)&RemoteMAC,&NewSocketSize);
						if (NewSocket<0) {
							fprintf(stderr,"Error: Thread_ListenToRobot: accept() returned an error.\n");
							return(0);
						} //if (NewSocket<0) {
						fprintf(stderr,"Robot connect from IP %s port %hd\n",inet_ntoa(RemoteMAC.sin_addr),ntohs(RemoteMAC.sin_port));
						FD_SET(NewSocket,&lmac->active_fd_set); //add this new socket to the list of sockets being listened to
					} else { //if (i==lmac->Socket) 
*/
						//Data arrived on an already-connected socket
						//read the data from the socket
						//NumBytes = recv(lmac->Socket,recvbuf,recvbuflen, 0);
						//
#endif
//						NumBytes = recvfrom(i,recvbuf,recvbuflen,0,&RemoteAddr,&RemoteAddrLen);
						//for 
					//recvfrom returns number of bytes received or -1 if an error occurred
						NumBytes = recvfrom(lmac->Socket,(char *)recvbuf,recvbuflen,0,&RemoteAddr,&RemoteAddrLen);
						if (NumBytes>0) {							

							//inet_ntop is thread-safe
							inet_ntop(AF_INET,&(((struct sockaddr_in*)&RemoteAddr)->sin_addr),RemoteIP,INET_ADDRSTRLEN);
							RemotePort = ntohs((((struct sockaddr_in*)&RemoteAddr)->sin_port));
							ConvertedData=0;


							//with polling a lot of data this should be commented:
							if (RStatus.flags&ROBOT_STATUS_INFO) {								
								//fprintf(stderr,"NumBytes=%d\n",NumBytes);
								ConvertBytesToString(recvbuf,DataStr,NumBytes); //convert bytes to string
								ConvertedData=1;
								fprintf(stderr,"%s: sent %d bytes to %s\n%s\n",RemoteIP,NumBytes,lmac->Name,DataStr);
							} 


							if ((RStatus.flags&ROBOT_STATUS_NETWORK_LOG) && (RStatus.flags&ROBOT_STATUS_LOG_INCOMING_DATA)) { //log all incoming data
									//log needs data in a text string so convert
									if (!ConvertedData) {
										ConvertBytesToString(recvbuf,DataStr,NumBytes); //convert bytes to string
										ConvertedData=1;
									}
									//LogRobotNetworkData(lmac,RemoteIP,DataStr,NumBytes);
									LogRobotNetworkData(RemoteIP,lmac->SrcIPAddressStr,DataStr,NumBytes);
							}	//if (RStatus.flags&ROBOT_STATUS_LOG_INCOMING_DATA) { //log all incoming data


							RecognizedSource = 0;
							FromLocalWiredMAC=0;

							//see if the packet was sent from the local wired interface
							if (!lmac!=0 && RemoteIP!=0 && !strcmp(lmac->SrcIPAddressStr, RemoteIP)) {
								FromLocalWiredMAC = 1;
							}


#if 0 //camera code
							//if camera data, just process it here and forget about everything below
							//f141- camera id's itself
							//camera will also send this if an f1e0 is not answered soon enough
							//and it means a check_user.cgi needs to be sent again
							//if (*(unsigned int *)recvbuf==0x140041f1) {
							if (*(unsigned short int *)recvbuf == 0x41f1) { 
								//get camera MAC_Connection
								//lcam = Get_MAC_Connection_By_Camera_IP(RemoteIP);
								RecognizedSource = 1;
								//lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								//get the connection to the RemoteIP that initiated the contact (port 32108)
								//and change to the port the camera chose to use
								//lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, 32108);
								//just check for dest IP (not ports), or possibly check by name- because port#'s change on same connection
								lcam = Get_MAC_Connection(RemoteIP);
								if (lcam == 0) {  //try the broadcast SrcIP
									memcpy(&TempAddress, &lmac->SrcAddress, sizeof(struct sockaddr_in));
									TempAddress.sin_addr.s_addr |= 0xff000000; //change to subnet broadcast IP
									inet_ntop(AF_INET, &TempAddress.sin_addr, TempIPAddressStr, INET_ADDRSTRLEN);
									lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, TempIPAddressStr, 32108);
								}
								if (lcam != 0) {  //found the connection

									//update last contact
									lcam->cam.LastContact = GetTimeInMS();

									//note we can reuse the same socket
									//closing and creating a new socket, changes the source port number									
									//update the dest ip (in case this was from a broadcast id)
									strcpy(lcam->DestIPAddressStr,RemoteIP);
									lcam->DestAddress=*(struct sockaddr_in *)&RemoteAddr; //this updates port too?
									//change the port on this connection
									lcam->DestPort = RemotePort;


									//and signal to the broadcast connection that a new camera connected?
									//indicate that this connection is a camera, so winCameras can easily recognize it among the other MAC_Connections
									lcam->flags |= ROBOT_MAC_CONNECTION_IS_CAMERA;
									//for now the camera name is just "cam"+MAC- to get the MAC the socket would need to be SOCK_RAW and src and dest MACs would be the first few bytes in recvbuf
									//todo: make this socket raw, and make a function to assemble a raw socket (UDP) header
									//sprintf(lcam->Name, lcam->PhysicalAddress);
									strcpy(lcam->Name, &recvbuf[14]);
									//copy the camera serial number 
									strcpy(lcam->cam.SerialNumber, &recvbuf[14]);
									/*
									//open the cameras window if not already open
									twin = GetFTWindow("winCameras");
									if (twin == 0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
										lmac->flags |= ROBOT_MAC_CONNECTION_OPEN_WINDOW;
										lmac->AddWindowFunction = (FTControlfunc *)winCameras_AddFTWindow;
									}
									else {
										//FT_SetFocus(twin,0); //give the window the focus
									}
									*/
									
									//populate/refresh the winCamera window if open so user can select camera and click GetImage
									//add the camera serial number as Location
									PopulateCameraDropdowns();

									//copy this text to communicate to the camera later if, for example, the connection is ended
									//memcpy(lcam->cam.idbuffer, recvbuf, NumBytes);
									//lcam->cam.idbuffersize = NumBytes;
									//send back the packet to establish the connection port for the camera
									SendDataToMAC(lcam, recvbuf, NumBytes);
									//camera will confirm with an 0xf1 0x42 up to 4x 
									//until getting an 0xf1 0xe0

									//if we already auth'd, we need to again
									lcam->cam.flags&=~ROBOT_CAMERA_INFO_AUTHENTICATED;


								} //if (lcam == 0) {  //unknown camera IP
								//ProcessCameraData(lmac,recvbuf,NumBytes,RemoteIP);
							} //if ((unsigned int)recvbuf==0x140041f1) {


							//f142 camera responds to establish connection port
							if (*(unsigned short int *)recvbuf == 0x42f1) { //f142
								RecognizedSource = 1;
								//get camera MAC_Connection
								//lcam = Get_MAC_Connection_By_Camera_IP(RemoteIP);
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam == 0 || (lcam->cam.flags&ROBOT_CAMERA_INFO_CONNECTED)) {  
									//unknown camera IP or already conencted (already got f142 and sent back f1e0)
								} else {
									//update last contact
									lcam->cam.LastContact = GetTimeInMS();

									lcam->cam.flags |= ROBOT_CAMERA_INFO_CONNECTED;
									//send back an f1 e0 00 00 to confirm the connection port for the camera
									Inst[0] = 0xf1;
									Inst[1] = 0xe0;
									Inst[2] = 0;
									Inst[3] = 0;
									SendDataToMAC(lcam, Inst,4);
									
									
#if 0 //this is now done by functions themselves using while(not authenticated)
									//if we are getting images do the authentication here
									if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {
										sprintf(tstr, "GET /livestream.cgi?streamid=10&rate=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lcam->cam.Username, lcam->cam.Password, lcam->cam.Username, lcam->cam.Password);
										SendCameraGET(lcam, tstr);
									} //if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {
#endif

								}//if (lcam == 0) {  //unknown camera IP
							} //if (*(unsigned short int *)recvbuf == 0x41f2) { //f142

							//f1e0 we need to respond with f1e1
							//answering f1e0 with f1e1 is a way of keeping the connection open
							if (*(unsigned short int *)recvbuf == 0xe0f1) { //f1e0
								RecognizedSource = 1;
								//get camera MAC_Connection
								//lcam = Get_MAC_Connection_By_Camera_IP(RemoteIP);
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam == 0) {  //unknown camera IP
								}
								else {
									//always reply to f1e0 with f1e0
									//if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {
									//if (RStatus.flags&(ROBOT_STATUS_GET_IMAGE | ROBOT_STATUS_GET_VIDEO)) { //temp
										//only keep the connection open if we are getting image(s)
										//send back an f1 e0 00 00 to confirm the connection port for the camera
										Inst[0] = 0xf1;
										Inst[1] = 0xe1;
										Inst[2] = 0;
										Inst[3] = 0;
										SendDataToMAC(lcam, Inst, 4);
									//} //if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {
								} //if (lcam == 0) {  //unknown camera IP
							} //if (*(unsigned short int *)recvbuf == 0xe0f1) { //f1e0

#if 0 
							//if camera is connected and last contact is <now-200ms, then send another f1e0
							//if last contact is <now-500ms, then close connection and try to reconnect 
							if (*(unsigned short int *)recvbuf == 0xe1f1) { //f1e1
								RecognizedSource = 1;
								//

								//get camera MAC_Connection
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam != 0) {  //camera IP still connected

									Inst[0] = 0xf1;
									Inst[1] = 0xe0;
									Inst[2] = 0;
									Inst[3] = 0;
									SendDataToMAC(lcam, Inst, 4);
								} //if (lcam != 0) {
							} //							if (*(unsigned short int *)recvbuf == 0xe1f1) { //f1e1
#endif

				
							//f1d0 contains an HTTP used to:
							//1: call check_user.cgi, and send "result=0" for check_user.cgi request
							//2: call live_stream.cgi, and for sending the actual image data
							//3: call snapshot.cgi, and for sending the actual image data
							if (*(unsigned short int *)recvbuf == 0xd0f1) { //f1d0
								RecognizedSource = 1;
							//get camera MAC_Connection
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam == 0) {  //unknown camera IP
								}
								else {
								//update last contact
								lcam->cam.LastContact = GetTimeInMS();

								//confirm that result=0
								if (!strncmp(&recvbuf[16], "result=0", 8)) {  //camera sent result=0 to HTTP request
									//removing because code may send another check_user even if already connected
									if (!(lcam->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
										lcam->cam.flags |= ROBOT_CAMERA_INFO_AUTHENTICATED;
										//send back an f1 d1 00 06 +6 bytes - to confirm recvd OK?
										Inst[0] = 0xf1;
										Inst[1] = 0xd1;
										Inst[2] = 0;
										Inst[3] = 6;
										Inst[4] = 0xd1;
										Inst[5] = 0x00;
										Inst[6] = 0;// lcam->cam.InstNum >> 8; //inst number- 2 bytes big endian
										Inst[7] = 1;// lcam->cam.InstNum & 0xff;// 0x01; //
										//Inst[6] = 0x00;  //cur inst number f1d0 is cur inst num, but f1d1 could be something else
										//Inst[7] = 0x01;
										Inst[8] = 0x00;
										Inst[9] = 0x00;
										SendDataToMAC(lcam, Inst, 10);

#if 0 //this is now done from functions using while(not authenticated)
										if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {

											sprintf(tstr, "GET /livestream.cgi?streamid=10&rate=0&loginuse=%s&loginpas=%s&user=%s&pwd=%s&", lcam->cam.Username, lcam->cam.Password, lcam->cam.Username, lcam->cam.Password);
											SendCameraGET(lcam, tstr);
										} //if (lcam->cam.flags&(ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO)) {
#endif
									}// if (!(lcam->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
								}
								else { //if (!strncmp(&recvbuf[16], "result=0", 8)) {  //send result=0 to HTTP request
									//not result=0
									//} //if (!(lcam->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
									//else {
									//we have already confirmed the connection with the camera
									//check for image data
									//d101 = livestream
									//d100 = snapshot?
									if (recvbuf[4] == 0xd1 && (recvbuf[5] == 0x01 || recvbuf[5] == 0x00)) {
										//images are sent f0 d0 len(2) + 36 byte header (992bytes*14 + 976 bytes)=14864bytes 0x3a10
										//36 byte header contains:
										//0-3: d1 01 PacketNumber(2)
										//4-35: currently unknown
										//verify with number of bytes sent in packet
										i = lcam->cam.EndImagePointer;

										if (NumBytes + lcam->cam.EndImagePointer > CAMERA_IMAGE_BUFFER_SIZE){
											FTMessageBox("Camera not sending EOI (ffd9) and overran memory buffer", FTMB_OK, "Robot - Error", 0);
										}
										//determine header size
										if (lcam->cam.EndImagePointer == lcam->cam.StartImagePointer) {
											if (recvbuf[5] == 0x01) {  //livestream					
												//first packet has more header info than the rest
												j = 40;
											} else {
												j = 16; //snapshot first packet has 16 byte header
											}											
										} else {//if (recvbuf[5] == 0x01) {  //livestream
												//snapshot only has 12 byte header
											j = 8; //
										} //if (recvbuf[5] == 0x01) {  //livestream
										if (lcam->cam.EndImagePointer == lcam->cam.StartImagePointer) {
											
										}
										else {
											ImageDataSize = NumBytes - 8;
											//copy the image data to the image buffer
											memcpy(&lcam->cam.Image[i], &recvbuf[8], ImageDataSize);
										}

										ImageDataSize = NumBytes - j;
										//copy the image data to the image buffer
										memcpy(&lcam->cam.Image[i], &recvbuf[j], ImageDataSize);

										lcam->cam.EndImagePointer += ImageDataSize;
										lcam->cam.CurrentPacket++;
										if (recvbuf[NumBytes - 2] == 0xff && recvbuf[NumBytes - 1] == 0xd9) {
											//got end of image ffd9
											//confirm all remaining packets
											//STUN header
											Inst[0] = 0xf1;
											Inst[1] = 0xd1;
											Inst[2] = 0x0;
											Inst[3] = (lcam->cam.CurrentPacket - lcam->cam.LastAckdPacket) * 2 + 4;

											Inst[4] = 0xd1;
											Inst[5] = 0x01;
											//Number of images we are acking
											j = lcam->cam.CurrentPacket - lcam->cam.LastAckdPacket;
											Inst[6] = j >> 8;
											Inst[7] = j & 0xff;
											j = 8;
											for (i = lcam->cam.LastAckdPacket; i < lcam->cam.CurrentPacket; i++) {
												Inst[j] = i >> 8;
												Inst[j + 1] = i & 0xff;
												j += 2;
											} //for i
											lcam->cam.LastAckdPacket = lcam->cam.CurrentPacket;
											SendDataToMAC(lcam, Inst, j);


											//after the image, server sends f1e0
											//perhaps to confirm- got last image?
											//and repeats this until the start of the next image
											//every 100 or 200ms
											Inst[0] = 0xf1;
											Inst[1] = 0xe0;
											Inst[2] = 0;
											Inst[3] = 0;
											SendDataToMAC(lcam, Inst, 4);

											//if only 1 image is wanted stop the stream
											//sometimes this is returned in an ICMP from the camera
											//because the camera already sent an f1f0- so added check for connection still active
											//but it didn't make any difference
											//currently sometimes a second image may be sent
											if ((lcam->cam.flags&ROBOT_CAMERA_INFO_GET_IMAGE) && (lcam->cam.flags &ROBOT_CAMERA_INFO_AUTHENTICATED)) {
												//if (lcam->cam.flags&ROBOT_CAMERA_INFO_GET_IMAGE) {
												//if (RStatus.flags&ROBOT_STATUS_GET_IMAGE) { //temp


#if 0 												
												//causes problem when getting second image
												//the company program never sends an f1f0
												//stop stream- to issue another snapshot.cgi, we need to reconnect to a new camera port each time apparently
												//because I don't think check_user.cgi or snapshot.cgi work after because
												//apparently the camera just responds to an interval of when to send 1,2,3 ...etc. fps
												Inst[0] = 0xf1;
												Inst[1] = 0xf0;
												Inst[2] = 0;
												Inst[3] = 0;
												SendDataToMAC(lcam, Inst, 4);
#endif												
												//lcam->flags = 0; //to make sure rest of program knows this connection is closed
												//lcam->cam.flags &= ~ROBOT_CAMERA_INFO_GET_IMAGE;
												//we need to keep the show and save flags
												//lcam->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED | ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO | ROBOT_CAMERA_INFO_SHOW_IMAGE  );
												//note currently camera stays connected (using same port)
												//lcam->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO | ROBOT_CAMERA_INFO_SHOW_IMAGE);
												//lcam->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO);
												lcam->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED | ROBOT_CAMERA_INFO_GET_IMAGE | ROBOT_CAMERA_INFO_GET_VIDEO);
												//currently camera remains connected with e1-e0

												//for each snapshot a check_user needs to preceed it

												//and delete MAC_Connection
											} //if (lcam->cam.flags&ROBOT_CAMERA_INFO_GET_IMAGE) { 


											if (lcam->cam.flags&ROBOT_CAMERA_INFO_SHOW_IMAGE) {
												//copy the image to the control
												tc2 = GetFTControl("imgCamera00");
												if (tc2 != 0) {
													//memcpy(tc->image)
													//LoadJPGToFTControl(lcam->cam.Image,lcam->cam.CurrentImagePointer, tc,0,0);


													LoadJPGToFTControl(&lcam->cam.Image[lcam->cam.StartImagePointer], lcam->cam.EndImagePointer - lcam->cam.StartImagePointer, tc2, 0, 0);
													//tc->image[0] = lcam->cam.Image;
													//tc->image[0] = &lcam->cam.Image[lcam->cam.StartImagePointer];
													//and refresh the control
													DrawFTControl(tc2); //causes the jpeg image data to end prematurely- possibly because a second jpeg is sometimes sent quickly

													//kind of a bad hack but works:- not for video
													//if (lcam->cam.flags&ROBOT_CAMERA_INFO_GET_IMAGE) {
													//	lcam->cam.flags &= ~ROBOT_CAMERA_INFO_SHOW_IMAGE; //need to stop second jpg from getting drawn
													//	}
												} //if (tc2!=0)
											} //if (lmac->cam.flags&ROBOT_CAMERA_INFO_SHOW_IMAGE) {

											if (lcam->cam.flags&ROBOT_CAMERA_INFO_SAVE_IMAGE) {
												//write the image to a file
												GetTimeStamp(timestamp);
												sprintf(tstr, "%s%s.jpg", lcam->cam.FilePath, timestamp);
												//sprintf(tstr, "c:\\temparea\\%s.jpg", timestamp);
												fptr = fopen(tstr, "wb");
												fwrite(&lcam->cam.Image[lcam->cam.StartImagePointer], 1, lcam->cam.EndImagePointer - lcam->cam.StartImagePointer, fptr);
												fclose(fptr);
												//kind of a bad hack but works:
												lcam->cam.flags &= ~ROBOT_CAMERA_INFO_SAVE_IMAGE; //need to stop second jpg from getting saved
											} //if (lmac->cam.flags&ROBOT_CAMERA_INFO_SAVE_IMAGE) {

											lcam->cam.FrameNum++;
											lcam->cam.StartImagePointer = lcam->cam.EndImagePointer + 1;
											if (lcam->cam.StartImagePointer + CAMERA_MAX_IMAGE_SIZE > CAMERA_IMAGE_BUFFER_SIZE) {
												lcam->cam.StartImagePointer = 0;
											}
											lcam->cam.EndImagePointer = lcam->cam.StartImagePointer;

										}
										else { //if (recvbuf[NumBytes - 2] == 0xff && recvbuf[NumBytes - 1] == 0xd9) {
											//not last image data packet

											if (lcam->cam.CurrentPacket > lcam->cam.LastAckdPacket + 4) {
												//at least 6 packets have been received- send an acknowledge for them
												//packets are acknowledged with:
												//f1 d1 00 12 NumberofPackets(2) PacketNum0(2) PacketNum1(2) ...
												//STUN header
												Inst[0] = 0xf1;
												Inst[1] = 0xd1;
												Inst[2] = 0x0;
												Inst[3] = (lcam->cam.CurrentPacket - lcam->cam.LastAckdPacket) * 2 + 4;
												Inst[4] = 0xd1;
												Inst[5] = 0x01;
												//Number of images we are acking
												j = lcam->cam.CurrentPacket - lcam->cam.LastAckdPacket;
												Inst[6] = j >> 8;
												Inst[7] = j & 0xff;
												j = 8;
												for (i = lcam->cam.LastAckdPacket; i < lcam->cam.CurrentPacket; i++) {
													Inst[j] = i >> 8;
													Inst[j + 1] = i & 0xff;
													j += 2;
												} //for i
												lcam->cam.LastAckdPacket = lcam->cam.CurrentPacket;
												SendDataToMAC(lcam, Inst, j);
											} //if (lcam->cam.CurrentPacket > lcam->cam.LastAckdPacket + 6) {

										} //if (recvbuf[NumBytes - 2] == 0xff && recvbuf[NumBytes - 1] == 0xd9) {

										//the receipt of each of the packets/frames needs to be acknowledged, 
										//multiple packets/frames can be acknowledged in a single return packet

									} //if (recvbuf[4] == 0xd1 && recvbuf[5] == 0x01) {
								} ////if (!strncmp(&recvbuf[16], "result=0", 8)) {  //send result=0 to HTTP request
								//} //if (!(lcam->cam.flags&ROBOT_CAMERA_INFO_AUTHENTICATED)) {
							} //if (lcam == 0) {  //unknown camera IP
							} //if (*(unsigned short int *)recvbuf == 0xe0f1) { //f1e0

							//f1d1- camera confirms GET (f1d0)- needed for motor moves, need to resend if camera ignores/misses
							//note that commands to move motors seem to be often sent at the end of a jpg frame
							if (*(unsigned short int *)recvbuf == 0xd1f1) { //f1d1
								RecognizedSource = 1;
								//get camera MAC_Connection
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam != 0) {
									//update last contact
									lcam->cam.LastContact = GetTimeInMS();
								} //if (lcam != 0)
							} //if (*(unsigned short int *)recvbuf == 0xd1f1) { //f1d1


							//f1e1 camera confirms server's f1e0
							if (*(unsigned short int *)recvbuf == 0xf0e1) { //f0e1
								RecognizedSource = 1;
								//get camera MAC_Connection
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam == 0) {  //unknown camera IP
								} else {
									//update last contact
									lcam->cam.LastContact = GetTimeInMS();
#if 0 //used only at certain times
									Inst[0] = 0xf1;
									Inst[1] = 0xe0;
									Inst[2] = 0;
									Inst[3] = 0;
									SendDataToMAC(lcam, Inst, 4);
#endif
								}
							} //if (*(unsigned short int *)recvbuf == 0xf0e1) { //f1e1


							//f1f0- close connection- get connection (by camera name?) and delete_MAC_Connection
							if (*(unsigned short int *)recvbuf == 0xf0f1) { //f1f0
								RecognizedSource = 1;
								//get camera MAC_Connection
								lcam = Get_MAC_Connection_By_IPs(lmac->SrcIPAddressStr, lmac->SrcPort, RemoteIP, RemotePort);
								if (lcam != 0) {  //camera IP still connected

#if 0 
											
									//ack stop stream request? - no returns ping dest unreachable
									Inst[0] = 0xf1;
									Inst[1] = 0xf1;
									Inst[2] = 0;
									Inst[3] = 0;
									SendDataToMAC(lcam, Inst, 4);
#endif

									//update last contact
									lcam->cam.LastContact = GetTimeInMS();

									//this also resets GET_IMAGE flag
//									lcam->cam.flags = 0;
									//we need to keep the show and save flags
									lcam->cam.flags &= ~(ROBOT_CAMERA_INFO_AUTHENTICATED | ROBOT_CAMERA_INFO_CONNECTED | ROBOT_CAMERA_INFO_GET_IMAGE|ROBOT_CAMERA_INFO_GET_VIDEO);
									lcam->cam.InstNum=0; //reset the instruction number



								}//if (lcam != 0) {  //camera still connected
							} //if (*(unsigned short int *)recvbuf == 0xf0f1) { //f1f0

							if (!RecognizedSource) { //not from a camera
#endif //camera code
								//If this data has a different sender IP connected to it (first 4 bytes), 
								//Pass the result back to the sender IP if different from our IP
								//currently for IP camera- the IP is not attached
								//if (strncmp((char *)recvbuf, (char *)((unsigned int *)&lmac->SrcAddress.sin_addr.s_addr), 4)) {
								if (strncmp((char *)recvbuf, (char *)((unsigned int *)&lmac->SrcAddress.sin_addr.s_addr), 4)) {
									//IP in packet shows that requester was different from this MAC
									//Currently, all sending of data to MACs outside the robot is done through the "Outside"
									//(usually wireless wlan0) MAC
									//So see if that outside (wireless) connection to the requester MAC exists, and if not create it
									//because we can't reach any MACs outside of the robot with any other MAC, including this one)
									strcpy(RequestingIP, "");
									if (NumBytes >= 4) {
										memcpy(&taddr.sin_addr, recvbuf, 4);
										//get the requesting IP in the form of a string:
										inet_ntop(AF_INET, &taddr.sin_addr, RequestingIP, INET_ADDRSTRLEN);
									} //if (NumBytes>=4) {
									//inet_ntop(AF_INET,&(((struct sockaddr_in*)&recvbuf)->sin_addr),RequestingIP,INET_ADDRSTRLEN);
									if (RStatus.flags&ROBOT_STATUS_INFO) {
										fprintf(stderr, "Send back to: %s\n", RequestingIP);
									}
									//Connect_Remote_MAC checks to see if a connection already exists, and if not	creates one
									//get the MAC_Connection of the Local to the Outside MAC (usually the wireless MAC - wlan0)
									lmacLocalToOutside = ListenToOutside();
									if (lmacLocalToOutside != 0) {
										//now create/get the MAC_Connection to the remote requestor via the LocalToOutside (Wireless) MAC
										lmacRequester = Create_Remote_MAC_Connection(lmacLocalToOutside->SrcIPAddressStr, DEFAULT_ROBOT_PORT, RequestingIP, DEFAULT_ROBOT_PORT);
										//we can just use the generic SendDataToMAC which only needs a MAC_Connection (that has src and dest IP, port, and socket), a pointer to the data, and the data length
										SendDataToMAC(lmacRequester, recvbuf, NumBytes);
									}
									else { //if (lmacLocalToOutside != 0) {
										if (RStatus.flags&ROBOT_STATUS_INFO) {
											fprintf(stderr, "Could not open Outside MAC connection\n");
										}
									}//if (lmacLocalToOutside != 0) {
								} //if (strncmp(recvbuf,(unsigned int *)&lmac->SrcAddress.sin_addr.s_addr,4)) {

								//Currently I just print the characters into a textbox without knowing if they are chars or ints
								//with no data type formatting
								//translate to ascii 
								if (!ConvertedData) {
									ConvertBytesToString(recvbuf, DataStr, NumBytes); //convert bytes to string
									ConvertedData = 1;
								}
								/*
															for(i=0;i<NumBytes;i++) {
															//go through each nibble and convert to ascii
															//0x88 will become "88"
															sprintf(&DataStr[i*2],"%x",(int)(recvbuf[i]&0xf0));
															sprintf(&DataStr[i*2+1],"%x",(int)(recvbuf[i]&0xf));
															//				itoa((int)(recvbuf[i]&0xf0),&tc->text[i*2],16);
															//				itoa((int)(recvbuf[i]&0xf),&tc->text[i*2+1],16);
															}
															DataStr[NumBytes*2]=0;
															*/
								//and put result in textbox
								//sprintf(DataStr,"%s",key);

								//Because this could be a reply to a sent broadcast packet (192.168.0.255)
								//determine if we have seen this IP address (todo: and mac address) before
								MAC_Already_Exists=1;
								lmacnew = Get_MAC_Connection(RemoteIP);  //get any existing MAC_Connection based on IP
								if (!lmacnew) {
									lmacnew = Create_Remote_MAC_Connection(lmac->SrcIPAddressStr, DEFAULT_ROBOT_PORT, RemoteIP, DEFAULT_ROBOT_PORT);
									if (!lmacnew) {
										fprintf(stderr, "Create_Remote_MAC_Connection(%s:%d,%s:%d) failed\n", lmac->SrcIPAddressStr, DEFAULT_ROBOT_PORT, RemoteIP, DEFAULT_ROBOT_PORT);
										
									}	else { //if (!lmacnew) {
										MAC_Already_Exists=0;
										fprintf(stderr, "New Remote IP:%s\n", RemoteIP);
									}//if (!lmacnew) {
								} //if (!lmacnew) {

								//check for an Eth PCB identifying itself
								if (!FromLocalWiredMAC && recvbuf[4] == 0x01) { //"01" instruction
									//the next 5 bytes is the MAC address of the PCB, so copy that to the MAC_Connection
									memcpy(lmacnew->PhysicalAddress, &recvbuf[5], 6);
									//and in a string format
									ConvertBytesToString(&recvbuf[5], lmacnew->PhysicalAddressStr, 6); //6 bytes
									strncpy(lmacnew->Name, (char *)&recvbuf[11], PCB_NAME_LENGTH);
									lmacnew->Name[PCB_NAME_LENGTH] = 0;
									//possibly check to see if the MAC already exists in a MAC_Connection, and if yes, then just ignore (but possibly should update that MAC_Connection, or delete it and create a new MAC_Connection)
									fprintf(stderr, "Identified PCB: \"%s\" with MAC: %s\n", lmacnew->Name, lmacnew->PhysicalAddressStr);
									lmacnew->flags|=ROBOT_MAC_CONNECTION_IS_ETHPCB; //to determine which MAC_Connections are PCBs (others are network only connections)
									//fprintf(stderr,"with MAC: %02x%02x%02x%02x%02x%02x\n",recvbuf[5],recvbuf[6],recvbuf[7],recvbuf[8],recvbuf[9],recvbuf[10]);
									//check this address against the lookup table to see 
									//where on the robot this PCB is located
									EthPCBNameNum = GetPCBNameNumByMAC(lmacnew->PhysicalAddress);
									if (EthPCBNameNum > -1) {
										//This PhysicalAddress has a PCB on the robot associated with it
										//fprintf(stderr,"MAC has PCB associated with it.\n");
										//copy the PCB type ("Accel", "Motor", "Power", etc.)
										memcpy(lmacnew->pcb.Kind,lmacnew->Name,strlen(lmacnew->Name));
										lmacnew->pcb.Kind[strlen(lmacnew->Name)]=0; //terminate string
										//copy the user given name from the currently loaded robot config files
										memcpy(lmacnew->pcb.Name, RStatus.EthPCBInfo[EthPCBNameNum].Name,strlen(RStatus.EthPCBInfo[EthPCBNameNum].Name));
										lmacnew->pcb.Name[strlen(RStatus.EthPCBInfo[EthPCBNameNum].Name)]=0; //terminate string
										//also copy the PCB name number to the MAC_Connection (for logging in win_Motors when sending a Turn_Motor, to get the motor name from the pcb and motor number):
										lmacnew->pcb.Num=EthPCBNameNum;
										//strcpy(lmacnew->PCBName,RStatus.pcb[EthPCBNameNum].PCBName);
										//memcpy(&lmacnew->pcb, &RStatus.pcb[EthPCBNameNum], sizeof(Robot_PCB));
										//lmacnew->EthPCBInfo=&RStatus.EthPCBInfo[EthPCBNameNum];
										//copy the loaded Robot's PCB info (name, MAC) into the MAC_Connection EthPCBInfo structure. Note that MAC_Connections keep their own copy of PCB info because some PCBs may not be recognized. 
										//memcpy(&lmacnew->pcb,&RStatus.EthPCBInfo[EthPCBNameNum],sizeof(EthPCBInfoTable));
										//									fprintf(stderr,"PCBName of PCB on Robot: %s\n",lmacnew->PCBName);
										fprintf(stderr, "Found PCB Name: %s\n", lmacnew->pcb.Name);
										//lmacnew->Num=EthPCBNameNum;

										//if this is an EthMotors or GAMTP PCB		
										if (!strcmp(lmacnew->Name, "Motor") || !strcmp(lmacnew->Name, "GAMTP")) {
											RecognizedSource = 1;												//Set Motor Duty Cycle (currently the default is 1ms, and needs to be 50us so turning motor frequency is above human hearing ~20khz)
											if (!SetEthMotorsDutyCycle(lmacnew,ROBOT_MOTORS_DEFAULT_MOTOR_DUTY_CYCLE_CLK)) {
												fprintf(stderr, "Failed to set %s Motor Duty Cycle clock to %d us",lmacnew->pcb.Name,ROBOT_MOTORS_DEFAULT_MOTOR_DUTY_CYCLE_CLK);
											} 

											//todo: add MAC_Already_Exists code similar to for EthAccels?
												//get EthMotorsPCBNameNum - index into RStatus.EthMotorsPCBInfo
//											EthMotorsPCBNameNum=GetEthMotorsPCBNameNum(lmacnew->pcb.Name);
//											lmacnew->pcb.EthMotorsPCBNameNum=EthMotorsPCBNameNum;


											//connect this PCB with the EthMotors window, because win_Motors matches this variable to 
											//identify new EthMotors PCBs	
											lmacnew->AddWindowFunction=(FTControlfunc *)winMotors_AddFTWindow;
											//don't need because is mrmcpy'd above from which was read in when config file was read in: lmacnew->pcb.Num=EthPCBNameNum;//used by Thread_RobotTasks() to match PCB to MAC_Connection

											//see if Motor window is open, if no open it
											twin=GetFTWindow("winMotors");
											if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
												lmacnew->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
											} else {  //if (twin==0) 
												//don't give focus because it's annoying if the user is looking at a different window
												//and because we don't know yet if this data is from the currently selected PCB in the Motors window
												//FT_SetFocus(twin,0); //give the window the focus

												//repopulate the list of PCBs if the EthMotors window is open
												tcmotor=GetFTControl("ddMotorPCBNum");
												if (tcmotor!=0) {
														Populate_EthMotors_Dropdown(tcmotor);
												} 

											} //if (twin==0) {  

										} //if (!strcmp(lmacnew->Name, "Motor")) {


										//If this is an EthAccelTouch PCB, check to see if new, if yes, initialize it's data structure and add this MAC to its EthAccelTouch data structure
										if (!strcmp(lmacnew->Name, "Accel") || !strcmp(lmacnew->Name, "GAMTP")) {
											RecognizedSource = 1;
											iAccelsPCB = 0;
											//if (RStatus.NumEthAccelsPCBs > 0) { //if there are already EthAccelTouch PCB identified
												//see if this EthAccelTouch PCB was already identified using its MAC_Connection (later could add multiple checks, for example on PCBName, MAC address, etc.)
												//EthAccelTouch = Get_EthAccelsPCB(lmacnew); //use MAC address to identify if PCB already has a MAC_Connection
												//EthAccelTouch=&lmacnew->pcb.EthAccelsPCB;
											//}
											//Set the Timer Interval for any EthAccelTouch PCB identifying itself (may need to reset timer so even for a non-new MAC) 
											if (!SetEthAccelTimerInterval(lmacnew, ACCEL_TIMER_INTERVAL_IN_MS)) {
												fprintf(stderr, "Failed to set Timer Interval for %s", lmacnew->pcb.Name);
											} else {
												fprintf(stderr, "Set %s timer interval to %d ms.\n", lmacnew->pcb.Name,ACCEL_TIMER_INTERVAL_IN_MS);
											}
											//clear the EthAccelTouch data structure (with all the accelerometer and analog sensor settings)
											//EthAccelTouch=&lmacnew->pcb.EthAccelsPCB;
											iAccelsPCB=&lmacnew->pcb.AccelsPCB;
											memset(iAccelsPCB,0,sizeof(Accels_PCB));//clear the iAccelsPCB data structure
											//is in MAC_Connection iAccelsPCB->PCBNum=EthPCBNameNum;  //used by Thread_RobotTasks() to match PCB to an accelerometer
											//lmacnew->pcb.Num=EthPCBNameNum;//-not needed- copied from RStatus.pcb used by Thread_RobotTasks() to match PCB to MAC_Connection
											//iAccelsPCB->mac = lmacnew; //todo: eventually replace by just sending lmac. copy the pointer to the MAC_Connection to get the IP to send instructions later on 	
											//set any calibration offsets that were read earlier from accel.conf, gyro.conf or mag.conf
											//get and set the EthPCBInfo index (PCBNum)

											AccelsPCBNameNum=GetAccelsPCBNameNum(lmacnew->pcb.Name);
											iAccelsPCB->AccelsPCBNum=AccelsPCBNameNum;
											NumAccels=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->NumAccels;
											iAccelsPCB->NumAccels=NumAccels;  //set Number of active accels on detected PCB struct
											iAccelsPCB->NumAnalogSensors=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->NumAnalogSensors;
											//Copy the Accel and Analog Sensor masks too (used in Start all accel and analog sensors)
											iAccelsPCB->AccelMask=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AccelMask;
											iAccelsPCB->AnalogSensorMask=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnalogSensorMask;

											for(i=0;i<NumAccels;i++) { //for each accel		
//														memcpy(iAccelsPCB->Accel[i].ACali,CaliFile[EthPCBNameNum-1].AOffset[i],sizeof(float)*3); 
													memcpy(iAccelsPCB->Accel[i].ACali,RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AOffset[i],sizeof(float)*3); 
													memcpy(iAccelsPCB->Accel[i].GCali,RStatus.AccelsPCBInfo[AccelsPCBNameNum]->GOffset[i],sizeof(float)*3); 
													memcpy(iAccelsPCB->Accel[i].MCali,RStatus.AccelsPCBInfo[AccelsPCBNameNum]->MOffset[i],sizeof(float)*3); 

													//copy the Accel Name number
													//AccelNameNum=(EthPCBNameNum)*3+i;
													AccelNameNum=GetAccelNameNum(RStatus.AccelsPCBInfo[AccelsPCBNameNum]->Name);
													
													iAccelsPCB->Accel[i].NameNum=AccelNameNum;  //used to match accel name
													if (RStatus.AccelInfo[AccelNameNum].flags&ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z) {
														iAccelsPCB->Accel[i].flags|=ACCELEROMETER_SWAP_Y_AND_MINUS_Z;
													}
													if (RStatus.AccelInfo[AccelNameNum].flags&ACCEL_INFO_TABLE_SWAP_MINUS_Y_AND_MINUS_Z) {
														iAccelsPCB->Accel[i].flags|=ACCELEROMETER_SWAP_MINUS_Y_AND_MINUS_Z;
													}
													//fprintf(stderr,"EthPCBNameNum-1=%d\n",EthPCBNameNum-1);
													//fprintf(stderr,"PCB %d Accel %d NameNum=%d\n",EthPCBNameNum,i,iAccelsPCB->Accel[i].NameNum);										
													//fprintf(stderr,"%03.3f %03.3f %03.3f\n",iAccelsPCB->Accel[i].GCali[0],iAccelsPCB->Accel[i].GCali[1],iAccelsPCB->Accel[i].GCali[2]);
											} //for i
											//reset Min magnetic values (Max at 0 is ok)
											for(i=0;i<NumAccels;i++) {
												//set an initial max and min for magnetometers
												//maximum uT possible
												iAccelsPCB->Accel[i].MinM.x=-20.0; 
												iAccelsPCB->Accel[i].MinM.y=-20.0;
												iAccelsPCB->Accel[i].MinM.z=1.0;
												iAccelsPCB->Accel[i].MaxM.x=18.0; 
												iAccelsPCB->Accel[i].MaxM.y=18.0;
												iAccelsPCB->Accel[i].MaxM.z=36.0;
											} //for(i=0;i<3;i++)			

											//Set Analog Sensor Min and Max for each a
											fprintf(stderr,"Setting Analog Sensors Min and Max Voltage\n");
											for (i=0;i<RStatus.AccelsPCBInfo[AccelsPCBNameNum]->NumAnalogSensors;i++) { 
												//for each analog sensor (starts at 1 on PCB but 0 in software because we only want to deal with the number of the actual sensors starting with sensor[0])
												//note SetAnalogSensorsMinAndMax takes a Mask, so 1==TS[0], this is why 1<<(i+1) is used
												//Set the MaxV and MinV on the actual PCB (firmware) for each touch sensor
												SensorNum=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnalogSensorInfo[i]->Num;

												SetAnalogSensorsMinAndMax(lmacnew,(unsigned int)1<<SensorNum,RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][0],RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][1]);		
												fprintf(stderr,"AS=%d min=%3.1fV max=%3.1fV\n",1<<SensorNum,RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][0],RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][1]);

												//get pointer to AnalogSensor user provided info
												AnalogInfo=RStatus.AccelsPCBInfo[AccelsPCBNameNum];
												//get pointer to current instance of actual AnalogSensor
												//Note: currently analog sensors start at 1
												lanalog=&iAccelsPCB->Analog[SensorNum];
												//Set AnalogSensor flag (kind of analog sensor, potentiometer or touch sensor)
												lanalog->flags=AnalogInfo->flags;

												//Set the MaxVAngle and MinVAngle from the AnalogSensorInfoTable into the AnalogSensor	instance.
												lanalog->MinV=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][0]; //MinV
												lanalog->MaxV=RStatus.AccelsPCBInfo[AccelsPCBNameNum]->AnOffset[SensorNum][1]; //MaxV
												lanalog->MaxVAngle=AnalogInfo->MaxVAngle;
												lanalog->MinVAngle=AnalogInfo->MinVAngle;
											} //for i


											if (!MAC_Already_Exists) { //a new AccelTouch PCB
												if (!strcmp(lmacnew->Name, "Accel")) { 
													RStatus.NumEthAccelsPCBs++;
												}
												RStatus.NumAccelsPCBs++;

											} //if (!MAC_Already_Exists) { //a new AccelTouch PCB

											//Problem: two windows need to be opened (winAccels and winAnalogSensors), but only 1 can be set to open currently
											//Create and open Accelerometers window if not already created and open
											twin = GetFTWindow("winAccels");
											if (twin == 0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
												lmac->AddWindowFunction = (FTControlfunc *)winAccels_AddFTWindow;
												lmac->flags |= ROBOT_MAC_CONNECTION_OPEN_WINDOW;
											}	else {
												//FT_SetFocus(twin,0); //give the window the focus
												FillAccelItemList(); //refresh Accel PCB itemlist- adds any new pcb since window was opened
											} //if (twin==0) {

											//Create and open Accelerometers window if not already created and open
											twin = GetFTWindow("winAnalogSensors");
											if (twin == 0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
												lmac->AddWindowFunction = (FTControlfunc *)winAnalogSensors_AddFTWindow;
												lmac->flags |= ROBOT_MAC_CONNECTION_OPEN_WINDOW;
											}	else {
												//FT_SetFocus(twin,0); //give the window the focus
												//FillAnalogSensorItemList(); //refresh Analog PCB itemlist- adds any new pcb since window was opened
											} //if (twin==0) {

										} //if (!strcmp(lmacnew->Name,"Accel")) {

										//if this is an EthPower PCB		
										if (!strcmp(lmacnew->Name, "Power")) {
											RecognizedSource = 1;
											//connect this PCB with the EthPower window, because win_Power matches this variable to identify new EthPower PCBs	
											lmacnew->AddWindowFunction=(FTControlfunc *)winPower_AddFTWindow;

											//see if Power window is open, if no open it
											twin=GetFTWindow("winPower");
											if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
												lmacnew->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
											} else {
												//don't give focus because it's annoying if the user is looking at a different window
												//and because we don't know yet if this data is from the currently selected PCB in the Power window
												//FT_SetFocus(twin,0); //give the window the focus

												//repopulate the list of PCBs if the EthMotors window is open
												tcpower=GetFTControl("ddPowerPCBNum");
												if (tcpower!=0) {
														Populate_EthPower_Dropdown(tcpower);
												} 

											}
										} //if (!strcmp(lmacnew->Name, "Power")) {




									}	else {  //if (EthPCBNameNum > -1) {
										if (strlen(lmacnew->pcb.Name)==0) { //no name/location (like "Motor") recognized above
											//so just add "Generic"
											fprintf(stderr, "Name of PCB on Robot: unknown\n");
											//because the user needs to change between Motor PCBs in the Motor window
											//add a unique generic PCBName
											//strcpy(lmacnew->PhysicalAddressStr, "123456789012");
											//sprintf(lmacnew->EthPCBInfo.Name, "Generic_%s", lmacnew->PhysicalAddressStr);
											sprintf(lmacnew->pcb.Name, "Generic_%s", lmacnew->PhysicalAddressStr);
											//to give an unlocated (generic/test) PCB a unique number just use the last 4 bytes of the MAC
											//lmacnew->Num=(((int)recvbuf[8])<<24)+(((int)recvbuf[7])<<16)+(((int)recvbuf[6])<<8)+(int)recvbuf[5];
										}

										RStatus.NewMac=lmacnew; //so NewEthPCB window can fill in MAC address

										//if a robot is loaded
										if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {
											//see if new Eth PCB window is open, if no have the main thread open it
											twin=GetFTWindow("win_NewEthPCB");
											if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on

												//open window so user can choose which Name/location is assocaited with this Eth PCB
												//win_NewEthPCB_CreateFTWindow();
												lmacnew->AddWindowFunction=(FTControlfunc *)win_NewEthPCB_CreateFTWindow;
												lmacnew->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
											} //twin==0

										} //if (EthPCBNameNum>-1) {
									} //if (RStatus.flags&ROBOT_STATUS_USER_ROBOT_LOADED) {

									//update EthPCB count on winMain
									tc=GetFTControl("lblPCBCount1");
									if (tc!=0) {
										sprintf(tc->text,"%d",GetCurrentPCBCount());
										DrawFTControl(tc);
									}

								} //if (!FromLocalWiredMAC && recvbuf[4]==0x01) { //"01" instruction




								//Determine where to route this data depending on client name
								if (!strncmp(lmacnew->Name, "Motor", 5) || (!strncmp(lmacnew->Name, "GAMTP", 5))) {	//EthMotors or GAMTP PCB

									//get Motor PCB number
									//lmacnew->Num=atoi(&lmacnew->Name[5]);
									ProcessMotorData(lmacnew, recvbuf, NumBytes, DataStr);
								} //if (!strncmp(lmacnew->Name,"Motor",5)) {	//EthMotors PCB

								if (!strncmp(lmacnew->Name, "Accel", 5)|| (!strncmp(lmacnew->Name, "GAMTP", 5))) {	//EthAccelTouch or GAMTP PCB
									//get Accel PCB number
									//lmacnew->Num=atoi(&lmacnew->Name[5]);
									//see if this is Accelerometer or Analog Sensor data
									if (NumBytes > 4) {
										//because we need to open the Accelerometers window we process a 0x01 "id" command too
										if (recvbuf[4] == 0x01 || (recvbuf[4] >= ROBOT_START_ACCEL_INSTRUCTIONS
 && recvbuf[4] <= ROBOT_END_ACCEL_INSTRUCTIONS)) { //Accelerometer
							 				//print sample
											//fprintf(stderr,"NumBytes: %d\n",NumBytes);
											ProcessAccelSensorData(lmacnew, recvbuf, NumBytes, DataStr);
										}
										if (recvbuf[4] >= ROBOT_START_ANALOG_SENSOR_INSTRUCTIONS && recvbuf[4] <= ROBOT_END_ANALOG_SENSOR_INSTRUCTIONS) { //analog sensor
											ProcessAnalogSensorData(lmacnew, recvbuf, NumBytes, DataStr);
										}
										if (recvbuf[4] >= ROBOT_START_GPS_INSTRUCTIONS && recvbuf[4] <= ROBOT_END_GPS_INSTRUCTIONS) { //GPS module
											ProcessGPSData(lmacnew, recvbuf, NumBytes, DataStr);
										}


									} //if (NumBytes>4) {
								} //if (!strncmp(lmacnew->Name,"Accel",5)) {	//EthAccelTouch PCB

								if (!strncmp(lmacnew->Name, "Power", 5)) {	//EthPower PCB
									//see if this is Accelerometer or Analog Sensor data
//									if (NumBytes > 4) {
										//because we need to open the Power window we process a 0x01 "id" command too
									ProcessPowerData(lmacnew, recvbuf, NumBytes, DataStr);
								} //if (!strncmp(lmacnew->Name,"Power",5)) {	//EthPower PCB


								if (!RecognizedSource && !FromLocalWiredMAC) {
									//didn't recognize the source of this data so just print to the main Robot textbox
									//somehow this code is being reached sometimes- with many fast accel and touch sample arriving
									//for now only print inst 0x20 because accel sample gets are being printed here
									//if (DataStr[4]==0x20) {
									//	strcpy(tc->text, DataStr);
									//	DrawFTControl(tc);
									//} //if (DataStr[4]==0x20) {
								} //if (!RecognizedSource) {


//camera code 							} //if (!RecognizedSource) { //not from a camera

						} else { //if (NumBytes>0) {
							if (NumBytes==0) {
								fprintf(stderr,"0 Bytes received - Connection closed\n");
								Close_MAC_Socket(lmac);
								//FD_CLR(i,&lmac->active_fd_set); //remove this socket from the list of active sockets being listened to
								//Currently, I'm not sure what to do here - probably just close the remote connection but keep listening to other connection

								//StopListeningToRobot
								//RStatus.flags&=~ROBOT_LISTENING_TO_INSIDE;
				
								//return(0);
							} else { //if (NumBytes==0) {
#if Linux
								fprintf(stderr,"recvfrom failed with error\n");
								perror("recvfrom:"); //Connection refused
#endif
#if WIN32
								//fprintf(stderr,"recvfrom failed with error: %d\n", WSAGetLastError());
								sprintf(tstr, "recvfrom failed with error: %d\n", WSAGetLastError());
								FTMessageBox(tstr, FTMB_OK, "Robot - Error", 0);
#endif
									//I think when this happens I need to close this socket, recreate it, rebind it, 
									//and then reconnect all those that were connected to it
								//return(0);
							} //if (NumBytes==0) {
						} // if (NumBytes > 0) {

#if 0 
	//				} //if (i==lmac->Socket) 
			} //		if (FD_ISSET(i,&read_fd_set)) {  //this socket has input pending	
		} //	for(i=0;i<FD_SETSIZE;++i) {	
#endif
#if !(USE_RTAI | USE_RT_PREEMPT)
//	}  //while(1)
	} //	while (lmac->flags&ROBOT_MAC_CONNECTION_LISTENING) {
#endif  //!(USE_RTAI | USE_RT_PREEMPT)


#if USE_RT_PREEMPT
	}  //while (keep_on_running) {
#endif //USE_RT_PREEMPT

#if USE_RTAI
	}  //while (keep_on_running) {
	rt_task_delete(RT_Listen_Robot_Task);
#endif //USE_RTAI

	return(1);
}   //Thread_ListenToRobot


//faster method from: http://nghiaho.com/?p=997
//reported maximum error 0.0015 radians (0.085944 degrees)
double FastArcTan(double x)
{
    return PI_DIV_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
} //double FastArcTan(double x)



int ProcessMotorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 
{
	//FTWindow *twin;
	FTControl *tc;

	//output data to Motor00 textbox depending on which motor of 6 (0,1,2,3,4,5)

	//determine if the data from this PCB is the same as the PCB selected in the Motors window
	//perhaps a faster way would be just to set some global flag to determine which PCB is selected
	tc=GetFTControl("txtMotorDestIPAddressStr");									
	if (tc!=0) {
		//control exists
		if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
			//The PCB connected to this data is curently selected in the motors window
			//so update the data
			tc=GetFTControl("txtGetMotorData");									
			if (tc!=0) {
				strcpy(tc->text,DataStr);
				DrawFTControl(tc);
				return(1);
			} //if (tc!=0) {
		} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
	} //if (tc!=0)

	return(0);
} //int ProcessMotorData(MAC_Connection *lmac, char *recvbuf,int NumBytes) 

int ProcessAccelSensorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 
{
#define GYROSCOPE_CALIBRATION_MINIMUM  1.0//1.0 //the minimum value a gyro scope value can be to use the data, otherwise the acclerometer sample is used because there is not enough motion to use the gyroscope sample.
#define NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST 50//200 //the number of samples with no motion (as measured by GYROSCOPE_CALIBRATION_MINIMUM) before robot is viewed as at rest 200= 2 seconds
#define NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD 0//500 //the number of samples after the robot is at rest for blending the accelerometer angle back into the EstSegmentAngle estimated angle the motion code uses
#define USE_FAST_TRIG 0 //use fast atan, sine, and cosine functions			
	
	FTWindow *twin;
	FTControl *tc,*tcX,*tcY,*tcZ,*tcDegrees;
	int AccelNum,GotError,i,SelectedPCB,j;
	int16_t SampleX,SampleY,SampleZ,MSampleX,MSampleY,MSampleZ,GSampleX,GSampleY,GSampleZ,TSample;
	AccelSample fSample,fMSample,fGSample,fSample2,fMSample2,fGSample2;//,LastfGSample;
	char tstrX[256],tstrY[256],tstrZ[256],tstrDegrees[256];
//	EthAccels_PCB *leth;
	Accels_PCB *leth;
	Accelerometer *laccel;
	int AccelWindowOpen,AccelAxis;
	char LogStr[512],LogStr2[256];
	MODEL *lmodel;
	POINT3D angle;
	float sintheta,costheta,sinphi,cosphi;
	float fsin,fcos;
	float Pitch,Yaw,Roll,APitch,AYaw,ARoll,BlendFactor;
	POINT3D Mag,Acc,MagComp;
	int16_t temp,MOffsetX,MOffsetY,MOffsetZ;
	POINT3D fMOffset;
	AccelSample *LastfSample;
	int SwapYandZ;

	GotError=0;


	switch(recvbuf[4]) { //determine what kind of accel data by instruction

		case ROBOT_ACCELMAGTOUCH_GET_HARD_IRON_OFFSET:
			//data is:
			//ip(0-3) inst(4) num(5)x(6-7)y(8-9)z(10)num(11)x(13-14)...
			i=5;
			while(i<NumBytes) {
				AccelNum=(int)recvbuf[i];
				MOffsetX=(uint16_t)((recvbuf[i+1]<<8)|recvbuf[i+2]); 
				fMOffset.x=((float)MOffsetX)/20.0;// /2 and /10 for uT
				MOffsetY=(uint16_t)((recvbuf[i+3]<<8)|recvbuf[i+4]);
				fMOffset.y=((float)MOffsetY)/20.0;
				MOffsetZ=(uint16_t)((recvbuf[i+5]<<8)|recvbuf[i+6]);  
				fMOffset.z=((float)MOffsetZ)/20.0;
		
				//print output to text for now
				//todo: add to window
				fprintf(stderr,"Accel: %d MOffset: %x %x %x\n",AccelNum,MOffsetX,MOffsetY,MOffsetZ);
				fprintf(stderr,"                   %f %f %f\n",fMOffset.x,fMOffset.y,fMOffset.z);

				i+=7;
			} //while i
			
		break;
		case ROBOT_ACCELMAGTOUCH_GET_ACCELEROMETER_VALUES:
		case ROBOT_ACCELMAGTOUCH_START_POLLING_ACCELEROMETER:
		case ROBOT_ACCELMAGTOUCH_START_ACCELEROMETER_INTERRUPT:

			//determine which EthAccelsPCB this data is from its MAC_Connection
			//leth=Get_EthAccelsPCB(lmac);
			//leth=&lmac->pcb.EthAccelsPCB;
			leth=&lmac->pcb.AccelsPCB;
			if (!leth) {
				fprintf(stderr,"Received data from an EthAccelsPCB with no known MAC_Connection.\n");	
				return(0);
			} //



			//see if Accelerometers window is open
			twin=GetFTWindow("winAccels");
			AccelWindowOpen=0;
			if (twin!=0 && twin->flags&WOpen) {  
				AccelWindowOpen=1;
			}

			if (AccelWindowOpen) {
				//determine if this data comes from an EthAccel PCB that is currently selected in the Accel Window
				//output data to the Accelerometer box depending on which Accelerometer (0-2) the data corresponds to
				SelectedPCB=0;
				tc=GetFTControl("txtAccelDestIPAddressStr");									
				if (tc!=0) {
					//control exists
					if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
						//The PCB connected to this data is curently selected in the Accelerometers window
						//so update the data in the textboxes
						SelectedPCB=1;
					} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
				} //if (tc!=0) {
			} //if (AccelWindow)

			//go through each sensor, convert x and y values to units of g, and print in the correct label
			//x and y arrive as 14-bit samples, with +2g (0x1fff) as the maximum and -2g (0x3fff) as the minimum
			//so to convert we only need to convert the sample to a float and divide by 0x3fff/2 (0x1fff 8191).
			//earlier note:I find that the accel is returning 0x413 when vertical, so it needs to be adjusted (and g measurements can be > 1.0g) 
			//accel from gravity of earth is 9.8 m/s^2

			//record to model log
			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				//sprintf(LogStr,"%s",lmac->EthPCBInfo.Name); //presumes pcb exists
//				sprintf(LogStr,"%s\tA",lmac->EthPCBInfo.Name); //presumes pcb exists A=Accelerometer data
				sprintf(LogStr,"%s\tA",lmac->pcb.Name); //presumes pcb exists A=Accelerometer data

			} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {


//			if (RStatus.flags&ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL) {
			if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
				//get the robot model
				lmodel=F3DStatus.imodel;
				//get the base accel #) 

#if 0 
				switch (leth->PCBNum) {

					case ROBOT_PCB_ETHACCELS_LEFT_LEG: //(!strcmp(lmac->EthPCBInfo.Name,"ETHACCELS_LEFT_LEG")) {
						AccelBase=1; //axis 1,2,3 are left foot, lower leg, upper leg 
						break; //} 
					case ROBOT_PCB_ETHACCELS_RIGHT_LEG: //if (!strcmp(lmac->EthPCBInfo.Name,"ETHACCELS_RIGHT_LEG")) {
						AccelBase=4; //axis 4,5,6 are right foot, lower leg, upper leg
				  	break; //}
				  case ROBOT_PCB_ETHACCELS_TORSO: //if (!strcmp(lmac->EthPCBInfo.Name, "ETHACCELS_TORSO")) {
						AccelBase=7; //axis 7,8,9 are waist, torso and head 
						break; //}
/*					case ROBOT_PCB_ETHACCELS_LEFT_ARM: //if (!strcmp(lmac->EthPCBInfo.Name, "ETHACCELS_LEFT_ARM")) {
						AccelBase = 9; //axis 9,10,11 are left upper arm, lower arm, and hand
						break;//}
					case ROBOT_PCB_ETHACCELS_RIGHT_ARM: //if (!strcmp(lmac->EthPCBInfo.Name, "ETHACCELS_RIGHT_ARM")) {
						AccelBase = 12; //axis 12,13,14 are right upper arm, lower arm, and hand
						break;//}
*/

					default:
						fprintf(stderr,"unknown PCB send accelerometer samples\n");
						break;

				} //switch
#endif 

				memset(&angle,0,sizeof(POINT3D)); //clear the angle
			} //if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) { 





			i=5;
			while(i<NumBytes) { //for each accelerometer in the EthAccelTouch packet
				AccelNum=(int)recvbuf[i];


				laccel = &leth->Accel[AccelNum];

				laccel->flags&=~ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_X;  //clear this flag, because change from gyro to accel only happens at one same interval- otherwise win_Tasks motion code would have to clear the flag, or would be adjusting the TargetAngle of an instruction with each new sample.
				//laccel->flags&=~ACCELEROMETER_CHANGE_FROM_GYRO_TO_MAG_Y;
				laccel->flags&=~ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_Z;

				//determine if any dimensions need to be swapped
				SwapYandZ=(laccel->flags&(ACCELEROMETER_SWAP_Y_AND_MINUS_Z|ACCELEROMETER_SWAP_MINUS_Y_AND_MINUS_Z));

				//convert the sample data to float
				//0x1fff=8191.0  
				//shift <<2 more which will be divided out later for +- sign to work
				//[0]=MSB 7:0 are 8 MSB [1]=LSB 7:2 are 6 LSB,
        //ReturnSampleX=((I2CData[1]<<0x8)| I2CData[2])>>0x2;
//				SampleX=(int16_t)((recvbuf[i+1]<<8|recvbuf[i+2])<<2);
#if USE_FXOS8700CQ
				SampleX=(int16_t)(recvbuf[i+1]<<8|recvbuf[i+2]);
				SampleY=(int16_t)(recvbuf[i+3]<<8|recvbuf[i+4]);
				SampleZ=(int16_t)(recvbuf[i+5]<<8|recvbuf[i+6]);

				//todo: convert .x .y .z to and array[3] because it is easier to just memcpy, and just as easy to understand
				//it needed to be done for the Thread_RobotTasks in order to easily choose which angle a condition was checking -otherwise I would need to equate 0 to .x, 1 to .y, 2 to .z instead of just angle[xyz]


//14-bit samples
				//bring down to +-2.0 scale  (8192/2=4096, so 8191/4096=+2.0g)
				//since /4096 is >>12 shifting back 2 places is >>14 	/16384
				//each bit is 0.244 mg (2g/8191=0.000244)

				fSample.x=(float)SampleX/16384.0;
				fSample.y=(float)SampleY/16384.0;
				fSample.z=(float)SampleZ/16384.0;

				if (RStatus.flags&ROBOT_STATUS_INFO) {
					//print sample
					fprintf(stderr,"Accel: %d Sample: %f %f %f\n",AccelNum,fSample.x,fSample.y,fSample.z);
				}

				MSampleX=(int16_t)(recvbuf[i+7]<<8|recvbuf[i+8]);
				MSampleY=(int16_t)(recvbuf[i+9]<<8|recvbuf[i+10]);
				MSampleZ=(int16_t)(recvbuf[i+11]<<8|recvbuf[i+12]);


				fMSample.x = ((float)MSampleX) / 10.0;
				fMSample.y = ((float)MSampleY) / 10.0;
				fMSample.z = ((float)MSampleZ) / 10.0;


				if ((RStatus.flags&ROBOT_STATUS_USE_CALIBRATION) && !(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { 
					fSample.x -= laccel->ACali[0];
					fSample.y -= laccel->ACali[1];
					fSample.z -= laccel->ACali[2];
					fMSample.x -= laccel->MCali[0];
					fMSample.y -= laccel->MCali[1];
					fMSample.z -= laccel->MCali[2];
				}//if ((RStatus.flags&ROBOT_STATUS_USE_CALIBRATION) && !(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { 


#endif  //USE_FXOS8700CQ
#if USE_MPU6050
//16-bit samples
				SampleX=-(int16_t)(recvbuf[i+1]<<8|recvbuf[i+2]); //note MPU X is opposite of FXOS8700
				switch(SwapYandZ) {
					case 0: //horizontal accel/gyro with (from robot perspective) +X=left, +Y=forward, +Z=up
						SampleY=-(int16_t)(recvbuf[i+3]<<8|recvbuf[i+4]); //-Y //note MPU Y is opposite of FXOS8700
						SampleZ=(int16_t)(recvbuf[i+5]<<8|recvbuf[i+6]);  //Z
					break;
					case ACCELEROMETER_SWAP_Y_AND_MINUS_Z:
						//if accel+gyro is mounted vertically on front of robot with Y- up, then swap Y and -Z
						SampleY=-(int16_t)(recvbuf[i+5]<<8|recvbuf[i+6]); //-Z
						SampleZ=-(int16_t)(recvbuf[i+3]<<8|recvbuf[i+4]); //-Y
					break;
					case ACCELEROMETER_SWAP_MINUS_Y_AND_MINUS_Z:
						//if accel+gyro is mounted vertically on back of robot with Y+ up, then swap -Y and -Z
						SampleY=(int16_t)(recvbuf[i+5]<<8|recvbuf[i+6]); //Z
						SampleZ=(int16_t)(recvbuf[i+3]<<8|recvbuf[i+4]); //Y
					break;
				} // switch(SwapYandZ)


				fSample.x=(float)SampleX/16384.0;
				fSample.y=(float)SampleY/16384.0; 
				fSample.z=(float)SampleZ/16384.0;

				if (RStatus.flags&ROBOT_STATUS_INFO) {
					//print sample
					//fprintf(stderr,"Accel: %d Sample: %d %d %d\n",AccelNum,SampleX,SampleY,SampleZ);
					fprintf(stderr,"Accel: %d Sample: %f %f %f\n",AccelNum,fSample.x,fSample.y,fSample.z);
				}

				//16-bit Temperature sample
				TSample=(int16_t)(recvbuf[i+7]<<8|recvbuf[i+8]);

				//Gyro data - note that Y and Z are swapped so that the x,y,z [0],[1] or [2] can be easily aligned
				//in angle comparisons: accel angle.x=gyro.x (pitch) accel angle.y=gyro.y (yaw) and accel angle.z=gyro.z (roll) 
				//note that gyro Z (originally yaw), needs to be negated
				GSampleX=(int16_t)(recvbuf[i+9]<<8|recvbuf[i+10]);
				switch(SwapYandZ) {
					case 0: //horizontal accel/gyro with (from robot perspective) +X=left, +Y=forward, +Z=up
						GSampleY=-(int16_t)(recvbuf[i+13]<<8|recvbuf[i+14]);  //is -Z
						GSampleZ=(int16_t)(recvbuf[i+11]<<8|recvbuf[i+12]);  //is Y
					break;
					case ACCELEROMETER_SWAP_Y_AND_MINUS_Z:
						//if accel+gyro is mounted vertically on front of robot with Y- up, then swap Y and -Z
						GSampleY=(int16_t)(recvbuf[i+11]<<8|recvbuf[i+12]);  //is Y
						GSampleZ=(int16_t)(recvbuf[i+13]<<8|recvbuf[i+14]);  //is Z
					break;
					case ACCELEROMETER_SWAP_MINUS_Y_AND_MINUS_Z:
						//if accel+gyro is mounted vertically on back of robot with Y+ up, then swap -Y and -Z
						GSampleY=-(int16_t)(recvbuf[i+11]<<8|recvbuf[i+12]);  //is -Y
						GSampleZ=-(int16_t)(recvbuf[i+13]<<8|recvbuf[i+14]);  //is -Z
					break;
				} // switch(SwapYandZ)


				//convert from int to float and from 0.1uT/LSB to 1uT/LSB
				//so this sample is in 0.1 uT
				//16-bit is -32768 (0x8000) to 32767 (0x7fff)
				//so 1=0.1uT 32767=1200uT, 32767 x f = 1200.0, f=.036622211 1/f=27.3058333
				//fMSample.x=(float)MSampleX*0.03662;
				//fMSample.y=(float)MSampleY*0.03662;
				//fMSample.z=(float)MSampleZ*0.03662;

	
				fGSample.x = (float)GSampleX / 131.0;
				fGSample.y = (float)GSampleY / 131.0;
				fGSample.z = (float)GSampleZ / 131.0;


				if ((RStatus.flags&ROBOT_STATUS_USE_CALIBRATION) && !(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { 
					fSample.x -= laccel->ACali[0];
					fSample.y -= laccel->ACali[1];
					fSample.z -= laccel->ACali[2];
					fGSample.x -= laccel->GCali[0];
					fGSample.y -= laccel->GCali[1];
					fGSample.z -= laccel->GCali[2];
				}//if ((RStatus.flags&ROBOT_STATUS_USE_CALIBRATION) && !(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { 

#endif  //USE_MPU6050



		//		fprintf(stderr,"%i %i %i\n",MSampleX,MSampleY,MSampleZ);
		//		fprintf(stderr,"%02x%02x %02x%02x %02x%02x\n",recvbuf[i+8],recvbuf[i+7],recvbuf[i+10],recvbuf[i+9],recvbuf[i+12],recvbuf[i+11]);

				//currently the lower leg accels (accelnum==1) have x is actual at y, and y at -x
				//you cannot simply negate it- you need to rotate it
		/*
				if (AccelNum==1) {
					temp=SampleX;
					SampleX=SampleY;
					SampleY=-temp;

					temp=MSampleX;
					MSampleX=MSampleY;
					MSampleY=-temp;
				}
		*/


				//shift the 14-bit int over <<2 then convert to a float and divide by 4 back to the correct scale

		/*
				if (SampleX>8191) { //0x1fff
					SampleX=~SampleX+1; //this changes 0x3fff (-1) to (0xc000+1) 
					SampleX&=0x1fff;    //0xc001 changes to 1
					SampleX=-SampleX;   //and then to -1
				}
				if (SampleY>8191) {
					SampleY=~SampleY+1; 
					SampleY&=0x1fff;    
					SampleY=-SampleY;   
				}
				if (SampleZ>8191) {
					SampleZ=~SampleZ+1; 
					SampleZ&=0x1fff;
					SampleZ=-SampleZ;
				}

				fSample.x=(float)SampleX/4096;
				fSample.y=(float)SampleY/4096;
				fSample.z=(float)SampleZ/4096;

			*/


		//		fprintf(stderr,"X,Y,Z= %x %x %x\n",SampleX,SampleY,SampleZ);

				if (!(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { //not calibrating accels 
					//for now round anything over 1.0 - for angles to work correctly in 3D rendering
					//verified on 12/13/15 that z readings definitely go> 1.0 (I saw 1.06) possibly because the
					//low-pass filter is not enabled, or because the accel needs to be calibrated more exactly
					if (fSample.x>1.0) {
						fSample.x=1.0;
					}
					if (fSample.y>1.0) {
						fSample.y=1.0;
					}
					if (fSample.z>1.0) {
						fSample.z=1.0;
					}
					if (fSample.x<-1.0) {
						fSample.x=-1.0;
					}
					if (fSample.y<-1.0) {
						fSample.y=-1.0;
					}
					if (fSample.z<-1.0) {
						fSample.z=-1.0;
					}
				} //if (!(RStatus.flags&ROBOT_STATUS_CALIBRATING)) { //not calibrating accels 



				//see if min or max
				//note that min and max can be retrieved from the accelerometer+magnetometer device toos
				//the registers are MIN_X_MSB and MIN_X_LSB, MIN_Y_MSB...etc.
				//currently I don't read min and max magnetic measurements
		/*
				laccel=&leth->Accel[AccelNum];
				if (fMSample.x>leth->Accel[AccelNum].MaxM.x) {
					laccel->MaxM.x=fMSample.x;
					//fprintf(stderr,"MaxM.x=%0.1f ",fMSample.x);
				}
				if (fMSample.x<laccel->MinM.x) {
					laccel->MinM.x=fMSample.x;
					//fprintf(stderr,"MinM.x=%0.1f ",fMSample.x);
				}
				if (fMSample.y>laccel->MaxM.y) {
					laccel->MaxM.y=fMSample.y;
					//fprintf(stderr,"MaxM.y=%0.1f ",fMSample.y);
				}
				if (fMSample.y<laccel->MinM.y) {
					laccel->MinM.y=fMSample.y;
					//fprintf(stderr,"MinM.y=%0.1f ",fMSample.y);
				}
				if (fMSample.z>laccel->MaxM.z) {
					laccel->MaxM.z=fMSample.z;
					//fprintf(stderr,"MaxM.z=%0.1f ",fMSample.z);
				}
				if (fMSample.z<laccel->MinM.z) {
					laccel->MinM.z=fMSample.z;
					//fprintf(stderr,"MinM.z=%0.1f ",fMSample.z);
				}
		*/
		



				//add the Sample to the EthAccelsPCB Accelerometer Status so other functions can get the sample
				//memcpy(&RStatus.Accel[AccelNum].Sample,&fSample,sizeof(AccelSample));
				//RStatus.Accel[AccelNum].flags|=ACCELEROMETER_GOT_SAMPLE;

//#if 0 
				//apply low-pass filter to accelerometer (not gyroscope): averages last sample and current sample by a factor alpha (0-1.0)
				//this helps to remove anomalous samples (which can cause problems when that sample occurs where the gyro switches back to the accel because there is no motion for awhile), and the gyroscope is used for motions so filtering out large accel changes does not make much difference.
				// apply low pass filter
				LastfSample=&laccel->ASample;  //last sample

				fSample.x=fSample.x*LOWPASS_FACTOR + LastfSample->x*LOWPASS_ONE_MINUS_FACTOR;
				fSample.y=fSample.y*LOWPASS_FACTOR + LastfSample->y*LOWPASS_ONE_MINUS_FACTOR;
				fSample.z=fSample.z*LOWPASS_FACTOR + LastfSample->z*LOWPASS_ONE_MINUS_FACTOR;
//#endif

				memcpy(&laccel->ASample,&fSample,sizeof(AccelSample));
				//memcpy(&laccel->LastASample,&fSample,sizeof(AccelSample));

#if USE_FXOS8700CQ
				memcpy(&laccel->MSample, &fMSample, sizeof(AccelSample));
#endif
#if USE_MPU6050
				//save last fGsample
				memcpy(&laccel->LastGSample,&laccel->GSample,sizeof(AccelSample));  //laccel->LastGSample are float
				memcpy(&laccel->GSample, &fGSample, sizeof(AccelSample));
				//now acceleration can be updated
				memcpy(&laccel->LastAcceleration,&laccel->Acceleration,sizeof(AccelSample));  
				for(j=0;j<3;j++) {
					laccel->Acceleration.i[j]=laccel->GSample.i[j]-laccel->LastGSample.i[j];
				} //for j
				//and Jerk can be determined
				for(j=0;j<3;j++) {
					laccel->Jerk.i[j]=laccel->Acceleration.i[j]-laccel->LastAcceleration.i[j];
				} //for j
#endif				

				//change timestamp was here- but moved below because some leai variables are changed below this

		/*
				memcpy(&leth->Accel[AccelNum].ASample,&fSample,sizeof(AccelSample));
				memcpy(&leth->Accel[AccelNum].MSample,&fMSample,sizeof(AccelSample));
				leth->Accel[AccelNum].flags|=ACCELEROMETER_GOT_SAMPLE;
		*/


				//Important note: for tilt compensated yaw to work correctly,
				//the 3 magnetic measurements (vector) needs to be calibrated (normalized)
				//to remove the hard-iron magnetic (unchanging) field which 
				//in my case moves that magnetic reading range (in particular for Z) into an all positive range
				//otherwise (without calibration) yaw changes with tilting of roll and pitch.
				//To calibrate the magnetic field readings, the max and min for each dimension is needed
				//this can be done by enabling autocalibration on the accelerometer+magnetometer
				//which will track these values (I found that it doesn't take too long or too much movement 
				//for the device to get a good calibration- it appears to adapt well as the device is moved into 
				//larger and larger angles)
				//or this can be done here in software without having to use the autocalibration feature
				//either way, you need to determine the min and max of the magnetic readings
				//and that takes time and requires the magnetometer to be moved 
				//in 360 degrees in at least two directions- so it's easiest just to use the magnetometer's
				//autocalibration feature. but for now, I am tracking the min and max magnetic readings.


				//todo make atan and sin and cos lookup tables

				//tilt compensated yaw 
				//from http://cache.freescale.com/files/sensors/doc/app_note/AN4248.pdf
				//note their code has Y in the left-right, X as the back-front (- to +)
				//while for the robot I view Y as back-front, and X left-right (- to +)
				//so accel x and y are switched

				//In terms of which direction is positive for pitch, yaw, and roll see this page:
				//https://www.grc.nasa.gov/www/k-12/airplane/rotations.html  and 
				//https://en.wikipedia.org/wiki/Aircraft_principal_axes
				//pitch: up is positive, yaw: right is positive, roll: clockwise from perspective of object (plane, robot, human) is positive
#if !USE_FAST_TRIG
				Roll=atan2(fSample.x,fSample.z); 
#else
				Roll=FastArcTan(fSample.x/fSample.z);
#endif

//faster method from: http://nghiaho.com/?p=997
//reported maximum error 0.0015 radians (0.085944 degrees)
//double FastArcTan(double x)
//{
//    return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
//}
//https://gist.github.com/wrl/9913185 sine interpolation lookup

				//fprintf(stderr,"Roll=%0.2f ",180*Roll/PI);
				/* calculate sin and cosine of roll angle Phi */
#if !USE_FAST_TRIG
				fsin=sin(Roll); // = 0 when horizontal
				fcos=cos(Roll); // = 1 when horizontal
#else
				fsin=sinlu(Roll); // = 0 when horizontal
				fcos=coslu(Roll); // = 1 when horizontal
#endif
				/* de-rotate by roll angle Phi */
				//tph: note they do not normalize the accel vector, but it doesn't matter because +-1g is the max/min
				//tph: roll back the magnetic vector by the roll
#if USE_FXOS8700CQ
				MagComp.y=fMSample.x*fcos-fMSample.z*fsin;	//=fMSample.x (horiz)
				fMSample2.z=fMSample.x*fsin+fMSample.z*fcos; //=fMSample.z (horiz)
#endif
				//tph: and calculate denominator to determine pitch
				fSample2.z=fSample.x*fsin+fSample.z*fcos;  //=1 (horiz)

				//calculate current pitch angle theta
#if !USE_FAST_TRIG
				Pitch=-atan2(fSample.y,fSample2.z); //note sign change in order to make pitch increase when turning up from perspective of robot (or plane), this needs to be reversed for the 3D model  //=0 (horiz)
#else
				Pitch=-FastArcTan(fSample.y/fSample2.z);
#endif

				/* restrict pitch angle to range -90 to 90 degrees */
				if (Pitch>PI_DIV_2) {
					Pitch=PI-Pitch;
				}
				if (Pitch<-PI_DIV_2) {
					Pitch=-PI - Pitch;
				}

				//fprintf(stderr,"Pitch=%0.2f ",180.0*Pitch/PI);

				/* calculate sin and cosine of pitch angle Theta */	
#if !USE_FAST_TRIG
				fsin=-sin(Pitch);  //0 (horiz)
				fcos=cos(Pitch);  //1 (horiz)
#else
				fsin=-sinlu(Pitch);  //0 (horiz)
				fcos=coslu(Pitch);  //1 (horiz)
#endif

				/* correct cosine if pitch not in range -90 to 90 degrees */
				if (fcos<0) {
					fcos=-fcos;
				}
#if USE_FXOS8700CQ
				/* de-rotate by pitch angle Theta */
				//note that again I reverse fMSample.x with fMSample.y
				MagComp.x=fMSample.y*fcos+fMSample2.z*fsin;
				MagComp.z=-fMSample.y*fsin+fMSample2.z*fcos;

				/* calculate current yaw = e-compass angle Psi */
#if !USE_FAST_TRIG
				Yaw=-atan2(-MagComp.y,MagComp.x);//note the sign change, or else (left/right) is backwards
#else
				Yaw=-FastArcTan(-MagComp.y/MagComp.x);
#endif
				Yaw+=PI; //to make 0 degrees appear at accel Y+
				//Yaw+=PI*3.0/4.0; //to make 0 degrees appear at accel Y+
#else
				//for now use gyro to estimate //Yaw = 0.0;
#endif
				//fprintf(stderr,"Yaw=%0.2f\n",180.0*Yaw/PI);


		//end tilt-compensated Yaw





		/*

		//second method
		//this method autocalibrates (normalizes) the magnetic field

				//tilt compensated yaw 

				//(note uses normalized accel- which I find is unnecessary- the normalized accel values are very close- a little higher by at most .02g)

				//from http://blog.solutions-cubed.com/lsm303-compass-tilt-compensation/
				//this works, but needs to be rotated so 0 degrees is at +Y on the accel

				//use calibration values to shift and scale magnetometer measurements
				//tph: calibrates (normalizes) Magnetic samples 
				Mag.x = (fMSample.x-laccel->MinM.x)/(laccel->MaxM.x-laccel->MinM.x)*2-1;  
				Mag.y = (fMSample.y-laccel->MinM.y)/(laccel->MaxM.y-laccel->MinM.y)*2-1;  
				Mag.z = (fMSample.z-laccel->MinM.z)/(laccel->MaxM.z-laccel->MinM.z)*2-1;  
				fprintf(stderr,"Calibrated Magnetic %0.1f %0.1f %0.1f\n",Mag.x,Mag.y,Mag.z);

				// Normalize acceleration measurements so they range from 0 to 1
				Acc.x = fSample.x/sqrt(fSample.x*fSample.x+fSample.y*fSample.y+fSample.z*fSample.z);
				Acc.y = fSample.y/sqrt(fSample.x*fSample.x+fSample.y*fSample.y+fSample.z*fSample.z);
				fprintf(stderr,"Normalized Accel %0.2f %0.2f\n",Acc.x,Acc.y);												

				// calculate pitch and roll
				Pitch = asin(-Acc.x);
				Roll = asin(Acc.y/cos(Pitch));

				// tilt compensated magnetic sensor measurements
				MagComp.x = Mag.x*cos(Pitch)+Mag.z*sin(Pitch);
				MagComp.y = Mag.x*sin(Roll)*sin(Pitch)+Mag.y*cos(Roll)-Mag.z*sin(Roll)*cos(Pitch);
				fprintf(stderr,"Compensated Magnetic %0.1f %0.1f\n",MagComp.x,MagComp.y);

				// arctangent of y/x converted to degrees
				Yaw=atan2(MagComp.y,MagComp.x);

		*/



		/*

							//non-tilt compensated Yaw
							//calculate arctan(MagSampleX/-MagSampleY) (for example -30uT/60uT atan(0.5)=26deg. 0.46radians)
							//57.29= 180/3.1415
							//all magnetic measurements need to be converted into a -180 to 180 degrees range
							//(for example tan(120deg)=-1.73 but atan(-1.73)=-60deg)
					
							if (fMSample.y>0) {
								angle.y=90-57.29*(atan(fMSample.x/fMSample.y)); //for 181 to 360 degrees	
							} else {
								//MagY<0
								angle.y=270-57.29*(atan(fMSample.x/fMSample.y)); //for 0 to 180 degrees
							} //if (fMSample.y>0) {
					
							if (fMSample.y==0) {
								if (fMSample.x<0) {
									angle.y=180.0;
								} else {
									angle.y=0.0;
								} //if (fMSample.x<0) {
							} //if (fMSample.y==0) {
		*/

							//tilt compensated Yaw
							//theta=pitch (angle.x), phi=roll (angle.z)

		/*
							angle.z = -asin(fSample.x); //roll (around z axis) 
							angle.x = asin(fSample.y); //pitch (around x axis)
							if (fSample.z<0) { //accel is upside down so make angles negative
								angle.z*=-1.0; //roll
								angle.x*=-1.0;  //pitch
							}

		*/

		/*
					//this works but X and Y are reversed on robot and above is less calculation
					angle.z=atan(fSample.y/fSample.z); //roll
					angle.x=atan(-fSample.x/(fSample.y*sin(angle.z)+fSample.z*cos(angle.z))); //pitch
		*/

		/*
							sintheta=sin(angle.x);
							costheta=cos(angle.x);
							sinphi=sin(angle.z);
							cosphi=cos(angle.z);

							angle.y=57.29*atan((fMSample.z*sinphi-fMSample.y*cosphi)/(fMSample.x*costheta+fMSample.y*sintheta*sinphi+fMSample.z*sintheta*cosphi));

							if (fMSample.y>0) {
								angle.y=90-angle.y;
							} else {
								angle.y=270-angle.y;
							}
		*/

					//convert pitch, yaw, and roll to angles - for tasks, angles are easier to understand than radian
					APitch = 180*Pitch/PI;
#if USE_FXOS8700CQ
					AYaw = -180*Yaw/PI;  //note negative sign, to make sure yaw increases as robot (or plane, etc.) rotates right
					if (AYaw < 0) {
							AYaw +=360;
					}
#endif //USE_FXOS8700CQ
					ARoll = 180*Roll/PI;
					//save the last estimated accel angles. 
					memcpy(laccel->LastEstAngle,laccel->EstAngle,sizeof(float)*3);
					//memcpy(laccel->LastAccelAngle,laccel->AccelAngle,sizeof(float)*3);

					//update the accelerometer angles
					//laccel->AngleR[0]=Pitch;
					//laccel->AngleR[1]=Yaw;
					//laccel->AngleR[2]=Roll;

//					laccel->AccelAngle[0]=APitch;
//					laccel->AccelAngle[1]=AYaw;
//					laccel->AccelAngle[2]=ARoll;


					//because during a motion a gyro scope may have a value between -1 and 1 for a few milliseconds
					//we should only use the accel values when the gyro has been between -1 and 1 for at least 100 samples ~1s.
					//if accel is not moving use accelerometer angle, but if moving add gyroscope motion
					//(currently add gyroscope motion for yaw because there is no magnetometer)

//#if 0 //blend accel and gyro when segment is not moving  
					if (fGSample.x>-GYROSCOPE_CALIBRATION_MINIMUM && fGSample.x<GYROSCOPE_CALIBRATION_MINIMUM) {  //GyroX is between -0.5 and 0.5
						//if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
						if (laccel->NoMotionCounter[0]<(NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD)) {
							if (laccel->NoMotionCounter[0]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST && NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD!=0) {
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									//sprintf(LogStr2,"Blending Gyro to Accel on X of %s %4.1f %4.1f",ROBOT_ACCEL_NAMES[(leth->PCBNum-1)*3+AccelNum],laccel->EstAngle[0],APitch);
									sprintf(LogStr2,"Blending Gyro to Accel on X of %s %4.1f %4.1f",RStatus.AccelInfo[laccel->NameNum].Name,laccel->EstAngle[0],APitch);
									LogRobotModelData(LogStr2);
								}							
							} //if (laccel->NoMotionCounter[0]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							laccel->NoMotionCounter[0]++;
						} else {
							if (laccel->NoMotionCounter[0]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD) {
								//switching to accel from gyro
								laccel->NoMotionCounter[0]++; //to stop this log message from printing again
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									//sprintf(LogStr2,"Switching from Gyro to Accel on X of %s %4.1f %4.1f",ROBOT_ACCEL_NAMES[(leth->PCBNum-1)*3+AccelNum],laccel->EstAngle[0],APitch);
									sprintf(LogStr2,"Switching from Gyro to Accel on X of %s %4.1f %4.1f",RStatus.AccelInfo[laccel->NameNum].Name,laccel->EstAngle[0],APitch);
									LogRobotModelData(LogStr2);
								}							
								//set flag so motion code can adjust associated TargetAngle and 2 offsets
								//Because this happens just one time, the Accel sample can be somewhat far off 
								laccel->flags|=ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_X;
								//save offset for motion code to use
								laccel->GyroAccelOffset[0]=laccel->EstAngle[0]-APitch;
							} // if (laccel->NoMotionCount[0]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST...
						} //if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST
					} else {
						laccel->NoMotionCounter[0]=0; //there is motion, so reset motion counter
					} 

					if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0 && laccel->NoMotionCounter[0]<(NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD)) {
						//segment is moving
						if (laccel->NoMotionCounter[0]>NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							//REST BLEND PERIOD - note that if ...BLEND_PERIOD==0, this code is never reached
							//accel has been stationary for 1 second so blend Accel samples in.
							//otherwise there is an instant 1-3degree difference that causes the motion code to respond
							//**NOTE currently accel sample interval of 10ms is presumed
							if (NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD != 0) {
								BlendFactor = ((float)(laccel->NoMotionCounter[0] - NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST)) / (float)NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD;
							} else {
								BlendFactor = 0.0;
							}							
							//mix the gyro and accel based on how long the body segment has not moved
							//ex: gyro=3degrees accel=1degree, NoMotionCounter[0]=10 BlendFactor=0.1 EstAngle[0]= 3*0.9+1*.1= 2.7+0.1=2.8
							laccel->EstAngle[0]=(laccel->EstAngle[0]+fGSample.x/100.0)*(1.0-BlendFactor)+ APitch*BlendFactor;						
						} else {  //if (laccel->NoMotionCounter[0]>NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) 
							//currently moving and still adding NoMotion counts until REST_BLEND_PERIOD, so just use Gyro sample
							//laccel->EstAngle[0]+=fGSample.x/100.0;	
							laccel->EstAngle[0]+=fGSample.x/100.0;	//was 110.0 because adding gyro generally adds too much to angle					
						} //if (laccel->NoMotionCounter[0]>NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) 
						APitch=laccel->EstAngle[0];
						//if (leth->PCBNum==ROBOT_PCB_ETHACCELS_TORSO && AccelNum==1) { 
							//fprintf(stderr,"G%03.1f ",fGSample.x);
						//} 
					} else { //if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0  ...
						//segment is not moving, use value obtained from acclerometer data only			
						laccel->EstAngle[0]=APitch;
					} //if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0  ...
//#endif
#if 0 //instant change to accel when segment is not moving
					if (fGSample.x>-GYROSCOPE_CALIBRATION_MINIMUM && fGSample.x<GYROSCOPE_CALIBRATION_MINIMUM) {  //GyroX is between -0.5 and 0.5
						if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							laccel->NoMotionCounter[0]++;
						} 
					} else {
						laccel->NoMotionCounter[0]=0; //there is motion, so reset motion counter
					} 

					if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
						//accel has not been stationary for 1 second so use gyro value
						//**NOTE currently accel sample interval of 10ms is presumed
						laccel->EstAngle[0]+=fGSample.x/100.0;
						APitch=laccel->EstAngle[0];
						//if (leth->PCBNum==ROBOT_PCB_ETHACCELS_TORSO && AccelNum==1) { 
							//fprintf(stderr,"G%03.1f ",fGSample.x);
						//} 
					} else { //if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
						//not moving in last 20ms, use value obtained from acclerometer data only			
						laccel->EstAngle[0]=APitch;
						if (leth->PCBNum==ROBOT_PCB_ETHACCELS_TORSO && AccelNum==1) { 
							//fprintf(stderr,"A%03.1f ",fGSample.x);
						} 
					} //if (laccel->NoMotionCounter[0]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
#endif

						//Currently yaw is only obtained from adding gyro data, presuming starting angle=0
					//if ((fSample.y>1.0 || fSample.y<-1.0) && (LastfSample.y>1.0 || LastfSample.y<-1.0)) { 	
						//accel X is moving, so add gyroscope sample/(sample interval)100 to motion
						//**NOTE currently accel sample interval of 10ms is presumed
						//laccel->EstAngle[1]+=fGSample.y/100.0;
						laccel->EstAngle[1]=0.0;
						AYaw=0.0;//laccel->EstAngle[1]; //for now set AYaw to angle obtained from gyro
					//} else {
						//not moving in last 20ms, use value obtained from acclerometer data only			
					//	laccel->EstAngle[1]=AYaw;
					//} 

//#if 0 //blend in accel to gyro slowly
					if (fGSample.z>-GYROSCOPE_CALIBRATION_MINIMUM && fGSample.z<GYROSCOPE_CALIBRATION_MINIMUM) {  //GyroZ is between -0.5 and 0.5
						if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD) {
							if (laccel->NoMotionCounter[2]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST && NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD!=0) {
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									//sprintf(LogStr2,"Blending Gyro to Accel on Z of %s %4.1f %4.1f",ROBOT_ACCEL_NAMES[(leth->PCBNum-1)*3+AccelNum],laccel->EstAngle[2],ARoll);
									sprintf(LogStr2,"Blending Gyro to Accel on Z of %s %4.1f %4.1f",RStatus.EthAccelsPCBInfo[laccel->NameNum].Name,laccel->EstAngle[2],ARoll);
									LogRobotModelData(LogStr2);
								}							
							} //if (laccel->NoMotionCounter[0]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							laccel->NoMotionCounter[2]++;
						} else {
							if (laccel->NoMotionCounter[2]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD) {
								//switching to accel from gyro
								laccel->NoMotionCounter[2]++; //to stop this log message from printing again
								if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
									sprintf(LogStr2,"Switching from Gyro to Accel on Z of %s %4.1f %4.1f",RStatus.EthAccelsPCBInfo[laccel->NameNum].Name,laccel->EstAngle[2],ARoll);
									LogRobotModelData(LogStr2);
								}				
								//set flag so motion code can adjust associated TargetAngle and 2 offsets
								laccel->flags|=ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_Z;
								//save offset for motion code to use
								laccel->GyroAccelOffset[2]=laccel->EstAngle[2]-ARoll;			
							} //if (laccel->NoMotionCount[2]==NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST...
						} //if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST
					} else {
						laccel->NoMotionCounter[2]=0; //there is motion, so reset motion counter
					} 

					if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0 && laccel->NoMotionCounter[2]<(NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST+NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD)) {
						//segment is moving
						if (laccel->NoMotionCounter[2]>NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							//REST BLEND PERIOD - note that if ...BLEND_PERIOD==0, this code is never reached
							//accel has been stationary for 1 second so blend Accel samples in.
							//otherwise there is an instant 1-3degree difference that causes the motion code to respond
							//**NOTE currently accel sample interval of 10ms is presumed
							if (NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD!=0) {
								BlendFactor=((float)(laccel->NoMotionCounter[2]-NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST))/(float)NUM_NO_MOTION_SAMPLES_IN_ROBOT_AT_REST_BLEND_PERIOD;
							} else {
								BlendFactor = 0.0;
							}
							//mix the gyro and accel based on how long the body segment has not moved
							//ex: gyro=3degrees accel=1degree, NoMotionCounter[2]=10 BlendFactor=0.1 EstAngle[2]= 3*0.9+1*.1= 2.7+0.1=2.8
							laccel->EstAngle[2]=(laccel->EstAngle[2]+fGSample.z/100.0)*(1.0-BlendFactor)+ ARoll*BlendFactor;						
						} else {  //if (laccel->NoMotionCounter[2]>NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) 
							//currently moving and still adding NoMotion counts until REST_BLEND_PERIOD, so just use Gyro sample
							//laccel->EstAngle[2]+=fGSample.z/100.0;
							laccel->EstAngle[2]+=fGSample.z/100.0;  //was 110.0 because adding gyro generally adds too much to angle
						} //if (laccel->NoMotionCounter[2]>0) {
						ARoll=laccel->EstAngle[2];
					} else {	//if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0  ...
						//not moving in last 20ms, use value obtained from acclerometer data only	
						laccel->EstAngle[2]=ARoll;
					} //if ((RStatus.flags&ROBOT_STATUS_ACCELEROMETERS_ONLY)==0  ...
//#endif
#if 0  //Switch to accel value instantly
					if (fGSample.z>-GYROSCOPE_CALIBRATION_MINIMUM && fGSample.z<GYROSCOPE_CALIBRATION_MINIMUM) {  //GyroZ is between -0.5 and 0.5
						if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
							laccel->NoMotionCounter[2]++;
						} 
					} else {
						laccel->NoMotionCounter[2]=0; //there is motion, so reset motion counter
					} 
					if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
						//accel has not been stationary for 1 second so use gyro value
					//if ((fGSample.z>1.0 || fGSample.z<-1.0) && (LastfGSample.z>1.0 || LastfGSample.z<-1.0)) { 	
						//accel X is moving, so add gyroscope sample/(sample interval)100 to motion
						//**NOTE currently accel sample interval of 10ms is presumed
						laccel->EstAngle[2]+=fGSample.z/100.0;
						ARoll=laccel->EstAngle[2];
					} else {	//if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
						//not moving in last 20ms, use value obtained from acclerometer data only	
						laccel->EstAngle[2]=ARoll;
					} //if (laccel->NoMotionCounter[2]<NUM_NO_MOTION_SAMPLES_BEFORE_ROBOT_AT_REST) {
#endif 

#if 0 //for now in the interest of speeding up code- not needed yet or perhaps ever, Task thread calculates this because angle change can be relative to another accelerometer.
					//save last AngleChange 
					memcpy(laccel->LastEstAngleChange,laccel->EstAngleChange,sizeof(float)*3);

					//laccel->AccelAngleChange[0]=APitch-laccel->LastAngle[0];
					//laccel->AccelAngleChange[1]=AYaw-laccel->LastAngle[1];
					//laccel->AccelAngleChange[2]=ARoll-laccel->LastAngle[2];

					laccel->EstAngleChange[0]=laccel->EstAngle[0]-laccel->LastEstAngle[0];
					laccel->EstAngleChange[1]=laccel->EstAngle[1]-laccel->LastEstAngle[1];
					laccel->EstAngleChange[2]=laccel->EstAngle[2]-laccel->LastEstAngle[2];
#endif					

					//laccel->flags|=ACCELEROMETER_GOT_SAMPLE;  //not used anymore - replaced by timestamp
					//save last timestamp
					//Note that changing the timestamp allows the task instruction code to move to the next sample-
					//so this basically starts the task code processing the new sample
					laccel->LastTimeStamp=laccel->TimeStamp;
					laccel->TimeStamp=GetTimeInMS();//save timestamp for received samples



					//add Sample data to Model log for modeling 
					if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
						//sprintf(LogStr,"%s\t%d\t%3.1f\t%3.1f\t%3.1f",LogStr,AccelNum,APitch,AYaw,ARoll);
						sprintf(LogStr,"%s\t%d\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f",LogStr,AccelNum,APitch,AYaw,ARoll,laccel->GSample.x,laccel->GSample.y,laccel->GSample.z);
						//note that now user cannot be sure if angle is from accel or gyro data unless seeing that associated gyro[xyz] is <-1 or >1 deg/sec
						//sprintf(LogStr,"%s\t%d\t%3.1f\t%3.1f\t%3.1f\t%3.1f\t%3.1f\t%3.1f",LogStr,AccelNum,laccel->EstAngle[0],laccel->EstAngle[1],laccel->EstAngle[2],laccel->GSample.x,laccel->GSample.y,laccel->GSample.z);

		//				sprintf(LogStr,"%s\t%d\t%0.3f\t%0.3f\t%0.3f\t%3.0f\t%3.0f\t%3.0f",LogStr,AccelNum,fSample.x,fSample.y,fSample.z);
		//				sprintf(LogStr,"%s\t%0.1f\t%0.1f\t%0.1f",LogStr,fMSample.x,fMSample.y,fMSample.z);
		//				sprintf(LogStr,"%s\t%3.0f\t%3.0f\t%3.0f",LogStr,APitch,AYaw,ARoll);
					} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {


				//if accel data should be shown in the accel window and this PCB is the currently selected PCB in the Accel Window
				//Note, here on the accelerometer window only the accelerometer angles are used because 
				//the user will probably only look at the acclerometer window when the body segment is not moving.
				if ((RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) && SelectedPCB) {  
//#if 0 //uncomment to see actual accelerations, currently I just display 3 angles
					sprintf(tstrX,"lblAccelx%d",AccelNum);
					sprintf(tstrY,"lblAccely%d",AccelNum);
					sprintf(tstrZ,"lblAccelz%d",AccelNum);
					tcX=GetFTControl(tstrX);
					tcY=GetFTControl(tstrY);
					tcZ=GetFTControl(tstrZ);
					if (tcX!=0 && tcY!=0 && tcZ!=0) {
						sprintf(tcX->text,"%0.3f g",fSample.x);
						sprintf(tcY->text,"%0.3f g",fSample.y);
						sprintf(tcZ->text,"%0.3f g",fSample.z);												
						DrawFTControl(tcX);
						DrawFTControl(tcY);
						DrawFTControl(tcZ);
					} else {
						GotError=1;
					}//if (tcX!=0 && tcY!=0 && tcZ!=0) {
//#endif

					//now magnetic data
					//a magnetometer provides a 3d vector pointing to the direction of the magnetic field
					//like a compass a magnetometer will point to the strongest magnetic field around it
					//so devices with permanent magnets or electromagnetic fields will cause
					//the compass orientation (yaw) to fail
					//the robot will have to recognize when the yaw values are from some other field
		//tilt compensation: http://cache.freescale.com/files/sensors/doc/app_note/AN4248.pdf
		//with hard and soft interference: http://cache.freescale.com/files/sensors/doc/app_note/AN4246.pdf
		//layout recommendations: http://cache.freescale.com/files/sensors/doc/app_note/AN4247.pdf
					//this page provides the magnetic field strength for your current location:
					//http://www.magnetic-declination.com/
					//46.846uT 
					//from: http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
					//Magnetic Flux Density: 10,000 gauss (G) = 1 tesla (T)
					//Magnetic Field: 1 oersted (Oe) = 79.58 amperes/meter (A/m)
					//tilt compensation link: http://blog.solutions-cubed.com/lsm303-compass-tilt-compensation/

			//to calculate horizontal an vertical components of local magnetic field: 
			//http://www.ngdc.noaa.gov/geomag-web/#igrfwmm
			//Latitude: 	33° 43' 27" N
			//Longitude: 	117° 54' 22" W
			//Elevation: 	0.0 km Mean Sea Level
			//Date 	2015-12-14 Declination 12° 2' 10"
			//( + E  | - W ) 	Inclination  58° 37' 39"
			//( + D  | - U ) 	Horizontal Intensity 24,407.1 nT
			//	North Comp (+ N  | - S) 23,870.5 nT
			// 	East Comp (+ E  | - W) 	 5,089.6 nT
			//Vertical Comp (+ D  | - U) 	40,028.3 nT 
			//Total Field		46,882.5 nT 
//#if 0 //uncomment to display actual magnetic readings, currently I only care about x,y,z (pitch,yaw,roll) degrees

						sprintf(tstrX,"lblMagx%d",AccelNum);
						sprintf(tstrY,"lblMagy%d",AccelNum);
						sprintf(tstrZ,"lblMagz%d",AccelNum);
						tcX=GetFTControl(tstrX);
						tcY=GetFTControl(tstrY);
						tcZ=GetFTControl(tstrZ);
						if (tcX!=0 && tcY!=0 && tcZ!=0) {
#if USE_FXOS8700CQ

							sprintf(tcX->text,"%0.1f uT",fMSample.x);
							sprintf(tcY->text,"%0.1f uT",fMSample.y);
							sprintf(tcZ->text,"%0.1f uT",fMSample.z);
#endif
#if USE_MPU6050
#if Linux
							sprintf(tcX->text,"%0.1f \xB0/s",fGSample.x);
							sprintf(tcY->text,"%0.1f \xB0/s",fGSample.y);
							sprintf(tcZ->text,"%0.1f \xB0/s",fGSample.z);												
#endif //Linux
#if WIN32
							sprintf(tcX->text, "%0.1f °/s", fGSample.x);
							sprintf(tcY->text, "%0.1f °/s", fGSample.y);
							sprintf(tcZ->text, "%0.1f °/s", fGSample.z);
#endif //WIN32
#endif
							DrawFTControl(tcX);
							DrawFTControl(tcY);
							DrawFTControl(tcZ);

						} else {
							GotError=1;
						}//if (tcX!=0 && tcY!=0 && tcZ!=0) {

//#endif //print magnetic data

						sprintf(tstrDegrees,"lblDegrees%d",AccelNum);
						tcDegrees=GetFTControl(tstrDegrees);
						if (tcDegrees!=0) {
								//sprintf(tcY->text,"%3.0f deg",angle.y);
#if Linux
								sprintf(tcDegrees->text,"%3.1f,%3.1f,%3.1f \xB0",APitch,AYaw,ARoll);
#endif //Linux
#if WIN32
								sprintf(tcDegrees->text, "%3.1f,%3.1f,%3.1f °", APitch, AYaw, ARoll); 
#endif //WIN32

								DrawFTControl(tcDegrees);
								//for now convert to radians for 3D drawing
								//angle.y/=-57.29;  //negative because is reversed currently
						} //if (tcDegrees!=0) {


				} //if ((RStatus.flags&ROBOT_STATUS_SHOW_ACCEL_DATA) && SelectedPCB) {  





					//by determining the angles yaw (from magnetic), pitch (from accel), and roll (from accel)

					//the accelerometer x sample represents a rotation around the z axis
					//and the accelerometer y sample represents a rotation around the x axis
					//get the angle from the acceleration 
					//for pitch 0g=0 degrees, 1g=90degrees
					//for roll 0g=0 degrees, 1g=90degrees
					//todo: make lookup table
					//x=pitch, y=yaw, z=roll

					//note that although North is to the accelerometer -X, 0 degrees is 90 in +Y (which is intuitive)- and works well for a robot body part. Perhaps it would be better for North to be in the +Y direction, then the 0 degrees calculation would match the foward side of the chip.


					//tph: this works
		//			angle.z = asin(fSample.x); //roll (around z axis) 
		//			angle.x = -asin(fSample.y); //pitch (around x axis)
					//tph: the above works, but I found was backwards (x and z are correct but positive was negative) from the robot perspective for the foot. Changing the sign of the two calculations below reverses the + and -

		/*
		//tph: this is now done above
					angle.z = -asin(fSample.x); //roll (around z axis) 
					angle.x = asin(fSample.y); //pitch (around x axis)
					if (fSample.z<0) { //accel is upside down so make angles negative
						angle.z*=-1.0; //roll
						angle.x*=-1.0;  //pitch
					}
		*/

				//from http://cache.freescale.com/files/sensors/doc/app_note/AN4248.pdf
		/*
					//this works but X and Y are reversed on robot and above is less calculation
					angle.z=atan(fSample.y/fSample.z); //roll
					angle.x=atan(-fSample.x/(fSample.y*sin(angle.z)+fSample.z*cos(angle.z))); //pitch

		*/


				//if (RStatus.flags&ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL) {
				if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
					//apply accelerometer+magnetic data to model axes

					//APitch = 180*Pitch/PI;
					//needs to convert back to radians from APitch and ARoll, otherwise gyroscope additions are not shown
					angle.x=-APitch*PI_DIV_180;//PI/180.0;//-Pitch;//laccel->EstAngle[0]; //-Pitch; //pitch needs to be negated to render correctly
					angle.y=0.0;//laccel->EstAngle[1];//0.0 //for now AYaw;
					angle.z=-ARoll*PI_DIV_180;//PI/180.0;//-Roll;//-laccel->EstAngle[2]/2.0*PI;//-Roll; //roll also needs to be reversed to render correctly

/*
					angle.x=-Pitch; //pitch needs to be negated to render correctly
					angle.y=0;//for now Yaw;
					angle.z=-Roll; //roll also needs to be reversed to render correctly
*/

		//			fprintf(stderr,"%d: %0.3f %0.3f %0.3f\n",AccelNum,angle.x,angle.y,angle.z);												
					//because there are 2 angles, a rotation matrix is made with the first angle
					//and then that matrix is rotated by the second angle
					//todo: the rotation functions could perhaps be faster, in assembler, but also without matrices 
					//MakeRotationMatrix_AroundZAxis(&lmodel->axis[AccelBase+AccelNum].m,angle.z);
					AccelAxis=RStatus.AccelInfo[laccel->NameNum].ModelAxis;
					MakeRotationMatrix_AroundZAxis(&lmodel->axis[AccelAxis].m,angle.z);
					
					//connect the accel to the correct axis
					angle.z=0;
					//angle.x=0; //removes so this will only be a 2 angle rotation
					//note that this also applies the yaw
					//RotateAxis3D(&lmodel->axis[AccelBase+AccelNum].m,&angle);			
					RotateAxis3D(&lmodel->axis[AccelAxis].m,&angle);			

					//MakeRotationMatrix_AroundXAxis(&lmodel->axis[AccelBase+AccelNum].m,angle.x);
					//RotateAxis3D(&lmodel->axis[AccelBase+AccelNum].m,&angle);
				} //				if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
					//if (RStatus.flags&ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL) {




				//i+=7;  //go to next accelerometer x,y,z samples
#if USE_FXOS8700CQ
				i+=13;  //go to next accelerometer x,y,z samples
#endif
#if USE_MPU6050
				i+=15;  //go to next accelerometer x,y,z samples
#endif
			} //	while(i<NumBytes) {


		/*
			//print the raw data string to the "Data received" box
			tc=GetFTControl("txtGetAccelData");									
			if (tc==0) {
					//don't print if responding to a broadcast
					//fprintf(stderr,"Error: Couldn't get FTControl: txtGetAccelData\n");										
				GotError=1;
			} else {
				strcpy(tc->text,DataStr);
				DrawFTControl(tc);
			} //if (tc==0) {
		*/ 
			//write all the accumulated entries into the Model log file 
			//adds timestamp and carriage return
			if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				LogRobotModelData(LogStr);
			}

			//if (RStatus.flags&ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL) {
			if (RMInfo.flags&ROBOT_MODEL_INFO_RENDER_IN_REALTIME) {
					//re-render the robot model
					RenderModelMainImage();
			}
		break;
	} //	switch(recvbuf[4]) {

	return(!GotError);
} //int ProcessAccelSensorData(MAC_Connection *lmac, char *recvbuf,int NumBytes) 


int ProcessAnalogSensorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 
{					
	FTWindow *twin;
	FTControl *tc;
//	EthAccels_PCB *leth;
	Accels_PCB *leth;
	AnalogSensor *lanalog;
	char tstr[256];
	int i,GotError,SensorNum,SelectedPCB;
	unsigned short int Sample;
	float PercentPress,PercentChange,fSample;
	char LogStr[512];
	uint16_t Mini,Maxi;
	float Minf,Maxf;
	int AnalogWindowOpen;


	//leth=&lmac->pcb.EthAccelsPCB;
	leth=&lmac->pcb.AccelsPCB;
	if (!leth) {
		fprintf(stderr,"Received data from an EthAccelsPCB with no known MAC_Connection.\n");	
		return(0);
	} //


	GotError=0;
	//see Analog Sensors window is open, if no open it
	twin=GetFTWindow("winAnalogSensors");
	if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
		lmac->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
	    lmac->AddWindowFunction=(FTControlfunc *)winAnalogSensors_AddFTWindow;
	} else {
		//FT_SetFocus(twin,0); //give the window the focus
	}


	//determine if this data comes from an EthAccel PCB that is currently selected in the Touch (and Accel) Window
	//output data to the Text box depending on which Analog Sensor (0-15) the data corresponds to
	SelectedPCB=0;
	tc=GetFTControl("txtAnalogDestIPAddressStr");									
	if (tc!=0) {
		//control exists
		if (twin!=0 && !strcmp(tc->text,lmac->DestIPAddressStr)) { //need to wait until Analog Sensor window is open
			//The PCB connected to this data is curently selected in the Analog Sensors window
			//so update the data in the textboxes
			SelectedPCB=1;
		} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
	} //if (tc!=0) {



	//if recording to model log, initialize Log String with PCB PCBName
	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		//sprintf(LogStr,"%s(TOUCH)",lmac->EthPCBInfo.Name); //presumes pcb exists
		//sprintf(LogStr,"%s\tT",lmac->EthPCBInfo.Name); //presumes pcb exists, T=Touch sensor data
		sprintf(LogStr,"%s\tT",lmac->pcb.Name); //presumes pcb exists, T=Touch sensor data
	} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {



	//Since there is more than 1 kind of analog sensor data that can be received
	//branch depending on the instruction byte

	switch(recvbuf[4]) {

		case ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_THRESHOLD:
				//not implemented yet
		break;
		case ROBOT_ACCELMAGTOUCH_GET_ANALOG_MINMAX:
			//data is:
			//ip(0-3) inst(4) num(5)min(6-7)max(8-9)num(10)min(11-12)max(13-14)...

			i=5;
			while(i<NumBytes) {
				SensorNum=(int)recvbuf[i];
				Mini=(uint16_t)((recvbuf[i+2]<<8)|recvbuf[i+1]); 
				Maxi=(uint16_t)((recvbuf[i+4]<<8)|recvbuf[i+3]); 
				//pic32mx 10-bit: 3.3/1024 0x400  
				//Minf=(float)Mini*0.003225806; //convert to volts
				//Maxf=(float)Maxi*0.003225806; //convert to volts
				//pic32mz 12-bit: 3.3/4096
				Minf=(float)Mini*0.000805664; //convert to volts
				Maxf=(float)Maxi*0.000805664; //convert to volts


				lanalog=&leth->Analog[SensorNum];  //get pointer to analog sensor

		
				//save these values for the tasks (like walking, and other scripts) to use				
				//Note that the firmware keeps adjusting these values- so 
				//if they are going to be used, they should be updated just before use
				memcpy(&lanalog->MinV,&Minf,sizeof(float));
				memcpy(&lanalog->MaxV,&Maxf,sizeof(float));

				//print output to text since window does not show min and max
				fprintf(stderr,"Sensor: %d. Min:%f Max:%f\n",SensorNum,Minf,Maxf);

				//add Analog Sensor data to Model log for modeling (and for human readability) 
				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					sprintf(LogStr,"%s\t%d\t%0.3f\t%0.3f",LogStr,SensorNum,Minf,Maxf);
				} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
				i+=5;
			} //while i

		break;
		case ROBOT_ACCELMAGTOUCH_GET_ANALOG_SENSOR_VALUES:
		case ROBOT_ACCELMAGTOUCH_START_POLLING_ANALOG_SENSORS:
		case ROBOT_ACCELMAGTOUCH_START_ANALOG_SENSORS_INTERRUPT:


			//see if Analog window is open
			twin=GetFTWindow("winAnalog");
			AnalogWindowOpen=0;
			if (twin!=0 && twin->flags&WOpen) {  
				AnalogWindowOpen=1;
			}

			if (AnalogWindowOpen) {
				//determine if this data comes from an EthAccel PCB that is currently selected in the Accel Window
				//output data to the Accelerometer box depending on which Accelerometer (0-2) the data corresponds to
				SelectedPCB=0;
				tc=GetFTControl("txtAnalogDestIPAddressStr");									
				if (tc!=0) {
					//control exists
					if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
						//The PCB connected to this data is curently selected in the Accelerometers window
						//so update the data in the textboxes
						SelectedPCB=1;
					} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
				} //if (tc!=0) {
			} //if (AccelWindow)



			//output data to AnalogSensor box depending on which Analog Sensor (0-15)
			//note this only works if the window is already open currently
			//don't print if responding to a broadcast
			//fprintf(stderr,"Error: Couldn't get FTControl: txtGetAccelData\n");										
			//go through each sensor, convert to a voltage, and print in the correct label
			i=5;
			while(i<NumBytes) {
				//Analog Sensor data contains:
				//AnalogSensor# [0], %Press [1], %Change [2], Sample [3-4], etc. for as many sensors as are active
				//may want min and max here too- it stops having to add a call to getminmax to
				//update the AnalogSensor data structure - I don't think scripts would need
				//the min and max regularly/automatically
				SensorNum=(int)recvbuf[i];
				PercentPress=(float)recvbuf[i+1]/2.55; //(scale is 0 to 255 which is 0-100%)
				PercentChange=(float)recvbuf[i+2]; //(scale is -128 to 127 which is 0-100%)
				if (PercentChange>127) {
					PercentChange-=256; //negative percent change
				}
				PercentChange/=1.28; //convert to a percentage
				//10-bit ADC sample, 0x3ff=1023.0  3.3/1023 = 0.003225806
				//12-bit ADC sample, 0xfff=4095.0  3.3/4095 = 0.000805861
				Sample=(unsigned short int)(recvbuf[i+4]<<8|recvbuf[i+3]);
				//fSample=(float)Sample*0.003225806;
				fSample=(float)Sample*0.000805664;

				lanalog=&leth->Analog[SensorNum];  //get pointer to analog sensor

				memcpy(&lanalog->Sample,&Sample,sizeof(uint16_t));
				memcpy(&lanalog->fSample,&fSample,sizeof(float));				
				memcpy(&lanalog->Percent,&PercentPress,sizeof(float));
				memcpy(&lanalog->PercentChange,&PercentChange,sizeof(float));

				//no need for separate timestamp since each 10ms EthAccel send is given a timestamp

				//lanalog[SensorNum].LastTimeStamp=laccel->TimeStamp;
				//lanalog[SensorNum].TimeStamp=GetTimeInMS();//save timestamp for received samples

				//if this AnalogSensor is associated with an angle (potentiometer), then calculate the angle.
				if (lanalog->flags&ANALOG_SENSOR_POTENTIOMETER) {
					//Angle = MinVAngle+ ((Voltage-MinV)/(MaxV-MinV))*(MaxAngle-MinAngle);
					//ex: V=2.0v MinV= 1.8v MaxV=2.4v, MaxAngle=180 MinAngle=0 
					//Angle=0+ ((2.0-1.8)/(2.4-1.8))*(180-0)=60 degrees
					lanalog->LastAngle=lanalog->Angle; //store last angle
					lanalog->Angle=lanalog->MinVAngle+ ((fSample-lanalog->MinV)/(lanalog->MaxV-lanalog->MinV))*(lanalog->MaxVAngle-lanalog->MinVAngle);
					lanalog->LastVelocity=lanalog->Velocity;
					lanalog->Velocity=(lanalog->LastAngle-lanalog->Angle)*ACCEL_SAMPLES_PER_SECOND;//*100 samples/second
					lanalog->Acceleration=lanalog->LastVelocity-lanalog->Velocity;
					//fprintf(stderr,"%d:%03.1f ",SensorNum,lanalog->Angle);
				} //if (lanalog->flags&) {

					//todo: add potentiometer data if poteniometer
				//add Analog Sensor data to Model log for modeling (and for human readability) 
				if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
					sprintf(LogStr,"%s\t%d\t%.1f\t%.1f\t%0.3f",LogStr,SensorNum,PercentPress,PercentChange,fSample);
				} //if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {



				//Analog Sensors data should be shown in the analog sensor window and 
				//the PCB this data is from is the currently selected PCB in the Analog Sensors window
				if ((RStatus.flags&ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA) && SelectedPCB) {  


					sprintf(tstr,"lblAnalogSensorPress%02d",SensorNum);		
					tc=GetFTControl(tstr);
					if (tc!=0) {
						if (lanalog->flags&ANALOG_SENSOR_POTENTIOMETER) {
							sprintf(tc->text,"%.0f  ",lanalog->Angle);			
						} else {
							sprintf(tc->text,"%.0f%%",(float)PercentPress);			
						}
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {

					sprintf(tstr,"lblAnalogSensorChange%02d",SensorNum);		
					tc=GetFTControl(tstr);
					if (tc!=0) {

						sprintf(tc->text,"%.0f%%",(float)PercentChange);			

						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {

					sprintf(tstr,"lblAnalogSensorVolt%02d",SensorNum);		
					tc=GetFTControl(tstr);
					if (tc!=0) {
						sprintf(tc->text,"%0.3f v",fSample);			
						//sprintf(tc->text,"%0.3f v",(float)Sample*0.003225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {

				} //if ((RStatus.flags&ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA) && SelectedPCB) {  
				i+=5;//3;
			} //while
			break;
			default:
				fprintf(stderr,"Unknown analog sensor data returned.\n");
			break;
		} //	switch(recvbuf[4]) {

/*
	//print the raw data string to the "Data received" box
	tc=GetFTControl("txtGetAnalogSensorData");									
	if (tc!=0) {
		strcpy(tc->text,DataStr);
		DrawFTControl(tc);
	} else {
		GotError=1;
	}//if tc==0
*/


	//if potentiometer then calculate angle, velocity and acceleration


	//write all the accumulated entries into the Model log file 
	if (RStatus.flags&ROBOT_STATUS_MODEL_LOG) {
		LogRobotModelData(LogStr);
	}


	return(!GotError);
} //int ProcessAnalogSensorData(MAC_Connection *lmac, char *recvbuf,int NumBytes) 

int ProcessPowerData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 
{					
	FTWindow *twin;
	FTControl *tc;
	char tstr[256];
	int i,GotError,SelectedPCB;
	unsigned short int Sample;

	SelectedPCB=0;
	GotError=0;
	//see if Power window is open, if no open it
	twin=GetFTWindow("winPower");
	if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
		lmac->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
		lmac->AddWindowFunction=(FTControlfunc *)winPower_AddFTWindow;
	} else {
		//FT_SetFocus(twin,0); //give the window the focus
	}


	//determine if this data comes from an EthPower PCB that is currently selected in the EthPower Window
	SelectedPCB=0;
	tc=GetFTControl("txtPowerDestIPAddressStr");									
	if (tc!=0) {
		//control exists
		if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
			//The PCB connected to this data is curently selected in the Power window
			//so update the data in the textboxes
			SelectedPCB=1;
		} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
	} //if (tc!=0) {


	//current an2,6-14 are in use 0=2,1=6,2=7,3=8,4=9,5=10,6=11,7=12,8=13,9=14

	//output data to Power window
	//note this only works if the window is already open currently
	//don't print if responding to a broadcast
	//fprintf(stderr,"Error: Couldn't get FTControl: txtGetAccelData\n");										
	//go through each sensor, convert to a voltage, and print in the correct label
//note, unlike motor current sense, and accel, power sends has no index before each 16-bit (of a 10-bit sample) data
	i=5;
	while(i<NumBytes) {
//		SensorNum=(int)recvbuf[i];
		//0x3ff=1023.0  3.3/1023 = 0.003225806
		Sample=(unsigned short int)(recvbuf[i+1]<<8|recvbuf[i]);

		if (SelectedPCB) {  //The PCB this data is from is the currently selected PCB in the Power window
			switch((i-5)/2) {
				case 0: //RB2 SenseCurrent12V
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
/*
					tc=GetFTControl("lblPowerSourceCurrent12V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 1: //RB6 SenseVolt12V
					//200k/1200k=.1667 x12=  2.00v (@12v), 2.0/3.3=0.606x 0x3ff (1023) = 620
					//12/620 = 0.01935v/per bit
/*
					tc=GetFTControl("lblPowerSourceVolt12V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f v",(float)Sample*0.01935);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 2: //RB7 SenseVolt36V
					//for 24v:					
					//100k/1100k=.0909 x24=  2.18v (@24v), 2.18/3.3=0.66x 0x3ff (1023) = 676
					//24/676 = 0.0354v/per bit
					//for 36v:
					//68k/1068k=.06367 x36=  2.292v (@36v), 2.292/3.3=0.6945x 0x3ff (1023) = 710
					//36/710=0.0507
					tc=GetFTControl("lblPowerSourceVolt36V");
					if (tc!=0) {
						//sprintf(tc->text,"%0.3f v",(float)Sample*0.0354);
						sprintf(tc->text,"%0.3f v",(float)Sample*0.0507);						
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
				break;
				case 3: //RB8 SenseVoltBattery2
					//200k/1200k=.1667 x12=  2.00v (@12v), 2.0/3.3=0.606x 0x3ff (1023) = 620
					//12/620 = 0.01935v/per bit
/*
					tc=GetFTControl("lblPowerSourceVoltBattery2");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f v",(float)Sample*0.01935);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 4: //RB9 SenseVoltBattery1
					//200k/1200k=.1667 x12=  2.00v (@12v), 2.0/3.3=0.606x 0x3ff (1023) = 620
					//12/620 = 0.01935v/per bit
/*
					tc=GetFTControl("lblPowerSourceVoltBattery1");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f v",(float)Sample*0.01935);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 5: //RB10 SenseCurrent36V
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
					tc=GetFTControl("lblPowerSourceCurrent36V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
				break;
				case 6: //RB11 SenseBatteryConnected
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
/*
					tc=GetFTControl("lblPowerSourceCurrent36V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 7: //RB12 SenseCurrentCharger/GNDBATTERY
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
/*
					tc=GetFTControl("lblPowerSourceCurrent36V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;
				case 8: //RB13 SenseWallConnected
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
/*
					tc=GetFTControl("lblPowerSourceCurrent36V");
					if (tc!=0) {
						sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);			
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
*/
				break;

				case 9: //RB14 SenseCurrent3.3V
					//0.1ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.1  *0.03226
					//0.05ohm sense resistor, 3.3v/1023=0.003226, so current=V/R
					//sample*0.003226/0.05  *0.06452

					tc=GetFTControl("lblPowerSourceCurrent3V");
					if (tc!=0) {
						//sprintf(tc->text,"%0.3f a",(float)Sample*0.03225806);
						sprintf(tc->text,"%0.3f a",(float)Sample*0.06452);						
						DrawFTControl(tc);
					} else {
						GotError=1;
					}//if (tc!=0) {
				break;

			} //switch(i)

		} //if (SelectedPCB) {
		i+=2;//i+=3;
	} //while


	//print the raw data string to the "Data received" box
	tc=GetFTControl("txtGetPowerData");									
	if (tc!=0) {
		strcpy(tc->text,DataStr);
		DrawFTControl(tc);
	} else {
		GotError=1;
	}//if tc==0


	return(!GotError);
} //int ProcessPowerData(MAC_Connection *lmac, char *recvbuf,int NumBytes) 

int ProcessCameraData(MAC_Connection *lmac, char *recvbuf, int NumBytes)
{
	//get IP address of camera
	lmac->cam.flags |= ROBOT_CAMERA_INFO_CONNECTED;

	return(1);
}

int ProcessGPSData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 
{
	FTWindow *twin;
	FTControl *tc;
	int SelectedPCB,len,i,j;
	float fval;
//	int latdeg,latmin,latsec;
//	int longdeg,longmin,longsec;
//	int alt;
	char conv[10];

	//todo: add current GPS data to a global structure


//good links for GPS:
//http://www.gpsinformation.org/dale/nmea.htm
//https://en.wikipedia.org/wiki/NMEA_0183
//https://www.sparkfun.com/datasheets/GPS/NMEA%20Reference%20Manual1.pdf

	//see if GPS window is open, if no open it
	twin=GetFTWindow("winGPS");
	if (twin==0) {  //this thread can't be the parent thread of the window, because GetInput doesn't work- the hourglass just spins and the window can't be clicked on
		lmac->flags|=ROBOT_MAC_CONNECTION_OPEN_WINDOW;
		lmac->AddWindowFunction=(FTControlfunc *)winGPS_AddFTWindow;
	} else {
		//FT_SetFocus(twin,0); //give the window the focus
	}

	//determine if this data comes from an EthAccel PCB that is currently selected in the Analog (and Accel) Window
	//output data to the Text box depending on which Analog Sensor (0-15) the data corresponds to
	SelectedPCB=0;
	tc=GetFTControl("txtGPSDestIPAddressStr");									
	if (tc!=0) {
		//control exists
		if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
			//The PCB connected to this data is curently selected in the Power window
			//so update the data in the textboxes
			SelectedPCB=1;
	
		} //if (!strcmp(tc->text,lmac->DestIPAddressStr)) {
	} //if (tc!=0) {


	if (SelectedPCB) {
		//get control to output data
		tc=GetFTControl("txtGPSData");
		if (tc!=0) {
			//sprintf(tc->text,"%s%c",tc->text,
			//add the new character(s)
			len=strlen(tc->text);
			if (len>200) {
				len=0;
			}
			i=5;
			while(i<NumBytes) {
				tc->text[len]=(char)recvbuf[i++];
				len++;
			}
			tc->text[len]=0;
			DrawFTControl(tc);
		} else {
			fprintf(stderr,"Could not get control txtGPSData\n");
		}

	//extract the Latitude, Longitude and Altitude from the $GPGGA line
	if (!strncmp(&recvbuf[5],"$GPGGA",6)) {
		//this is a $GPGGA line


		//probably this should be done with strtok
		//because some fields can be empty (two commas in a row)
		//I'm not sure how strtok handles 2 commas in a row

		//get each control and copy the data into them
		tc=GetFTControl("lblGPSLatitudeDeg");
		if (tc!=0) {
			sprintf(tc->text,"%c%c%c",recvbuf[23],recvbuf[24],176);
			DrawFTControl(tc);
		}
		tc=GetFTControl("lblGPSLatitudeMin");
		if (tc!=0) {
			sprintf(tc->text,"%c%c'",recvbuf[25],recvbuf[26]);
			DrawFTControl(tc);
		}
		//seconds are represented as a decimal fraction
		tc=GetFTControl("lblGPSLatitudeSec");
		if (tc!=0) {
			strncpy(conv,&recvbuf[28],4);
			conv[4]=0;
			fval=0.006*atof(conv); //convert the fraction to minutes
			//for example 0117 (.0117 * 60) = 0.702 minutes

			//sprintf(tc->text,"%.0f\" %c",fval,recvbuf[33]); //can round to 60"
			sprintf(tc->text,"%d\" %c (%s)",(int)fval,recvbuf[33],conv);
			DrawFTControl(tc);
		}

		tc=GetFTControl("lblGPSLongitudeDeg");
		if (tc!=0) {
			sprintf(tc->text,"%c%c%c%c",recvbuf[35],recvbuf[36],recvbuf[37],176); //°
			DrawFTControl(tc);
		}
		tc=GetFTControl("lblGPSLongitudeMin");
		if (tc!=0) {
			sprintf(tc->text,"%c%c'",recvbuf[38],recvbuf[39]);
			DrawFTControl(tc);
		}
		//seconds are represented as a decimal fraction
		tc=GetFTControl("lblGPSLongitudeSec");
		if (tc!=0) {
			strncpy(conv,&recvbuf[41],4);
			conv[4]=0;
			fval=0.006*atof(conv); //convert the fraction to minutes
			//for example 0117 (.0117 * 60) = 0.702 minutes
			
			//sprintf(tc->text,"%.0f\" %c",fval,recvbuf[46]);//can round up to 60"
			sprintf(tc->text,"%d\" %c (%s)",(int)fval,recvbuf[46],conv);
			DrawFTControl(tc);
		}

		//the next field is number of satellites (2 characters)

		//altitude
		tc=GetFTControl("lblGPSAltitudeValue");
		if (tc!=0) {
			i=53;
			//move forward to the next comma
			while(recvbuf[i]!=0x2c) { //not another comma
				i++; //move past 
			}
			//move forward to the next comma to get the size of the altitude field
			j=i+1;
			while(recvbuf[j]!=0x2c) { //not another comma
				j++; //move past 
			}

			strncpy(conv,&recvbuf[i+1],j-i-1);
			conv[5]=0;
			sprintf(tc->text,"%s %c",conv,recvbuf[j+1]);
			DrawFTControl(tc);
		}


	} //	if (!strncmp(&recvbuf[5],"$GPGGA",6)) {

	} //if (SelectedPCB) {

	return(1);
} //int ProcessGPSData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr) 


//given PCB NameNum return PCB name
char *GetPCBName(int NameNum)
{
	int i;

	return(RStatus.EthPCBInfo[NameNum].Name);
/*
	i=0;
	while (i<RStatus.NumEthPCBsExpected) {
		if (RStatus.EthPCBInfo[i].Num==NameNum) {
			return(RStatus.EthPCBInfo[i].Name);
		}
		i++;	
	} //while

	return(0);
*/
} //char *GetPCBName(int NameNum)



//Note: Only compares name to loaded Robot PCB names (not unrecognized detected EthPCB "unknown" names)
int GetPCBNameNum(char *Name)
{
	int i;

	if (Name==0) {
		fprintf(stderr,"Error: GetPCBNameNum called with Name==0\n");
		return(-1);
	} 

	i=0;
	while (i<RStatus.NumEthPCBsExpected) {

		if (!strcmp(RStatus.EthPCBInfo[i].Name,Name)) {
			return(i);  //index is same as PCB NameNum
		}
		i++;
	} //while

return(-1);
} //int *GetPCBNameNum(char *)

//Get_PCB_PCBName - return the PCB number based on MAC (which is read from the Robot configuration files)
//this way the PCBName of any PCB on the robot can be easily changed by the user 
//(by changing RobotApp.conf)
//without having to reflash the firmware, or recompile the Robot software
//Get_PCB_Number_By_MAC(unsigned char *mac)
int GetPCBNameNumByMAC(unsigned char *Mac)
{
	int i;

//perhaps better to go through macs

	//go through the PCB lookup table and see if any MAC matches
//	for(i=0;i<NUM_PCBS;i++) {
//	for(i=0;i<NUM_ROBOT_PCB_NAMES;i++) {
	fprintf(stderr,"RStatus.NumEthPCBsExpected=%d\n",RStatus.NumEthPCBsExpected);
	for(i=0;i<RStatus.NumEthPCBsExpected;i++) {
		if (!memcmp(RStatus.EthPCBInfo[i].MAC,Mac,6)) {	//found match
		//if (!memcmp(RStatus.pcb[i].MAC,Mac,6)) {	//found match
			//fprintf(stderr,"found match return %d\n",i);

			return(i); //return PCB "Name" number - unique to each PCB
		}
	}	//for i
	return(-1);	
} //int GetPCBNameNumByMAC(unsigned char *Mac)

//Gets the AccelsPCBNum (EAPCBNum, the index into RSTatus.AccelsPCBInfo[]), not the Robot PCBNum (the index into RStatus.EthPCBInfo[])
//Note an AccelsPCB can be either an EthAccels or GAMTP PCB
int GetAccelsPCBNameNum(char *Name)
{
	int i;

	if (Name==0) {
		fprintf(stderr,"Error: GetAccelsPCBNameNum called with Name==0\n");
		return(-1);
	} 

	i=0;
//	while (i<RStatus.NumEthAccelsPCBsExpected) {
	while (i<RStatus.NumAccelsPCBsExpected) {

//		if (!strcmp(RStatus.EthAccelsPCBInfo[i].Name,Name)) {
		if (!strcmp(RStatus.AccelsPCBInfo[i]->Name,Name)) {
			return(i);  //index is same as PCB NameNum
		}
		i++;
	} //while

return(-1);
} //int GetAccelsPCBNameNum(char *Name)

char *GetAccelReadableName(char *AccelName)
{
	int i;

	i=0;
	while (i<RStatus.NumAccelsExpected) {
	
		if (!strcmp(RStatus.AccelInfo[i].Name,AccelName)) {
			return(RStatus.AccelInfo[i].ReadableName);
		} 
		i++;	
	} //while
	
	fprintf(stderr,"GetAccelReadableName: Found no Accel with the name %s.\n",AccelName);
	return(0);

} //int GetAccelReadableName(char *AccelName)


//Note that there is a difference between motor number and motor name number, motor number is the port number on the EthMotors PCB (there can be more than one motor number 0), while motor name number are (and must be) unique and are the numbers for the various motor names (MOTOR_LEFT_FOOT, etc.)
int GetMotorNum(char *param)
{
	int i;
	char *endptr;

	//user can enter Motor name or number
	//check for name first

	i=0;
//	while (i<NUM_MOTOR_NAMES) {
	while (i<RStatus.NumMotorsExpected) {
	
		if (!strcmp(RStatus.MotorInfo[i].Name,param)) {
			return(RStatus.MotorInfo[i].Num);
		}
		i++;	
	} //while


	//no matches with ROBOT_MOTOR_NAMES array, so presume param must be a number
	i=(int)strtol(param,&endptr,16);

	if (endptr[0]==0) {
		//entire string was converted
		return(i);
	} else {
		fprintf(stderr,"Error: Script parameter %s could not be converted into a Motor number.\n",param);
		return(-1);
	}

} //int GetMotorNum(char *param)

char *GetMotorName(int PCBNum,int num)
{
	int i;

//could just return ROBOT_PCB_ETHMOTORS_LOWER_BODY[(PCBNum-6)*12+num]
//but possibly the number of motors on an EthMotors PCB may vary.

	i=0;
//	while (i<NUM_MOTOR_NAMES) {
	while (i<RStatus.NumMotorsExpected) {
	
		if (RStatus.MotorInfo[i].PCBNum==PCBNum && RStatus.MotorInfo[i].Num==num) {
			return(RStatus.MotorInfo[i].Name);
		} //		if (RStatus.MotorInfo[i].Num==num) {
		i++;	
	} //while

	fprintf(stderr,"GetMotorName: Found no PCBNum %d Motor num %d match. Set MotorInfo correctly for this combination.\n",PCBNum,num);	
	return(0);

} //int GetMotorName(int PCBNum,int num)

char *GetMotorReadableName(char *MotorName)
{
	int i;

	i=0;
	while (i<RStatus.NumMotorsExpected) {
	
		if (!strcmp(RStatus.MotorInfo[i].Name,MotorName)) {
			return(RStatus.MotorInfo[i].ReadableName);
		} //		if (RStatus.MotorInfo[i].Num==num) {
		i++;	
	} //while
	
	fprintf(stderr,"GetMotorReadableName: Found no Motor with the name %s.\n",MotorName);
	return(0);

} //int GetMotorReadableName(int param)


char *GetMotorReadableName_By_PCB(int PCBNum,int num)
{
	int i;

	i=0;
//	while (i<NUM_MOTOR_NAMES) {
	while (i<RStatus.NumMotorsExpected) {
	
		if (RStatus.MotorInfo[i].PCBNum==PCBNum && RStatus.MotorInfo[i].Num==num) {
			return(RStatus.MotorInfo[i].ReadableName);
		} //		if (RStatus.MotorInfo[i].Num==num) {
		i++;	
	} //while
	
	fprintf(stderr,"GetMotorReadableName: Found no PCBNum %d Motor num %d match in MotorInfo. Set MotorInfo correctly for this combination.\n",PCBNum,num);
	return(0);

} //int GetMotorReadableName_By_PCB(int param)

//Get the number of the motor in the ROBOT_MOTOR_NAMES array from the motor name
int GetMotorNameNum(char *MotorName)
{
	int i;

	//user can enter Motor name or number
	//check for name first

	i=0;
//	while (i<NUM_MOTOR_NAMES) {
		while (i<RStatus.NumMotorsExpected) {

//		if (!strcmp(ROBOT_MOTOR_NAMES[i],MotorName)) {
		if (!strcmp(RStatus.MotorInfo[i].Name,MotorName)) {
			return(i);
		}
		i++;	
	} //while


	return(-1);

} //int GetMotorNameNum(char *MotorName)


int GetMotorNameNumByPCBNum(int PCBNum,int num)
{
	int i;

//could just return ROBOT_PCB_ETHMOTORS_LOWER_BODY[(PCBNum-6)*12+num]
//but possibly the number of motors on an EthMotors PCB may vary.

	i=0;
	while (i<RStatus.NumMotorsExpected) {
	
		if (RStatus.MotorInfo[i].PCBNum==PCBNum && RStatus.MotorInfo[i].Num==num) {
			return(i);
		} //		if (RStatus.MotorInfo[i].Num==num) {
		i++;	
	} //while

	fprintf(stderr,"GetMotorNameNumFromPCBNum: Found no PCBNum %d Motor num %d match in MotorInfo. Set MotorInfo correctly for this combination.\n",PCBNum,num);	
	return(0);

} //int GetMotorNameNumByPCBNum(int PCBNum,int num)


/* commented because not currently used, and number of accels on EthAccels PCB may vary (but probably will not)
//Get Accel Name: uses Eth PCBNum (see enum ROBOT_PCB_NAME_NUMBERS) and AccelNum to get name from ROBOT_ACCEL_NAMES array
char *GetAccelName(int PCBNum,int num)
{
	int i;


	if (PCBNum>0 && PCBNum<NUM_ROBOT_PCB_NAMES && num>0 && num<3) {
		return(ROBOT_ACCEL_NAMES[(PCBNum-1)*3+num]);
	} 

	return(0);
} //int GetAccelName(int PCBNum,int num)
*/



//Get the Accelerometer "Name" number: all acceleretomers have a unique name and NameNumber (see above array ROBOT_ACCEL_NAMES[]).
int GetAccelNameNum(char *AccelName)
{
	int i;

	i=0;
	while (i<RStatus.NumAccelsExpected) {
	
		if (!strcmp(RStatus.AccelInfo[i].Name,AccelName)) {
			return(i);
		}
		i++;	
	} //while

	return(-1);
} //int GetAccelNameNum(char *AccelName)


//PCBNum is index into RStatus.EthPCBInfo
int GetAccelNameNum_By_PCB(int PCBNum,int AccelNum)
{
	int i;

	//go through all accels and find match for PCBNum and AccelNum
	for(i=0;i<RStatus.NumAccelsExpected;i++) {
		if (RStatus.AccelInfo[i].PCBNum==PCBNum && RStatus.AccelInfo[i].Num==AccelNum) { 
			//this accel, return index
			return(i);
		} 
	} //for i

return(-1);
} //int GetAccelNameNum_By_PCB(int PCBNum,int AccelNum)

//PCBNum is index into RStatus.EthPCBInfo
char *GetAccelName_By_PCB(int PCBNum,int AccelNum)
{
	int i;

	//go through all accels and find match for PCBNum and AccelNum
	for(i=0;i<RStatus.NumAccelsExpected;i++) {
		if (RStatus.AccelInfo[i].PCBNum==PCBNum && RStatus.AccelInfo[i].Num==AccelNum) { 
			//this accel, return Name
			return(RStatus.AccelInfo[i].Name);
		} 
	} //for i

return(0);
} //char *GetAccelName_By_PCB(int PCBNum,int AccelNum)



//Get the Analog Sensor "Name" number: all analog sensors have a unique name and NameNumber (see above array ROBOT_ANALOG_SENSOR_NAMES[]). Note that AnalogSensor Name number is different than AnalogSensor Number (ADC number on PCB, for PIC32MZ starts with 1)
int GetAnalogSensorNameNum(char *AnalogSensorName)
{
	int i;

	i=0;
	while (i<RStatus.NumAnalogSensorsExpected) {	
		if (!strcmp(RStatus.AnalogSensorInfo[i].Name,AnalogSensorName)) {
			return(i);
		}
		i++;	
	} //while

	return(-1);
} //int GetAnalogSensorNameNum(char *AnalogSensorName)

//PCBNum is index into RStatus.EthPCBInfo
int GetAnalogSensorNameNum_By_PCB(int PCBNum,int SensorNum)
{
	int i;

	//go through all analog sensors and find match for PCBNum and AnalogSensorNum
	for(i=0;i<RStatus.NumAnalogSensorsExpected;i++) {
		if (RStatus.AnalogSensorInfo[i].PCBNum==PCBNum && RStatus.AnalogSensorInfo[i].Num==SensorNum) { 
			//this analog sensor, return index
			return(i);
		} 
	} //for i

return(-1);
} //int GetAnalogSensorNameNum_By_PCB(int PCBNum,int SensorNum)


//given AnalogSensor name, return name of PCB Analog Sensor is on
char *GetAnalogSensorPCBName(char *Name)
{
	int i,NameNum;

	i=0;
	while (i<RStatus.NumAnalogSensorsExpected) {
		//fprintf(stderr,"%d %s\n",i,RStatus.AnalogSensorInfo[i].Name);
		if (!strcmp(RStatus.AnalogSensorInfo[i].Name,Name)) {
			NameNum=RStatus.AnalogSensorInfo[i].PCBNum;
			return(RStatus.EthPCBInfo[NameNum].Name);
		}
		i++;	
	} //while

	return(0);
} //char *GetAnalogSensorPCBName(char *Name)


int GetCurrentPCBCount(void) {

	MAC_Connection *lmac;
	int count;

	//count up external mac connections	
	count=0;
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		//check for remote match

//		if (!(lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL)) {
		//if (lmac->pcb.Num!=-1) {
//		if (lmac->pcb.Name[0]!=0) {
		if (lmac->flags&ROBOT_MAC_CONNECTION_IS_ETHPCB) {
			count++;
		}
	lmac=lmac->next;
	} //while

	return(count);
} //int GetCurrentPCBCount(void) {

int GetCurrentEthAccelsPCBCount(void) {

	MAC_Connection *lmac;
	int count;

	//count up external mac connections	
	count=0;
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		if (!strncmp(lmac->Name,"ETHACCELS",9)==0) {
			count++;
		}
	lmac=lmac->next;
	} //while

	return(count);

} //int GetCurrentEthAccelsPCBCount(void) {

int DeleteAllExternalPCBConnections(void) {

	MAC_Connection *lmac,*next;

	//count up external mac connections	
	lmac=RStatus.iMAC_Connection;
	while(lmac!=0) {
		//check for remote match

		next=lmac->next;
//		if (!(lmac->flags&ROBOT_MAC_CONNECTION_IS_LOCAL)) {
		if (lmac->pcb.Num!=0) {
//		if (lmac->EthPCBInfo.Name[0]!=0) {
			Delete_MAC_Connection(lmac);
		}
	lmac=next;
	} //while

	return(1);
} //int DeleteAllExternalPCBConnections(void) {

//Get the motor thrust (force) that can surpass the force needed
int GetNeededThrust(int MotorNameNum,float ForceNeeded,int WithGravity) {
	int i;
	MotorInfoTable *lmi;

	i=1;

	lmi=&RStatus.MotorInfo[MotorNameNum];
	while(i<ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE) {
		//fprintf(stderr,"i=%d %5.0f\n",i,lmi->Thrust[i]);
		if (ForceNeeded<=lmi->Thrust[i]) {
			return(i);
		} 
		if (WithGravity && i==lmi->MaxThrustWG) {
			return(i);
		}
		if (!WithGravity && i==lmi->MaxThrustAG) {
			return(i);
		}
		i++;
	} //while(!endloop)
	return(0);
} //int GetNeededThrust(int MotorNameNum,float ForceNeeded) {

//return the string length until the character with ascii num
int GetWordLen(char *tstr,int num) 
{
	int i,len;

	len=strlen(tstr);
	i=0;
	while (i<len) {
		if (tstr[i]==num) {
			return(i);
		}
		i++;
	} //while (i<len)
return(i);
}//int GetWordLen(char *tstr,int num) 

// update mac in EthAccels.conf, or EthMotors.conf, or EthPower.conf
int UpdateEthConfig(char *PCBName,char *Kind)
{
	int KindNum,FoundPCB,LastChar,PCBNameNum;
	char ConfFileName[256];
	char tline[FTMedStr],tstr[FTMedStr],filename[FTMedStr];
	int bSize,result;
	int64_t fSize;
	char *buf,*param;//,*holdptr;
	int NumLines,CurLine,i,j,k,CurChar;
	FILE *fptr;

	KindNum=0;
	//Load .conf file
	if (!strcmp(Kind,"Accel")) {
		KindNum=1;
		strcpy(ConfFileName,"EthAccels.conf");
	} 
	if (!strcmp(Kind,"Motor")) {
		KindNum=2;
		strcpy(ConfFileName,"EthMotors.conf");
	} 
	if (!strcmp(Kind,"Power")) {
		KindNum=3;
		strcpy(ConfFileName,"EthPower.conf");
	} 

	if (KindNum==0) {
			fprintf(stderr,"Unknown PCB kind %s for PCB named %s\n",Kind,PCBName);
			return(0);
	}


	PCBNameNum=GetPCBNameNum(PCBName);
	if (PCBNameNum<0) {
		fprintf(stderr,"GetPCBNameNum failed for PCB Name %s\n",PCBName);
		return(0);	
	}


#if Linux
	sprintf(filename,"%s/%s",RStatus.CurrentRobotFolder,ConfFileName);
#endif
#if WIN32
	sprintf(filename,"%s\\%s",RStatus.CurrentRobotFolder,ConfFileName);
#endif


	fptr=fopen(filename,"rb"); //currently presumes config is in current working directory
	if (fptr!=0) {

		//read in the entire file

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
		buf=(char *)malloc(fSize+1);  //+1 for 0 added at end
		if (buf == NULL) {
			sprintf(tstr,"Allocating memory for config file failed.\n");
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			return(0);
		}

		//copy the file into the buffer:
		result=fread(buf,1,fSize,fptr);
		if (result != fSize) {
			sprintf(tstr,"Error reading config file %s\n",filename);
			FTMessageBox(tstr,FTMB_OK,"Error",0);
			fclose(fptr);
			free(buf);
			return(0);
		}

		fclose(fptr);
		buf[fSize]=0; //close string of buffer - so strlen will give correct size
		bSize=fSize;


		//go through each line and each word (separated by spaces or a comma)
		//determine total number of lines in order to allocate space for a pointer to each for strtok
		FoundPCB=0;
		NumLines=0;
		i=0;
		while(i<bSize) {
			if (buf[i]==10|| (buf[i]!=10 && i==bSize-1)) {
				NumLines++;
			}
			i++;
		}  //while(i<bSize)



		CurLine=0;
		CurChar=0;
		LastChar=0;
		while(CurLine<NumLines) { 

			//get a line from the file text
			Robot_GetLineFromText(tline,buf+CurChar,bSize-CurChar);
			LastChar=CurChar;
			CurChar+=strlen(tline);
			CurLine++;
			if (PStatus.flags&PInfo) {
				fprintf(stderr,"tline=%s\n",tline);
			}

			//todo trim any initial spaces
			//get a word on this line
			param=strtok(tline,"="); 
			//see if this word matches any known section
			i=0;
			if (param!=0) {
				//determine which field is on the current line
				if (!strcmp(param,"PCB_NAME")) {
					param=strtok(NULL,"#\n"); //get next token
					if (param!=0) {
						if (!strcmp(param,PCBName)) {
							FoundPCB=1;
						} //if (!strcmp(param,PCBName)) {
						RStatus.ModelConf.ColumnWidth=atoi(param);
					} //if (param!=0) {
				} //if (!strcmp(param,"PCB_NAME")) {
				if (!strcmp(param,"PCB_MAC") && FoundPCB) {
					param=strtok(NULL,"#\n"); //get next token
					if (param!=0) {
						FoundPCB=0;
						//replace MAC in Config file (todo: could be extra spaces in user file, etc. currently ignoring this possibility)
						ConvertBytesToString(RStatus.EthPCBInfo[PCBNameNum].MAC,tstr,6); //convert bytes to string
						memcpy(buf+LastChar+8,tstr,12);
						//PCBInfoTable is already updated
					} //if (param!=0) {
				} //if (!strcmp(param,"PCB_MAC") && FoundPCB) {

			} //if (param!=0) {

		} //while(CurLine<NumLines) 
			

			//write file back out
		fptr=fopen(filename,"wb"); //currently presumes config is in current working directory
		if (fptr!=0) {

			//copy the file into the buffer:
			result=fwrite(buf,1,fSize,fptr);
			if (result != fSize) {
				sprintf(tstr,"Error writing config file %s\n",filename);
				FTMessageBox(tstr,FTMB_OK,"Error",0);
				fclose(fptr);
				free(buf);
				return(0);
			}

			fclose(fptr);
			buf[fSize]=0; //close string of buffer - so strlen will give correct size
			bSize=fSize;

			switch(KindNum) {
				case 1:
					fprintf(stderr,"Updated EthAccels.conf for PCB names %s\n",PCBName);
				break;
				case 2:
					fprintf(stderr,"Updated EthMotors.conf for PCB names %s\n",PCBName);
				break;
				case 3:
					fprintf(stderr,"Updated EthPower.conf for PCB names %s\n",PCBName);
				break;
			} //switch(KindNum) {


		} else { //if (fptr!=0) {
			fprintf(stderr,"Could not open file %s\n",filename);
			free(buf);
			return(0);
		} //else if (fptr!=0) {
	} else { //if (fptr!=0) {
		fprintf(stderr,"Could not open file %s\n",filename);
		return(0);
	} //else if (fptr!=0) {

	free(buf);

	return(1);
} //int UpdateEthConfig(RStatus.EthPCBInfo[PCBNameNum].Name,"Accel")

char *GetAccelNameFromReadableName(char *tstr)
{
	int i;

	i=0;
	while(i<RStatus.NumAccelsExpected) {
		if (!strcmp(RStatus.AccelInfo[i].ReadableName,tstr)) {
			return(RStatus.AccelInfo[i].Name);
		}
		i++;
	} //while

	return(0);
} //int GetAccelNameFromReadableName(char *tstr)


char *GetMotorNameFromReadableName(char *tstr)
{
	int i;

	i=0;
	while(i<RStatus.NumMotorsExpected) {
		if (!strcmp(RStatus.MotorInfo[i].ReadableName,tstr)) {
			return(RStatus.MotorInfo[i].Name);
		}
		i++;
	} //while

	return(0);
} //int GetMotorNameFromReadableName(char *tstr)


