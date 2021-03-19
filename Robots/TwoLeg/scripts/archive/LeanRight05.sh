#LeanRight05.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,5000,ACCEL_LEFT_UPPER_LEG,Z,6,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,5000,ACCEL_TORSO,Z,0,-5,5,ACCEL_LEFT_UPPER_LEG,Z,6,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#leg motors make sure torso does not fall forward or backward
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,5000,ACCEL_TORSO,X,CurrentAngle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#left knee holds upper leg X
0,MotorAccel(MOTOR_LEFT_KNEE,2,5000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
0,MotorAccel(MOTOR_RIGHT_KNEE,2,5000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle[RULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left knee helps hold lower leg X up
#0,MotorAccel(MOTOR_LEFT_KNEE,2,3000,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward, make timeout same as last inst
0,MotorAccel(MOTOR_LEFT_FOOT,1,5000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,5000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)


6000,StopLogging

