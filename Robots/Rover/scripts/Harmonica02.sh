#Harmonica02.sh
#0,StartLogging 
#0,LogScriptFileName

#right down
0,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,-18,600)
#right up
600,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,18,300)
900,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,-18,300)
1200,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,18,150)
1350,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,-18,600)
1950,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE_TEMP,18,300)

#2000,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
#2000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)
#10000,StopLogging
