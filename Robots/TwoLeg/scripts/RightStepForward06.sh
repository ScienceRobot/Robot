#RightStepForward06.sh
#for 24rpm feet, ankle, knee, leg, side and torso motors
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#***legs spread 12cm from left foot to right foot center*** - otherwise stage 2 does not work

#constant thrust step right
#no thrust analysis, but does check for angle

0,StoreAngle(ACCEL_LEFT_LOWER_LEG,X,LLLX0)  #-11.8
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

##Stage 1: LEAN LEFT - needs to be somewhat fast, and leaves a little momentum
##just to make sure robot is leaning at ankle before large thrust of side motors- otherwise side motors turn strongly and robot does not lean- left side rack rises up- but robot ankles have not moved
#not having |NO_STOP_ON_TARGET on side motors does make a difference
#stage 1 requires: Accel Left Leg lower and upper leg z to reach target zone
1,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-5,-4,1,ACCEL_LEFT_UPPER_LEG,Z,-3,-5,1,WAIT_STAGE|MIN_VELOCITY=5|MAX_VELOCITY=7|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1|MAINTAIN_THRUST|ABORT_ON_TIMEOUT)
1,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-5,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-3,-5,1,WAIT_STAGE|HOLD_ANGLE|MIN_VELOCITY=5|MAX_VELOCITY=7|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1)
#|MAINTAIN_THRUST
#set ankle motor to target a farther angle, because then it will likely still have some velocity when stage 1 ends 
#1,MotorAccel(MOTOR_LEFT_ANKLE,1,5000,ACCEL_LEFT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|FUTURE_LOG|MAINTAIN_THRUST)
#1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|MAINTAIN_THRUST)
1,MotorAccel(MOTOR_LEFT_ANKLE,1,5000,ACCEL_LEFT_LOWER_LEG,Z,-5,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-2,-2,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|FUTURE_LOG|MAINTAIN_THRUST)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_LEFT_LOWER_LEG,Z,-5,-2,1,ACCEL_LEFT_UPPER_LEG,Z,-2,-2,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|MAINTAIN_THRUST)
1,MotorAccel(MOTOR_TORSO,1,5000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE=1:1|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAINTAIN_THRUST|MAX_VELOCITY=10|MIN_ACCEL=1)
#NO_STOP_ON_TARGET|


#stage 2: LEFT_SIDE reverses, to make torso Z straighten, and put torso weight over left leg, can be more positive than 1deg
#2,MotorAccel(MOTOR_LEFT_SIDE,4,4000,ACCEL_LEFT_LOWER_LEG,Z,-7,-2,0.5,ACCEL_TORSO,Z,-5,-1,1,WAIT_STAGE|MAX_THRUST0=4|MAX_REVERSE_THRUST0=4|MIN_VELOCITY0=5|MAX_VELOCITY0=8|MAX_THRUST1=10|MAX_REVERSE_THRUST1=4|MIN_VELOCITY1=20|MAX_VELOCITY1=40|MAX_ACCEL0=0.1|MAX_ACCEL1=5|FUTURE_LOG|NO_STOP_ON_TARGET|ABORT_ON_TIMEOUT)
#sometimes the LULZ is apparently reaching even nearly -8, so make sure stage can end if that happens
2,MotorAccel(MOTOR_LEFT_SIDE,4,4000,ACCEL_LEFT_UPPER_LEG,Z,-2,-6,1,ACCEL_TORSO,Z,-5,-1,1,WAIT_STAGE|MAX_THRUST0=4|MAX_REVERSE_THRUST0=4|MIN_VELOCITY0=5|MAX_VELOCITY0=8|MAX_THRUST1=11|MAX_REVERSE_THRUST1=4|MIN_VELOCITY1=20|MAX_VELOCITY1=50|MAX_ACCEL0=0.1|MAX_ACCEL1=5|FUTURE_LOG|NO_STOP_ON_TARGET|ABORT_ON_TIMEOUT)
#important note: right side motor picks up momentum in stage 2, but needs to reverse for right leg lift (stage 3), 
2,MotorAccel(MOTOR_LEFT_ANKLE,4,4000,ACCEL_LEFT_LOWER_LEG,Z,-5,-1,0.5,ACCEL_LEFT_UPPER_LEG,Z,-3,-3,1,WAIT_STAGE|MAX_THRUST=6|MAX_REVERSE_THRUST=6|MIN_VELOCITY=5|MAX_VELOCITY=8|ABORT_ANGLE0=30|HOLD_ANGLE=20:5|MAX_ACCEL1=0.1|NO_STOP_ON_TARGET|MAINTAIN_THRUST|FUTURE_LOG)
#2,MotorAccel(MOTOR_RIGHT_ANKLE,4,4000,ACCEL_RIGHT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-7.0,-1,0.5,WAIT_STAGE|HOLD_ANGLE=20:5|MAX_THRUST=6|MAX_REVERSE_THRUST=6|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL1=0.1|NO_STOP_ON_TARGET)
2,MotorAccel(MOTOR_RIGHT_ANKLE,4,4000,ACCEL_LEFT_LOWER_LEG,Z,-5,-1,0.5,ACCEL_LEFT_UPPER_LEG,Z,-3,-3,1,WAIT_STAGE|HOLD_ANGLE=20:5|MAX_THRUST=6|MAX_REVERSE_THRUST=6|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL1=0.1|NO_STOP_ON_TARGET)
#|NO_STOP_ON_TARGET
#feet hold up lower legs
2,MotorAccel(MOTOR_LEFT_FOOT,3,4000,ACCEL_LEFT_LOWER_LEG,X,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE=0.25:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MAX_VELOCITY=10)
2,MotorAccel(MOTOR_RIGHT_FOOT,3,4000,ACCEL_RIGHT_LOWER_LEG,X,CurrentAngle,-1,1,WAIT_STAGE|HOLD_ANGLE=0.25:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MAX_VELOCITY=10)
#2,MotorAccel(MOTOR_TORSO,3,4000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE=1:1|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAINTAIN_THRUST|MAX_VELOCITY=10|MIN_ACCEL=1)
#push forward from -12 (TX0) to -14
2,MotorAccel(MOTOR_TORSO,3,4000,ACCEL_TORSO,X,-14,-2,2,WAIT_STAGE|HOLD_ANGLE=1:1|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAINTAIN_THRUST|MAX_VELOCITY=10|MIN_ACCEL=1)
#OPPOSE_MOTION=0.25:0.75

#Stage 3: LIFT RIGHT LEG only (no knee or foot movement) 
#(LEFT FOOT MOTOR HOLDS ROBOT FROM FALLING FORWARD)
#todo: allow MAINTAIN_THRUST flag to work on MOTOR_RIGHT_LEG  +3 because can skip from 19 to 21
#was angle=16
#important: changing these 3 20s to 18s can cause the EthMotors PCB (presumably PIC) to stop communication, presumably from EMF, and noise can be seen in the Touch sensor lines.
3,MotorAccel(MOTOR_RIGHT_LEG,20,600,ACCEL_RIGHT_UPPER_LEG,X,15,-1,5,WAIT_STAGE|CONSTANT_THRUST)
3,MotorAccel(MOTOR_LEFT_LEG,20,600,ACCEL_RIGHT_UPPER_LEG,X,15,-1,5,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
#bend right knee so foot lifts in place - decrease angle X (lower leg rotates forward)
#14=too much momentum into stage 4
3,MotorAccel(MOTOR_RIGHT_KNEE,20,600,ACCEL_RIGHT_LOWER_LEG,X,-20,-1,1,WAIT_STAGE|CONSTANT_THRUST|HOLD_ANGLE)
3,MotorAccel(MOTOR_LEFT_KNEE,3,600,ACCEL_LEFT_UPPER_LEG,X,LULX0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=9|MAX_REVERSE_THRUST=6|MAINTAIN_THRUST|ABORT_ANGLE0=25)
#TODO: *** MAINTAIN_THRUST is not working here ***
#3,MotorAccel(MOTOR_LEFT_ANKLE,12,600,ACCEL_LEFT_LOWER_LEG,Z,-7,-1,0,WAIT_STAGE|HOLD_ANGLE=20.0:2.0|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAINTAIN_THRUST|ABORT_ANGLE0=30|FUTURE_LOG)
#MAX_THRUST for 9rpm was=20
3,MotorAccel(MOTOR_LEFT_ANKLE,12,600,ACCEL_LEFT_FOOT,Z,0,-10,15,ACCEL_LEFT_LOWER_LEG,Z,-7,-2,0,WAIT_STAGE|HOLD_ANGLE=10.0:2.0|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=0.5|MAINTAIN_THRUST|ABORT_ANGLE0=30|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET|FUTURE_LOG)
#3,MotorAccel(MOTOR_LEFT_ANKLE,12,600,ACCEL_LEFT_FOOT,Z,0,-10,20,ACCEL_LEFT_LOWER_LEG,Z,-8,-1,1,WAIT_STAGE|HOLD_ANGLE=20.0:2.0|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=0.5|MAINTAIN_THRUST|CONSTANT_THRUST=2:20|ABORT_ANGLE0=30|FUTURE_LOG)
#right ankle needs to repeat stage 2 inst because the right foot is still on the ground for a long time in stage 3
3,MotorAccel(MOTOR_RIGHT_ANKLE,3,600,ACCEL_RIGHT_FOOT,Z,0,-10,5,ACCEL_LEFT_LOWER_LEG,Z,-7,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=0.5)
#3,MotorAccel(MOTOR_LEFT_SIDE,20,600,ACCEL_TORSO,Z,-8,-1,1,ACCEL_LEFT_LOWER_LEG,Z,-13,-1,1,WAIT_STAGE|HOLD_ANGLE=40:0|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MIN_VELOCITY=5|MAX_VELOCITY=30|MAINTAIN_THRUST|NO_STOP_ON_TARGET|FUTURE_LOG)
#pretty much there is not a lot of way to save the ankle one the ankle starts falling
#ACCEL_LEFT_LOWER_LEG,Z,-6,-1,6,
#it seems likely that RSM should be no higher MAX_THRUST than LSM in stages 3 and 4 because otherwise it seems that the robot falls to the right as the RSM is using more thrust than the LSM
3,MotorAccel(MOTOR_LEFT_SIDE,12,600,ACCEL_LEFT_LOWER_LEG,Z,-7,-2,7,ACCEL_TORSO,Z,-8,-1,1,WAIT_STAGE|HOLD_ANGLE=40:0|MAX_THRUST=20|MAX_REVERSE_THRUST=2|MIN_VELOCITY=20|MAX_VELOCITY=50|MAX_ACCEL=5.0|MAINTAIN_THRUST|NO_STOP_ON_TARGET|FUTURE_LOG)
#3,MotorAccel(MOTOR_RIGHT_SIDE,13,600,ACCEL_RIGHT_UPPER_LEG,Z,-10,-1,1,ACCEL_LEFT_UPPER_LEG,Z,0,0,0,WAIT_STAGE|HOLD_ANGLE=20:0|MAX_THRUST=13|MAX_REVERSE_THRUST=2|MIN_VELOCITY=5|MAX_VELOCITY=40|MAX_ACCEL=3.0|NO_STOP_ON_TARGET|RELATIVE_ANGLE|MAINTAIN_THRUST)
#was -7
3,MotorAccel(MOTOR_RIGHT_SIDE,13,600,ACCEL_RIGHT_UPPER_LEG,Z,-8,-1,1,ACCEL_LEFT_UPPER_LEG,Z,0,0,0,WAIT_STAGE|HOLD_ANGLE=20:0|MAX_THRUST=13|MAX_REVERSE_THRUST=2|MIN_VELOCITY=5|MAX_VELOCITY=40|MAX_ACCEL=3.0|NO_STOP_ON_TARGET|RELATIVE_ANGLE|MAINTAIN_THRUST)
#send LLL forward slightly to stop fall backwards -12 to -14
#3,MotorAccel(MOTOR_LEFT_FOOT,5,600,ACCEL_LEFT_FOOT,X,0,-10,10,ACCEL_LEFT_LOWER_LEG,X,LLLX0,-1,1,WAIT_STAGE|HOLD_ANGLE=2:2|ABORT_ANGLE1=-30|MAX_THRUST=16|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20)
#send LLLX forward from -12 to -16
3,MotorAccel(MOTOR_LEFT_FOOT,5,600,ACCEL_LEFT_FOOT,X,0,-10,10,ACCEL_LEFT_LOWER_LEG,X,-16,-1,1,WAIT_STAGE|HOLD_ANGLE=2:2|ABORT_ANGLE1=-30|MAX_THRUST=16|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20)
#3,MotorAccel(MOTOR_LEFT_FOOT,5,600,ACCEL_LEFT_FOOT,X,0,-10,10,ACCEL_LEFT_LOWER_LEG,X,-14,-1,1,WAIT_STAGE|HOLD_ANGLE=2:2|ABORT_ANGLE1=-30|MAX_THRUST=16|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20)
#right foot should move RFX to 0 to decrease friction with the floor from the front of the right foot.
#rt foot is still on ground for part of stage 3- so better to have little to no movement
3,MotorAccel(MOTOR_RIGHT_FOOT,10,600,ACCEL_RIGHT_FOOT,X,0,-5,5,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=12|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)
#Torso needs to lean forward 
#use low TVR because MaxThrust=4 and <20 degrees not a lot of thrust is needed
#3,MotorAccel(MOTOR_TORSO,3,600,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.25:0.25|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MIN_VELOCITY=3|MAX_VELOCITY=5|MAX_ACCEL=1|NO_MOTION_NO_THRUST|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET|MAINTAIN_THRUST)
3,MotorAccel(MOTOR_TORSO,3,600,ACCEL_TORSO,X,-14,-2,2,WAIT_STAGE|HOLD_ANGLE=0.25:1|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=3|MAX_VELOCITY=5|MAX_ACCEL=0.5|NO_MOTION_NO_THRUST|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET|MAINTAIN_THRUST)


#Stage 4: LOWER RIGHT LEG SWINGS BACK, UPPER LEG COMES BACK DOWN, left side holds right leg off the ground 
#***note no HOLD_ANGLE***
#end stage if Right foot front or back touch sensors > 3
#required to end stage 4: RULX reaches RULX0 +-1 or Right foot front or back Touch Sensor > 3
4,TouchSensor(TOUCH_RIGHT_FOOT_FRONT,GT,3.0,250,400)
4,TouchSensor(TOUCH_RIGHT_FOOT_BACK,GT,3.0,250,400)
#4,MotorAccel(MOTOR_RIGHT_LEG,20,400,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-1,1,WAIT_STAGE|CONSTANT_THRUST)
4,MotorAccel(MOTOR_RIGHT_LEG,16,400,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-1,1,WAIT_STAGE|CONSTANT_THRUST)
4,MotorAccel(MOTOR_LEFT_LEG,16,400,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-1,1,WAIT_STAGE|HOLD_ANGLE|CONSTANT_THRUST)
#right knee swings forward - needs HOLD_ANGLE upper leg motor can stop stage
#target angle was RLLX0 (-12), but needs to be relative angle (RULX-18  so when RULX=21 LULX target will be +3 and work down to -12
#4,MotorAccel(MOTOR_RIGHT_KNEE,17,700,ACCEL_RIGHT_LOWER_LEG,X,3,0,0,WAIT_STAGE|CONSTANT_THRUST=20:5|HOLD_ANGLE|ABORT_ANGLE0=-30)
#-18
4,MotorAccel(MOTOR_RIGHT_KNEE,20,400,ACCEL_RIGHT_LOWER_LEG,X,0,-1,1,ACCEL_RIGHT_UPPER_LEG,X,0,0,0,WAIT_STAGE|HOLD_ANGLE|ABORT_ANGLE0=-40|MAX_ACCEL=10|MIN_VELOCITY=30|MAX_VELOCITY=50|MAX_THRUST=20|CONSTANT_THRUST20:4|RELATIVE_ANGLE)
4,MotorAccel(MOTOR_LEFT_ANKLE,3,400,ACCEL_LEFT_FOOT,Z,0,-10,20,ACCEL_LEFT_LOWER_LEG,Z,-7,-1,1,WAIT_STAGE|HOLD_ANGLE=10:2|MAX_THRUST=20|MAX_REVERSE_THRUST=2|ABORT_ANGLE0=40|MAINTAIN_THRUST)
#try a controlled lean back to the right, will need to be biased against gravity and the above weight of the robot
#4,MotorAccel(MOTOR_LEFT_ANKLE,3,400,ACCEL_LEFT_FOOT,Z,0,-10,20,ACCEL_LEFT_LOWER_LEG,Z,LLLZ0,-1,1,WAIT_STAGE|HOLD_ANGLE=10:2|MAX_THRUST=20|MAX_REVERSE_THRUST=2|ABORT_ANGLE0=40|MIN_VELOCITY1=2|MAX_VELOCITY1=5|MAX_ACCEL1=0.1|MAINTAIN_THRUST)
#try RELATIVE_ANGLE, +6 (LULZ0=3 and RULZ0=-3)
4,MotorAccel(MOTOR_LEFT_SIDE,6,400,ACCEL_LEFT_UPPER_LEG,Z,8,-1,1,ACCEL_RIGHT_UPPER_LEG,Z,0,0,0,WAIT_STAGE|HOLD_ANGLE=10:0|MAX_THRUST=10|MAX_REVERSE_THRUST=2|MAINTAIN_THRUST|RELATIVE_ANGLE|FUTURE_LOG)
#was -12, -3 is RLLZ0, -6 is correct relative angle and does work well, but -8 will send a little bit farther
#4,MotorAccel(MOTOR_RIGHT_SIDE,10,400,ACCEL_RIGHT_UPPER_LEG,Z,-6,-1,1,WAIT_STAGE|HOLD_ANGLE=10:1|MAX_THRUST=8|MAX_REVERSE_THRUST=2)
4,MotorAccel(MOTOR_RIGHT_SIDE,10,400,ACCEL_RIGHT_UPPER_LEG,Z,-8,-1,1,ACCEL_LEFT_UPPER_LEG,Z,0,0,0,WAIT_STAGE|HOLD_ANGLE=10:2|MAX_THRUST=10|MAX_REVERSE_THRUST=2|MAX_ACCEL=1|RELATIVE_ANGLE)
#|CONSTANT_THRUST=20:4
#use 2:2 because can fall backwards or forwards and needs more thrust
#LLLX0=-12
4,MotorAccel(MOTOR_LEFT_FOOT,10,400,ACCEL_LEFT_FOOT,X,0,-10,10,ACCEL_LEFT_LOWER_LEG,X,-16,-1,1,WAIT_STAGE|HOLD_ANGLE=2:2|MAX_THRUST=16|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20)
#LULX0=6 -> 0
4,MotorAccel(MOTOR_LEFT_KNEE,10,400,ACCEL_LEFT_UPPER_LEG,X,0,-2,2,ACCEL_HIP,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=12|MAX_REVERSE_THRUST=6|MAINTAIN_THRUST|ABORT_ANGLE0=30)
#right ankle and right foot motors should straighten right foot
4,MotorAccel(MOTOR_RIGHT_ANKLE,10,400,ACCEL_RIGHT_FOOT,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE=1:1|MAX_THRUST=18|MAX_REVERSE_THRUST=6)
4,MotorAccel(MOTOR_RIGHT_FOOT,10,400,ACCEL_RIGHT_FOOT,X,0,-2,2,WAIT_STAGE|HOLD_ANGLE=1:1|MAX_THRUST=12|MAX_REVERSE_THRUST=6)
#TX0=-12
4,MotorAccel(MOTOR_TORSO,3,400,ACCEL_TORSO,X,-14,-2,2,WAIT_STAGE|HOLD_ANGLE=0.25:1|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=3|MAX_VELOCITY=5|MAX_ACCEL=0.5|NO_MOTION_NO_THRUST|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET|MAINTAIN_THRUST)


#Stage 5: BALANCE stand for 3 seconds
#(Basically try to lean back to the center and hold those angles)
#5,MotorAccel(MOTOR_LEFT_SIDE,3,3000,ACCEL_LEFT_UPPER_LEG,Z,LULZ0,-1,1,ACCEL_TORSO,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3)
#5,MotorAccel(MOTOR_RIGHT_SIDE,3,3000,ACCEL_RIGHT_UPPER_LEG,Z,RULZ0,-1,1,ACCEL_TORSO,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3)
#|USE_ACCEL_WITH_MOST_V - will work on HIP Z if leg Z has less velocity
#NO_MOTION_NO_THRUST0- if upper leg Z is not moving, then probably it is in a stable orientation, so do not thrust any more
#LULX=3
#5,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_UPPER_LEG,Z,LULZ0,-3,1,ACCEL_HIP,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MIN_VELOCITY=10|MAX_VELOCITY=15|MAX_ACCEL=1|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST)
#left side motor needs to push up left leg on forceful return to left side
5,MotorAccel(MOTOR_LEFT_SIDE,3,5000,ACCEL_LEFT_UPPER_LEG,Z,10,-1,1,ACCEL_RIGHT_UPPER_LEG,Z,0,0,0,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MIN_VELOCITY=10|MAX_VELOCITY=15|MAX_ACCEL=1|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST|RELATIVE_ANGLE)
#5,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_LEFT_UPPER_LEG,Z,LULZ0,-3,3,ACCEL_HIP,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MIN_VELOCITY=10|MAX_VELOCITY=15|MAX_ACCEL=1|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST)
5,MotorAccel(MOTOR_RIGHT_SIDE,3,5000,ACCEL_RIGHT_UPPER_LEG,Z,RULZ0,-3,3,ACCEL_HIP,Z,0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=7|MAX_REVERSE_THRUST=3|MIN_VELOCITY=10|MAX_VELOCITY=15|MAX_ACCEL=1|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST)
#the legs need to push back the weight of the torso in the +X
#perhaps legs need to use Torso X but Hipx is less variable 
5,MotorAccel(MOTOR_LEFT_LEG,3,5000,ACCEL_HIP,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=6|MAX_REVERSE_THRUST=3|MAX_VELOCITY=40|MAX_ACCEL=1)
5,MotorAccel(MOTOR_RIGHT_LEG,3,5000,ACCEL_HIP,X,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST=6|MAX_REVERSE_THRUST=3|MAX_VELOCITY=40|MAX_ACCEL=1)
#5,MotorAccel(MOTOR_RIGHT_KNEE,16,3000,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-1,1,ACCEL_HIP,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=8|MAX_REVERSE_THRUST0=8|MAX_THRUST1=3|MAX_REVERSE_THRUST1=3|MAX_VELOCITY=20|ABORT_ANGLE0=40)  
#5,MotorAccel(MOTOR_LEFT_KNEE,16,3000,ACCEL_LEFT_UPPER_LEG,X,LULX0,-1,1,ACCEL_HIP,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=8|MAX_REVERSE_THRUST0=8|MAX_THRUST1=3|MAX_REVERSE_THRUST1=3|MAX_VELOCITY0=20|MAX_VELOCITY1=10)
#5,MotorAccel(MOTOR_RIGHT_KNEE,8,5000,ACCEL_HIP,Z,0,-3,3,ACCEL_RIGHT_UPPER_LEG,X,RULX0,-3,3,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=5|MAX_REVERSE_THRUST0=5|MAX_THRUST1=7|MAX_REVERSE_THRUST1=6|MAX_VELOCITY0=10|MAX_VELOCITY=10|MAX_ACCEL=0.5|ABORT_ANGLE0=40|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST)  
5,MotorAccel(MOTOR_RIGHT_KNEE,8,5000,ACCEL_HIP,Z,0,-3,3,ACCEL_HIP,X,0,-3,3,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=5|MAX_REVERSE_THRUST0=5|MAX_THRUST1=7|MAX_REVERSE_THRUST1=6|MAX_VELOCITY0=10|MAX_VELOCITY=10|MAX_ACCEL=0.5|ABORT_ANGLE0=40|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST)  
#5,MotorAccel(MOTOR_LEFT_KNEE,8,3000,ACCEL_HIP,Z,0,-1,1,ACCEL_LEFT_UPPER_LEG,X,LULX0,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST0=5|MAX_REVERSE_THRUST0=5|MAX_THRUST1=6|MAX_REVERSE_THRUST1=6|MAX_VELOCITY0=20|MAX_VELOCITY1=10|MAX_ACCEL=1.0|FUTURE_LOG)
#LULX0=6 -> 0
#5,MotorAccel(MOTOR_LEFT_KNEE,8,5000,ACCEL_HIP,Z,0,-3,3,ACCEL_LEFT_UPPER_LEG,X,0,-3,3,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=5|MAX_REVERSE_THRUST0=5|MAX_THRUST1=7|MAX_REVERSE_THRUST1=6|MAX_VELOCITY0=10|MAX_VELOCITY1=30|MAX_ACCEL=0.5|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST|FUTURE_LOG)
5,MotorAccel(MOTOR_LEFT_KNEE,8,5000,ACCEL_HIP,Z,0,-3,3,ACCEL_HIP,X,0,-3,3,WAIT_STAGE|HOLD_ANGLE=0.5:0.5|MAX_THRUST0=5|MAX_REVERSE_THRUST0=5|MAX_THRUST1=7|MAX_REVERSE_THRUST1=6|MAX_VELOCITY0=10|MAX_VELOCITY1=30|MAX_ACCEL=0.5|FARTHEST_FROM_TARGET|NO_MOTION_NO_THRUST|FUTURE_LOG)
#NO_MOTION_NO_THRUST|
5,MotorAccel(MOTOR_LEFT_ANKLE,3,5000,ACCEL_LEFT_FOOT,Z,0,-10,10,ACCEL_LEFT_LOWER_LEG,Z,LLLZ0,-3,3,WAIT_STAGE|HOLD_ANGLE=1:2|MAX_THRUST=14|MAX_REVERSE_THRUST=3|MAX_ACCEL=1|MIN_VELOCITY=20|MAX_VELOCITY=30|NO_MOTION_NO_THRUST1|ABORT_ANGLE0=40)
#RAM needs to push right side back to left RAM=- RULZ0=-3
#right foot Z can become >10 so set to 20 just to make sure right foot will be countering landing (which requires RAM-, RAM+ corrects RFZ>10) - need a high MinV and accel
5,MotorAccel(MOTOR_RIGHT_ANKLE,3,5000,ACCEL_RIGHT_FOOT,Z,0,-10,20,ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0,-3,3,WAIT_STAGE|HOLD_ANGLE=1:2|MAX_THRUST=14|MAX_REVERSE_THRUST=3|MAX_ACCEL=1|MIN_VELOCITY=20|MAX_VELOCITY=30|NO_MOTION_NO_THRUST1|ABORT_ANGLE0=30|FUTURE_LOG)
#LLLX0=-12
5,MotorAccel(MOTOR_LEFT_FOOT,10,5000,ACCEL_LEFT_FOOT,X,0,-10,10,ACCEL_LEFT_LOWER_LEG,X,-16,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:2|MAX_THRUST=11|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20|NO_MOTION_NO_THRUST1|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET)
5,MotorAccel(MOTOR_RIGHT_FOOT,10,5000,ACCEL_RIGHT_FOOT,X,0,-10,10,ACCEL_RIGHT_LOWER_LEG,X,RLLX0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.5:2|MAX_THRUST=10|MAX_REVERSE_THRUST=3|MAX_VELOCITY=20|NO_MOTION_NO_THRUST1|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET)
5,MotorAccel(MOTOR_TORSO,3,5000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE=0.2:0.2|MAX_THRUST=5|MAX_REVERSE_THRUST=5|MIN_VELOCITY=5|MAX_VELOCITY=10|MAX_ACCEL=0.5|NO_MOTION_NO_THRUST|OPPOSE_MOTION_IN_RANGE_MOVING_TO_TARGET|FUTURE_LOG)
#possibly right leg motor needs to hold up leg or possibly falls down

10000,StopLogging

