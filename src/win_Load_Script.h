//win_Load_Script.h

#ifndef _WIN_LOAD_SCRIPT_H
#define _WIN_LOAD_SCRIPT_H

#define NUM_ROBOT_INSTRUCTIONS	15

typedef enum
{
	ROBOT_INSTRUCTION_START_LOGGING              = 0,    
	ROBOT_INSTRUCTION_LOG_SCRIPT_FILENAME,
	ROBOT_INSTRUCTION_STOP_LOGGING,
	ROBOT_INSTRUCTION_START_ACCELEROMETER,
	ROBOT_INSTRUCTION_STOP_ACCELEROMETER,
	ROBOT_INSTRUCTION_START_ANALOG_SENSOR,
	ROBOT_INSTRUCTION_STOP_ANALOG_SENSOR,
	ROBOT_INSTRUCTION_TURN_MOTOR,
	ROBOT_INSTRUCTION_MOTOR_ANGLE,
	ROBOT_INSTRUCTION_STORE_ANGLE,
	ROBOT_INSTRUCTION_START_LOOP,
	ROBOT_INSTRUCTION_END_LOOP,
//	ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE,
//	ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_RELATIVE_ANGLE,
//	ROBOT_INSTRUCTION_TURN_MOTOR_UNTIL_ANGLE_WAIT,
//	ROBOT_INSTRUCTION_HOLD_ANGLE_WITH_MOTOR,
//	ROBOT_INSTRUCTION_HOLD_RELATIVE_ANGLE_WITH_MOTOR,
	ROBOT_INSTRUCTION_ANALOG_SENSOR,
	ROBOT_INSTRUCTION_START_BALANCING,
	ROBOT_INSTRUCTION_STOP_BALANCING
} ROBOT_INSTRUCTIONS;

//#define ETHACCELS_INSTRUCTION_CONDITION_EQUAL 0
//#define ETHACCELS_INSTRUCTION_CONDITION_GT 1
//#define ETHACCELS_INSTRUCTION_CONDITION_LT 2
//#define ETHACCELS_INSTRUCTION_CONDITION_BETWEEN 3

//was: typedef struct RobotInstructionCondition
#define NUM_ETHACCEL_ACQUISITION_KINDS 4
typedef enum
{
ETHACCELS_INSTRUCTION_SINGLE_SAMPLE 	=0,
ETHACCELS_INSTRUCTION_POLLING,
ETHACCELS_INSTRUCTION_ABSOLUTE_INTERRUPT,	
ETHACCELS_INSTRUCTION_RELATIVE_INTERRUPT
} ETHACCEL_ACQUISITION_KINDS_NUM;

#define ALL_ETHACCELS -1  //PCBNum will == -1
#define MAX_ANGLES_PER_INSTRUCTION 5 //no more than 5 accelerometers per instruction
#define MAX_MOTORS_PER_INSTRUCTION 2 //no more than 2 motors per instruction
#define ANGLE_KIND_ACCEL 1 //for EthAccel instruction which kind of angle (accelerometer angle)
#define ANGLE_KIND_ANALOG 2 //for EthAccel instruction which kind of angle (potentiometer/analog sensor angle)

//EthAccelInstrucion - info relating to acclerometers in an instruction.
//Note: The main reason for using 2 accelerometers is because one motor may be examining two accelerometers, and determining what thrust to set the motor to using two separate instructions can cause conflicting consecutive Thrusts (ex: -3 then 4 then -3 again). The smoothest approach is for the motor to address only one accel at a time. If the first accel is out of the target range, then the motor should work only to bring that accel back into range, and ignore the other accel (accel[1]) until accel[0] is in range again. This is mostly for the foot motors to make sure the feet X do not exceed x=+-5degrees. An alternative design could have these accel samples constantly watched and when a range is over a pre-set limit, the motor could be changed to move the accel/segment back to the target/safe zone- but this approach is more simple, uses less processing, and so far appears to work well (although having to specify which of 2 accels is being referred to adds complexity and processing time). 
//#define ETHACCELS_INSTRUCTION_SINGLE_SAMPLE 0x00000001
//#define ETHACCELS_INSTRUCTION_POLLING 0x00000002
//#define ETHACCELS_INSTRUCTION_ABSOLUTE_INTERRUPT 0x00000004
//#define ETHACCELS_INSTRUCTION_RELATIVE_INTERRUPT 0x00000008
#define ETHACCELS_INSTRUCTION_HAS_THRESHOLD 					0x00000001  //a threshold should be set with an interrupt instruction
//#define ETHACCELS_INSTRUCTION_DETECTED_ANGLE_OUT_OF_RANGE		0x00000002  //The segment angle has already been recognized to be out of the target angle range. Used so that "Out of angle range" does not get printed with each sample.
//#define ETHACCELS_INSTRUCTION_SEGMENT_ANGLE_IN_RANGE	0x00000002  //the condition (ex: to reach a particular angle range) was fulfilled- this is neededso Thread_RobotTasks will not keep entered "Condition fulfilled" in the log, and so that the condition being lost can be known			
#define ETHACCELS_INSTRUCTION_PCB_NOT_FOUND						0x00000002 
#define ETHACCELS_INSTRUCTION_PCB_NOT_RESPONDING			0x00000004
//#define ETHACCELS_INSTRUCTION_MAX_THRUST_1_WHEN_GRAVITY_ASSIST		0x00000100    //foot, knee, and leg motors should never thrust more than +/-1 when moving a body segment with gravity assist (from a lower to higher magnitude angle, ex: 0 to 30 or -30).
#define ETHACCELS_INSTRUCTION_CATCH_UP_SECOND_ACCEL		0x00000008   //for an instruction keeping two accels synced, the other (second) accel is falling behind and so this (the first) accel needs to be slowed down to wait for the other accel to catch up or pass the angle of the first accel.
#define ETHACCELS_INSTRUCTION_SENT_INITIAL_THRUST			0x00000010   //sent initial thrust: The current model is to estimate the initial thrust based on gravity and then inc or dec thrust based on velocity and accel after that. todo:move to EthMotorInstruction flags
#define ETHACCELS_INSTRUCTION_DECREASING_OSCILLATION	0x00000020  //decreasing oscillation has been detected so skip thrust analysis section (robot body segment is oscillating back and forth, but slowing to a stop- so do not thrust to offset motion toward TargetAngle or in opposite direction if OPPOSE_MOTION is set).
typedef struct EthAccelsInstruction
{
uint32_t flags;
uint32_t origflags;  //preserve original flags to recopy to flags when inst is done being run once.
//char EthAccelsPCB[64]; //name of EthAccels PCB (can be ALL_ETHACCELS)
//EthAccels_PCB *PCB[MAX_ANGLES_PER_INSTRUCTION]; //stores pointer to EthAccel PCB(s) (2 if a relative angle comparison) for easy reference during running scripts
Accels_PCB *PCB[MAX_ANGLES_PER_INSTRUCTION]; //stores pointer to EthAccel PCB(s) (2 if a relative angle comparison) for easy reference during running scripts
int32_t PCBNum[MAX_ANGLES_PER_INSTRUCTION]; //PCB Number (ex: ROBOT_PCB_ETHACCELS_LEFT_LEG), a second for comparing angles between 2 accels
int32_t AccelMask; //mask of which accelerometer(s) on PCB 1=0,2=1,3=0&1,4=2,5=0&2,6=1&2,7=ALL
int32_t AngleKind[MAX_ANGLES_PER_INSTRUCTION]; //which kind of angle (1=accelerometer, 2=potentiometer)
int32_t AccelNum[MAX_ANGLES_PER_INSTRUCTION]; //number of accel on EthAccel PCB, that instruction uses [0]=Accel0,[1]=Accel1, etc.(AccelMask could be used, but just to be clearer for MotorAngle() which will use a single Accel number)
int32_t AccelNameNum[MAX_ANGLES_PER_INSTRUCTION]; //An individual accel/gyro name number. Used by motion code to make exceptions for certain accels (for example not to counter thrust when torso Z is in range but moving against gravity)
int32_t AnalogSensorNum[MAX_ANGLES_PER_INSTRUCTION]; //number of analog sensor on EthAccel PCB, that instruction uses
int32_t AnalogSensorNameNum[MAX_ANGLES_PER_INSTRUCTION]; //An individual analog sensor name number. Note that for a potentiometer the angles are relative to the joint, not to the center of earth
int32_t AcqKind; //acquisition kind (see above defines) SINGLE_SAMPLE, POLLING, ABSOLUTE_INTERRUPT, RELATIVE_INTERRUPT
int32_t Threshold; //threadhold of interrupt (optional)
int32_t xyz[MAX_ANGLES_PER_INSTRUCTION]; //which accel dimension (0=x,1=y,2=z) [2] for comparison of 2 accelerometers
//int32_t oper; //kind of operation/condition to check: equal to, greater than, less than, or between a range
float Angle[3]; //[0]=Target Angle, [1]=Min Angle, [2]=Max Angle (used for motion/thrust analysis code, can be from Accel[0] or [1])
float oAngle[MAX_ANGLES_PER_INSTRUCTION][3]; //Original for Accel[0]:[0][0]=Target Angle, [0][1]=Min Angle, [0][2]=Max Angle, etc. 
//float DegOff[2]; //[0]=Degrees less than target, [1]=Degrees more than target (can come from Accel[0] or Accel[1])
float oDegOff[MAX_ANGLES_PER_INSTRUCTION][2]; //Original [0]=Degrees less than target, [1]=Degrees more than target, [3][4]=for Accel[1], etc.
float AngOffset[MAX_ANGLES_PER_INSTRUCTION]; //is added to the current angle when "CurrentAnglePlus" is specified for Accel[0] and Accel[1]. One for each accelerometer. AngOffset[0] is also used to store the offset for the TargetAngle of Accel[0] for RELATIVE_ANGLE.
int32_t IncAngDir[MAX_ANGLES_PER_INSTRUCTION][2]; //[AccelNum][MotorNum]. MOTOR_CLOCKWISE (1) or MOTOR_COUNTERCLOCKWISE (-1)- the direction the motor is turned to try and increase the associated angle. Because there can be 2 motors and 4 accels, that creates 8 possible MotorAngleAssociations ([0][0]:Accel[0]+Motor[0],[0][1]:Accel[0]+Motor[1],[1][0]:Accel[1]+Motor[0],[1][1]:Accel[1]+Motor[1],etc. A possible alternative is just determining if the Motors have opposite IncAngDirs, but this method is good enough.
//int32_t AngNeedsToInc[2]; //[0]=Accel0 [1]==Accel1. To get to target, angle needs to increase (1=Yes, 0=No). Ex: TargetAngle=10 CurrentAngle=0 AngNeedsToInc=1 (yes). TargetAngle=-10, CurrentAngle=-8 AngNeedsToInc=0 (no).
int32_t LastAngNeedsToInc[MAX_ANGLES_PER_INSTRUCTION]; //needed mainly for CONSTANT_THRUST flag, because segment can pass over target without even being InRange, and when this happens constant thrust needs to be reversed
float ThrustVelocityRatio[MAX_ANGLES_PER_INSTRUCTION][4]; //currently for OPPOSE_MOTION only, Thrust to Velocity ratio (TVR) (if TVR=1.0 to oppose velocity Thrust=velocity ex: 1dps=thrust 1, 2 dps=thrust 2, etc., if TVR=2.0 Thrust=2*velocity, etc.). ([0][0]:Accel[0]Motor[0] positive dir, [0][1]:Accel[0]Motor[0] negative dir, [0][2]:Accel[0]Motor[1] positive dir [0][3]:Accel[0]Motor[1] negative dir, [1][0]:Accel[1]Motor[0] positive dir, etc.
int MinTVRThrust[MAX_ANGLES_PER_INSTRUCTION]; //The minimum thrust that a TVR opposition thrust can be. Helps to eliminate low thrusts when velocity in wrong direction is very low, so resulting TVR thrust is very low. Is given as third number in TVR=T+:T-:MinTVRThrust. MinTVRThrust[AccelNum]
float ConstantThrustBias[MAX_ANGLES_PER_INSTRUCTION][4]; //same as ThrustVelocityRatio but for ConstantThrustBias (ex: CONSTANT_THRUST=20:2)
float Weight[MAX_ANGLES_PER_INSTRUCTION]; //weight of mass that assists or resists motor (from MotorAngleAssociation). One for each accelerometer.
float Mass[MAX_ANGLES_PER_INSTRUCTION]; //mass that assists or resists motor (from MotorAngleAssociation). One for each accelerometer.
float TargetSpeed0; //Initial target speed provided by user. units: Degrees/sec 
float TargetSpeed; //Current target speed. Braking decreases this to 0. units: Degrees/SampleInterval (currently Degrees/10^-2sec)
//float MaxSpeed; //Maximum speed the segment angle associated with this instruction can move in. units: Degrees/SampleInterval (currently Degrees/10^-2sec)
//int SampleCount;  //running total of number of samples (currently equals 10ms interval) from the associated accelerometer that have been received while this instruction runs 
float MinVelocity[MAX_ANGLES_PER_INSTRUCTION]; //the minimum velocity- code will not decrease thrust if Segment Velocity is below MinVelocity (stops choppy motion). One for each accelerometer. Always positive values.
float MedianVelocity[MAX_ANGLES_PER_INSTRUCTION]; //MaxVelocity-MinVelocity/2.0;  [0]=accel0 [1]=accel1.  Always positive values.
float MaxVelocity[MAX_ANGLES_PER_INSTRUCTION]; //the maximum velocity- code will not increase thrust if Segment Velocity is above MaxVelocity (stops Segment from gaining too much velocity). One for each accelerometer. Always positive values.
float MaxAccel[MAX_ANGLES_PER_INSTRUCTION]; //Maximum acceleration; [0]=accel0 [1]=accel1. Always positive values.
float MinAccel[MAX_ANGLES_PER_INSTRUCTION]; //Minimum acceleration; [0]=accel0 [1]=accel1. Always positive values.
float BrakeDecel; //The target decelleration rate. Currently calculated as 10% of TargetSpeed (and determines duration of brake).
float BrakeAngle; //The actual angle that the Body Segment will cross to enter into the brake zone (calculated when running script) units:degrees 
float BrakeAngleTotal;   //The distance needed to brake from target speed to a stop. units:degrees
int LastUseAngle; //store the last UseAngle for 1 motor 2 accel arrangement, to know which accel was last used to detect a change in accel in use.
float MaxSpread; //the maximum angle (in degrees) that two accelerometers being synced can be spread apart.
//uint32_t LastSampleTimeStamp[MAX_ANGLES_PER_INSTRUCTION]; //timestamp (in ms) of the last associated accelerometer sample- used to make sure the code is not analyzing the same sample twice.  One for each accelerometer.
uint32_t CurrentSampleTimeStamp[MAX_ANGLES_PER_INSTRUCTION]; //timestamp (in ms) of the current associated accelerometer sample. One for each accelerometer.
uint32_t ETA; //in ACCEL_TIMER_INTERVAL_IN_MS units (*1000/ACCEL_TIMER_INTERVAL_IN_MS for seconds) estimated time of arrival to target angle in ms based on current rate AngleChange when moving in the correct direction)
float CurrentAngle; //Current angle of body segment (or relative angle between moving body segment and second accel)
float LastAngle; //last CurrentAngle
float TotalAngle; //(TargetAngle-CurrentAngle) the full size of the angle the body segment needs to turn alt:TotalDistance
float RemainingAngle; //the remaining size of the angle the body segment needs to turn alt:RemainingDistance
float AngleChange; //(Velocity) stores the change in the body segment angle (or change in relative angle for comparisons between two accels)
float LastAngleChange; //stores the last AngleChange
char SensorReadableName[MAX_ANGLES_PER_INSTRUCTION][128]; //name of segment being moved ("Left Foot", "Left Lower Leg", etc.) - for logging.  One for each accelerometer.
float Velocity; //Velocity of Body Segement taken only from Gyroscope (with no regard to Accelerometer sample)
float LastVelocity; //last Velocity of Body Segement taken only from Gyroscope (with no regard to Accelerometer sample)
float Acceleration[5]; //last 5 acceleration samples for the axis in question- todo: make for each of 4 accels
//float AccelAvg5; //average of acceleration of last 5 samples
//Motion Analysis variables
float MotionThreshold[MAX_ANGLES_PER_INSTRUCTION]; //the magnitude of velocity for this instruction (motor-accel association) that is considered to be no motion after a TargetAngle is reached (or for OPPOSE_MOTION, the level of velocity below which the segment is viewed to be not moving). One for each accelerometer.
int WaitAndSeeDelay[MAX_ANGLES_PER_INSTRUCTION]; //number of samples of Thrust=0 to wait before resuming Thrust Analysis. This is needed or otherwise the Segment oscillates around the TargetAngle. This provides 50-100ms (depending on motor-accel combination) for the Segment to come to a rest, or to be sure that any motion can only be from an external source (not from the associated motor thrust). This value never changes and is currently a constant but may later come from MotorAngleAssociation list, or just from the RStatus.AccelInfo, and is set at the time of loading the script. One for each accelerometer.
int WaitAndSeeTime; //actual count of samples- if >0, WaitAndSeeTime is decremented by Thrust Analaysis code until 0 and ThrustAnalysis resumes. 
//Check for decreasing oscillation - (only during WaitAndSeeTime beacuse there has to be a period of thrust=0)
int CurrentDirection,LastDirection; //=1 if Velocity>LastVelocity, =-1 if Velocity<LastVelocity
float PeakPlus; //Highest velocity achieved in potential oscillation
float PeakMinus; //Lowest negative velocity achieved in potential oscillation
float LastPeakPlus; //Highest peak in last oscillation
float LastPeakMinus;  //Lowest Peak in last oscillation
int NumPeaks; //number of peaks found (can only be 0 or 1)
int SampleCount; //number of samples 
int Period; //Number of samples in the previous oscillation. Once a period is complete, SampleCount is compared to Period to see if a decreasing oscillation is detected.
//ine LastPeriod; //Period of last detected oscillation
int NumPeriod; //number of periods (or cycles) detected (sequentially- is set to 0 when a decreasing oscllation is ruled out because SampleCount before a Peak is too high)
float AbortVelocity[MAX_ANGLES_PER_INSTRUCTION]; //Velocity at and beyond which the script aborts (Accel/Gyro[0] or [1])
float AbortAngle[MAX_ANGLES_PER_INSTRUCTION]; //+-SegmentAngle at and beyond which the script aborts (Accel[0] or [1])
int AngleVariableIndex[MAX_ANGLES_PER_INSTRUCTION]; //index into angle variable array [0]=for accel[0] [1]=for accel[1], =-1 is not set since index into variable array can be 0
//struct RobotInstructionCondition *next;  //needed for Thread_RobotTasks to keep a list of active conditions
} EthAccelsInstruction;


//#define ETHMOTORS_INSTRUCTION_THRUST_IN_CORRECT_DIRECTION	0x00000001	//Thrust is pushing in the correct direction (only relevant if there is a TargetAngle)
typedef struct EthMotorsInstruction
{
//uint32_t flags; 
//char EthMotorsPCB[64]; //name of EthMotors PCB
int32_t PCBNum[MAX_MOTORS_PER_INSTRUCTION]; //PCB Number (ex: ROBOT_PCB_ETHMOTORS_LOWER_BODY), there can be 2 motors in one instruction
int32_t MotorNum[MAX_MOTORS_PER_INSTRUCTION]; //number of motor on PCB
int32_t Thrust; //thrust (or strength) of motor move (includes direction: +=cw, -=ccw)
int32_t Duration; //duration of motor move (in milliseconds) - may evolve to be just Duration of instruction
struct MAC_Connection *mac[MAX_MOTORS_PER_INSTRUCTION]; //MAC_Connection to Motor PCB for ease in sending TurnMotor commands
int32_t UserThrust; //Initial thrust specified by user in MotorAngleInst()
int32_t LastThrust; //the last thrust (and direction) the motor was turned in. Used by Thread_RobotTasks to determine if a new TurnMotor command needs to be issued. 
int ThrustCount; //used to count how long thrust has been applied before evaluating the need to thrust more
char MotorReadableName[128]; //name of motor doing the moving ("Left Foot", "Left Ankle") - for logging
int MotorNameNum[MAX_MOTORS_PER_INSTRUCTION]; //number of motor name. For Motors 0 and 1.
int MaxThrustAG; //Maximum thrust against gravity (each Motor has a max thrust with and against gravity)
int MaxThrustWG; //Maximum thrust with gravity
int MaxThrust[MAX_ANGLES_PER_INSTRUCTION]; //temporary holder for MaxThrust depends on gravity. Note: this MaxThrust is determined by which Accel, not which motor. 
int LastMaxThrust[MAX_ANGLES_PER_INSTRUCTION]; //used when an instruction needs to change the MaxThrust at runtime, instead of determining gravity, or userset, is easiest just to set MaxThrust back to LastMaxThrust once inst stops setting MaxThrust
int MaxReverseThrust[MAX_ANGLES_PER_INSTRUCTION]; //temporary holder for MaxReverseThrust depends on gravity. Note: this MaxThrust is determined by which Accel, not which motor. 
//int ThrustInCorrectDir; //1=Thrust is in direction of TargetAngle
} EthMotorsInstruction;


//EthAnalogSensorInstrucion - info relating to an analog (touch or potentiometer) sensor in an instruction.
typedef struct EthAnalogSensorInstruction
{
//uint32_t flags;
//int32_t origflags;  //preserve original flags to recopy to flags when inst is done being run once.
//EthAccels_PCB *PCB; //stores pointer to EthAccel PCB
Accels_PCB *PCB; //stores pointer to EthAccel or GAMTP PCB
int32_t PCBNum; //PCB Number (ex: ROBOT_PCB_ETHACCELS_LEFT_LEG)
int32_t AnalogSensorNum; //Touch sensor number (relative to EthAccelTouch PCB)
int32_t AnalogSensorNameNum; //An individual touch sensor name number from the array of all touch sensors on the robot. 
int32_t GTorLT; //greater than or less than (to compare current touch sensor value to value give by user in instruction)
float Value; //value (PercentPressed) given by user in instruction
int32_t Delay; //Time in milliseconds of a stage to wait before checking the AnalogSensor value
} EthAnalogSensorInstruction;

//#define ROBOT_INSTRUCTION_HAS_CONDITION 0x00000001
//#define ROBOT_INSTRUCTION_WAITING_FOR_CONDITION 0x00000002  //Thread_RobotTasks will not resend instruction, but will know to check to see if the condition has been met
#define ROBOT_INSTRUCTION_WAIT_STAGE								0x00000001  //wait until this instruction is done to execute all later instructions except those that have the same start time (stage) 
//#define ROBOT_INSTRUCTION_WAIT_ALL									0x00000002  //wait until this instruction is done to execute all later instructions (including those that have the same start time) 
#define ROBOT_INSTRUCTION_END_SCRIPT_ON_TIMEOUT			0x00000002  //if inst times out, end the script (sequence of instructions)
#define ROBOT_INSTRUCTION_MOTION_LOG								0x00000004  //save motion data to Motion Log file
#define ROBOT_INSTRUCTION_FUTURE_LOG								0x00000008  //save estimated future motion data to the Future Log file
#define ROBOT_INSTRUCTION_DOUBLE_THRUST							0x00000010   //two motors are thrusting together (legs to move torso), so task motion code knows that thrust is double for these instructions
#define ROBOT_INSTRUCTION_CONSTANT_THRUST						0x00000020  //Use a constant thrust, no change in thrust (until reached target angle zone) 
#define ROBOT_INSTRUCTION_CONSTANT_THRUST_BIAS			0x00000040  //use different constant thrust for positive and negative directions toward target
//#define ROBOT_INSTRUCTION_HOLD_ANGLE								0x00000080  //Hold the body segment to the specified angle range using the specified motor even after the body segment has entered the angle range, otherwise the instruction (and motor thrust) ends
#define ROBOT_INSTRUCTION_NO_HOLD						0x00000080  //(alt: NO_HOLD_ANGLE) The default MotorAngle instruciton is to hold the body segment to the specified angle range using the specified motor even after the body segment has entered the angle range. With NO_HOLD the instruction (and motor thrust) ends once the target range is reached
//#define ROBOT_INSTRUCTION_HOLD_ANGLE_TVR						0x00000100  //same as HOLD_ANGLE but use ThrustVelocityRatio method of opposing motion, where thrust is set to oppose some fraction of the current segment velocity when the segment is moving away from the target angle.
#define ROBOT_INSTRUCTION_TVR												0x00000100  	//Use ThrustVelocityRatio method of opposing motion, where thrust is set to oppose some fraction of the current segment velocity when the segment is moving away from the target angle.
#define ROBOT_INSTRUCTION_OPPOSE_MOTION							0x00000200  //The instruction has no TargetAngle, but instead just simply thrusts against any motion until the associated body segment is not moving (|Velocity|<1.0). Automatically sets HOLD_ANGLE flag. OPPOSE_MOTION holds until timeout or if WAIT_STAGE flag is set in group of insts with a non-HOLD_ANGLE inst, ends if non-HOLD_ANGLE inst ends.
#define ROBOT_INSTRUCTION_OPPOSE_MOTION_MAINTAIN		0x00000400  //same as OPPOSE_MOTION but Thrust is not stopped when associated body segment is not moving (without flag, thrust is maintained when segment has no motion). 
#define ROBOT_INSTRUCTION_ABORT_ON_TIMEOUT					0x00000800  //aborts script if instruction times out
#define ROBOT_INSTRUCTION_SYNC_TWO_ACCELS						0x00001000		//instruction requires keeping two accels syncronyzed (in one dimension) to have the same angle
#define ROBOT_INSTRUCTION_TWO_MOTORS								0x00002000		//instruction moves two motors identically based on (one dimension of) a single accelerometer. If used for complementary motors, first angle will be strived for, then second angle (but not simultaneously)
#define ROBOT_INSTRUCTION_LAST_STAGE								0x00004000		//instruction lasts through LAST_STAGE=# 
#define ROBOT_INSTRUCTION_MAINTAIN_THRUST						0x00008000		//maintain the motor thrust once the instruction is done (do not set thrust=0 once the instruction is done- will not work if last stage)
//#define ROBOT_INSTRUCTION_NO_INITIAL_THRUST					0x00200000		//Do not thrust motor at start of instruction, then use thrust analysis after that
//#define ROBOT_INSTRUCTION_INITIAL_THRUST						0x00400000		//Thrust motor at start of instruction, then use thrust analysis after that- note: probably should remove- there is always an initial thrust unless the maintain_thrust flag was used in the previous instruction
#define ROBOT_INSTRUCTION_LAST_MAINTAIN_THRUST			0x00010000		//last instruction that used this motor had a MAINTAIN_THRUST flag (be aware that insts can have 2 paired motors)
#define ROBOT_INSTRUCTION_INSTRUCTION_START					0x00020000		//set when instruction starts, is used to determine if target angle is in range or not, and then cleared (currently just for MotorAngle instructions) - possibly could be replaced by EthAccels SENT_INITIAL_THRUST
//#define ROBOT_INSTRUCTION_START_LOOP								0x00800000		//start a loop with this and all subsequent insts until EndLoop inst
//#define ROBOT_INSTRUCTION_END_LOOP									0x01000000		//end loop after this instruction if conditions are met
#define ROBOT_INSTRUCTION_NO_STOP_ON_TARGET					0x00040000			//skip code to try to stop on the target, when Target is within 1000ms	
#define ROBOT_INSTRUCTION_MOST_VELOCITY							0x00080000			//Use the accel that has the most velocity with no regard to if the accel is in range or not (default behavior w/o this flag is to use the first accel, unless in range, and then to use the second accel)
#define ROBOT_INSTRUCTION_FARTHEST_FROM_TARGET			0x00100000			//Use the accel that is the farthest from the target angle with no regard to if the accel is in range or not (default behavior w/o this flag is to use the first accel, unless in range, and then to use the second accel)
#define ROBOT_INSTRUCTION_OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET		0x00200000			//Use the TVR method of opposing motion when the accel (currently both [0] and [1] if two accels are given) angle is in range and Velocity is moving towards the target angle (V=Y). When HOLD_ANGLE=+:- is used, the TVR plus-minus bias only applies to when Velocity is away from the target (V=N). Using this flag, the TVR method is used when V=Y, but only when the body segment angle is in the Target angle range. Note that the TVR method will skip the default code used to try to stop a body segment on the target once the body segment is in the target range.
//#define ROBOT_INSTRUCTION_USE_MIN_VELOCITY					0x00000004			//When V=Y (Velocity is in the correct direction), thrust until the min velocity is achieved (the default is to thrust until MaxVelocity is achieved) 
#define ROBOT_INSTRUCTION_USE_MAX_VELOCITY					0x00400000			//When V=Y (Velocity is in the correct direction), thrust until the max velocity is achieved (the default is to only thrust until MinVelocity is achieved) 
#define ROBOT_INSTRUCTION_REQUIRED									0x00800000			//This flag indicates that all specified accel angles must be in their target ranges before the instruction (and stage) can be ended (otherwise instruction time out will end instruction and stage). 
#define ROBOT_INSTRUCTION_IF_A2_THEN_A0_ELSE_A1			0x01000000			//if Accel2 is in range then use Accel0, otherwise use Accel1
#define ROBOT_INSTRUCTION_IF_A3_THEN_A1_ELSE_A2			0x02000000			//if Accel3 is in range then use Accel1, otherwise use Accel2
#define ROBOT_INSTRUCTION_IF_A2_AND_A3_THEN_A0_ELSE_A1			0x04000000			//if Accel2 and Accel 3 are in range then use Accel0, otherwise use Accel1
//#define ROBOT_INSTRUCTION_IF_NOT_A_MAXTHRUST								0x08000000			//if Accel2, 3 or4 (below flagInstAccelMaxThrust determines which accel) is not in range then set MaxThrust (currently for all Accels)
//#define ROBOT_INSTRUCTION_REQUIRE_VELOCITY									0x10000000			//Require a velocity before an accel condition is true. Uses flagRequireAccelVelocity to determine which accel. 
typedef struct RobotInstruction
{
uint32_t flags; 
uint8_t flagAngleInRange; //Angle associated with an accelerometer is within the target range (bit0=Accel[0], bit1=Accel[1], etc.)  
//uint8_t flagLastAngleInRange; //above bit from the previous sample
uint8_t	flagRelativeAngle; //The TargetAngle for an Accel is NextAccel+offset (bit0=Accel[0] needs to be Accel[1]+offset, bit1=Accel[1]=Accel[2]+offset, etc.)
uint8_t flagUseCurrentAngle; //use the current angle the accelerometer is at as a target angle instead of a user specified target angle (bit0=Accel[0], bit1=Accel[1], etc.)
//uint8_t flagSetCurrentAngle; //to indicate the current angle has been set already (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagUseCurrentAnglePlus; //same as USE_CURRENT_ANGLE but adds +/-offset (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagStoreAngleVariable; //store an angle variable (bit0=Accel[0], bit1=Accel[1], etc.) 
uint8_t flagUseAngleVariable; //use an angle variable (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagUserMaxThrust;  //User provided MaxThrust that motor(s) can have for Accel[0]- needed because currently the motor code tends to increase thrust in the beginning, and different motions require different maximum thrusts (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagInstAccelMaxThrust;  //Used (in conjuction with above IF_NOT_A_MAXTHRUST flag) to determine which accel can set another accel MaxThrust (IF_NOT_A3, or IF_NOT_A4) (bit0=Accel[0],etc.) 
uint8_t flagInstMaxThrust;  //Instruction is configured to set MaxThrust that motor(s) can have for various Accels (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagInstSetMaxThrust; //Instruction actually set the MaxThrust of the Accel(s) (bit0=Accel[0], bit1=Accel[1], etc.)  
uint8_t flagUserMaxReverseThrust; //User provided MaxReverseThrust that motor(s) can have for Accel[0]- needed because different motions require different maximum reverse (braking) thrust strengths (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagAbortVelocity; //Flag that indicates that the script should be aborted if the velocity associated to the gyroscope equals or exceeds the given value (bit0=Accel/Gyro[0], bit1=Accel[1], etc.)
uint8_t flagAbortAngle; //Flag that indicates that the script should be aborted if the angle magnitude (+ or -) associated with the accelerometer equals or exceeds the given value. (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagIgnoreGravity; //ignore gravity for Motor-accel angle (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagNoThrustWhenMAGIR; //no thrust when moving against gravity when in range (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagNoMotionNoThrust; //If accel has no motion (as defined by MotionThreshold), then even if not in range, set Thrust=0 (do not thrust, and perform no thrust analysis) (bit0=Accel[0], bit1=Accel[1], etc.)
uint8_t flagAccelEndsStage; //mask to indicate which accel ends the stage (usually the last accel unless it is the second accel in a relative angle, then the second to last accel)
uint8_t flagIgnoreAccel; //ignore this accel (the accel is only used for logic purposes like IF_NOT_A1), otherwise the UseAngle code would possibly use an accel that it should not, this also removes checking for a Motor-Angle Association (bit=0=Accel[0],etc.)
uint8_t flagTVR; //mask to indicate which accel each TVR flag (TVR0, TVR1, etc.) should be assigned to
uint8_t flagRequireAccelVelocity; //denotes which accel condition(s) require a certain velocity (usually > or < some value) to be true.
uint8_t flagRequireVelocityOperators[MAX_ANGLES_PER_INSTRUCTION]; //bit0 0=less than, 1=greater than, bit1 0=AND 1=OR.  Used with RequiredVelocity to determine is an accel has the correct Velocity (as specified in a VELOCITY_A#{< or >}# flag)
uint32_t origflags;  //preserve original flags to recopy to flags when inst is done being run once.
int32_t StartTime; //start time of instruction (in ms. Note: in Robot time (large number), not script time (small number))
int32_t OrigStartTime; //original StartTime of instruction (needed when an inst uses the WAIT_STAGE, to keep track of the original start times of all the instrucitons to see which are in the same group)
//int32_t EndTime; //end time of instruction (was in script time- so was confusing)
int32_t InstTime; //time in ms since Inst started
int32_t SeqNum;  //Sequence Number of instruction: number of instruciton relative to other instructions in script (needed for looping). Insts are numbered sequentially starting with 1
int32_t InstNum; //instruction number- see above definitions in RobotInstructionNames
int32_t LineNum; //Line number of instruction in script
EthMotorsInstruction EMInst; //EthMotors PCB instruciton details
EthAccelsInstruction EAInst; //EthAccelTouchGPS PCB instruction details
EthAnalogSensorInstruction ETInst; //EthAccelTouchGPS PCB instruction details (for touch sensors only)
//RobotInstructionCondition InstCond; //Robot instruction condition- motor move depends on accelerometer value- todo: make multiple conditions
//MotionVariables Motion; //Motion variables
int32_t NumAngles; //number of angles used by this instruction was: NumAccels; //number of accelerometers used by this instruction
//int32_t NumActiveAccels; //number of accelerometers used by this instruction that actually seek target ranges (not those used for logic like IF_NOT_A2, etc.) (not those that have their corresponding flagIgnoreAccel bit set)
float RequiredVelocity[MAX_ANGLES_PER_INSTRUCTION]; //stores the value of the velocity required for accels in an instruction.(by VELOCITY_A#{< or >}# flag) 
int AngleVariable[MAX_ANGLES_PER_INSTRUCTION]; //index of variable angle 0=accel[0] 1=accel[1], etc.. Used to set or get the variable value from the script AngleVariable[] array
int LastStage; //holds last stage start time when LAST_STAGE flag is set
int NumLoops; //number of times to loop a group of insts if this inst is the first in a group of insts to loop
unsigned int KeyCode[MAX_ANGLES_PER_INSTRUCTION]; //Loop KeyCodes  0=KeyCode1 (on keypress end loop after last inst) 1=KeyCode2 (on keypress end loop instantly)
int InstMaxThrust[MAX_ANGLES_PER_INSTRUCTION]; //Stores MaxThrust an instruction may set
FILE *MotionLogfptr; //pointer to any motion log file associated with this instruction
char MotionLogFileName[512];  //name of motion log file associated with this instruction
FILE *FutureLogfptr; //pointer to any future motion log file associated with this instruction
char FutureLogFileName[512];  //name of future motion log file associated with this instruction
char InstText[1024]; //actual text of instruction
struct RobotInstruction *next;
} RobotInstruction;

//structure for linked list of Robot scripts
//alternative names: RobotTask
//RStatus contains iScript which points to this list of load robot scripts
#define MAX_SCRIPT_NAME_LENGTH 2048 //maximum length for name of script
#define MAX_SCRIPT_FILENAME_LENGTH 2048 //maximum length for filename of script
#define ROBOT_SCRIPT_IS_FROM_FILE 0x00000001  //so we can choose not to list internal scripts (not those loaded from a file by the user)
#define MAX_NUM_ANGLE_VARIABLES	50 //maximum number of variable angles (angles that are stored at script run-time)
#define MAX_ANGLE_VARIABLE_NAME_LENGTH 20 //maximum name of the variable used to store a run-time angle (ex: "TZ", "LULX", "LLLZ", etc.)
typedef struct RobotScript
{
uint32_t flags;
char name[MAX_SCRIPT_NAME_LENGTH]; //name of script file including file extension
char filename[MAX_SCRIPT_FILENAME_LENGTH]; //filename of script (if any) (includes full path)
int NumAngleVariables; //Current number of angle variables in use
float AngleVariable[MAX_NUM_ANGLE_VARIABLES]; //angles that are stored and retrieved at run-time
char AngleVariableName[MAX_NUM_ANGLE_VARIABLES][MAX_ANGLE_VARIABLE_NAME_LENGTH]; //names of variable angles (ex: "TZ", "LULX", "LLLZ", etc.)
RobotInstruction *iRobotInst;
struct RobotScript *next;
} RobotScript;




int winLoadScript_AddFTWindow(void);
int winLoadScript_AddFTControls(FTWindow *twin);
int winLoadScript_OnOpen(FTWindow *twin);
void btn_LoadScript_Cancel_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
unsigned int btn_LoadScript_OK_Click(FTWindow *twin,FTControl *tFTControl,int x,int y);
RobotScript* LoadRobotScriptFile(char *filename);
RobotScript* LoadRobotScript(char *name,char *buf,int bSize);
int GetLineFromText(char *tline,char *buf);
int UnloadRobotScript(RobotScript *lscript);
int AddRobotScript(RobotScript *lscript);
int RemoveRobotScript(RobotScript *lscript);
int FreeRobotScript(RobotScript *lscript);
int FreeAllRobotScripts(void);
int UpdateScriptList(FTControl *tcontrol);
RobotScript *GetRobotScript(char *name);
int GetEthAccelAcqNum(char *Name);
//int GetSensorName(char *Name,int PCBNum,int AccelNum);
#endif /* _WIN_LOAD_SCRIPT_H */
