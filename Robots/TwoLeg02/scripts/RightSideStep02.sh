#RightSideStep01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 22cm from left foot to right foot center

#constant thrust step right
#no thrust analysis, but does check for angle


0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,Z,RULZ0)

#Stage 1: LEAN LEFT 
#both ankles lean ankle until LLLZ is 0, thrust=7
#alt: left ankle turns until LLLZ=-1 LLLZ starts at 3
1,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)
1,MotorAccel(MOTOR_LEFT_ANKLE,3,5000,ACCEL_LEFT_FOOT,Z,0,-1,1,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=3)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-1,1,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=3)


#Left leg motor holds up torso x
1,MotorAccel(MOTOR_LEFT_LEG,10,2000,ACCEL_TORSO,X,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=2)


#1,MotorAccel(MOTOR_LEFT_ANKLE,MOTOR_RIGHT_ANKLE,7,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|DOUBLE_THRUST|FUTURE_LOG|CONSTANT_THRUST)

#Stage 2: PUSH RIGHT LEG TO RIGHT
2,MotorAccel(MOTOR_RIGHT_SIDE,10,2000,ACCEL_RIGHT_UPPER_LEG,Z,-18,-1,1,WAIT_STAGE|CONSTANT_THRUST)
2,MotorAccel(MOTOR_LEFT_SIDE,10,2000,ACCEL_RIGHT_UPPER_LEG,Z,-18,-1,1,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)

#lift right leg a little to help right leg get off the ground
2,MotorAccel(MOTOR_RIGHT_LEG,10,2000,ACCEL_RIGHT_UPPER_LEG,X,CurrentAngle+3,-1,1,WAIT_STAGE|HOLD_ANGLE)
#bend right knee a little to help right leg get off the ground
2,MotorAccel(MOTOR_RIGHT_KNEE,10,2000,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle-3,-1,1,WAIT_STAGE|HOLD_ANGLE)
#important that left knee hold ULX to 17 standing is 10 - left knee needs to push hard initially
#2,MotorAccel(MOTOR_LEFT_KNEE,10,2000,ACCEL_LEFT_UPPER_LEG,X,CurrentAngle,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#important the left side holds torso Z to 0 (changed to CurrentAngle)
#2,MotorAccel(MOTOR_LEFT_SIDE,9,2000,ACCEL_TORSO,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=10)
#important that left ankle holds lower leg Z or else robot falls right
2,MotorAccel(MOTOR_LEFT_ANKLE,10,2000,ACCEL_LEFT_FOOT,Z,0,-3,3,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG|MAX_THRUST=10)
#torso motor holds up torso X
#2,MotorAccel(MOTOR_TORSO,9,2000,ACCEL_TORSO,X,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)


#2,MotorAccel(MOTOR_RIGHT_KNEE,9,1000,ACCEL_RIGHT_LOWER_LEG,X,-20,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#2,MotorAccel(MOTOR_RIGHT_FOOT,1,1000,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE) #does not use constant thrust
#right side pushes away from left leg
#2,MotorAccel(MOTOR_RIGHT_SIDE,6,2000,ACCEL_RIGHT_UPPER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE)

#left foot holds lower leg to current angle (keep robot from falling forward) 
2,MotorAccel(MOTOR_LEFT_FOOT,5,2000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|FUTURE_LOG)
#left ankle holds robot weight from falling left
#2,MotorAccel(MOTOR_LEFT_ANKLE,10,6100,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|FUTURE_LOG|LAST_STAGE=6100)

#right ankle hold Z at 0
#2,MotorAccel(MOTOR_RIGHT_ANKLE,10,2000,ACCEL_RIGHT_FOOT,Z,0,-3,3,WAIT_STAGE|FUTURE_LOG|HOLE_ANGLE)


#2,TurnMotor(MOTOR_RIGHT_LEG,6,300)
#2,TurnMotor(MOTOR_LEFT_LEG,6,300)
#2,MotorAccel(MOTOR_RIGHT_KNEE,9,300)
#2,MotorAccel(MOTOR_RIGHT_FOOT,1,300) #does not use constant thrust
#left foot hold robot weight from falling forward. inst lasts through final stage ABORT_ANGLE1=35 
#2,MotorAccel(MOTOR_LEFT_FOOT,1,6000,ACCEL_LEFT_FOOT,X,0,-5,5,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle[LLLX],-1,1,WAIT_STAGE|HOLD_ANGLE|LAST_STAGE=3|FUTURE_LOG|ABORT_ANGLE1=35)


#Stage 3: LOWER RIGHT LEG
#3,MotorAccel(MOTOR_RIGHT_LEG,6,400,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST)
#3,MotorAccel(MOTOR_RIGHT_KNEE,9,400,ACCEL_RIGHT_LOWER_LEG,X,RLLX,-0.5,0.5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#3,MotorAccel(MOTOR_RIGHT_FOOT,1,400,ACCEL_RIGHT_FOOT,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE) #does not use constant thrust
#left ankle leans back
#3,MotorAccel(MOTOR_LEFT_ANKLE,10,800,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,LLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|FUTURE_LOG|LAST_STAGE=6100)


#Stage 4: BALANCE
#right ankle needs to lean back too
#4,MotorAccel(MOTOR_RIGHT_ANKLE,10,800,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=16|LAST_STAGE=6100)
#left and right leg motors kep torso from falling in X
#4,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,4,2000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|FUTURE_LOG)



5000,StopLogging

