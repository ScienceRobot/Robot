#RightStep01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#lift right leg
2,MotorAccel(MOTOR_RIGHT_LEG,4,3000,ACCEL_RIGHT_UPPER_LEG,X,32,-1,1,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend
2,MotorAccel(MOTOR_RIGHT_KNEE,9,3000,ACCEL_RIGHT_LOWER_LEG,X,-30,-1,1,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
2,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#hold right leg away from left leg using right side motor
2,MotorAccel(MOTOR_RIGHT_SIDE,3,3000,ACCEL_RIGHT_UPPER_LEG,Z,-7,-1,1,WAIT_STAGE|HOLD_ANGLE)


#lower right leg
3,MotorAccel(MOTOR_RIGHT_LEG,2,3000,ACCEL_RIGHT_UPPER_LEG,X,15,-1,1,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend
3,MotorAccel(MOTOR_RIGHT_KNEE,6,3000,ACCEL_RIGHT_LOWER_LEG,X,-14.5,-1,1,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
3,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#hold right leg away from left leg using right side motor
3,MotorAccel(MOTOR_RIGHT_SIDE,3,3000,ACCEL_RIGHT_UPPER_LEG,Z,-7,-1,1,WAIT_STAGE|HOLD_ANGLE)


#left leg counters- holding torso
#2,MotorAccel(MOTOR_LEFT_LEG,6,3000,ACCEL_TORSO,X,CurrentAngle,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left lower leg Z
#2,MotorAccel(MOTOR_LEFT_ANKLE,1,3000,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-0.5,0.5,WAIT_STAGE|FUTURE_LOG)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,3000,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle,-0.5,0.5,WAIT_STAGE)



6000,StopLogging

