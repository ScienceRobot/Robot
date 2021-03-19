#Squat_18cm_Up_03.sh
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


#leg motors holds torso x (-12)
1,MotorAngle(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,4,8000,ACCEL_TORSO,X,-12,-1,1,WAIT_STAGE|DOUBLE_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1)

#torso motor also holds torso x (-12)
#1,MotorAngle(MOTOR_TORSO,4,8000,ACCEL_TORSO,X,-12,-1,1,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)

#knees slowly bend upper leg X from from 28 to 6 degrees
1,MotorAngle(MOTOR_LEFT_KNEE,2,8000,ACCEL_LEFT_UPPER_LEG,X,6,-1,1,ACCEL_RIGHT_UPPER_LEG,X,6,-1,1,WAIT_STAGE|MAX_THRUST=7|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MIN_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5|FUTURE_LOG)
1,MotorAngle(MOTOR_RIGHT_KNEE,2,8000,ACCEL_RIGHT_UPPER_LEG,X,6,-1,1,ACCEL_LEFT_UPPER_LEG,X,6,-1,1,WAIT_STAGE|MAX_THRUST=7|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MIN_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5|FUTURE_LOG)
#1,MotorAngle(MOTOR_LEFT_KNEE,MOTOR_RIGHT_KNEE,4,8000,ACCEL_LEFT_UPPER_LEG,X,6,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|DOUBLE_THRUST)

#feet slowly bend lower leg X  -40 to -12 degrees
#1,MotorAngle(MOTOR_LEFT_FOOT,2,8000,ACCEL_LEFT_LOWER_LEG,X,-12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
#1,MotorAngle(MOTOR_RIGHT_FOOT,2,8000,ACCEL_RIGHT_LOWER_LEG,X,-12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
1,MotorAngle(MOTOR_LEFT_FOOT,2,8000,ACCEL_LEFT_LOWER_LEG,X,-12,-1,1,ACCEL_RIGHT_LOWER_LEG,X,-12,-1,1,WAIT_STAGE|MAX_THRUST=8|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5)
1,MotorAngle(MOTOR_RIGHT_FOOT,2,8000,ACCEL_RIGHT_LOWER_LEG,X,-12,-1,1,ACCEL_LEFT_LOWER_LEG,X,-12,-1,1,WAIT_STAGE|MAX_THRUST=8|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1|SYNC_MAX_SPREAD=0.5)
#1,MotorAngle(MOTOR_LEFT_FOOT,MOTOR_RIGHT_FOOT,6,8000,ACCEL_LEFT_LOWER_LEG,X,-12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=6|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|DOUBLE_THRUST)

#probably Z imbalance should be resolved with X motors (knees in particular), but it can be alleviated somewhat with Z motors (mainly ankle motors)
#1,MotorAngle(MOTOR_LEFT_ANKLE,3,12000,ACCEL_LEFT_FOOT,Z,0,-5,5,ACCEL_LEFT_LOWER_LEG,Z,3,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)
#1,MotorAngle(MOTOR_RIGHT_ANKLE,3,12000,ACCEL_RIGHT_FOOT,Z,0,-5,5,ACCEL_RIGHT_LOWER_LEG,Z,-3,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=3|MAX_REVERSE_THRUST=3|MAX_VELOCITY=5)


12000,StopLogging
