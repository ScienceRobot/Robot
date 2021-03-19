#RightStep03.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#constant thrust step right
#no thrust analysis, but does check for angle


0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX)

#Stage 1: LEAN LEFT 
#both ankles lean ankle until LLLZ is 0, thrust=7
#alt: left ankle turns until LLLZ=0 and right ankle turns until RLLZ=-6
1,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,7,5000,ACCEL_LEFT_LOWER_LEG,Z,-3,-1,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)

#Stage 2: LIFT RIGHT LEG and BEND RIGHT KNEE and RIGHT FOOT 23 is too long
#(LEFT FOOT MOTOR HOLDS ROBOT FROM FALLING FORWARD)
2,MotorAccel(MOTOR_RIGHT_LEG,6,400,ACCEL_RIGHT_UPPER_LEG,X,19,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
2,MotorAccel(MOTOR_LEFT_LEG,6,400,ACCEL_RIGHT_UPPER_LEG,X,19,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
2,MotorAccel(MOTOR_RIGHT_KNEE,9,400,ACCEL_RIGHT_LOWER_LEG,X,-20,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
2,MotorAccel(MOTOR_RIGHT_FOOT,1,400,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE) #does not use constant thrust
#left foot holds robot weight from falling forward. inst lasts through final stage ABORT_ANGLE1=35 
2,MotorAccel(MOTOR_LEFT_FOOT,1,800,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=3|FUTURE_LOG|ABORT_ANGLE1=35)
#left ankle holds robot weight from falling left
2,MotorAccel(MOTOR_LEFT_ANKLE,10,6100,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|FUTURE_LOG|LAST_STAGE=6100)

#2,TurnMotor(MOTOR_RIGHT_LEG,6,300)
#2,TurnMotor(MOTOR_LEFT_LEG,6,300)
#2,MotorAccel(MOTOR_RIGHT_KNEE,9,300)
#2,MotorAccel(MOTOR_RIGHT_FOOT,1,300) #does not use constant thrust
#left foot hold robot weight from falling forward. inst lasts through final stage ABORT_ANGLE1=35 
#2,MotorAccel(MOTOR_LEFT_FOOT,1,6000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=3|FUTURE_LOG|ABORT_ANGLE1=35)


#Stage 3: LOWER RIGHT LEG
3,MotorAccel(MOTOR_RIGHT_LEG,6,400,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
3,MotorAccel(MOTOR_RIGHT_KNEE,9,400,ACCEL_RIGHT_LOWER_LEG,X,RLLX,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
3,MotorAccel(MOTOR_RIGHT_FOOT,1,400,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE) #does not use constant thrust
#left ankle leans back
#3,MotorAccel(MOTOR_LEFT_ANKLE,10,800,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,LLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|FUTURE_LOG|LAST_STAGE=6100)


#Stage 4: BALANCE
#right ankle needs to lean back too
#4,MotorAccel(MOTOR_RIGHT_ANKLE,10,800,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|LAST_STAGE=6100)
#left and right leg motors kep torso from falling in X
4,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,4,2000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|FUTURE_LOG)



6100,StopLogging

