#LeanCenter03.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center


#lean left 
#ankle motors
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,-4,-1,1,MOTOR_LEFT_ANKLE,2000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,Z,-4,-1,1,MOTOR_RIGHT_ANKLE,2000,WAIT_STAGE)
#0,TurnMotorUntilAngle(MOTOR_LEFT_ANKLE,30,2000,ACCEL_TORSO,Z,4,-1,1,WAIT_STAGE)
#0,TurnMotorUntilAngle(MOTOR_RIGHT_ANKLE,30,2000,ACCEL_TORSO,Z,4,-1,1,WAIT_STAGE)

#side motors
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-1,1,MOTOR_LEFT_SIDE,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-1,1,MOTOR_RIGHT_SIDE,10,2000,WAIT_STAGE)
#0,TurnMotorUntilAngle(MOTOR_LEFT_SIDE,20,2000,ACCEL_TORSO,Z,-4,-1,1,WAIT_STAGE)
#0,TurnMotorUntilAngle(MOTOR_RIGHT_SIDE,20,2000,ACCEL_TORSO,Z,-4,-1,1,WAIT_STAGE)


#hold torso X forward -2/-3 +-2
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_RIGHT_LEG,10,2000,WAIT_STAGE)

#hold upper leg (or torso?) X with knee motor -2/-3 +-2 (ex: left = 2, right=6)
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,CurrentAngle,-4,4,MOTOR_LEFT_KNEE,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle,-4,4,MOTOR_RIGHT_KNEE,10,2000,WAIT_STAGE)


#hold lower leg up with feet motors (possibly relative to foot or upper leg x?) -18 degrees- but changes a lot
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,CurrentAngle,-4,4,MOTOR_LEFT_FOOT,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle,-4,4,MOTOR_RIGHT_FOOT,10,2000,WAIT_STAGE)

#hold ankle motors X to between -5 and 5
0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,Z,0,-5,5,MOTOR_LEFT_ANKLE,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,Z,0,-5,5,MOTOR_RIGHT_ANKLE,10,2000,WAIT_STAGE)


4,StopLogging

