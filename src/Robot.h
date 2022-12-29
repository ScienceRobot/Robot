//Robot.h
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
#ifndef _ROBOT_H
#define _ROBOT_H


#include <stdint.h> //for uint_32
#if Linux
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <math.h>  //for sin,sinf
#if USE_RT_PREEMPT
#include <sys/mman.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#if USE_FFMPEG
#include "libavcodec/avcodec.h"  //for MJPEG needs to be on top because sizeof(AVFrame) !=
#include "libavformat/avformat.h"  //for avformatcontext an avoutputformat
#endif//USE_FFMPEG
#if USE_SOUND
#include <alsa/asoundlib.h> //for alsa
#include <pulse/simple.h> //for pulseaudio sound system
#include <pulse/error.h> //for pulseaudio sound system
#endif //USE_SOUND
#endif //Linux
#include "robot_motor_mcu_instructions.h"
//#include "RobotConfig.h"

#if USE_RT_PREEMPT
#define RT_PRIORITY (49) //PREEMPT_RT uses 50 for kernel tasklets and interrupt handlers by default so use 49
#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
#define TasksTimerInterval 10000000 //10ms - TasksTimerInterval: Interval that the Tasks thread runs
#define TurnMotorTimerInterval 150000 //150us - TurnMotorTimerInterval: Interval that the TurnMotor thread runs
#endif


#define USE_MPU6050 1 //currently using MPU6050 for accelerometer and gyroscope
#define USE_FXOS8700CQ 0

#define ROBOT_MAX_INST_LEN	256 //maximum robot instruction length in bytes
#define ROBOT_ADD_IP_ADDRESS_TO_INSTRUCTION 0x00000001 //add the source IP address to the instruction to send to the robot
#define PCB_NAME_LENGTH 5  //the number of characters used by an Eth PCB to identify itself (for example "Motor00")
#define MAX_NUM_ACCELS_ON_ETHACCELS 8 //8 accels on GAMTP 3  //no more than 3 accelerometers on any EthAccels PCB
#define MAX_NUM_ANALOG_SENSORS_ON_ETHACCELS 16  //no more than 16 analog (touch or potentiometer) sensors on any EthAccels PCB
#define DEFAULT_NETWORK_BUFLEN 2048 //512 - cam packets are 1074

//motor constants
#define MOTOR_CLOCKWISE 1 //0 //make 1?
#define MOTOR_COUNTER_CLOCKWISE -1//1  //make -1?
#define MOTOR_SPEED_1_7 1  //perhaps Slowest, Slow,Medium, Fast, Faster, Fastest, Full
#define MOTOR_SPEED_2_7 2
#define MOTOR_SPEED_3_7 3
#define MOTOR_SPEED_4_7 4
#define MOTOR_SPEED_5_7 5
#define MOTOR_SPEED_6_7 6
#define MOTOR_SPEED_7_7 7


//Accel Angles, one of the 3 accelerometer angles
#define ACCEL_ANGLE_X 0
#define ACCEL_ANGLE_Y 1
#define ACCEL_ANGLE_Z 2

//PCB constants 
//#define NUM_CAMERAS 1 //number of cameras on the robot
#define MAX_PCB_NAME_LENGTH 128 //length of string that describes the PCB Name
#define MAX_PCB_KIND_LENGTH 50 //length of string that describes PCB Kind ("Accel", "Motor", "Power", etc.)
//#define ROBOT_MAX_NUM_CONNECTIONS 4096 //4096 connections?
#define DEFAULT_MOTOR_MAX_THRUST_AG 4  //default motor MaxThrust when moving against gravity
#define DEFAULT_MOTOR_MAX_THRUST_WG 2  //default motor MaxThrust when moving with gravity

//Defaults for MotorAngle Associations
#define DEFAULT_MOTION_THRESHOLD 1.0
#define DEFAULT_MIN_VELOCITY 1.0
#define DEFAULT_MAX_VELOCITY 5.0
#define DEFAULT_MAX_ACCEL 0.5
#define DEFAULT_TVR 1.0


#if USE_RT_PREEMPT
#define TURN_MOTOR_NUM_INSTS 100  //number of TurnMotor instructions in the TurnMotorQueue (using RT_PREEMPT only) 
#endif 

#define NUM_ROBOT_CONFIG_SECTIONS 9
typedef enum
{
	ROBOT_CONFIG_HOME_FOLDER =0,  
	ROBOT_CONFIG_READABLE_NAME,
	ROBOT_CONFIG_NUM_PCBS,
	ROBOT_CONFIG_NUM_ETHMOTORS_PCBS,
	ROBOT_CONFIG_NUM_ETHACCELS_PCBS,
	ROBOT_CONFIG_NUM_ETHPOWER_PCBS,
	ROBOT_CONFIG_NUM_GAMTP_PCBS,
	ROBOT_CONFIG_3D_MODEL_NAME,
	ROBOT_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS
} ROBOT_CONFIG_FILE_SECTIONS;

#define NUM_ETHMOTORS_CONFIG_SECTIONS 13 
typedef enum
{
	ETHMOTORS_CONFIG_NUM_ETHMOTORS_PCBS =0,  
	ETHMOTORS_CONFIG_PCB_NUM,
	ETHMOTORS_CONFIG_PCB_NAME,
	ETHMOTORS_CONFIG_PCB_MAC,
	ETHMOTORS_CONFIG_NUM_MOTORS,
	ETHMOTORS_CONFIG_MOTOR_NUM,
	ETHMOTORS_CONFIG_MOTOR_NAME,
	ETHMOTORS_CONFIG_MOTOR_READABLE_NAME,
	ETHMOTORS_CONFIG_MOTOR_ORIENTATION,
	ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_AG,
	ETHMOTORS_CONFIG_MOTOR_MAX_THRUST_WG,
	ETHMOTORS_CONFIG_MOTOR_PAIR,
	ETHMOTORS_CONFIG_MOTOR_COMPLEMENT
} ETHMOTORS_CONFIG_FILE_SECTIONS;

#define NUM_ETHACCELS_CONFIG_SECTIONS 19 
typedef enum
{
	ETHACCELS_CONFIG_NUM_ETHACCELS_PCBS =0,  
	ETHACCELS_CONFIG_PCB_NUM,
	ETHACCELS_CONFIG_PCB_NAME,
	ETHACCELS_CONFIG_PCB_MAC,
	ETHACCELS_CONFIG_NUM_ACCELS,
	ETHACCELS_CONFIG_ACCEL_NUM,
	ETHACCELS_CONFIG_ACCEL_NAME,
	ETHACCELS_CONFIG_ACCEL_READABLE_NAME,
	ETHACCELS_CONFIG_ACCEL_ORIENTATION,
	ETHACCELS_CONFIG_ACCEL_CALIB_PITCH,
	ETHACCELS_CONFIG_ACCEL_CALIB_YAW,
	ETHACCELS_CONFIG_ACCEL_CALIB_ROLL,
	ETHACCELS_CONFIG_ACCEL_3DMODEL_AXIS,
	ETHACCELS_CONFIG_NUM_ANALOG_SENSORS,
	ETHACCELS_CONFIG_ANALOG_SENSOR_NUM,
	ETHACCELS_CONFIG_ANALOG_SENSOR_KIND,
	ETHACCELS_CONFIG_ANALOG_SENSOR_NAME,
	ETHACCELS_CONFIG_ANALOG_SENSOR_MINV_ANGLE,
	ETHACCELS_CONFIG_ANALOG_SENSOR_MAXV_ANGLE
} ETHACCELS_CONFIG_FILE_SECTIONS;

#define NUM_ETHPOWER_CONFIG_SECTIONS 4
typedef enum
{
	ETHPOWER_CONFIG_NUM_ETHPOWER_PCBS =0,  
	ETHPOWER_CONFIG_PCB_NUM,
	ETHPOWER_CONFIG_PCB_NAME,
	ETHPOWER_CONFIG_PCB_MAC
} ETHPOWER_CONFIG_FILE_SECTIONS;

#define NUM_GAMTP_CONFIG_SECTIONS 28 
typedef enum
{
	GAMTP_CONFIG_NUM_GAMTP_PCBS =0,  
	GAMTP_CONFIG_PCB_NUM,
	GAMTP_CONFIG_PCB_NAME,
	GAMTP_CONFIG_PCB_MAC,
	GAMTP_CONFIG_NUM_MOTORS,
	GAMTP_CONFIG_MOTOR_NUM,
	GAMTP_CONFIG_MOTOR_NAME,
	GAMTP_CONFIG_MOTOR_READABLE_NAME,
	GAMTP_CONFIG_MOTOR_ORIENTATION,
	GAMTP_CONFIG_MOTOR_MAX_THRUST_AG,
	GAMTP_CONFIG_MOTOR_MAX_THRUST_WG,
	GAMTP_CONFIG_MOTOR_PAIR,
	GAMTP_CONFIG_MOTOR_COMPLEMENT,
	GAMTP_CONFIG_NUM_ACCELS,
	GAMTP_CONFIG_ACCEL_NUM,
	GAMTP_CONFIG_ACCEL_NAME,
	GAMTP_CONFIG_ACCEL_READABLE_NAME,
	GAMTP_CONFIG_ACCEL_ORIENTATION,
	GAMTP_CONFIG_ACCEL_CALIB_PITCH,
	GAMTP_CONFIG_ACCEL_CALIB_YAW,
	GAMTP_CONFIG_ACCEL_CALIB_ROLL,
	GAMTP_CONFIG_ACCEL_3DMODEL_AXIS,
	GAMTP_CONFIG_NUM_ANALOG_SENSORS,
	GAMTP_CONFIG_ANALOG_SENSOR_NUM,
	GAMTP_CONFIG_ANALOG_SENSOR_KIND,
	GAMTP_CONFIG_ANALOG_SENSOR_NAME,
	GAMTP_CONFIG_ANALOG_SENSOR_MINV_ANGLE,
	GAMTP_CONFIG_ANALOG_SENSOR_MAXV_ANGLE
} GAMTP_CONFIG_FILE_SECTIONS;

#define NUM_MOTOR_ANGLE_CONFIG_SECTIONS 16
typedef enum
{
	//MOTOR_ANGLE_CONFIG_NUM_ASSOCIATIONS =0,  
	//MOTOR_ANGLE_CONFIG_ASSOCIATION_NUM,
	MOTOR_ANGLE_CONFIG_DEFAULT_MOTION_THRESHOLD=0,
	MOTOR_ANGLE_CONFIG_DEFAULT_MIN_VELOCITY,
	MOTOR_ANGLE_CONFIG_DEFAULT_MAX_VELOCITY,
	MOTOR_ANGLE_CONFIG_DEFAULT_MAX_ACCEL,
	MOTOR_ANGLE_CONFIG_DEFAULT_THRUST_VELOCITY_RATIO,
	MOTOR_ANGLE_CONFIG_MOTOR_NAME,
	MOTOR_ANGLE_CONFIG_ACCEL_NAME,
	MOTOR_ANGLE_CONFIG_ANALOG_SENSOR_NAME,
	MOTOR_ANGLE_CONFIG_ANGLE_XYZ,
	MOTOR_ANGLE_CONFIG_INC_ANG_DIR,
	MOTOR_ANGLE_CONFIG_MOTION_THRESHOLD,
	MOTOR_ANGLE_CONFIG_MIN_VELOCITY,
	MOTOR_ANGLE_CONFIG_MAX_VELOCITY,
	MOTOR_ANGLE_CONFIG_MAX_ACCEL,
	MOTOR_ANGLE_CONFIG_THRUST_VELOCITY_RATIO,
	MOTOR_ANGLE_CONFIG_FLAGS
} MOTOR_ANGLE_CONFIG_FILE_SECTIONS;

#define NUM_ROBOT_APP_CONFIG_SECTIONS	4//28
//#define NUM_ROBOT_APP_CONFIG_MOTOR_SECTIONS	15 //number of config sections that are motor orientation, used for saving config file
typedef enum
{
	ROBOT_APP_CONFIG_ROBOTS_FOLDER =0,
	ROBOT_APP_CONFIG_DEFAULT_ROBOT,
	ROBOT_APP_CONFIG_LOG_FOLDER,
	ROBOT_APP_CONFIG_DELETE_LOGS_OLDER_THAN_DAYS
} ROBOT_APP_CONFIG_FILE_SECTIONS;






//Probably there will be an EthMotor_PCB data structure to store EthMotors PCBs detected (and motor current sense values)

//Camera Info
//#define DEFAULT_CAMERA_BUFLEN 512
#define ROBOT_CAMERA_WIDTH 640//1280//640
#define ROBOT_CAMERA_HEIGHT 480//720//480
#define ROBOT_CAMERA_NETWORK_TIMEOUT 1000 //3000ms
#define CAMERA_IMAGE_BUFFER_SIZE 400000//55000//250000//55000 //jpeg image buffer <1mb- a smaller buffer makes less corrupt image errors
#define CAMERA_MAX_IMAGE_SIZE 100000//25000 //max jpeg size = typical is 18k
#define ROBOT_CAMERA_INFO_CONNECTED	0x00000001  //camera ip and port is established
#define ROBOT_CAMERA_INFO_AUTHENTICATED	0x00000002  //camera ip and port has been confirmed
#define ROBOT_CAMERA_INFO_GET_IMAGE 0x00000004 //get a single image
#define ROBOT_CAMERA_INFO_GET_VIDEO 0x00000008 //get a stream of images (video)
#define ROBOT_CAMERA_INFO_SHOW_IMAGE 0x00000010 //show each image in window
#define ROBOT_CAMERA_INFO_SAVE_IMAGE 0x00000020 //save each image to a file
#define ROBOT_CAMERA_INFO_GET_IMAGE_PERIODICALLY 0x00000040 //get a single image at a regular interval
//#define ROBOT_CAMERA_INFO_CONFIRM_MOTOR_MOVE 0x00000080 //set until a GET motor instruction is confirmed by the camera
typedef struct CameraInfo {
	uint32_t flags;
//	char IPStr[INET_ADDRSTRLEN];  //string of camera IP address ex:"192.168.0.100"
//	MAC_Connection *mac; //to easily get the IP for sending instructions to the camera (could be just IP and port, but it might change -as might this, but the MAC connection contains more useful info too- like MAC address, name, location, etc.)
	char SerialNumber[20]; //ex: "63VYTFW"
	int CurrentPacket;  //current packet received from camera
	int LastAckdPacket; //Last packet number acknowledged by host
	int StartImagePointer; //place in Image[] array where image starts
	int EndImagePointer; //place in Image[] array to write new data (also images size)
	int FrameNum; //number of complete image frames received
	int InstNum; //number of instructions sent to camera (for example GET)
	uint32_t LastContact; //time (in ms) of last contact from camera
	char Username[128]; //name of user for camera authentication
	char Password[128]; //password of user for camera authentication
	char FilePath[256]; //path of where to save files
		//char idbuffer[40]; //text used to initiate contact with a camera whose IP is already known (if, for example, the connection was closed)
	//int idbuffersize; //size of idbuffer text
//	int ImageSize; //size of the image in the buffer
	unsigned char Image[CAMERA_IMAGE_BUFFER_SIZE]; //image data- 10 buffers of 100k
} CameraInfo;



//Structures used for EthAccels PCBs
typedef union {
	struct {
		float i[3];
	};
	struct {
		float x; //for speed probably should be integer
		float y;
		float z;
	};
} AccelSample;


//when balancing, this tracks what change in the accelerometer angle is expected
typedef  struct AccelExpect {
	int32_t x;  //currently -1= expect a decrease, +1=expect an increase, but later maybe I may set a more specific quantity (like -0.2)
	int32_t y;  //
	int32_t z;  //
} AccelExpect;

#define ACCELEROMETER_GOT_SAMPLE									0x0000001  //this is cleared by a function, and set when the data arrives, so the function can then read the Sample - update this is now replaced with timestamp
//note that you can just use the sample for a lot of processing, which would be faster than calculating the 3 accel angles, but using angles is probably more accurate when the body segment Z is not = 1g, and so is probably necessary.  
#define ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_X	0x0000002 //changing from adding gyroscope X values to using accelerometer X samples because segment has been at rest for a while. This flag is needed so that instructions can add the offset to the TargetAngle (and target range offsets). Otherwise, the change in angle (which can be 2 or 3 degrees sometimes) often causes "re-adjusting": the angle of the body segment from the accelerometer is out of range, which causes the motion code to thrust the motor to turn the body segment back into the target range. This way, the motion code just updates the Segment and adjusts the Target angles by the gyro-accel difference at the same instant, and the body segment remains in (the new) target range. Note that this changes the user's TargetAngle, but generally this only occurs when an instruction has ended and the robot is transitioning back to not moving- so it is better to just change the user's TargetAngle if the robot has not been moving for (typically) 1 second.  
#define ACCELEROMETER_CHANGE_FROM_GYRO_TO_MAG_Y		0x0000004  //changing from gyro Y to using magnetometer sample
#define ACCELEROMETER_CHANGE_FROM_GYRO_TO_ACCEL_Z	0x0000008  //changing from gyro Z to using accelerometer Z angle
#define ACCELEROMETER_SWAP_Y_AND_MINUS_Z  				0x0000010  //swap accel and gyro Y and -Z (because accel/gyro is mounted vertically with Y- pointing up and +X in the normal horizontal orientation)
#define ACCELEROMETER_SWAP_MINUS_Y_AND_MINUS_Z  	0x0000020  //swap accel and gyro -Y and Z (because accel/gyro is mounted vertically with Y+ pointing up and +X in the normal horizontal orientation)  
//#define ACCELEROMETER_FRESH_SAMPLE	0x00000040  //Used by Task thread to know that the Accel sample is no more than 10ms old  
typedef  struct Accelerometer {
	uint32_t	flags;
	uint32_t NameNum; //index into RStatus.AccelInfo[] was: Accel name number (ex: ACCEL_NAME_LEFT_FOOT, ACCEL_NAME_LEFT_LOWER_LEG, etc.), used to get AccelInfoTable
	AccelSample ASample;  //accelerometer sample
	AccelSample LastASample;  //previous accelerometer sample
	AccelSample MSample;  //magnetic sample
	AccelSample LastMSample; //previous magnetic sample
	AccelSample GSample;  //gyroscope sample (angular velocity)
	AccelSample LastGSample; //previous gyroscope sample (angular velocity)
	AccelSample Acceleration; //angular acceleration according to gyroscope
	AccelSample LastAcceleration; //last angular acceleration according to gyroscope (GSample-LastGSample)
	AccelSample Jerk; //angular jerk according to gyroscope (Acceleration-LastAcceleration)
	float EstAngle[3];  //estimated angles (vector of pitch, yaw and roll) of body segment associated with accelerometer and calculated by accelerometer (when not moving) and gyroscope (when moving)  units: degrees
	float LastEstAngle[3];  //last angles (vector of pitch, yaw and roll) of Angle[3] angle
	//float AngleR[3];  //Angle[3] in radians
	float EstAngleChange[3]; //change between current and last angles - used to see which direction robot is falling in
	float LastEstAngleChange[3]; //the last change between current and last angles - used to see if fall is decreasing or increasing
	float AccelAngle[3];  //angles (vector of pitch, yaw and roll) of accelerometer as calculated by accelerometer and magnetometer (for yaw)  (not using gyroscope samples) units; degrees
	int NoMotionCounter[3]; //counts up number of samples where G[xyz] was between -1 and 1. If >=100 (1 second), the accel samples can be used to determine body segment orientation.
	//float AccelAngleR[3];  //AccelAngle units: radians
	//float AccelAngleChange[3]; //change between current and last angles - used to see which direction robot is falling in
	//float LastAccelAngleChange[3]; //the last change between current and last angles - used to see if fall is decreasing or increasing
	float GyroAccelOffset[2]; //when switching from gyro to accel, the difference between gyro-accel, [0]=x, [1]=y, [2]=z
	AccelSample MinM;  //minimum magnetic field in each dimension
	AccelSample MaxM;  //maximum magnetic field in each dimension
	AccelExpect Expect;  //change in accelerometer sample expected from turning a motor
	AccelExpect Unexpected; //running total of unexpected results from turning motors to change this accelerometer
	uint32_t LastTimeStamp; //timestamp of the previous sample
	uint32_t TimeStamp; //timestamp of this sample in ms- used for Balance code to know that this sample is different from the last sample
	float ACali[3]; //accelerometer calibration - possibly move these to EthAccelsPCB and make AOffset[3][3]
	float MCali[3]; //magnetometer calibration
	float GCali[3]; //gyroscope calibration
} Accelerometer;

#define ANALOG_SENSOR_TOUCH_SENSOR 0x00000001 //analog sensor is a touch sensor
#define ANALOG_SENSOR_POTENTIOMETER 0x00000002 //analog sensor is a potentiometer
typedef struct AnalogSensor {
  	uint32_t	flags;
		uint16_t Sample;  //voltage of analog (touch/pot) sensor
		float fSample; //voltage of analog (touch/pot) sensor (float) (perhaps change to fVoltage)
		float Percent; //percent of touch sensor that is pressed, or percentage that is voltage of potentiometer over max voltage
		float PercentChange; //percent change in analog sensor voltage from last sample
		float MinV;  //minimum voltage analog sensor can have
		float MaxV;  //maximum voltage analog sensor can have
//    float Range; //range (in degrees) potentiometer covers, usually 275 degrees- could be more constrained depending on application
		//Potentiometer only variables:
		float MinVAngle; //angle at minimum voltage
		float MaxVAngle; //angle at maximum voltage
		float Angle; //Angle potentiometer represents (Vpot/3.3)*(MaxVAngle-MinVAngle)
		float LastAngle; //Angle at previous sample
		float Velocity; //in degrees/second
		float LastVelocity; //in degrees/second (in order to determine acceleration)
		float Acceleration; //in degrees/second^2
//	float Expect;  //change in analog sensor expected from turning a motor
//	float Unexpected; //running total of unexpected results from turning motors to change this analog sensor
} AnalogSensor;



//TODO: possible to remove this struct in favor of AccelsPCBInfo?
//Accels PCBs detected (sub field of Robot_PCB structure)
//an Ethernet Accelerometer and Analog sensor PCB
//each PCB has 3 accelerometers (#0-2) and 15 analog sensors
//Currently there is one Accels PCB on each leg for a two leg robot
typedef struct Accels_PCB {
//is on the mac 	int PCBNum; //Number of PCB (PCB NameNum) used by Thread_RobotTasks() to match a PCB number to its MAC_Connection
	int AccelsPCBNum; //Number of PCB this PCB is in the RStatus.AccelsPCBInfo[] array of just AccelsPCBs (different from PCBNum which is the index into the RStatus.PCBInfo[] array of all Robot PCBs)
	//accels:
	int NumAccels;  //populated from AccelInfo if PCB is recognized
	unsigned int AccelMask; //mask to enable/disable active accelerometers (ex:=0x5 for accel[0] and accel[2] only)
	Accelerometer Accel[MAX_NUM_ACCELS_ON_ETHACCELS]; //currently each EthAccels PCB can have 3 Accelerometers (#0-2)
	//analog sensors:
	int NumAnalogSensors;  //populated from AccelInfo if PCB is recognized
	unsigned int AnalogSensorMask; //mask to enable/disable active analog (touch or potentiometer) sensors(ex:=0x6 for ts[1] and ts[2] only)
	AnalogSensor Analog[MAX_NUM_ANALOG_SENSORS_ON_ETHACCELS]; //currently each EthAccelTouch PCB can have 16 Analog Sensors (#0-15) (but currently 0 is ignored)
} Accels_PCB;


//Robot PCBs detected (sub field of MAC_Connection)
//todo: possibly move EthMotorsPCBInfoTable and EthAccelsPCBInfoTable into Robot_PCB. Robot PCB may have motor capability, accel capability, etc. 
typedef struct Robot_PCB {
//unsigned int flags;
//unsigned int PCBKind; //see above defines  - note that RStatus.pcb[ROBOT_PCB_ETHPOWER] array index indicates what kind of PCB this is
int Num; //change to PCBNum? number of PCB ***if recognized then is the PCBNameNum from RStatus.EthPCBInfo*** was: (see above enum: ROBOT_PCB_ETHMOTORS_LOWER_BODY, etc.)- can also be taken from RStatus.pcb[] index, but is needed because the RStatus.pcb is memcpy'd to the MAC_Connection and so the index number is lost there if just looking at the MAC_Connection
unsigned char MAC[6]; //MAC address
char Name[MAX_PCB_NAME_LENGTH]; //name of PCB (ex: ROBOT_PCB_ETHACCELS_LEFT_LEG,ROBOT_PCB_ETHMOTORS_LOWER_BODY,...)
char Kind[MAX_PCB_KIND_LENGTH]; //also in the MAC_Connection "MOTOR", "ACCEL", "POWER", "GAMTP", etc.
//EthAccels_PCB EthAccelsPCB;  //specific values for EthAccels PCB (Accelerometer/Gyroscope and Touch Sensor values)
Accels_PCB AccelsPCB;  //specific values for an Accels PCB (Accelerometer/Gyroscope and Touch Sensor values, on an EthAccels or GAMTP)
} Robot_PCB;


//structure for each network (Ethernet/Mac/NIC/IP) connection
//alternative names: IPConnection, NICConnection or NicConnection, or SocketList, because with UDP that is the only thing connecting the source and dest IP
#define ROBOT_MAC_CONNECTION_SOCKET_OPENED	0x00000001 //Send/receive socket to robot has been opened
//#define ROBOT_MAC_CONNECTION_CONNECTED			0x00000002 //network connect() to Mac succeeded
#define ROBOT_MAC_CONNECTION_LISTENING			0x00000002 //Listening for data from Mac
#define ROBOT_MAC_CONNECTION_IS_LOCAL				0x00000004 //This is a local Mac
#define ROBOT_MAC_CONNECTION_IS_WIRED			0x00000008 //so that the wired socket can have IP_MULTICAST_IF
#define ROBOT_MAC_CONNECTION_OPEN_WINDOW		0x00000010 //open a window from the main thread- because it can't be opened from a listening thread- the GetInput fails, the AddWindow function is in MStatus.AddWindowFunc
#define ROBOT_MAC_CONNECTION_IS_CAMERA			0x00000020 //currently this is needed to determine if data recv is from a camera because there are no EthCamera PCBs
#define ROBOT_MAC_CONNECTION_IS_ETHPCB			0x00000040 //the MAC_Connection is connected to a PCB
typedef struct MAC_Connection
{
unsigned int flags;  //Status of Mac Connection
char Name[256];  //Name of the Interface (if local) or the EthPCB (if remote)
//char Location[MAX_PCB_LOCATION_LENGTH];  //location of PCB on Robot (LEG_LEFT,LEG_RIGHT, etc.)
//int Num; //PCB Number (Motor00 is 0, Accel01 is 1, etc.)- probably not used anymore
//EthPCBInfoTable EthPCBInfo; //info about the PCB this MAC Connection is connected to. ***Important: the Robot APp may detect EthPCBs that are not listed in the currently loaded robot's config files, and so get a generic name; not all PCBs may be recognized, so this cannot just be a pointer to the RStatus.EthPCBInfo field.
Robot_PCB pcb; //info about a PCB, if any, that is associated with this connection (note that RStatus.pcb[] is just a reference list for known PCB macs- the connection is the main source for PCB info. Currently this is so generic/unknown EthAccel and EthMotor PCBs can be easily tested using the same Robot software)
CameraInfo cam; //camera info and status
unsigned char PhysicalAddress[6];  //for remote connections this is the destination MAC address
char PhysicalAddressStr[20];
#if Linux
pthread_t ReceiveThread; //thread that listens for data from the robot
int Socket;  //socket that sends/receives data internally to robot PCBs
#endif
#if WIN32
HANDLE ReceiveThread; //thread that listens for data from the robot
SOCKET Socket; //socket that sends/receives data internally to robot PCBs
#endif
struct sockaddr_in SrcAddress; // The host socket address to be passed to bind - almost always the Robot Eth0
struct sockaddr_in DestAddress; //the remote client address to be passed to connect (EthMotor0,EthAccelTouch0, etc.)
char SrcIPAddressStr[INET_ADDRSTRLEN];  //string of source IP address ex:"192.168.0.100"
unsigned int SrcPort;  //Source port# of this mac connection
char DestIPAddressStr[INET_ADDRSTRLEN];  //string of destination IP address
unsigned int DestPort; //Destination port# of this mac connection
//fd_set active_fd_set; //list of active sockets
FTControlfunc *AddWindowFunction; //function to add a window- because they cannot be parented by a listening thread- the GetInput doesn't work
struct MAC_Connection *next;
} MAC_Connection;


#if 0 
//moved to EthAccelsPCBInfo 
//CalibrationFile holds the accel,gyro, and magnetic offsets until from the accel.conf, gyro.conf, and mag.conf files until the PCBs are identified
//at which time, the offsets are copied to each Accelerometer structure on the PCB.
//an array: CalibrationFile CaliFile[NUM_ETHACCEL_PCBS] holds all the offsets, and the function CalibrationFile GetCalibration(PCBNum) is used to check if an entry exists and to return it if so.
typedef struct CalibrationFile {
	int PCBNum; //number of PCB
	float AOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //accelerometer Offset[AccelNum][x,y or z]
	float TOffset[MAX_NUM_TOUCH_SENSORS_ON_ETHACCELS][2]; //touch sensor MinMax[TouchSensorNum][0=Min,1=Max]
	float GOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //gyroscope Offset[AccelNum][x,y or z]
	float MOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //magnetometer Offset[AccelNum][x,y or z]
} CalibrationFile;
#endif

//Structure used to associate a motor with an accelerometer angle- 
//because the Robot needs to know which direction will increase or decrease the angle associated with a motor.
//(used in TurnMotorUntilAngle())
//for example if the foot angle x is -45, the robot has to know to turn motor 0 ccw to increase angle x on the foot (for example back to 0 degrees).
//note that the same Motor can be contained in more than one record, if they are associated with
//more than one accel/joint- like the knee motor- it moves the upper and lower leg
//alt: MotorAccelAngleAssociation, MotorAccelAssociation
#define MOTOR_ANGLE_ASSOCIATION_IGNORE_GRAVITY					0x00000001 //ignore gravity when calculating NeededThrust - ex: LeftSideMotor+LeftUpperLegZ - better to ignore gravity when determining what minimum thrust is needed.
#define NO_THRUST_WHEN_MOVING_AGAINST_GRAVITY_IN_RANGE	0x00000002 //Do not thrust against the motion of a segment when the segment is moving against gravity while in the target range- this is mostly for the FootMotor->LLAccel, KneeMotor->ULAccel, LegMotor->TorsoAccel, HeadMotor->HeadAccel.
typedef struct MotorAngleAssociation {
	uint32_t flags;
	int MotorNameNum; //Name number of motor (all motors in loaded robot have a unique number)
	int AccelNameNum; //Name number of accel (all accels in loaded robot have a unique number)
	int AnalogSensorNameNum; //Name number of analog sensor (potentiometer or touch sensor) (all analog sensors in loaded robot have a unique number)
//	char MotorName[128]; //Name of Motor
//	char AccelName[128]; //Name of Accel
//	int EMPCBNum; //EthMotors PCB number
//	int MotorNum; //number of the motor (on the PCB) associated with the angle
//	int EAPCBNum; //EthAccels PCB number
//	int AccelNum; //number of the accel associated with the motor
	int xyz; //which triordinate of the 3 accelerometer angles x, y, or z (0,1 or 2)
	int IncAngDir; //which direction to turn the motor to increase the associated angle (1=CW,-1=CCW)
	float Mass; //Mass of the segment associated with this accel and motor (=Weight/9.98) (units: grams)
	float Weight; //Weight of the segment associated with this accel and motor (units: gram-m/s^2)
	float MotionThreshold; //Velocity magnitude below which the Segment is viewed as no moving (units: degrees/s)
	int WaitAndSeeDelay; //number of samples to wait once target angle has been reached (or motion has stopped for OPPOSE_MOTION instructions) to keep Thrust=0 and wait to see if there are externally caused motions (motions that are not the result of the associated motor thrust).
	float MinVelocity; //for each motor-accelerometer angle combination, there needs to be a minimum velocity. When moving towards the TargetAngle, the Thrust will not be decreased if the current velocity is lower than MinVelocity. Stops choppy motion/makes motion more fluid- Velocity tends not to reverse until near the TargetAngle. Important to note that MinVelocity is always positive so if comparing to actual Velocity fabs(Velocity) is needed. (units: degrees/s)
	float MaxVelocity; //Thrust will not increase is fabs(Velocity)>MaxVelocity. MaxVelocity is always positive. (units: degrees/s).
	float MaxAccel; //Maximum acceleration for the Associated accelerometer. Always positive.
	float MinAccel; //Minimum acceleration for the Associated accelerometer. Always positive. (not currently used)
//	int BrakeDistance; //How soon to drop MinVelocity in favor of braking to a stop. (units: 10ms, ex: BrakeDistance of 10 is 100ms)
	float ThrustVelocityRatio; //currently for OPPOSE_MOTION (was for EvasiveAction, when HOLD_ANGLE, InRange, IsMoving, and V>VMax then) Thrust is set to Velocity*ThrustVelocityRatio. So, for example, if Vel=20dps, and ThrustVelocityRatio (TVR) is set to 0.7 Thrust will be set to (int)(20*0.7)=14 (or MaxThrust, if MaxThrust is lower) in the opposite direction
//	struct MotorAngleAssociation *next;
} MotorAngleAssociation;

#if 0 
typedef struct MotorPair {
	int EMPCBNum; //EthMotors PCB number
	int MotorNum; //number of the motor
	int OppMotor; //Opposite side motor (left or right body) =-1 if there is none (for example head and neck motors have no left or right)
	int OppMotorDir; //1=OppMotor moves body part in same direction as Motor, -1=OppMotor moves body part in opposite direction as Motor
	struct MotorPair *next;
} MotorPair;
#endif

//todo: remove MotorInfoTable (replaced by MotorInfo within EthMotorsInfoTable)
#define MOTOR_INFO_TABLE_MOTOR_ORIENTATION_CCW  0x00000001  //motor turns CCW to increase associated angle (X, Y, or Z) of lower body segment
#define MOTOR_INFO_TABLE_MAINTAIN_THRUST			  0x00000002  //last instruction that turned this motor had MAINTAIN_THRUST flag set, cleared by next instruction
//#define MOTOR_INFO_TABLE_MOTOR_PAIR_OPPOSITE		0x00000004  //to possibly do, Paired motor turns in opposite direction
typedef struct MotorInfoTable {
	unsigned int flags;
	char Name[MAX_PCB_NAME_LENGTH]; //Name of motor ("MOTOR_LEFT_FOOT", etc.)
	char ReadableName[MAX_PCB_NAME_LENGTH]; //Readable name of motor "Left Foot Motor", etc.)
	int PCBNum; //PCB Number- needed because Motor Numbers are the same on different PCBS (ex: Num=0,1,etc.)
	int Num; //Number of motor port (number of motor on PCB)
	int MaxThrustAG; //Maximum thrust (against gravity) this motor can have (0 to ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE) (magnitude only, no sign). Currently 2/7 is for head, ankle, side and 3/7 is for foot,knees,leg,neck
	int MaxThrustWG; //Maximum thrust (with gravity) (magnitude only) 
	int LastThrust; //currently only used for instruction after instruction that had MAINTAIN_THRUST flag set, to set the initial thrust to the LastThrust. LastThrust is set in TurnMotor().
	float Thrust[ROBOT_MOTORS_DEFAULT_NUM_CLKS_IN_MOTOR_DUTY_CYCLE]; //Thrust (force) associated with each Motor Thrust (units: g-m/s^2 (milliNewtons))
	char Pair[128]; //Name of paired motor (ex: "MOTOR_RIGHT_FOOT" for "MOTOR_LEFT_FOOT") - not complimentary motor on same joint
	char Complement[128]; //Name of complementarily paired motor (ex: "MOTOR_LEFT_FOOT_RIGHT" for "MOTOR_LEFT_FOOT_LEFT") (two motors operating on the same joint)
	//int MotorPairNameNum; //to possibly do
} MotorInfoTable;


//Note: Unlike the Accelerometer struct, AccelInfoTable holds Accel info at program start, even with no EthAccels connected and recognized yet.
#define ACCEL_INFO_TABLE_SWAP_Y_AND_MINUS_Z 			0x00000001 //for accels that are mounted vertically on front, swap Y with -Z
#define ACCEL_INFO_TABLE_SWAP_MINUS_Y_AND_MINUS_Z 0x00000002 //for accels that are mounted vertically on back, swap -Y with -Z
typedef struct AccelInfoTable {
	unsigned int flags;
//int NameNum; //same as index, but is useful when retrieved from EthAccelsPCBInfo struct
	AccelSample	Calib; //User given Accel calibration values for .i[0]=Pitch .i[1]=Yaw .i[2]=Roll (otherwise are all=0.0) units: degrees	//gyro and mag
	char Name[MAX_PCB_NAME_LENGTH]; //Name of accel ("ACCEL_LEFT_FOOT", etc.)
	char ReadableName[MAX_PCB_NAME_LENGTH]; //Readable name of accelerometer "Left Foot Accelerometer", etc.)
	int PCBNum; //Robot PCBNum not EA_PCBNum
	int Num; //number of accel port this accel/gyroscope uses
	uint32_t ModelAxis; //axis number on Robot 3D model that each accel/gyro is associated with 
} AccelInfoTable;

//note AnalogSensorInfoTable flags just use AnalogSensor flags (since these flags are copied to the AnalogSensor struct when an EthAccels PCB is found)
//#define ANALOG_SENSOR_INFO_TABLE_TOUCH_SENSOR  0x00000001 //analog sensor kind is touch sensor
//#define ANALOG_SENSOR_INFO_TABLE_POTENTIOMETER 0x00000002 //analog sensor kind is potentiometer
typedef struct AnalogSensorInfoTable {
	unsigned int flags;
	//touch sensor calibration values?
	char Name[MAX_PCB_NAME_LENGTH]; //Name of analog sensor ("TOUCH_SENSOR_FOOT_LEFT_FRONT", etc.)
	int Num; //number of touch sensor
	//char ReadableName[128]; //Currently there is no Readable Name for analog sensors (potentiometers or touch sensors) Readable name of analog sensor "Left Foot touch sensor", etc.)
	int PCBNum; //Robot PCBNum not EA_PCBNum
  float MinVAngle,MaxVAngle; //Angle of sensor at minimum voltage and at maximum voltage
} AnalogSensorInfoTable;

//MotorsPCBInfoTable: Table of info from EthMotors.conf or GAMTPs.conf about (expected) Motors in Robot
//pulled out of EthMotorsPCBInfoTable so that any PCB with a different name, but the same Motor firmware code can use this structure
typedef struct MotorsPCBInfoTable {
	int NumMotors; //Number of motors in use on this EthMotors PCB
	MotorInfoTable **MotorInfo; //pointer to table of pointers to each MotorInfo structure in the quick lookup RStatus.MotorInfo array of all motors. (is curently mallocd/freed, in order to leave the number of motors on an EthMotors PCB variable)
} MotorsPCBInfoTable;

//AccelsInfoTable: Table of info from EthAccels.conf or GAMTPs.conf about (expected) Accels and Analog Sensors in Robot
//pulled out of EthAccelsPCBInfoTable so that any PCB with a different name, but the same Accel and Analog Sensor firmware code can use this structure
typedef struct AccelsPCBInfoTable {
	char *Name; //pointer to name of Accels PCB ("ETHACCELS_LEFT_LEG", "GAMTP_RIGHT_LEG", etc.) makes name matching easier
	int PCBNum; //PCB Number (makes matching to PCB easier)
	int NumAccels; //Number of accels in use on this EthAccels PCB
	AccelInfoTable **AccelInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to not duplicate AccelInfo, and to leave the number of accels on an EthAccels PCB variable)
	unsigned int AccelMask; //mask to enable/disable active accelerometers (ex:=0x5 for accel[0] and accel[2] only)
	float AOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //accelerometer Offset[AccelNum][x,y or z]
	float GOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //gyroscope Offset[AccelNum][x,y or z]
	float MOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //magnetometer Offset[AccelNum][x,y or z]
	//Analog Sensors:
	int NumAnalogSensors; //Number of touch sensors in use on this EthAccels PCB
	AnalogSensorInfoTable **AnalogSensorInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to leave the number of analog (touch or pot) sensors on an EthAccels PCB variable)
	unsigned int AnalogSensorMask; //mask to enable/disable active analog (touch or pot) sensors (ex:=0x6 for ts[1] and ts[2] only)
//Calibration offsets from the accel.conf, gyro.conf, and mag.conf files. If a PCB is identified
//the offsets are copied to each Accelerometer structure on the detected PCB (EthAccels_PCB).
//***todo: move to AccelInfo and AnalogSensorInfo tables, and link from here?
//note that currently these offsets are in a separate file because the app can automatically determine and write these- so this was done in order to not have to overwrite an entire .conf file, but instead to just overwrite a small calibration file- in particular for gyroscopes which are easy to determine the Vxyz=0 offsets. Touch sensors could be calibrated with help from the user, and the same is true for potentiometers- although I have not visualized this yet.
	float AnOffset[MAX_NUM_ANALOG_SENSORS_ON_ETHACCELS][2]; //analog sensor MinV and MaxV[AnalogSensorNum][0=Min,1=Max]
} AccelsPCBInfoTable;


//EthMotorsPCBInfoTable: Table of info from EthMotors.conf about (expected) EthMotors in Robot
typedef struct EthMotorsPCBInfoTable {
	char Name[MAX_PCB_NAME_LENGTH]; //Name of EthMotors PCB ("ETHMOTORS_LOWER_BODY", etc.)
	//char ReadableName[128]; //Readable name of motor "Lower Body EthMotors PCB", etc.)
	char Kind[MAX_PCB_KIND_LENGTH];
	int PCBNum; //PCB Number
	unsigned char MAC[6]; //MAC address
	MotorsPCBInfoTable motors; //expected motors info from EthMotors.conf file
	//int NumMotors; //Number of motors in use on this EthMotors PCB
	//MotorInfoTable **MotorInfo; //pointer to table of pointers to each MotorInfo structure in the quick lookup RStatus.MotorInfo array of all motors. (is curently mallocd/freed, in order to leave the number of motors on an EthMotors PCB variable)
} EthMotorsPCBInfoTable;

//GAMTPInfoTable: Table of info from GAMTPs.conf about (expected) GAMTP in Robot
typedef struct GAMTPPCBInfoTable {
	char Name[MAX_PCB_NAME_LENGTH]; //Name of GAMTP PCB ("GAMTP_LOWER_BODY", etc.)
	//char ReadableName[128]; //Readable name of motor "Lower Body GAMTP PCB", etc.)
	char Kind[MAX_PCB_KIND_LENGTH];
	int PCBNum; //PCB Number
	unsigned char MAC[6]; //MAC address
	//Motors:	
	MotorsPCBInfoTable motors; //expected motors info from GAMTPs.conf file
	//int NumMotors; //Number of motors in use on this GAMTP PCB
	//MotorInfoTable **MotorInfo; //pointer to table of pointers to each MotorInfo structure in the quick lookup RStatus.MotorInfo array of all motors. (is curently mallocd/freed, in order to leave the number of motors on an EthMotors PCB variable)
//accels:
	//Accels:
	AccelsPCBInfoTable accels; //expected accel and analog sensor info from GAMTPs.conf file
//	int NumAccels; //Number of accels in use on this EthAccels PCB
//	AccelInfoTable **AccelInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to not duplicate AccelInfo, and to leave the number of accels on an EthAccels PCB variable)
//	unsigned int AccelMask; //mask to enable/disable active accelerometers (ex:=0x5 for accel[0] and accel[2] only)
//	float AOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //accelerometer Offset[AccelNum][x,y or z]
//	float GOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //gyroscope Offset[AccelNum][x,y or z]
//	float MOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //magnetometer Offset[AccelNum][x,y or z]
	//Analog Sensors:
//	int NumAnalogSensors; //Number of touch sensors in use on this EthAccels PCB
//	AnalogSensorInfoTable **AnalogSensorInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to leave the number of analog (touch or pot) sensors on an EthAccels PCB variable)
//	unsigned int AnalogSensorMask; //mask to enable/disable active analog (touch or pot) sensors (ex:=0x6 for ts[1] and ts[2] only)
//	float AnOffset[MAX_NUM_ANALOG_SENSORS_ON_ETHACCELS][2]; //analog sensor MinV and MaxV[AnalogSensorNum][0=Min,1=Max]
	
} GAMTPPCBInfoTable;


//EthAccelsInfoTable: Table of info from EthAccels.conf about (expected) EthAccels in Robot
//Maybe change to AccelsPCBInfoTable - includes EthAccels and GAMTP, Name is then Name of Accel group?
typedef struct EthAccelsPCBInfoTable {
	char Name[MAX_PCB_NAME_LENGTH]; //Name of EthAccels PCB ("ETHACCELS_LEFT_LEG", etc.)
	//char ReadableName[128]; //Readable name of motor "Left Leg", etc.)
	int PCBNum; //PCB Number
	unsigned char MAC[6]; //MAC address
	AccelsPCBInfoTable accels; ////expected accel and analog sensor info from GAMTPs.conf file
	//Accels:
//	int NumAccels; //Number of accels in use on this EthAccels PCB
//	AccelInfoTable **AccelInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to not duplicate AccelInfo, and to leave the number of accels on an EthAccels PCB variable)
//	unsigned int AccelMask; //mask to enable/disable active accelerometers (ex:=0x5 for accel[0] and accel[2] only)
//	float AOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //accelerometer Offset[AccelNum][x,y or z]
//	float GOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //gyroscope Offset[AccelNum][x,y or z]
//	float MOffset[MAX_NUM_ACCELS_ON_ETHACCELS][3]; //magnetometer Offset[AccelNum][x,y or z]
	//Analog Sensors:
//	int NumAnalogSensors; //Number of touch sensors in use on this EthAccels PCB
//	AnalogSensorInfoTable **AnalogSensorInfo; //pointer to table of pointers into quick lookup RStatus.AccelInfo table. (is curently mallocd/freed, in order to leave the number of analog (touch or pot) sensors on an EthAccels PCB variable)
//	unsigned int AnalogSensorMask; //mask to enable/disable active analog (touch or pot) sensors (ex:=0x6 for ts[1] and ts[2] only)
//Calibration offsets from the accel.conf, gyro.conf, and mag.conf files. If a PCB is identified
//the offsets are copied to each Accelerometer structure on the detected PCB (EthAccels_PCB).
//***todo: move to AccelInfo and AnalogSensorInfo tables, and link from here?
//note that currently these offsets are in a separate file because the app can automatically determine and write these- so this was done in order to not have to overwrite an entire .conf file, but instead to just overwrite a small calibration file- in particular for gyroscopes which are easy to determine the Vxyz=0 offsets. Touch sensors could be calibrated with help from the user, and the same is true for potentiometers- although I have not visualized this yet.
//	float AnOffset[MAX_NUM_ANALOG_SENSORS_ON_ETHACCELS][2]; //analog sensor MinV and MaxV[AnalogSensorNum][0=Min,1=Max]
} EthAccelsPCBInfoTable;

//EthPowerInfoTable: Table of info from EthPower.conf about (expected) EthPower PCBs in Robot
typedef struct EthPowerPCBInfoTable {
	char Name[MAX_PCB_NAME_LENGTH]; //Name of EthAccels PCB ("ETHACCELS_LEFT_LEG", etc.)
	//char ReadableName[128]; //Readable name of motor "Left Leg", etc.)
	int PCBNum; //PCB Number
	unsigned char MAC[6]; //MAC address
} EthPowerPCBInfoTable;

//Robot "Eth" PCBs expected in currently loaded robot
typedef struct EthPCBInfoTable {
//int NameNum; //NameNum of PCB = not needed, is the same as the index
//unsigned char MAC[6]; //MAC address
unsigned char *MAC; //points to MAC address of PCB stored in EthMotorsPCB or EthAccelsPCB structure 
char *Name; //points to Name of PCB stored in EthMotorsPCB or EthAccelsPCB structure
//char *Name[MAX_PCB_NAME_LENGTH]; //name of PCB (ex: ROBOT_PCB_ETHACCELS_LEFT_LEG,ROBOT_PCB_ETHMOTORS_LOWER_BODY,...)
char Kind[MAX_PCB_KIND_LENGTH]; //"Motor", "Accel", "Power" etc.
} EthPCBInfoTable;


#if USE_RT_PREEMPT
typedef struct TurnMotorQueue {
	unsigned char inst[5];  //5 byte TurnMotor instruction
	MAC_Connection *mac;
} TurnMotorQueue;
#endif //USE_RT_PREEMPT

typedef struct ModelConfTableEntry {
	int Row;
	int Column;
	char Name[MAX_PCB_NAME_LENGTH];  //for touch sensors can include a | to list multiple touch sensors
} ModelConfTableEntry;


typedef struct ModelConfTable {
	int NumRows;
	int NumColumns;
	int ColumnWidth;
	int NumEntries;
	ModelConfTableEntry *Entry;  //needs to be malloc'd and free'd
} ModelConfTable;

#define ROBOT_FALLING													0x00000001  //Robot is falling
#define ROBOT_FALLING_FORWARD									0x00000002	//Robot is falling forward
#define ROBOT_FALLING_BACKWARD								0x00000004	//Robot is falling backward
#define ROBOT_FALLING_LEFT										0x00000008	//Robot is falling left
#define ROBOT_FALLING_RIGHT										0x00000010	//Robot is falling right
#define ROBOT_FALLING_LEFT_RIGHT_START				0x00000020  //first indication of robot left-right fall
#define ROBOT_FALLING_FORWARD_BACKWARD_START	0x00000040  //first indication of robot forward-backward fall
#define ROBOT_FALL_LEFT_RIGHT_REVERSED				0x00000080	//Fall left or right was reversed as a result of thrusting
#define ROBOT_FALL_FORWARD_BACKWARD_REVERSED	0x00000100	//Fall forward or backward was reversed as a result of thrusting
#define ROBOT_FALL_LEFT_RIGHT_SLOWED					0x00000200	//Fall left or right was slowed as a result of thrusting
#define ROBOT_FALL_FORWARD_BACKWARD_SLOWED		0x00000400	//Fall forward or backward was slowed as a result of thrusting
#define ROBOT_LEFT_FOOT_ON_GROUND							0x00000800	//Left foot is on ground (as revealed by heal and toe touch sensors)
#define ROBOT_RIGHT_FOOT_ON_GROUND						0x00001000	//Right foot is on ground (as revealed by heal and toe touch sensors)
typedef struct RobotBalance {
	uint32_t flags;
	uint32_t TimeStamp; //timestamp (in ms) that balance was last checked- used to make sure that the accel sample being analyzed is a new sample
//	int32_t Thrust[NUM_MOTOR_NAMES]; //the current thrust applied to each motor from the balancing code (starts at 1 and increases if expected result is not achieved, or decreases if expected result is achieved)
	float AngleBeforeFall[3];  //last angle before fall
	float AngleBeforeFallLeft[3];  //needed in case one foot is off the ground
	float AngleBeforeFallRight[3];  //needed in case one foot is off the ground
	float TotalAngleChange[3];  //total change in angle since fall started
	uint32_t BalanceCountMS;  //number of ms that Robot joint has not sensed a fall
} RobotBalance;


#define ROBOT_MAX_SND_POINTERS 100 //100 pointers into the SND buffer
#define ROBOT_MAX_SND_BUFFER 960000//48000 x 2 =1sec 25KB=10 seconds of WAV sound data
//#define ROBOT_SOUNDSIZE 100000//65536//16384//8096//65536//9600//131072//65536//16384//8192//96000 //5512  //buffer size of individual reads = 1 second of 48khz 2 channel wav data
#define ROBOT_DEFAULT_SOUND_BUFFER_SIZE 100000  //bytes to read or write from/to sound card

#define ROBOT_SNDTYPE_RAW 1
#define ROBOT_SNDTYPE_WAV 2
#define ROBOT_SNDTYPE_MP3 3


//Robot network constants
//todo: eventually port numbers used should be up to the OS kernel
#define DEFAULT_ROBOT_PORT 53510 //27015  //is destination network port
#define DEFAULT_HOST_PORT 53510 //27015  //is local network port
#define DEFAULT_OUTSIDE_PORT 53510 //is port used on connections to outside

#define ROBOT_MAXPENDING_CONNECTIONS 10 //10 connections can be queued  

//SNDbits (SB) bits
#define ROBOT_SB_READ_SND					0x00000001 //SNDdata needs to be read or is reading
#define ROBOT_SB_PLAY_SND					0x00000002 //SNDdata needs to be played or is playing
#define ROBOT_SB_SAROBOT_SND					0x00000004 //SNDdata needs to be saved or is saving
#define ROBOT_SB_EXPORT_SND				0x00000008 //SNDdata needs to be exported or is exporting
#define ROBOT_SB_DRAW_SND_TO_TRACK	0x00000010 //SNDdata needs to be drawn or is drawing to track
#define ROBOT_SB_USE_SND2					0x00000020 //use SND2 buffer
#define ROBOT_SB_PROCESSED					0x00000040 //SND buffer was processed - do not process again

//Robot Status flags
/* commented for now- probably the implementation of sound will change
#define ROBOT_STATUS_SOUND_OPEN			0x00000001 //Sound is initialized and open
#define ROBOT_STATUS_SND_COMPRESS					0x00000002  
#define ROBOT_STATUS_READ_SND							0x00000004  //Read sound from sound input
#define ROBOT_STATUS_PLAY_SND							0x00000008  //play sound data in SNDdata buffer
#define ROBOT_STATUS_PLAY_TRACK_SND				0x00000010  //play sound data on tracks
#define ROBOT_STATUS_SAROBOT_SND							0x00000020
#define ROBOT_STATUS_STOPSAROBOT_SND					0x00000040
#define ROBOT_STATUS_DRAW_SND_TO_TRACK		0x00000080
#define ROBOT_STATUS_EXPORT_TRACK_SND			0x00000100  //export track data to file
#define ROBOT_STATUS_USE_SND2							0x00000200
#define ROBOT_STATUS_SND_PROCESS					0x00000400 //there is sound data to process (to save, play, send, etc)
#define ROBOT_STATUS_SND_FILEOPEN					0x00000800  
#define ROBOT_STATUS_SND2_FILEOPEN				0x00001000
*/
/*
#define ROBOT_PROCESS_USB					0x00002000 //process USB events thread is active
#define ROBOT_USB_OPEN							0x00004000
#define ROBOT_READ_USB							0x00008000
#define ROBOT_USB_OPEN_DEVICE			0x00010000  //connected with USB device

#define ROBOT_INSIDE_SOCKET_OPENED	0x00020000  //Send/receive socket to robot has been opened
#define ROBOT_INSIDE_CONNECTED			0x00040000	//network connect() to robot succeeded
#define ROBOT_LISTENING_TO_INSIDE	0x00080000  //Listening for data from robot
#define ROBOT_OUTSIDE_SOCKET_OPENED	0x00100000  //Send/receive socket to outside has been opened
#define ROBOT_OUTSIDE_CONNECTED			0x00200000	//network connect() to outside succeeded
#define ROBOT_LISTENING_TO_OUTSIDE		0x00400000	//Listening for data from outside (for example from wireless connection to outside)
*/

#include "win_Tasks.h"  //for RobotTask iTask

#define NUM_SINE_TABLE_ENTRIES 4096 //1000 //number of entries in the sine lookup table

#define ROBOT_STATUS_NETWORK_LOG				0x00000001	//log all network data to logs/Robot_Net_{timestamp}.log
#define ROBOT_STATUS_MODEL_LOG					0x00000002	//log all model data (accel and motor data) to logs/Robot_Model_{timestamp}.log
#define ROBOT_STATUS_LOG_INCOMING_DATA			0x00000004  //log all incoming data to Robot.log (config file determines where)
#define ROBOT_STATUS_LOG_OUTGOING_DATA			0x00000008  //log all outgoing data to Robot.log
#define ROBOT_STATUS_INFO						0x00000010  //provide more info while running: print sent/recvd data
#define ROBOT_STATUS_REMOTE						0x00000020	//this program is controlling a robot remotely. The instruction is sent to the Robot wireless interface with an added 4-byte IP address in front of the instruction.
#define ROBOT_STATUS_USE_WIRELESS_INTERFACE		0x00000040	//use wireless interface to send instructions to remote (not wired interface)
#define ROBOT_STATUS_INIT_FREE3D				0x00000080  //Free3D struct was initialized	
#define ROBOT_STATUS_LISTEN_TO_ROBOT			0x00000100 //so Thread_ListenToRobot knows that the MAC_Connection is done being initialized
#define ROBOT_STATUS_LISTEN_TO_OUTSIDE			0x00000200 //so Thread_ListenToOutside knows that the MAC_Connection is done being initialized
#define ROBOT_STATUS_SHOW_ACCEL_DATA			0x00000400 	//Show accel data in the Accel window
#define ROBOT_STATUS_SHOW_ANALOG_SENSOR_DATA		0x00000800  //Show touch sensor data in the Touch Sensor window
#define ROBOT_STATUS_USE_CALIBRATION			0x00001000  //add calibration offsets on samples from accelerometers, gyroscopes, and magnetometers
#define ROBOT_STATUS_CALIBRATING					0x00002000	//during calibration do not round sapmles to +-1.000
#define ROBOT_STATUS_TASK_THREAD_RUNNING					0x00004000	//task thread is running- so thread can be exited cleanly
#define ROBOT_STATUS_TURN_MOTOR_THREAD_RUNNING		0x00008000 //TurnMotor thead is running (USE_RT_PREEMPT only)
#define ROBOT_STATUS_AUTOCALIBRATE_THREAD_RUNNING	0x00010000 //autocalibrating accel and gyro thread running
#define ROBOT_STATUS_MOTORS_DISABLED							0x00020000	//Motors are disabled- TurnMotor commands are just ignored
#define ROBOT_STATUS_ACCELEROMETERS_ONLY					0x00040000  //only use accelerometer data, not gyroscope data, used to calibrate accelerometers
#define ROBOT_STATUS_USER_ROBOT_LOADED						0x00080000  //a user robot is loaded
//#define ROBOT_STATUS_APPLY_ACCELEROMETERS_TO_MODEL	0x00000100 //apply the accelerometer data to the robot 3D model
//#define ROBOT_CAMERA_ENABLED  //slows listen thread if enabled
//#define ROBOT_STATUS_GET_IMAGE 0x00000800 //temp: get a single image
//#define ROBOT_STATUS_GET_VIDEO 0x00001000 //temp: get a stream of images (video)

//global structure for Robot Program
typedef struct 
{
unsigned int flags;  //Robot Program Status bits/flags

//network variables
//todo: probably MAC_Connection should be changed to MAC_IP, MAC_NIC, or NetworkMAC, and drop SourceAddress, DestAddress and replace with just a single Address, Source should be determined at the time of send (locally: if to robot, wired, if to outside, wireless, and remotely: to either wired or wireless)
MAC_Connection *iMAC_Connection;  //first pointer to a linked list of connections to a variety of network cards (macs)
MAC_Connection *WiredInterface;  //quick pointer to Wired Interface info (IP, etc.) for sending data
MAC_Connection *WirelessInterface; //quick pointer to Wireless Interface info (IP, etc.) for sending data
MAC_Connection *NewMac; //temporary holder for a new Eth PCB so New Eth PCB window can fill in MAC address
char WiredIPAddressStr[INET_ADDRSTRLEN];  //quick pointer to Wired interface IP address ex:"192.168.0.100"
char WirelessIPAddressStr[INET_ADDRSTRLEN];  //quick pointer to Wireless interface IP address

#if 0 
#if Linux
pthread_t RobotReceiveThread; //thread that listens for data from the robot
int RobotSocket;  //socket that sends/receives data internally to robot PCBs
pthread_t OutsideReceiveThread; //thread that listens for data from the outside
int OutsideSocket; //socket that sends/receives data from outside sources
#endif
#if WIN32
HANDLE RobotReceiveThread; //thread that listens for data from the robot
SOCKET RobotSocket; //socket that sends/receives data internally to robot PCBs
HANDLE OutsideReceiveThread; //thread that listens for data from the outside
SOCKET OutsideSocket; //socket that sends/receives data from outside sources
#endif

//Robot internal connection - usually eth0
struct sockaddr_in RobotSrcAddress; // The host socket address to be passed to bind
struct sockaddr_in RobotDestAddress; //the remote client address to be passed to connect
char RobotSrcIPAddressStr[20];
unsigned int RobotSrcPort;
char RobotDestIPAddressStr[20];
unsigned int RobotDestPort;

//Robot connection to outside - usually wlan0
struct sockaddr_in OutsideSrcAddress; // The host socket address to be passed to bind
struct sockaddr_in OutsideDestAddress; //the remote client address to be passed to connect
char OutsideSrcIPAddressStr[20];
unsigned int OutsideSrcPort;
char OutsideDestIPAddressStr[20];
unsigned int OutsideDestPort;
#endif

//motor settings
unsigned int MotorDutyCycleClock; //duration of motor duty cycle timer interrupt in microseconds, default = 1000us
unsigned int NumClksInMotorDutyCycle; //the number of timer interrupt clocks in a motor duty cycle, default=14 (total duty cycle length=14ms)
char LogFolder[1024]; //full path to folder where all logs are written
char ModelLogFolder[1024]; //full path to folder where all Model logs are written
char ScriptFolder[1024]; //full path to folder where all scripts are stored
char RobotsFolder[1024]; //full path to folder where Robots (Robot folders) are
FILE *fRobotNetworkLog; //pointer Robot network log file
char CurrentRobotName[512]; //Currently loaded robot name
char RobotNetworkLogFileName[512]; //currently doesn't include path- files are currently in ./logs
FILE *fRobotModelLog; //pointer Robot model log file
char RobotModelLogFileName[512]; //currently doesn't include path- files are currently in ./logs
char DefaultRobot[512]; //Name of Robot (folder) that is loaded when app is opened (can be blank)
//moved to win_RobotModel.h FILE *fRobotLoadedLog; //pointer to loaded Robot log file (used to play back earlier robot movements on the animated 3D model)
//unsigned int LogFilePos; //current index in bytes into loaded log file
//uint64_t PlaybackTime; //current playback time for playing back a log file (in ms)
int DeleteLogsOlderThanDays; //delete all logs older than this many days (specified in Robot.conf)
//Currently Loaded Robot variables:
char CurrentRobotFolder[1024]; //full path to currently loaded Robot home folder
char CurrentRobotLogFolder[1024]; //full path to currently loaded Robot log folder
char CurrentRobotModelLogFolder[1024]; //full path to currently loaded Robot Model log folder
char CurrentRobotModelFile[512]; //file name of model
char CurrentRobotScriptFolder[1024]; //full path to folder where currently loaded Robot Model scripts are located 
char CurrentRobotFutureLogFolder[1024]; //full path to currently loaded Robot Future log folder
//int CurrentDeleteLogsOlderThanDays; //delete all logs older than this many days (specified in Robot.conf)
char RobotReadableName[512];  //Readable name (ex: "Two Leg Robot") of currently loaded robot
int NumEthPCBsExpected; //total number of PCBs expected in the currently loaded robot
//int NumPCBsDetected; //total number of PCBs detected by the Robot App listening thread
int NumEthMotorsPCBsExpected; //Number of EthMotors PCBs in robot (expected to be found/detected) as given by user in EthMotors.conf
int NumEthAccelsPCBsExpected; //total number of EthAccels PCBs expected in the currently loaded robot
int NumAccelsPCBsExpected; //total number of Accels PCBs expected in the currently loaded robot (includes both EthAccels and GAMTP)
int NumEthPowerPCBsExpected; //total number of EthPower PCBs expected in the currently loaded robot
int NumGAMTPPCBsExpected; //total number of GAMTP PCBs expected in the currently loaded robot
int NumMotorsExpected; //Number of motors in robot (expected to be found/detected) as given by user in EthMotors.conf
int NumAccelsExpected; //total number of accels expected in the currently loaded robot
int NumAnalogSensorsExpected; //total number of analog (touch or potentiometer) sensors expected in the currently loaded robot
int NumEthAccelsPCBs; //number of EthAccels PCBs found
int NumAccelsPCBs; //number of Accels PCBs found (includes both EthAccels and GAMTP PCBs)
//Robot_PCB pcb[NUM_ROBOT_PCB_NAMES]; //associates each pcb with where they are located on the robot (left leg, etc.) - 
int NumMotorAngleAssociations; //the number of MotorAngleAssociations used by the currently loaded robot
EthPCBInfoTable *EthPCBInfo; //Information about all robot EthPCBs expected in currently loaded robot (ex: PCB Name).
EthMotorsPCBInfoTable *EthMotorsPCBInfo; //Table with info loaded from EthMotors.conf file
GAMTPPCBInfoTable *GAMTPPCBInfo; //Table with info loaded from GAMTPs.conf file
//todo: delete below MotorInfo - will be in EthMotorsInfo
MotorInfoTable *MotorInfo;  //Quick lookup table to all motors in robot. Index is MotorNameNum (each motor has a unique number). 
//MotorInfoTable MotorInfo[NUM_MOTOR_NAMES];  //Table to link hardcoded motor name to variable motor (port) number- the motor number the ETHMotors uses may change (ex: one motor port fails, and the motor is moved to a spare). This way the name and code associated with the name can stay the same if the motor port number changes. 
//EthAccels_PCB *EthAccelsPCB; //linked list, stores all information relating to each EthAccelTouch PCB detected 
//EthMotors_PCB *EthMotorsPCB; //linked list, stores all information relating to each EthMotors PCB detected
EthAccelsPCBInfoTable *EthAccelsPCBInfo; //pointer to table with info loaded from EthAccels.conf file (All EthAccels PCBs in robot, expected to detect)
AccelsPCBInfoTable **AccelsPCBInfo;  //array of pointers to each AccelsPCBInfo table loaded from EthAccels.conf or GAMTP.conf file (All AccelsPCBs in robot, expected to detect)
AccelInfoTable *AccelInfo;  //Quick lookup table/list of all accelerometers/gyroscopes (expected to be in robot)
//AccelInfoTable AccelInfo[NUM_ACCEL_NAMES]; //Table to link hardcoded accel name to various accel settings (currently only MaxSpeed)
AnalogSensorInfoTable *AnalogSensorInfo; //Quick lookup table/list of all analog (touch or potentiometer) sensors (expected to be in robot)
EthPowerPCBInfoTable *EthPowerPCBInfo; //Table with info loaded from EthPower.conf file (All EthPower in robot, expected to detect)
//int NumCalibSamples; //number of calirbation samples to use for calibration, default=300 (3 seconds)	(in addition code captures an extra 100 samples initially)
MotorAngleAssociation *MotorAngle; //pointer to list of associated motors and accelerometer angles
//MotorPair *iMotorPair; //linked list of motors and their opposite (right or left matching motor, ex: left foot motor and right foot motor)
ModelConfTable ModelConf;
//Balance settings
#if Linux
pthread_t BalanceThread; //thread that balances robot
#endif
#if WIN32
HANDLE BalanceThread; //thread that balances robot
#endif

//Robot Tasks settings
#if Linux
pthread_t RobotTasksThread; //thread that processes all robot tasks (run scripts, balance)
#endif
#if WIN32
HANDLE RobotTasksThread; //thread that processes all robot tasks (run scripts, balance)
#endif

//Turn Motor thread
#if Linux
pthread_t TurnMotorThread; //thread that runs one TurnMotor inst every 100us
#endif
#if WIN32
//HANDLE TurnMotorThread; //thread that runs one TurnMotor inst every 100us
#endif


//autocalibrating accels and gyros thread
#if Linux
pthread_t AutoCalibrateAccelGyroThread; //thread that processes autocalibrating accels and gyros
#endif
#if WIN32
HANDLE AutoCalibrateAccelGyroThread; //thread that processes autocalibrating accels and gyros
#endif


//CameraInfo CamInfo[NUM_CAMERAS]; //camera info and status
//temporary - this needs to move to the MAC_Connection camera

RobotScript *iScript; //linked list of loaded robot scripts
RobotTask *iTask; //linked list of active Robot tasks (can be scripts that are running, or to balance)
RobotBalance iRobotBalance; //structure that has all the data needed for balancing module/thread
#if USE_RT_PREEMPT
unsigned int TurnMotorStart;
unsigned int TurnMotorEnd;
TurnMotorQueue TurnMotorInst[TURN_MOTOR_NUM_INSTS];
#endif
float SineTable[NUM_SINE_TABLE_ENTRIES];




//FTWindow *ButtonWin[4];  //window that has FTControl button 1,2,3 is being held down on is
int TimeSpace;   ///space between time unit lines (tcontrol->TimeScale is time units/pixel, TimeSpace is how many pixels).
long double CursorStartTime;  //currently from btnMarkIn
long double CursorEndTime; 
time_t ttime;  //stores time (for ctime)
struct tm *ctime;  //current time 10:14:32  should be pointer? 
//ctime->tm_year=100 (2000) ctime->tm_hour=6;  ctime->tm_min=0;  ctime->tm_sec=0;
//int secs; //for now only store seconds for expose
//time_t ttime2;  //stores time (for lastexpose)
//struct tm *lastexpose;  //redraw on expose only after 1 second
char SourceFolder[FTMedStr]; //location of source
char VideoDevice[FTMedStr];
char SpeakerDevice[FTMedStr];
char MicrophoneDevice[FTMedStr];
char RecordingSource[FTMedStr]; //"Audio", or "USB"
char HomeFolder[FTMedStr]; //location of $HOME/.Robot
char TempFolder[FTMedStr]; //location of $HOME/.Robot/temp
struct timeval utime;   //current time seconds and microseconds .tv_usec
struct timeval timertime; //last time timer check ran - runs every 1ms
long long timescale;  //for data FTControls. (in ns)
//Sound variables:
char SoundMethod[FTMedStr]; //Sound method="ALSA", "Pulse"
//Record and Play DINET4_ADDRSTRLENata parameters are currently the same (project settings), but may later separate into 2 sets of variables
int SamplesPerSecond; 
int BitsPerSample;
int BytesPerSample;
int BitsPerChannel;
int Channels;
int Signed; //0=no 1=yes, data is signed
int RecordToTrack; //0=no,1=yes
int RecordToFile; //0=no,1=yes
char RecordToFileName[FTMedStr]; //Filename to record to if recording directly to file
char RecordToTempFileName[FTMedStr]; //temporary Filename to record to
char SoundCompression[FTMedStr]; //Sound compression= "No compression", "MP2 compression", "MP3 compression"
int DataFormat; //ROBOT_SNDTYPE_RAW, ROBOT_SNDTYPE_WAV, ROBOT_SNDTYPE_MP3
//Import Data parameters
char ImportSource[FTMedStr]; //ImportSource
int ImportDataFormat; //ROBOT_SNDTYPE_RAW, ROBOT_SNDTYPE_WAV, ROBOT_SNDTYPE_MP3
int ImportSamplesPerSecond; //RecordSamplesPerSecond, ImportSamplesPerSecond,ExportSamplesPerSecond
int ImportBitsPerSample;
int ImportBytesPerSample;
int ImportBitsPerTrack;
int ImportChannels;
int ImportSigned; //0=no 1=yes, data is signed
char ImportFromFileName[FTMedStr]; //Filename to record to if recording directly to file
char ImportFromTempFileName[FTMedStr]; //temporary Filename to record to
int ImportDataToTraINET4_ADDRSTRLENck; //0=no 1=yes (no=only import to video window)
int ImportDataToImage; //0=no,1=yes (draws data as image in window-important for SETI work)
int ImportWidth; //width of image
int ImportHeight; //height of image
//Export Data parameters
int ExportSamplesPerSecond; //RecordSamplesPerSecond, ImportSamplesPerSecond,ExportSamplesPerSecond
int ExportBitsPerSample;
int ExportBytesPerSample;
int ExportBitsPerChannel;
int ExportChannels;
int ExportSigned; //0=no 1=yes, data is signed
char ExportToFileName[FTMedStr]; //Filename to export track data to
//char ExportToTempFileName[FTMedStr]; //temporary Filename to record to
int ExportSelected; //0=export all 1=export selected
char ExportSoundCompression[FTMedStr]; //Sound compression= "No compression", "MP2 compression", "MP3 compression"
int ExportDataFormat; //ROBOT_SNDTYPE_RAW, ROBOT_SNDTYPE_WAV, ROBOT_SNDTYPE_MP3
int NumSamples;
int SoundBufferSize; //possibly: RecordSoundBufferSize, PlaySoundBufferSize
int SpeakerFile;
int MicrophoneFile; 
//int PlayRecordedAudio; //0=no, 1=yes - it should not be necessary to play recorded audio while recording - simply set mixer to correct volume
char ALSASpeakerDevice[FTMedStr];
char ALSAMicrophoneDevice[FTMedStr];
FILE *AudioFile,*AudioFile2;
#if Linux
#if USE_SOUND
snd_pcm_t *hplay;//device handle
snd_pcm_t *hrec;//device handle
snd_pcm_hw_params_t *hw_params; //sound device hardware parameters
snd_pcm_stream_t alsa_play; //ALSA play stream
snd_pcm_stream_t alsa_rec; //ALSA record stream
//struct pollfd *ufds;  //for polling writing sound out - otherwise sound is choppy
//int count; //for polling
pa_simple *pas_play; //pulseaudio connection to server - play stream
pa_simple *pas_rec; //pulseaudio connection to server - record stream
pa_sample_spec pass; //pulseaudio sound specifications
#endif  //USE_SOUND
#endif //Linux

#if Linux
pthread_mutex_t ROBOT_ReadSound_Lock; //mutex variable to indicate RecordSound thread is running
pthread_cond_t ROBOT_ReadSound_Cond; //conditional variable which indicates that recording is done
pthread_mutex_t ROBOT_SoundAccess_Lock; //mutex variable to indicate read or write of /dev/dsp device
//pthread_mutex_t ROBOT_startSND_Lock; //lock startSND variable to increment it
//pthread_cond_t ROBOT_startSND_Cond; //conditional variable which indicates that recording is done
pthread_mutex_t ROBOT_curSND_Lock; //lock curSND variable to increment it

pthread_mutex_t ROBOT_PlaySound_Lock;//indicates SND data needs to be played
pthread_cond_t ROBOT_PlaySound_Cond; //conditional variable which indicates that playing is done

pthread_mutex_t ROBOT_PlayTrackSound_Lock;//indicates Track data needs to be played
pthread_cond_t ROBOT_PlayTrackSound_Cond; //conditional variable which indicates that playing Track data is done

pthread_mutex_t ROBOT_ProcessData_SND_Lock; //indicates SND data needs to be processed
pthread_cond_t ROBOT_ProcessData_SND_Cond; //conditional variable that indicates that there is SND data that needs to be processed
/*
pthread_mutex_t ROBOT_Process_USB_Lock; //indicates USB data needs to be processed
pthread_cond_t ROBOT_Proc#include <sched.h>ess_USB_Cond; //conditional variable that indicates that there is USB data to process 
*/
pthread_mutex_t ROBOT_CurrentStatus_USB_Lock; //to change the status of USB data
#endif

//SND DATA BUFFER
unsigned int SNDbits[ROBOT_MAX_SND_POINTERS]; //Bits describing how to process this data: Draw/Play, Save, Send, etc - uses same bits as VStatus.flags - but perhaps should use its own perhaps ROBOT_SNDbits_PLAY_SND, etc
unsigned char SNDdata[ROBOT_MAX_SND_BUFFER]; //(uncompressed) SND data buffer (WAV)
int SNDptr[ROBOT_MAX_SND_POINTERS]; //pointers into SND data buffer 
int SNDlen[ROBOT_MAX_SND_POINTERS]; //length of SND data this pointer contains
int SNDtype[ROBOT_MAX_SND_POINTERS]; //data subtype (pktWAV, pktMP3, etc) - this way one ProcessData_SND thread can handle all types
int startSND;  //first (earliest) pointer into uncompressed SND data buffer
int curSND;  //current (latest) pointer into uncompressed SND data buffer
//SND2 DATA BUFFER (Only used to store compressed or decompressed SND data and is not processed directly)
unsigned char SND2data[ROBOT_MAX_SND_BUFFER]; //(uncompressed) SND data buffer (WAV)
int SND2ptr[ROBOT_MAX_SND_POINTERS]; //pointers into wav data buffer 
int SND2len[ROBOT_MAX_SND_POINTERS]; //length of SND2 data this pointer contains
int SND2type[ROBOT_MAX_SND_POINTERS]; //data subtype (pktWAV, pktMP3, etc) - this way one ProcessData_SND 
unsigned char *data;//[ROBOT_SOUNDSIZE]; //used to store mixed SNDdata for final output to audio out
//unsigned char *data; //used as a pointer to mixed SNDdata for final output to audio out
//FFMPEG variables:
#if Linux
#if USE_FFMPEG
AVCodec *avc_enc;  //format to encode video call audio - currently set to MP3
AVCodecContext *avcc_enc; //audio encoding codec context 
AVOutputFormat *avof; //
AVFormatContext *avfc;  /#include <sched.h>/
AVCodec *avc_dec;  //format to encode video call audio - currently set to MP3
AVCodecContext *avcc_dec; //audio decoding codec context 
AVPacket avpkt_dec; //packet for decoding mp3 audio to wav audio
int BitRate; //bitrate of mp3 compression (bits per second)
#endif  //USE_FFMPEG
#endif //Linux


} RobotStatus;


int InitRobot(void);
int InitSineTable(void);
float sinlu(float Angle);
float coslu(float Angle);
int CloseRobot(void);
int StopAllRobotThreads(void);
int DeleteOldLogs(int NumDays,char *RobotFolder);
int LoadCalibrationFileAccel(char *filename);
int LoadCalibrationFileTouch(char *filename);
int LoadCalibrationFileGyro(char *filename);
int LoadCalibrationFileMag(char *filename);
int LoadCalibrationFile(char *filename,int ATGM);
int LoadRobot(char *RobotPath,char *RobotName);
int UnloadRobot(void);
int LoadRobotAppConfigFile(void);
int SaveRobotAppConfigFile(void);
int Robot_GetLineFromText(char *tline, char *buf, int len);
int Robot_GetLineBackwardsFromText(char *tline,char *buf,int len);
int AssociateMotorAndAngle(MotorAngleAssociation *MAAssociation);
//int AssociateMotorAndAngle(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz,int IncAngDir,float Weight);
//MotorAngleAssociation *GetMotorAngleAssociation(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz);
//MotorAngleAssociation *GetMotorAngleAssociation(int MotorNameNum,int AccelNameNum,int xyz);
MotorAngleAssociation *GetMotorAngleAssociation(RobotInstruction *linst, int MotorNum, int AngleNum); 
int GetIncAngDir(int MotorNameNum,int AccelNameNum,int xyz);
//int GetIncAngDir(int EMPCBNum,int MotorNum,int EAPCBNum,int AccelNum,int xyz); 
//int FreeMotorAngleAssociationList(void);
//int AddMotorPair(int EMPCBNum,int MotorNum,int OppMotor,int OppMotorDir);
//MotorPair *GetMotorPair(int EMPCBNum,int MotorNum); 
//int FreeMotorPairList(void);
int Open_MAC_Socket(MAC_Connection *lmac);
int Close_MAC_Socket(MAC_Connection *lmac);
//int ConnectToRemoteMAC(MAC_Connection *lmac);
//MAC_Connection *Create_Remote_MAC_Connection(char *SourceIP,char *DestIP);
MAC_Connection *Create_Remote_MAC_Connection(char *SourceIP, int SrcPort, char *DestIP, int DestPort);
int DisconnectFromRemoteMAC(MAC_Connection *lmac);
int Add_MAC_Connection(MAC_Connection *lmac);
MAC_Connection *Get_MAC_Connection(char *ip);
MAC_Connection *Get_MAC_Connection_By_WindowFunction(FTControlfunc *winFunction);
//MAC_Connection *Get_MAC_Connection_By_IPs(char *SrcIP,char *DestIP);
MAC_Connection *Get_MAC_Connection_By_IPs(char *SrcIP, int SrcPort, char *DestIP, int DestPort);
//MAC_Connection *Get_MAC_Connection_By_DestIP(char *DestIP,int DestPort);
MAC_Connection *Get_MAC_Connection_By_PCBName(char *PCBName);
MAC_Connection *Get_MAC_Connection_By_PCBNum(int num);
MAC_Connection *Get_MAC_Connection_By_Name(char *Name);
//MAC_Connection *Get_MAC_Connection_By_Dest_MAC(char *MACStr);
//MAC_Connection *Get_MAC_Connection_By_Camera_IP(char *CamIP);
//EthAccels_PCB *Get_EthAccelsPCB(MAC_Connection *lmac);
//EthAccels_PCB *Get_EthAccelsPCB_By_PCBName(char *PCBName);
//EthAccels_PCB *Get_EthAccelsPCB_By_PCBNum(int num);
//int Get_EthAccelsPCBNum_By_PCBName(char *PCBName);
Accels_PCB *Get_AccelsPCB_By_PCBName(char *PCBName);
Accels_PCB *Get_AccelsPCB_By_PCBNum(int num);
int Get_AccelsPCBNum_By_PCBName(char *PCBName);
int Delete_MAC_Connection(MAC_Connection *lmac);
int Delete_All_MAC_Connections(void);
int SendDataToMAC(MAC_Connection *lmac, unsigned char *Inst,int numbyte);
int SendBroadcastInstruction(void);
MAC_Connection *Connect2IPs(char *SrcIP,char *DestIP);
MAC_Connection *Create_Local_MAC_Connection(char *ip,int Wireless);
int ConnectToOutside(char *ip);
#if Linux
int Thread_ListenToOutside(void); 
#endif
#if WIN32
DWORD WINAPI Thread_ListenToOutside(LPVOID lpParam);
#endif
int ConvertBytesToString(unsigned char *ByteStr,char *ResultStr,int NumBytes);
int ConvertStringToBytes(unsigned char *InstStr,unsigned char *InstBytes);
int ConvertStringToBytesNum(unsigned char *InstStr,unsigned char *InstBytes, int NumByte);
int GetTimeStamp(char *timestamp);
uint32_t GetTimeInMS(void); //get day num, hour, min, sec, and ms, in milliseconds
int OpenRobotNetworkLogFile(void);
int CloseRobotNetworkLogFile(void);
int LogRobotNetworkData(char *FromIP,char *ToIP,char *DataStr,int NumBytes);
int OpenRobotModelLogFile(void);
int CloseRobotModelLogFile(void);
int LogRobotModelData(char *LogStr);
int SendCameraGET(MAC_Connection *lmac, char *GETStr);
#if Linux
int Thread_ListenToRobot(void); 
#endif
#if WIN32
DWORD WINAPI Thread_ListenToRobot(LPVOID lpParam);
#endif
double FastArcTan(double x);
int ProcessMotorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr);
int ProcessAccelSensorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr);
int ProcessAnalogSensorData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr);
int ProcessPowerData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr);
int ProcessCameraData(MAC_Connection *lmac, char *recvbuf, int NumBytes);
int ProcessGPSData(MAC_Connection *lmac,unsigned char *recvbuf,int NumBytes,char *DataStr); 
char *GetPCBName(int NameNum);
int GetPCBNameNum(char *Name);
int GetPCBNameNumByMAC(unsigned char *Mac);
int GetAccelsPCBNameNum(char *Name);
int GetAccelNameNum(char *AccelName);
int GetAccelNameNum_By_PCB(int PCBNum,int AccelNum);
char *GetAccelName_By_PCB(int PCBNum,int AccelNum);
char *GetAccelReadableName(char *AccelName);
int GetMotorNum(char *param);
char *GetMotorName(int PCBNum,int num);
char *GetMotorReadableName(char *MotorName);
char *GetMotorReadableName_By_PCB(int PCBNum,int num);
int GetMotorNameNum(char *MotorName);
int GetMotorNameNumByPCBNum(int PCBNum,int num);
int GetAnalogSensorNameNum(char *AnalogSensorName);
int GetAnalogSensorNameNum_By_PCB(int PCBNum,int SensorNum);
char *GetAnalogSensorPCBName(char *Name);
int GetCurrentPCBCount(void);
int GetCurrentEthAccelsPCBCount(void);
int DeleteAllExternalPCBConnections(void);
int GetNeededThrust(int MotorNameNum,float ForceNeeded,int WithGravity);
int GetWordLen(char *tstr,int num);
int UpdateEthConfig(char *PCBName,char *Kind);
char *GetAccelNameFromReadableName(char *tstr);
char *GetMotorNameFromReadableName(char *tstr);
#endif //_ROBOT_H
