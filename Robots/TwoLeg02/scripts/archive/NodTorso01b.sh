#NodTorso01b.sh 
0,StartLogging
0,LogScriptFileName
#0,MakeMotionLog(NodTorso01b)
#Turn Head motor until Head X has -37 degree with Torso X (note that Head X will be around -46 degrees, and Torso around -9 degrees relative to ground) with TargetVelocity=50deg/s
#0,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
#1,TurnMotorUntilAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,50,1000,ETHACCELS_TORSO,1,0,BETWEEN,0,-1,1,WAIT_STAGE)
#1,TurnMotorUntilAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,50,1000,ACCEL_TORSO,1,0,BETWEEN,0,-1,1,WAIT_STAGE)
#1,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,50,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)
#1,TurnMotorUntilRelativeAngle(MOTOR_HEAD,50,2000,ACCEL_HEAD,X,ACCEL_TORSO,X,0,-1,1,WAIT_STAGE)
#2,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
#3,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,-2,2000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)

#0,TurnMotorUntilAngle(MOTOR_HEAD,10,5000,ACCEL_HEAD,X,-9,-1,1,WAIT_STAGE)
0,HoldAngleWithMotor(ACCEL_TORSO,X,-10,0,15,MOTOR_LEFT_LEG,3,5000,WAIT_STAGE|MOTION_LOG|DOUBLE_THRUST|CONSTANT_THRUST)
0,HoldAngleWithMotor(ACCEL_TORSO,X,-10,0,15,MOTOR_RIGHT_LEG,3,5000,WAIT_STAGE|DOUBLE_THRUST|CONSTANT_THRUST)



3000,StopLogging
