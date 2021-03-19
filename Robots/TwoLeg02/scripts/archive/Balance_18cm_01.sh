#Balance_18cm_01.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration/TimeOut,flags)

#presumes legs are close together

#hold lower legs X with feet motors (-12), alt: use CurrentAngle  (if rt=-9 it will send body back, and torso will have to lean forward)
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,-12,-2,2,MOTOR_LEFT_FOOT,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,-12,-2,2,MOTOR_RIGHT_FOOT,5,3000,WAIT_STAGE)
#hold feet X with feet motors (0) (higher priority)
0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,X,0,-3,3,MOTOR_LEFT_FOOT,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,X,0,-3,3,MOTOR_RIGHT_FOOT,5,3000,WAIT_STAGE)


#hold a relative angle between upper and lower legs X with knee motors (-27 degrees), 
#0,HoldRelativeAngleWithMotor(ACCEL_LEFT_LOWER_LEG,X,ACCEL_LEFT_UPPER_LEG,X,-27,-2,2,MOTOR_LEFT_KNEE,5,3000,WAIT_STAGE)
#0,HoldRelativeAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,X,ACCEL_RIGHT_UPPER_LEG,X,-27,-2,2,MOTOR_RIGHT_KNEE,5,3000,WAIT_STAGE)
#hold upper legs X with knee motors (19), 
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,X,10,-2,2,MOTOR_LEFT_KNEE,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,X,10,-2,2,MOTOR_RIGHT_KNEE,5,3000,WAIT_STAGE)
#hold torso X with knee motors (-6),  (higher priority)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-6,-2,2,MOTOR_LEFT_KNEE,5,3000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-6,-2,2,MOTOR_RIGHT_KNEE,5,3000,WAIT_STAGE)



#hold torso X with leg motors (-5), 
0,HoldAngleWithMotor(ACCEL_TORSO,X,-5,-2,2,MOTOR_LEFT_LEG,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,-5,-2,2,MOTOR_RIGHT_LEG,5,3000,WAIT_STAGE)


#hold torso Z with side motors (-2)  - note would be usually Z=0 but the torso has an inherent tilt I think due to the hip 
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_LEFT_SIDE,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-3,3,MOTOR_RIGHT_SIDE,5,3000,WAIT_STAGE)
#hold upper leg Z with side motors (-2)  (higher priority) - otherwise legs go sideways and robot falls
0,HoldAngleWithMotor(ACCEL_LEFT_UPPER_LEG,Z,3,-2,2,MOTOR_LEFT_SIDE,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_UPPER_LEG,Z,-3,-2,2,MOTOR_RIGHT_SIDE,5,3000,WAIT_STAGE)


#hold lower leg Z with ankle motors (3 and -3), - note that this presumes that legs are 18cm apart (legs slightly spread apart)
0,HoldAngleWithMotor(ACCEL_LEFT_LOWER_LEG,Z,3,-3,3,MOTOR_LEFT_ANKLE,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_LOWER_LEG,Z,-3,-3,3,MOTOR_RIGHT_ANKLE,5,3000,WAIT_STAGE)
#hold feet Z with ankle motors (0) (higher priority)
0,HoldAngleWithMotor(ACCEL_LEFT_FOOT,Z,0,-3,3,MOTOR_LEFT_ANKLE,5,3000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_RIGHT_FOOT,Z,0,-3,3,MOTOR_RIGHT_ANKLE,5,3000,WAIT_STAGE)

4,StopLogging

