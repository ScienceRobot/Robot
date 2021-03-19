#TurnHeadRight02.sh
#Turn Head right to yaw=3 degrees relative to the torso 
0,StartLogging
1,TurnMotorUntilRelativeAngle(ETHMOTORS_UPPER_BODY,MOTOR_NECK,-2,1000,ETHACCELS_TORSO,1,0,ETHACCELS_TORSO,0,0,BETWEEN,5,-1,1,WAIT_STAGE)
#0,TurnMotor(ETHMOTORS_UPPER_BODY,MOTOR_NECK,2,500)
4,StopLogging
