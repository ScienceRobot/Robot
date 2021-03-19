#TestStages01.sh 
0,StartLogging
0,LogScriptFileName
#Turn Head motor until Head X has -37 degree with Torso X (note that Head X will be around -46 degrees, and Torso around -9 degrees relative to ground) with TargetSpeed=50deg/s
#0,TurnMotorUntilAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,50,2000,ETHACCELS_TORSO,1,0,BETWEEN,-37,-5,5,WAIT_STAGE)
#0,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,50,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
#nod head down
0,TurnMotorUntilRelativeAngle(MOTOR_HEAD,50,2000,ACCEL_HEAD,X,ACCEL_TORSO,X,-37,-5,5,WAIT_STAGE)
#tilt torso in +roll (+Z) using both side motors to 8 degrees
0,HoldAngleWithMotor(ACCEL_TORSO,Z,8,-1,1,MOTOR_LEFT_SIDE,2000,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,Z,8,-1,1,MOTOR_RIGHT_SIDE,2000,WAIT_STAGE)
#Hold torso X to -3 degrees
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-1,1,MOTOR_LEFT_LEG,2000,WAIT_STAGE)
#0,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-1,1,MOTOR_RIGHT_LEG,2000,WAIT_STAGE)

#stage 1
#turn head back up
2001,TurnMotorUntilRelativeAngle(MOTOR_HEAD,50,2000,ACCEL_HEAD,0,ACCEL_TORSO,0,0,-1,1,WAIT_STAGE)
#tilt torso in +roll (+Z) using both side motors to 0 degrees
2001,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-1,1,MOTOR_LEFT_SIDE,2000,WAIT_STAGE)
2001,HoldAngleWithMotor(ACCEL_TORSO,Z,0,-1,1,MOTOR_RIGHT_SIDE,2000,WAIT_STAGE)
#Hold torso X to -3 degrees
#1,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-1,1,MOTOR_LEFT_LEG,2000,WAIT_STAGE)
#1,HoldAngleWithMotor(ACCEL_TORSO,X,-3,-1,1,MOTOR_RIGHT_LEG,2000,WAIT_STAGE)

#1,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,-2,2000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)
#2,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
#3,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,-2,2000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)


2004,StopLogging
