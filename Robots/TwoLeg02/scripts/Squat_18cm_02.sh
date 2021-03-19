#Squat_18cm_02.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)


0,StoreAngle(ACCEL_TORSO,X,TX0)

#presumes legs are 18cm apart
#lower legs are around -16 degrees
#(upper legs are around 9 degrees)
#torso at -3,-2


#leg motors holds torso x
1,MotorAccel(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,2,4000,ACCEL_TORSO,X,TX0,-1,1,WAIT_STAGE|HOLD_ANGLE|DOUBLE_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)

#knees slowly bend upper leg X from 6 to 28 degrees
1,MotorAccel(MOTOR_LEFT_KNEE,2,4000,ACCEL_LEFT_UPPER_LEG,X,12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
1,MotorAccel(MOTOR_RIGHT_KNEE,2,4000,ACCEL_RIGHT_UPPER_LEG,X,12,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)

#feet slowly bend lower leg X  -12 to -40 degrees
1,MotorAccel(MOTOR_LEFT_FOOT,2,4000,ACCEL_LEFT_LOWER_LEG,X,-20,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)
1,MotorAccel(MOTOR_RIGHT_FOOT,2,4000,ACCEL_RIGHT_LOWER_LEG,X,-20,-1,1,WAIT_STAGE|HOLD_ANGLE|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5)


8000,StopLogging

