#Calibrate_Accelerometers_0_degrees_01.sh
#Robot should be hanging with torso level before running this script.
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#head
0,HoldAngleWithMotor(ACCEL_HEAD,X,0,-0.2,0.2,MOTOR_HEAD,5,5000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_HEAD,X,0,-0.2,0.2,MOTOR_HEAD,5,5000,WAIT_STAGE)

#hold upper legs X with leg motors  
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,0,-0.2,0.2,MOTOR_LEFT_LEG,5,5000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,0,-0.2,0.2,MOTOR_RIGHT_LEG,5,5000,WAIT_STAGE)
#hold upper legs Z with side motors 
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,Z,0,-0.2,0.2,MOTOR_LEFT_SIDE,5,5000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,Z,0,-0.2,0.2,MOTOR_RIGHT_SIDE,5,5000,WAIT_STAGE)

#hold lower legs X with knee motors  
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,0,-0.2,0.2,MOTOR_LEFT_KNEE,5,5000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,0,-0.2,0.2,MOTOR_RIGHT_KNEE,5,5000,WAIT_STAGE)

#hold feet X with foot motors  
0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,X,0,-0.2,0.2,MOTOR_LEFT_FOOT,5,5000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-0.2,0.2,MOTOR_RIGHT_FOOT,5,5000,WAIT_STAGE)
#hold feet Z with ankle motors  
0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,X,0,-0.2,0.2,MOTOR_LEFT_ANKLE,5,5000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-0.2,0.2,MOTOR_RIGHT_ANKLE,5,5000,WAIT_STAGE)

4,StopLogging

