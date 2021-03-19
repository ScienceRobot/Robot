#LeanLeftStep01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center


#side motors  |MAXTHRUST=2 |MINTHRUST
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,3000,ACCEL_LEFT_UPPER_LEG,Z,0,-1,1,WAIT_STAGE|DOUBLE_THRUST)
#0,MotorAccel(MOTOR_LEFT_SIDE,2,3000,ACCEL_LEFT_UPPER_LEG,Z,0,-0.3,0.3,WAIT_STAGE|DOUBLE_THRUST)
#0,MotorAccel(MOTOR_RIGHT_SIDE,2,3000,ACCEL_LEFT_UPPER_LEG,Z,0,-0.3,0.3,WAIT_STAGE|DOUBLE_THRUST)


#use ankle to shift torso weight over left leg
1,MotorAccel(MOTOR_LEFT_ANKLE,3,3000,ACCEL_LEFT_UPPER_LEG,Z,-3,-0.5,0.5,WAIT_STAGE)

#then lift right leg
#2,MotorAccel(MOTOR_RIGHT_LEG,6,3000,ACCEL_RIGHT_UPPER_LEG,X,8,-0.5,0.5,WAIT_STAGE|FUTURE_LOG|CONSTANT_THRUST)
#right lower leg needs to bend
#2,MotorAccel(MOTOR_RIGHT_KNEE,2,3000,ACCEL_RIGHT_LOWER_LEG,X,-20,-0.5,0.5,WAIT_STAGE|FUTURE_LOG|CONSTANT_THRUST)
#right foot needs to maintain X=0 degrees
#2,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|FUTURE_LOG)

#left leg counters- holding torso
#2,MotorAccel(MOTOR_LEFT_LEG,6,3000,ACCEL_TORSO,X,CurrentAngle,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left lower leg Z
#2,MotorAccel(MOTOR_LEFT_ANKLE,1,3000,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-0.5,0.5,WAIT_STAGE|FUTURE_LOG)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,3000,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle,-0.5,0.5,WAIT_STAGE)



6000,StopLogging

