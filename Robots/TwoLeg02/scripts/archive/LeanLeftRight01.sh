#LeanLeftRight01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#LEAN LEFT
#lean left 
#move torso to Z=5
#side motors
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,5,0,3,MOTOR_LEFT_SIDE,10,2000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,5,0,3,MOTOR_RIGHT_SIDE,10,2000,WAIT_STAGE)
0,TurnMotorUntilAngle(MOTOR_LEFT_SIDE,10,2000,ACCEL_TORSO,Z,5,0,3,WAIT_STAGE)
0,TurnMotorUntilAngle(MOTOR_RIGHT_SIDE,10,2000,ACCEL_TORSO,Z,5,0,3,WAIT_STAGE)
#hold torso X forward -2/-3 +-2
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

#LEAN RIGHT
2000,TurnMotorUntilAngle(MOTOR_LEFT_SIDE,10,2000,ACCEL_TORSO,Z,-5,-3,0,WAIT_STAGE)
2000,TurnMotorUntilAngle(MOTOR_RIGHT_SIDE,10,2000,ACCEL_TORSO,Z,-5,-3,0,WAIT_STAGE)
2000,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
2000,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

2001,StopLogging

