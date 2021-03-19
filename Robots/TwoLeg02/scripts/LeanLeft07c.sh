#LeanLeft07c.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#constant thrust = 3
#no thrust analysis, but stops at angle range

#both ankles lean ankle until LLLZ is 0, thrust=4
#alt: left ankle turns until LLLZ=0 and right ankle turns until RLLZ=-6
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG|MIN_VELOCITY=3|MAX_VELOCITY=4)
0,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,4,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG|MIN_VELOCITY=3|MAX_VELOCITY=4)

#0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG|CONSTANT_THRUST)
#0,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,7,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG|CONSTANT_THRUST)

6000,StopLogging

