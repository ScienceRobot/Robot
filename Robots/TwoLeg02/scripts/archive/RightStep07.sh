#RightStep07.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#right leg step up
#turn right leg to x=20
0,TurnMotorUntilAngle(MOTOR_RIGHT_LEG,50,2000,ACCEL_RIGHT_UPPER_LEG,X,20,0,3,WAIT_STAGE)
#hold torso X using left leg - because rl motor makes torso fall back +
0,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
#bend right knee back to x=-14
0,TurnMotorUntilAngle(MOTOR_RIGHT_KNEE,50,2000,ACCEL_RIGHT_LOWER_LEG,X,-14,-3,0,WAIT_STAGE)
#hold right foot at x=0
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-2,2,MOTOR_RIGHT_FOOT,10,2000,WAIT_STAGE)
#swing hip back by holding torso z to Current +-3 using both side motors
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_LEFT_SIDE,10,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_RIGHT_SIDE,10,2000,WAIT_STAGE)



#right leg step down
#turn right leg back down to x=10
1,TurnMotorUntilAngle(MOTOR_RIGHT_LEG,50,2000,ACCEL_RIGHT_UPPER_LEG,X,10,-3,0,WAIT_STAGE)
#continue to hold torso X using left leg
1,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,2000,WAIT_STAGE)
#bend right knee forward to x=-4
1,TurnMotorUntilAngle(MOTOR_RIGHT_KNEE,50,2000,ACCEL_RIGHT_LOWER_LEG,X,-4,0,3,WAIT_STAGE)
#hold right foot at x=0
1,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-2,2,MOTOR_RIGHT_FOOT,10,2000,WAIT_STAGE)
#hold torso z to 0 +-3 using both side motors
1,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_LEFT_SIDE,10,2000,WAIT_STAGE)
1,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_RIGHT_SIDE,10,2000,WAIT_STAGE)


4,StopLogging

