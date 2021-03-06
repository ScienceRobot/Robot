#Squat_18cm_Full_04.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)


0,StoreAngle(ACCEL_TORSO,X,TX0)
0,StoreAngle(ACCEL_LEFT_LOWER_LEG,Z,LLLZ0)
0,StoreAngle(ACCEL_RIGHT_LOWER_LEG,Z,RLLZ0)

#presumes legs are 18cm apart
#lower legs are around -16 degrees
#(upper legs are around 9 degrees)
#torso at -3,-2


#leg motors holds torso x
1,MotorAngle(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,2,12000,ACCEL_TORSO,X,-12,-1,1,WAIT_STAGE|DOUBLE_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1)

#torso motor also holds torso x
#1,MotorAngle(MOTOR_TORSO,4,12000,ACCEL_TORSO,X,-12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=5|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)


#knees slowly bend upper leg X from 6 to 28 degrees
#1,MotorAngle(MOTOR_LEFT_KNEE,2,12000,ACCEL_LEFT_UPPER_LEG,X,20,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
#1,MotorAngle(MOTOR_RIGHT_KNEE,2,12000,ACCEL_RIGHT_UPPER_LEG,X,20,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
1,MotorAngle(MOTOR_LEFT_KNEE,2,12000,ACCEL_LEFT_UPPER_LEG,X,20,-1,1,ACCEL_RIGHT_UPPER_LEG,X,20,-1,1,WAIT_STAGE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MIN_VELOCITY=5|SYNC_MAX_SPREAD=0.5|MAX_ACCEL=1|FUTURE_LOG)
1,MotorAngle(MOTOR_RIGHT_KNEE,2,12000,ACCEL_RIGHT_UPPER_LEG,X,20,-1,1,ACCEL_LEFT_UPPER_LEG,X,20,-1,1,WAIT_STAGE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MIN_VELOCITY=5|SYNC_MAX_SPREAD=0.5|MAX_ACCEL=1|FUTURE_LOG)

#feet slowly bend lower leg X  -12 to -40 degrees
#1,MotorAngle(MOTOR_LEFT_FOOT,2,12000,ACCEL_LEFT_LOWER_LEG,X,-30,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
#1,MotorAngle(MOTOR_RIGHT_FOOT,2,12000,ACCEL_RIGHT_LOWER_LEG,X,-30,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
1,MotorAngle(MOTOR_LEFT_FOOT,2,12000,ACCEL_LEFT_LOWER_LEG,X,-30,-1,1,ACCEL_RIGHT_LOWER_LEG,X,-30,-1,1,WAIT_STAGE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5)
1,MotorAngle(MOTOR_RIGHT_FOOT,2,12000,ACCEL_RIGHT_LOWER_LEG,X,-30,-1,1,ACCEL_LEFT_LOWER_LEG,X,-30,-1,1,WAIT_STAGE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5)


#probably Z imbalance should be resolved with X motors (knees in particular), but it can be alleviated somewhat with Z motors (mainly ankle motors)
#1,MotorAngle(MOTOR_LEFT_ANKLE,3,12000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,3,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)
#1,MotorAngle(MOTOR_RIGHT_ANKLE,3,12000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,-3,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)


16000,StopLogging

