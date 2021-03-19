#NodHead06.sh - same as NodHead04b.sh but using relative angle
0,StartLogging
#Turn Head motor until Head X has -37 degree with Torso X (note that Head X will be around -46 degrees, and Torso around -9 degrees relative to ground)
0,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
1,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,-2,2000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)
2,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,-37,-5,5,WAIT_STAGE)
3,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_HEAD,-2,2000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,0,-1,1,WAIT_STAGE)


4,StopLogging
