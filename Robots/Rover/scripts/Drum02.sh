#Drum02.sh
#0,StartLogging 
#0,LogScriptFileName

#right down
0,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
#left up
0,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
#right up
100,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
#left down
100,TurnMotor(MOTOR_LEFT_WRIST,19,100)
200,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
200,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
300,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
300,TurnMotor(MOTOR_LEFT_WRIST,19,100)
400,TurnMotor(MOTOR_RIGHT_WRIST,-14,100)
400,TurnMotor(MOTOR_LEFT_WRIST,-14,100)
500,TurnMotor(MOTOR_RIGHT_WRIST,16,100)
500,TurnMotor(MOTOR_LEFT_WRIST,19,100)
#2000,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
#2000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)
#10000,StopLogging

