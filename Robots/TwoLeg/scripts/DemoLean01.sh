#DemoLean01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#Stage 1: LEAN LEFT 
#turn both ankles and leg side motors
1,TurnMotor(MOTOR_RIGHT_ANKLE,-8,500)
1,TurnMotor(MOTOR_LEFT_ANKLE,-8,500)
#1,TurnMotor(MOTOR_RIGHT_SIDE,8,500)
#1,TurnMotor(MOTOR_LEFT_SIDE,8,500)

1,StartLoop(-1,Any,31)  #***START LOOP***  Any key ends loop, escape key aborts script

#Stage 2: LEAN RIGHT 
#turn both ankles and leg side motors
500,TurnMotor(MOTOR_RIGHT_ANKLE,8,1000)
500,TurnMotor(MOTOR_LEFT_ANKLE,8,1000)
#500,TurnMotor(MOTOR_RIGHT_SIDE,-8,1500)
#500,TurnMotor(MOTOR_LEFT_SIDE,-8,1500)

#Stage 3: LEAN LEFT 
#turn both ankles and leg side motors
1500,TurnMotor(MOTOR_RIGHT_ANKLE,-8,1000)
1500,TurnMotor(MOTOR_LEFT_ANKLE,-8,1000)
#2500,TurnMotor(MOTOR_RIGHT_SIDE,8,2000)
#2500,TurnMotor(MOTOR_LEFT_SIDE,8,2000)

2500,EndLoop  #  ***END LOOP****

5000,StopLogging

