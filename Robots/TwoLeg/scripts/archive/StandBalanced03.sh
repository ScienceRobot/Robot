#StandBalanced03.sh
0,StartLogging 
0,LogScriptFileName

#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#nod head down
0,TurnMotorUntilRelativeAngle(MOTOR_HEAD,50,2000,ACCEL_HEAD,X,ACCEL_TORSO,X,-37,-5,5,WAIT_STAGE)

#hold lower leg up with feet motors (possibly relative to foot or upper leg x?)
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,-13,-1,1,MOTOR_LEFT_FOOT,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,-13,-1,1,MOTOR_RIGHT_FOOT,2000,WAIT_STAGE)

#turn head back up
1,TurnMotorUntilRelativeAngle(MOTOR_HEAD,50,2000,ACCEL_HEAD,X,ACCEL_TORSO,X,0,-1,1,WAIT_STAGE)

#hold lower leg up with feet motors (possibly relative to foot or upper leg x?)
1,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,-13,-1,1,MOTOR_LEFT_FOOT,2000,WAIT_STAGE)
1,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,-13,-1,1,MOTOR_RIGHT_FOOT,2000,WAIT_STAGE)


#lean left with side motors
#0,TurnMotorUntilAngle(MOTOR_LEFT_SIDE,30,2000,ACCEL_TORSO,Z,4,-1,1,WAIT_STAGE)
#0,TurnMotorUntilAngle(MOTOR_RIGHT_SIDE,30,2000,ACCEL_TORSO,Z,4,-1,1,WAIT_STAGE)
10,StopLogging

