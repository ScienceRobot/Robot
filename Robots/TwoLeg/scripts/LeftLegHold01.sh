#LeftLegHold01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#legs spread 15cm from left foot to right foot center

#constant thrust step right
#no thrust analysis, but does check for angle

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,X,LLLX0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,X,RLLX0)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)
0,StoreAngle(ACCEL_TORSO,Z,TZ0)
0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_HIP,X,HX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,X,RULX0)
0,StoreAngle(ACCEL_RIGHT_UPPER_LEG,Z,RULZ0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,X,LULX0)
0,StoreAngle(ACCEL_LEFT_UPPER_LEG,Z,LULZ0)

#Stage 1: LEAN LEFT 
#both ankles lean ankle until LLLZ is 0, thrust=7
#alt: left ankle turns until LLLZ=-1 LLLZ starts at 3, important to limit thrust
#possibly 2 motors is the reason stage 1 lasts for a few ms after the target range is reached
#1,MotorAccel(MOTOR_LEFT_SIDE,MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,WAIT_STAGE|DOUBLE_THRUST|CONSTANT_THRUST)
1,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-3.5,-1,1,WAIT_STAGE|DOUBLE_THRUST|CONSTANT_THRUST)
1,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-3.5,-1,1,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|CONSTANT_THRUST)
1,MotorAccel(MOTOR_LEFT_ANKLE,3,5000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,-3.5,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3|MAX_REVERSE_THRUST=8|MAX_VELOCITY=5|FUTURE_LOG|MAINTAIN_THRUST)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,-3.5,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3)
#feet, knee, leg, and torso motors keep torso from falling forward or backward
#torso holds torso x until stage 4- note LAST_STAGE=4 needs 10s timeout, torso should not be using a lot of thrust, only small adjustments
#1,MotorAccel(MOTOR_TORSO,3,10000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=3|LAST_STAGE=4|MAX_VELOCITY=20)
1,MotorAccel(MOTOR_TORSO,3,5000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=3|MAINTAIN_THRUST|MAX_VELOCITY=20)
#knees make sure robot does not fall forward
1,MotorAccel(MOTOR_LEFT_KNEE,3,5000,ACCEL_LEFT_UPPER_LEG,X,LULX0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAINTAIN_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2)
1,MotorAccel(MOTOR_RIGHT_KNEE,3,5000,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAINTAIN_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2)



#user now moves motor 4 to lift right leg while script balances lower left

#stage 2: LEFT_SIDE reverses, to make torso Z straighten, and put torso weight over left leg, can be more positive than 1deg
#2,MotorAccel(MOTOR_LEFT_SIDE,6,3000,ACCEL_TORSO,Z,1,-1,4,WAIT_STAGE|MAX_THRUST=7)
#inst lasts until stage 4 (balance with foot in air) Roll degrees 1 to -5
#2,MotorAccel(MOTOR_LEFT_SIDE,6,3000,ACCEL_TORSO,Z,-2,-2,1,WAIT_STAGE|MAX_THRUST=9|ABORT_ON_TIMEOUT) -5 to -2  -7 to -3
#2,MotorAccel(MOTOR_LEFT_SIDE,8,2000,ACCEL_LEFT_LOWER_LEG,Z,-4,-2,1,ACCEL_TORSO,Z,-4,-3,1,WAIT_STAGE|MAX_THRUST=10|MAINTAIN_THRUST|ABORT_ON_TIMEOUT)
#note right side is just holding the initial angle
#2,MotorAccel(MOTOR_RIGHT_SIDE,2,2000,ACCEL_RIGHT_UPPER_LEG,Z,CurrentAngle,-4,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=10|MAINTAIN_THRUST)

#ABORT_ANGLE0=5 - end script if ankle Z turns past 5 degrees was -6
#  ***Important for the Left Ankle to have a large negative LLLZ range, because the side will make LLLZ positive and if the ankle
# is thrusting in that direction- it builds up too much velocity
2,MotorAccel(MOTOR_LEFT_ANKLE,3,20000,ACCEL_LEFT_FOOT,Z,0,-10,10,ACCEL_LEFT_LOWER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=14|MAX_REVERSE_THRUST=14|MAX_VELOCITY=5|FUTURE_LOG|ABORT_ANGLE0=15)




22000,StopLogging

