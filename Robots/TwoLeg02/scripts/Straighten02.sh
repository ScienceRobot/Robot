#Straighten02.sh
0,StartLogging 
0,LogScriptFileName
#StartTimeInMS,HoldAngleWithMotor(AccelName,AccelAngleXYorZ,TargetAngle,DegreesPlus,DegreesMinus,MotorName,Dir*Strength,Duration

#first spread apart sides
#side
#0,MotorAccel(MOTOR_LEFT_SIDE,5,5000,ACCEL_LEFT_UPPER_LEG,Z,8,-1,1,WAIT_STAGE|HOLD_ANGLE)
#0,MotorAccel(MOTOR_RIGHT_SIDE,5,5000,ACCEL_RIGHT_UPPER_LEG,Z,-8,-1,1,WAIT_STAGE|HOLD_ANGLE)

#legs
1,MotorAccel(MOTOR_LEFT_LEG,5,5000,ACCEL_LEFT_UPPER_LEG,X,0,-0.5,0.5,ACCEL_HIP,X,0,0,0,WAIT_STAGE|HOLD_ANGLE|RELATIVE_ANGLE)
1,MotorAccel(MOTOR_RIGHT_LEG,5,5000,ACCEL_RIGHT_UPPER_LEG,X,0,-0.5,0.5,ACCEL_HIP,X,0,0,0,WAIT_STAGE|HOLD_ANGLE|RELATIVE_ANGLE)

#knees
2,MotorAccel(MOTOR_LEFT_KNEE,5,5000,ACCEL_LEFT_LOWER_LEG,X,-12,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
2,MotorAccel(MOTOR_RIGHT_KNEE,5,5000,ACCEL_RIGHT_LOWER_LEG,X,-12,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)

#side back together
3,MotorAccel(MOTOR_LEFT_SIDE,5,5000,ACCEL_LEFT_UPPER_LEG,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE)
3,MotorAccel(MOTOR_RIGHT_SIDE,5,5000,ACCEL_RIGHT_UPPER_LEG,Z,0,-1,1,WAIT_STAGE|HOLD_ANGLE)

#feet |ABORT_VELOCITY=40)
4,MotorAccel(MOTOR_LEFT_FOOT,5,5000,ACCEL_LEFT_FOOT,X,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
4,MotorAccel(MOTOR_RIGHT_FOOT,5,5000,ACCEL_RIGHT_FOOT,X,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#ankles
4,MotorAccel(MOTOR_LEFT_ANKLE,5,5000,ACCEL_LEFT_FOOT,Z,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
4,MotorAccel(MOTOR_RIGHT_ANKLE,5,5000,ACCEL_RIGHT_FOOT,Z,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)


#feet |ABORT_VELOCITY=40)
#0,MotorAccel(MOTOR_LEFT_FOOT,5,2000,ACCEL_LEFT_FOOT,X,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#0,MotorAccel(MOTOR_RIGHT_FOOT,5,2000,ACCEL_RIGHT_FOOT,X,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#ankles
#1,MotorAccel(MOTOR_LEFT_ANKLE,5,2000,ACCEL_LEFT_FOOT,Z,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#1,MotorAccel(MOTOR_RIGHT_ANKLE,5,2000,ACCEL_RIGHT_FOOT,Z,0,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#knees
#2,MotorAccel(MOTOR_LEFT_KNEE,5,2000,ACCEL_LEFT_LOWER_LEG,X,-12,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#2,MotorAccel(MOTOR_RIGHT_KNEE,5,2000,ACCEL_RIGHT_LOWER_LEG,X,-12,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#legs
#3,MotorAccel(MOTOR_LEFT_LEG,5,2000,ACCEL_LEFT_UPPER_LEG,X,6,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#3,MotorAccel(MOTOR_RIGHT_LEG,5,2000,ACCEL_RIGHT_UPPER_LEG,X,6,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#side
#4,MotorAccel(MOTOR_LEFT_SIDE,3,2000,ACCEL_LEFT_UPPER_LEG,Z,3,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)
#4,MotorAccel(MOTOR_RIGHT_SIDE,3,2000,ACCEL_RIGHT_UPPER_LEG,Z,-3,-0.5,0.5,WAIT_STAGE|HOLD_ANGLE)

#torso
#0,MotorAccel(MOTOR_LEFT_LEG,1,5000,ACCEL_TORSO,X,0,-0.5,0.5,WAIT_STAGE|DOUBLE_THRUST)
#0,MotorAccel(MOTOR_RIGHT_LEG,1,5000,ACCEL_TORSO,X,0,-0.5,0.5,WAIT_STAGE|DOUBLE_THRUST)


25100,StopLogging
