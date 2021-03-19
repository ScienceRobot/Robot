#RightStepUp01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#STAGE 0
#LEAN WITH SIDE
#lean left until upper leg Z=0 and
#side motors tilt torso Z 
#0,StoreAngle(ACCEL_TORSO,Z,TZ_ORIG)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,2000,ACCEL_LEFT_UPPER_LEG,Z,-1,-1,1,WAIT_STAGE|DOUBLE_THRUST)
#leg motors make sure torso does not fall forward or backward
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,2000,ACCEL_TORSO,X,CurrentAngle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#left knee holds upper leg X
0,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#left foot inst lasts through STAGE=3 so there is no interruption in the thrust, needs longer timeout
#removed DOUBLE_THRUST, because could be working individually, DOUBLE_THRUST is more for 2 motors on 1 accel
0,MotorAccel(MOTOR_LEFT_FOOT,1,4000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=3)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#ankle moves TORSO to Z=0 (torso Z rotates from positive to 0, -1 - -1=0)
#0,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,-1,-1,1,WAIT_STAGE)


#STAGE 1
#LEAN WITH ANKLE
#use ankle to shift torso weight over left leg
#1,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,3,-1,1,WAIT_STAGE)
1,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_UPPER_LEG,Z,-4,-0.5,0.5,WAIT_STAGE)
#leg motors make sure torso does not fall forward or backward
1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#1,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
1,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,Angle[RLLX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)



#STAGE 2
#LIFT RIGHT LEG
#lift right leg
2,MotorAccel(MOTOR_RIGHT_LEG,4,2000,ACCEL_RIGHT_UPPER_LEG,X,32,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend
2,MotorAccel(MOTOR_RIGHT_KNEE,9,2000,ACCEL_RIGHT_LOWER_LEG,X,-30,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE|CONSTANT_THRUST)
#right foot needs to maintain X=0 degrees
2,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#hold right leg away from left leg using right side motor
2,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_RIGHT_UPPER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE)

#left leg counters- holding torso X up
2,MotorAccel(MOTOR_LEFT_LEG,6,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#left side motor counters- holding torso Z up
2,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_TORSO,Z,CurrentAngle[TZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left knee holds upper leg X
2,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left upper leg Z- upper leg falls faster and sooner than lower leg
2,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_UPPER_LEG,Z,CurrentAngle[LULZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#STAGE 3
#HOLD RIGHT LEG UP
#left leg counters- holding torso X up
3,MotorAccel(MOTOR_LEFT_LEG,6,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#left side motor counters- holding torso Z up
3,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_TORSO,Z,Angle[TZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left knee holds upper leg X
3,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left lower leg Z
3,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_UPPER_LEG,Z,Angle[LULZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left foot holds left lower leg X
#3,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


4000,StopLogging

