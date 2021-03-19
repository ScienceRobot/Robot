#Test01.sh - test touchj sensor inst
0,StartLogging 
0,LogScriptFileName
0,StoreAngle(ACCEL_TORSO,X,TX0)

##Stage 1: LEAN LEFT - needs to be somewhat fast, and leaves a little momentum
##just to make sure robot is leaning at ankle before large thrust of side motors- otherwise side motors turn strongly and robot does not lean- left side rack rises up- but robot ankles have not moved
#not having |NO_STOP_ON_TARGET on side motors does make a difference
1,MotorAccel(MOTOR_LEFT_SIDE,3,3000,ACCEL_LEFT_UPPER_LEG,Z,-4,-1,1,WAIT_STAGE|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1|NO_STOP_ON_TARGET)
1,MotorAccel(MOTOR_RIGHT_SIDE,3,3000,ACCEL_LEFT_UPPER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_THRUST=3|MAX_REVERSE_THRUST=5|MAX_ACCEL=0.1|NO_STOP_ON_TARGET)
#set ankle motor to target a farther angle, because then it will likely still have some velocity when stage 1 ends 
1,MotorAccel(MOTOR_LEFT_ANKLE,1,3000,ACCEL_LEFT_FOOT,Z,0,-10,5,ACCEL_LEFT_UPPER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=3|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|NO_STOP_ON_TARGET|FUTURE_LOG)
1,MotorAccel(MOTOR_RIGHT_ANKLE,3,3000,ACCEL_RIGHT_FOOT,Z,0,-10,5,ACCEL_LEFT_UPPER_LEG,Z,-4,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=3|MIN_VELOCITY=5|MAX_VELOCITY=8|MAX_ACCEL=0.1|NO_STOP_ON_TARGET)
1,MotorAccel(MOTOR_TORSO,1,3000,ACCEL_TORSO,X,TX0,-2,2,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)
1,TouchSensor(TOUCH_RIGHT_FOOT_FRONT,GT,4,3000)


3000,StopLogging
