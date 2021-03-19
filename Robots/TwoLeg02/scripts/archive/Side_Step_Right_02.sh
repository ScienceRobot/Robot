#Side_Step_Right_02.sh
0,StartLogging
#0,StartAccelerometer(ETHACCELS_LEFT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_RIGHT_LEG,7,POLLING)
#0,StartAccelerometer(ETHACCELS_TORSO,3,POLLING)
#0,StartTouchSensor(ETHACCELS_LEFT_LEG,0x3,POLLING)
#0,StartTouchSensor(ETHACCELS_RIGHT_LEG,0x3,POLLING)
#side step takes about 1 second
#stage 1: right knee bends, right leg lifts
#stage 2: side step motor turns
#stage 3: right knee unbends, right leg goes back down
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,6,300) 
0,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,6,300)
200,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_SIDE,6,400)
600,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_KNEE,-6,300) 
600,TurnMotor(ETHMOTORS_LOWER_BODY,MOTOR_RIGHT_LEG,-6,300)

#END,StopAllTouchSensorsInterrupt
#END,StopAllAccelerometersInterrupt
2000,StopLogging
