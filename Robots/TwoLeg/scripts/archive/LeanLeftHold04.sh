#LeanLeftHold04.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center


#side motors do the actual lean  |MAXTHRUST=2 |MINTHRUST
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,3000,ACCEL_LEFT_UPPER_LEG,Z,-2,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#leg motors make sure torso does not fall forward or backward
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,3000,ACCEL_TORSO,X,CurrentAngle,-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
0,MotorAccel(MOTOR_LEFT_FOOT,1,3000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,-14,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,-14,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
0,MotorAccel(MOTOR_LEFT_ANKLE,1,3000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,-1,-1,1,WAIT_STAGE)


#use ankle to shift torso weight over left leg - makes sure foot Z does not go over +-5
#1,MotorAccel(MOTOR_LEFT_ANKLE,1,3000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#leg motors make sure torso does not fall forward or backward
#1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,3000,ACCEL_TORSO,X,CurrentAngle,-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#1,MotorAccel(MOTOR_LEFT_FOOT,1,3000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,-14,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#1,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,-14,-3,3,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)


6000,StopLogging

