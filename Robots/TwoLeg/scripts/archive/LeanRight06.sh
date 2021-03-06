#LeanRight06.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)

#Robot needs a little side motor tilt to get the lean started more consistently, just turning the ankle can be too slow.
#0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,5000,ACCEL_LEFT_UPPER_LEG,Z,1.5,-1,0,WAIT_STAGE|DOUBLE_THRUST)


#both ankles lean ankle until LLLZ is 3
#alt: left ankle turns until LLLZ=3 and right ankle turns until RLLZ=0  - currently 6 (symmetrical angle) is a little too much
0,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,2,5000,ACCEL_LEFT_LOWER_LEG,Z,6,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=5)
#side motors hold torso Z at 0
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,5000,ACCEL_TORSO,Z,-0,-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#legs hold torso X to current angle
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,5000,ACCEL_TORSO,X,CurrentAngle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#left knee holds upper leg X
0,MotorAccel(MOTOR_LEFT_KNEE,2,5000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
0,MotorAccel(MOTOR_RIGHT_KNEE,2,5000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle[RULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#feet motors make sure lower leg does not fall forward or backward, make timeout same as last inst
0,MotorAccel(MOTOR_LEFT_FOOT,1,5000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,5000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)

6000,StopLogging

