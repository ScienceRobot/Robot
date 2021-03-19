#RightStepDown01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#right leg step down
#turn right leg back down to x=10
#1,TurnMotorUntilAngle(MOTOR_RIGHT_LEG,50,2000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAnglePlus,-10,-3,0,WAIT_STAGE)
1,TurnMotorUntilAngle(MOTOR_RIGHT_LEG,50,3000,ACCEL_RIGHT_UPPER_LEG,X,12,-3,0,WAIT_STAGE)
#continue to hold torso X using left leg
1,HoldAngleWithMotor(ACCEL_TORSO,X,CurrentAngle,-2,2,MOTOR_LEFT_LEG,10,3000,WAIT_STAGE)
#bend right knee forward to x=-4
#1,TurnMotorUntilAngle(MOTOR_RIGHT_KNEE,50,2000,ACCEL_RIGHT_LOWER_LEG,X,CurrentAnglePlus,5,0,3,WAIT_STAGE)
1,TurnMotorUntilAngle(MOTOR_RIGHT_KNEE,50,3000,ACCEL_RIGHT_LOWER_LEG,X,-13,0,3,WAIT_STAGE)
#hold right foot at x=0
1,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-2,2,MOTOR_RIGHT_FOOT,10,3000,WAIT_STAGE)
#hold torso z to 0 +-3 using both side motors
#1,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_LEFT_SIDE,10,2000,WAIT_STAGE)
#1,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_RIGHT_SIDE,10,2000,WAIT_STAGE)


4,StopLogging
