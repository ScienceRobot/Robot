#RightStepSolo01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center


#Stage 0: LEAN WITH SIDE
#lean left until upper leg Z=0 and
#side motors tilt torso Z 
#0,StoreAngle(ACCEL_TORSO,Z,TZ_ORIG)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,2000,ACCEL_LEFT_UPPER_LEG,Z,-1,-2,0,WAIT_STAGE|DOUBLE_THRUST)
#leg motors make sure torso does not fall forward or backward
0,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,2000,ACCEL_TORSO,X,CurrentAngle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#left knee holds upper leg X
0,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#left foot inst lasts through stage=3 (needs longer timeout)
0,MotorAccel(MOTOR_LEFT_FOOT,1,6000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=10|FUTURE_LOG|ABORT_ANGLE1=30)
0,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-1,1,WAIT_STAGE|HOLD_ANGLE)
#ankle moves TORSO to Z=0 (torso Z rotates from positive to 0, -1 - -1=0)
#0,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,-1,-1,1,WAIT_STAGE)


#Stage 1: LEAN WITH ANKLE
#use ankle to shift torso weight over left leg - note that stopping this motion at the target cannot be done
#1,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,3,-1,1,WAIT_STAGE)
1,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,1,3000,ACCEL_LEFT_UPPER_LEG,Z,-3.5,-1,0,WAIT_STAGE|FUTURE_LOG)
#leg motors make sure torso does not fall forward or backward1
1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,3000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#1,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
1,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,Angle[RLLX],-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#nothing with the knee
#1,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,2000,ACCEL_TORSO,Z,0,-2,2,WAIT_STAGE|DOUBLE_THRUST)

#Stage 2: WAIT FOR TORSO Z=0 (Puts weight over left leg)
#2,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,2000,ACCEL_TORSO,Z,0,-1,1,WAIT_STAGE|DOUBLE_THRUST)
#2,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,1,3000,ACCEL_LEFT_UPPER_LEG,Z,-3.5,-1,0,WAIT_STAGE|FUTURE_LOG)
#2,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,3000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#2,MotorAccel(MOTOR_RIGHT_FOOT,1,3000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,Angle[RLLX],-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#nothing with knee

#LIFT RIGHT LEG
#lift right leg
3,MotorAccel(MOTOR_RIGHT_LEG,4,2000,ACCEL_RIGHT_UPPER_LEG,X,32,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend
3,MotorAccel(MOTOR_RIGHT_KNEE,9,2000,ACCEL_RIGHT_LOWER_LEG,X,-30,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
3,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#hold right leg away from left leg using right side motor - maintain current Angle, is around -12
3,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_RIGHT_UPPER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE)

#left leg counters- holding torso X up
3,MotorAccel(MOTOR_LEFT_LEG,6,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|HOLD_ANGLE|INITIAL_THRUST)
#left side motor counters- holding torso Z up
3,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_TORSO,Z,CurrentAngle[TZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG) #LAST_STAGE=3
#left knee holds upper leg X
3,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left lower leg Z
3,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#LOWER RIGHT LEG
#lower right leg
4,MotorAccel(MOTOR_RIGHT_LEG,4,2000,ACCEL_RIGHT_UPPER_LEG,X,15,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend
4,MotorAccel(MOTOR_RIGHT_KNEE,6,2000,ACCEL_RIGHT_LOWER_LEG,X,-14.5,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
4,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#right side Z needs to hold right upper leg at Z=-6 degrees
4,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_RIGHT_UPPER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE)
#alt: right side Z now helps to push torso back counterclockwise
#3,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_TORSO,Z,Angle[TZ0],-2,2,WAIT_STAGE|HOLD_ANGLE)
#and right ankle helps to push weight back to the left
4,MotorAccel(MOTOR_RIGHT_ANKLE,1,2000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,Angle[RLLZ0],-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#left leg counters- holding torso up- push weight of body back to the right
4,MotorAccel(MOTOR_LEFT_LEG,4,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#left side shifts weight back to right side while leg comes down
4,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_TORSO,Z,Angle[TZ0],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left knee holds upper leg X
4,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#left ankle holds left lower leg Z: to 3 or 0?
4,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,Angle[LLLZ0],-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left foot holds left lower leg X
#3,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)


#BALANCE - STRAIGHTEN RIGHT KNEE 
#sides hold torso Z to 0
10,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,2000,ACCEL_TORSO,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE)
#legs hold torso X to initial angle (TX)
10,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,3,2000,ACCEL_TORSO,X,Angle[TX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#L knee holds upper leg X to 
10,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#R knee holds upper leg X to 
10,MotorAccel(MOTOR_RIGHT_KNEE,2,2000,ACCEL_RIGHT_UPPER_LEG,X,Angle[RULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#NO NEED TO USE left ankle holds left lower leg Z
#10,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_LOWER_LEG,Z,3,-2,2,WAIT_STAGE|HOLD_ANGLE)
#NO NEED TO USE right ankle holds right lower leg Z
#10,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_LOWER_LEG,Z,-3,-2,2,WAIT_STAGE|HOLD_ANGLE)
#left foot holds left lower leg X
#stage0 goes to stage10 10,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_FOOT,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE)
#right foot holds right lower leg X
10,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,Angle[RLLX],-1,1,WAIT_STAGE|HOLD_ANGLE)



6100,StopLogging

