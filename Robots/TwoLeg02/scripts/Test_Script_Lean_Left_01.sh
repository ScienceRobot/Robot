#Test_Script_Lean_Left_01.sh
#for 24rpm feet, ankle, knee, leg, side and torso motors
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#***legs spread 12cm from left foot to right foot center*** - otherwise stage 2 does not work

#constant thrust step right
#no thrust analysis, but does check for angle

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,X,LLLX0)  #-11.8
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX0)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_HIP,X,HX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,Z,RULZ0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,X,LULX0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,Z,LULZ0)

##Stage 1: LEAN LEFT - needs to be somewhat fast, and leaves a little momentum
##just to make sure robot is leaning at ankle before large thrust of side motors- otherwise side motors turn strongly and robot does not lean- left side rack rises up- but robot ankles have not moved
#not having |NO_STOP_ON_TARGET on side motors does make a difference
#stage 1 requires: Accel Left Leg lower and upper leg z to reach target zone
1,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-6,1,ACCEL_LEFT_UPPER_LEG,Z,-2,-5,1,WAIT_STAGE|MIN_VELOCITY=5|MAX_VELOCITY=7|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1|ABORT_ON_TIMEOUT)
1,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-3,-5,1,WAIT_STAGE|HOLD_ANGLE|MIN_VELOCITY=5|MAX_VELOCITY=7|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1)
#
#set ankle motor to target a farther angle, because then it will likely still have some velocity when stage 1 ends 
#1,MotorAccel(MOTOR_LEFT_ANKLE,1,5000,ACCEL_LEFT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|FUTURE_LOG)
#1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1)
1,MotorAccel(MOTOR_LEFT_ANKLE,1,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-3,-2,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|FUTURE_LOG)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-3,-2,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1)
1,MotorAccel(MOTOR_TORSO,1,5000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=10|MIN_ACCEL=1)
#NO_STOP_ON_TARGET|



6000,StopLogging 

