#RoverScript01.sh
#0,StartLogging 
#0,LogScriptFileName

0,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
0,TurnMotor(MOTOR_RIGHT_WHEEL,-3,1000)
2000,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
2000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)
4000,TurnMotor(MOTOR_LEFT_WHEEL,-3,1000)
4000,TurnMotor(MOTOR_RIGHT_WHEEL,-3,1000)
6000,TurnMotor(MOTOR_LEFT_WHEEL,-3,1000)
6000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)

#10000,StopLogging

