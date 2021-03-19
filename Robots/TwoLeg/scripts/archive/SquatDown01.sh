#SquatDown01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#legs-torso X
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-1,1,MOTOR_LEFT_LEG,1,4000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-1,1,MOTOR_RIGHT_LEG,1,4000,WAIT_STAGE)

#knees-upper leg X  (ex: 20 to 25)
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,CurrentAnglePlus,3,-2,2,MOTOR_LEFT_KNEE,1,4000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,CurrentAnglePlus,3,-2,2,MOTOR_RIGHT_KNEE,1,4000,WAIT_STAGE)

#knees-torso X (higher priority than knees-upper leg X)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAnglePlus,5,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAnglePlus,5,-2,2,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

#feet-lower leg X  (ex: -13 to -18)
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,CurrentAnglePlus,-3,-2,2,MOTOR_LEFT_FOOT,1,4000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,CurrentAnglePlus,-3,-2,2,MOTOR_RIGHT_FOOT,1,4000,WAIT_STAGE)

#feet-feet

#feet-torso

4,StopLogging

