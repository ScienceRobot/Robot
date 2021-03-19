#RightStep02.sh
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


#Robot needs a little side motor tilt to get the lean started more consistently, just turning the ankle can be too slow.
#0,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,2000,ACCEL_LEFT_UPPER_LEG,Z,1.5,-1,0,WAIT_STAGE|DOUBLE_THRUST)
#0,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,2,2000,ACCEL_LEFT_LOWER_LEG,Z,-3,-1,1,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)


#both ankles lean ankle until LLLZ is -2 (was 0) +-1
#alt: left ankle turns until LLLZ=0 and right ankle turns until RLLZ=-6 - ankle is not HOLD_ANGLE in this stage
1,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,2,5000,ACCEL_LEFT_LOWER_LEG,Z,-3,-1,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|MAX_THRUST=5)
#side motors hold torso Z at 0
1,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,2,5000,ACCEL_TORSO,Z,0,-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE|FUTURE_LOG)
#leg motors make sure torso does not fall forward or backward
1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,1,5000,ACCEL_TORSO,X,CurrentAngle[TX],-2,2,WAIT_STAGE|DOUBLE_THRUST|HOLD_ANGLE)
#left knee holds upper leg X
1,MotorAccel(MOTOR_LEFT_KNEE,2,5000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#feet motors make sure lower leg does not fall forward or backward
#left foot inst lasts through stage=3 (needs longer timeout) ABORT_ANGLE1=40 - left leg falls under weight of robot
1,MotorAccel(MOTOR_LEFT_FOOT,1,6000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=10|FUTURE_LOG|ABORT_ANGLE1=35)
1,MotorAccel(MOTOR_RIGHT_FOOT,1,5000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle[RLLX],-1,1,WAIT_STAGE|HOLD_ANGLE)
#ankle moves TORSO to Z=0 (torso Z rotates from positive to 0, -1 - -1=0)
#0,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_TORSO,Z,-1,-1,1,WAIT_STAGE)

#LIFT RIGHT LEG
#Store RULX and RLLX for lowering the right leg after this stage
#1,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX1)
#1,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX1)
#lift right leg  thrust was 4 is now 6, angle was 32 is now 23
2,MotorAccel(MOTOR_RIGHT_LEG,6,2000,ACCEL_RIGHT_UPPER_LEG,X,23,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#note for a single two-motor instruction with different thrusts for each motor, the code needs to change UserThrust to an array[2]
#2,MotorAccel(MOTOR_LEFT_LEG,6,2000,ACCEL_RIGHT_UPPER_LEG,X,23,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend angle was -30 is now -20
2,MotorAccel(MOTOR_RIGHT_KNEE,9,2000,ACCEL_RIGHT_LOWER_LEG,X,-20,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
2,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#hold right leg away from left leg using right side motor - maintain current Z Angle, is around -6, more thrust is needed so MAX_THRUST=14, needs to start with 12 because rt leg Z falls fast once lifted into the air
2,MotorAccel(MOTOR_RIGHT_SIDE,12,2000,ACCEL_RIGHT_UPPER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=14|INITIAL_THRUST)

#left leg counters- holding torso X up
2,MotorAccel(MOTOR_LEFT_LEG,6,2000,ACCEL_TORSO,X,TX,-2,2,WAIT_STAGE|HOLD_ANGLE|INITIAL_THRUST)
#left side motor counters- holding torso Z up - LAST_STAGE=3- side holds TZ for leg up and down, otherwise Thrust=0 in between stages , added timeout of 4000ms, start with Thrust=12
2,MotorAccel(MOTOR_LEFT_SIDE,12,4000,ACCEL_TORSO,Z,CurrentAngle[TZ],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=14|LAST_STAGE=3|INITIAL_THRUST) 
#left knee holds upper leg X
#1,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,Angle[LULX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
2,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,LULX,-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds left lower leg Z note: LAST_STAGE=3, because otherwise Ankle Motor will set Thrust=0
2,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|LAST_STAGE=3|MAX_THRUST=16)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#LOWER RIGHT LEG
#lower right leg, was X=15
3,MotorAccel(MOTOR_RIGHT_LEG,4,2000,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#right lower leg needs to bend was X=-14.5, changed initial thrust from 6 to 8 because needs to be a little faster
3,MotorAccel(MOTOR_RIGHT_KNEE,8,2000,ACCEL_RIGHT_LOWER_LEG,X,RLLX,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#right foot needs to maintain X=0 degrees
3,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
#right side Z needs to hold right upper leg at Z=-6 degrees
3,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_RIGHT_UPPER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE)
#alt: right side Z now helps to push torso back counterclockwise
#2,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_TORSO,Z,Angle[TZ0],-2,2,WAIT_STAGE|HOLD_ANGLE)
#and right ankle helps to push weight back to the left
3,MotorAccel(MOTOR_RIGHT_ANKLE,1,2000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#left leg counters- holding torso up- push weight of body back to the right
3,MotorAccel(MOTOR_LEFT_LEG,4,2000,ACCEL_TORSO,X,TX,-2,2,WAIT_STAGE|HOLD_ANGLE)
#left side shifts weight back to right side while leg comes down- Max_Thrust needs to be higher to hold the weight of the torso Z
#2,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_TORSO,Z,Angle[TZ0],-2,2,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=12)
#left knee holds upper leg X
3,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,LULX,-2,2,WAIT_STAGE|HOLD_ANGLE)
#left ankle holds left lower leg Z: to 3 or 0?
#2,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,Angle[LLLZ0],-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left foot holds left lower leg X
#2,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-2,2,WAIT_STAGE|HOLD_ANGLE)


#BALANCE -  
#sides hold torso Z to 0
10,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,2000,ACCEL_TORSO,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE)
#legs hold torso X to initial angle (TX)
10,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,3,2000,ACCEL_TORSO,X,TX,-2,2,WAIT_STAGE|HOLD_ANGLE)
#L knee holds upper leg X to 
10,MotorAccel(MOTOR_LEFT_KNEE,2,2000,ACCEL_LEFT_UPPER_LEG,X,LULX,-2,2,WAIT_STAGE|HOLD_ANGLE)
#R knee holds upper leg X to 
#10,MotorAccel(MOTOR_RIGHT_KNEE,2,2000,ACCEL_RIGHT_UPPER_LEG,X,Angle[RULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
10,MotorAccel(MOTOR_RIGHT_KNEE,2,2000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle[RULX],-2,2,WAIT_STAGE|HOLD_ANGLE)
#ankles can help to correct any Z problem
10,MotorAccel(MOTOR_LEFT_ANKLE,1,2000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,LLLZ0,-2,2,WAIT_STAGE|HOLD_ANGLE)
10,MotorAccel(MOTOR_RIGHT_ANKLE,1,2000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,RLLZ0,-2,2,WAIT_STAGE|HOLD_ANGLE)
#left foot holds left lower leg X
#stage0 goes to stage10 10,MotorAccel(MOTOR_LEFT_FOOT,1,2000,ACCEL_LEFT_FOOT,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,Angle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE)
#right foot holds right lower leg X
10,MotorAccel(MOTOR_RIGHT_FOOT,1,2000,ACCEL_RIGHT_FOOT,X,0,-5,5,ACCEL_RIGHT_LOWER_LEG,X,RLLX,-1,1,WAIT_STAGE|HOLD_ANGLE)



6100,StopLogging

