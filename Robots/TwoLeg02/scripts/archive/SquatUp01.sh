#SquatUp01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#legs-torso X
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-1,1,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-1,1,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

#knees-upper leg X  (ex: 25 to 20)
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,CurrentAnglePlus,-5,-2,2,MOTOR_LEFT_KNEE,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,CurrentAnglePlus,-5,-2,2,MOTOR_RIGHT_KNEE,10,2000,WAIT_STAGE)

#knees-torso X (higher priority than knees-upper leg X)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAnglePlus,-5,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAnglePlus,-5,-2,2,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

#feet-lower leg X  (ex: -18 to -13)
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,CurrentAnglePlus,5,-2,2,MOTOR_LEFT_FOOT,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,CurrentAnglePlus,5,-2,2,MOTOR_RIGHT_FOOT,10,2000,WAIT_STAGE)

#feet-feet

#feet-torso

4,StopLogging

