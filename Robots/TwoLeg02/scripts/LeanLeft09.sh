#LeanLeft09.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 15cm from left foot to right foot center

#constant thrust step right
#no thrust analysis, but does check for angle

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,X,LLLX0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX0)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,Z,RULZ0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,X,LULX0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,Z,LULZ0)

#Stage 1: LEAN LEFT 
#both ankles lean ankle until LLLZ is 0, thrust=7
#alt: left ankle turns until LLLZ=-1 LLLZ starts at 3, important to limit thrust
#possibly 2 motors is the reason stage 1 lasts for a few ms after the target range is reached
#1,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)
1,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)
1,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)
1,MotorAccel(MOTOR_LEFT_ANKLE,3,5000,ACCEL_LEFT_FOOT,Z,0,-1,1,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=3)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-1,1,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=3)
#feet, knee, leg, and torso motors keep torso from falling forward or backward
#torso holds torso x until stage 4- note LAST_STAGE=4 needs 10s timeout, torso should not be using a lot of thrust, only small adjustments
#1,MotorAccel(MOTOR_TORSO,3,10000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=4|MAX_REVERSE_THRUST=3|LAST_STAGE=4|MAX_VELOCITY=20)
1,MotorAccel(MOTOR_TORSO,3,5000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=4|MAX_REVERSE_THRUST=3|MAINTAIN_THRUST|MAX_VELOCITY=20)


5000,StopLogging
