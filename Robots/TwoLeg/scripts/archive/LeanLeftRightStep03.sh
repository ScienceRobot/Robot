#LeanLeftRightStep03.sh
0,StartLogging 

#legs spread 22cm from center to center

#note too that DegreesPlus is to the right viewing robot 2 degrees or less
0,HoldAngleWithMotor(ETHACCELS_LEFT_LEG,2,2,6,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_LEFT_SIDE,5000,WAIT_STAGE)
#upper right leg x
#use right leg accel as reference -14 or less
#0,HoldAngleWithMotor(ETHACCELS_RIGHT_LEG,2,2,-10,-10,0,ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,5000,WAIT_STAGE)
0,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,1,5000,ETHACCELS_RIGHT_LEG,2,2,BETWEEN,-10,-10,0,WAIT_STAGE)


#x=8.8 is normal
#right leg
1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,1,1000,ETHACCELS_RIGHT_LEG,2,0,BETWEEN,11,0,10,WAIT_STAGE)
#right knee -13.2 regularly
1,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,1,1000,ETHACCELS_RIGHT_LEG,1,0,BETWEEN,-14,-10,0,WAIT_STAGE)

#right leg back
2,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,1,1000,ETHACCELS_RIGHT_LEG,2,0,BETWEEN,9,-10,0,WAIT_STAGE)
#right knee back
2,TurnMotorUntilAngle(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,1,1000,ETHACCELS_RIGHT_LEG,1,0,BETWEEN,-12,0,10,WAIT_STAGE)


4,StopLogging
