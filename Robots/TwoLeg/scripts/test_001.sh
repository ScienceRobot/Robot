#Squat_18cm_Up_03.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)


0,StoreAngle(ACCEL_TORSO,X,TX0)

#presumes legs are 18cm apart
#lower legs are around -16 degrees
#(upper legs are around 9 degrees)
#torso at -3,-2


#leg motors holds torso x (-12)
#1,MotorAngle(MOTOR_LEFT_LEG,MOTOR_RIGHT_LEG,4,8000,ACCEL_TORSO,X,-12,-1,1,WAIT_STAGE|DOUBLE_THRUST|MAX_THRUST=4|MAX_REVERSE_THRUST=2|MAX_VELOCITY=5|MAX_ACCEL=1)


12000,StopLogging

