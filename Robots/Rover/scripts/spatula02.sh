#spatula02.sh
#0,StartLogging 
#0,LogScriptFileName


0,StartLoop(-1,Any,31)  #***START LOOP***  Any key ends loop, escape key aborts script

#arm up
#0,TurnMotor(MOTOR_RIGHT_WRIST,-16,100)
0,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,5,1000)
0,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,-5,1000)
#arm down
1000,TurnMotor(MOTOR_LEFT_LOWER_ARM_INSIDE,-3,1000)
1000,TurnMotor(MOTOR_LEFT_LOWER_ARM_OUTSIDE,3,1000)
#2000,TurnMotor(MOTOR_LEFT_WHEEL,3,1000)
#2000,TurnMotor(MOTOR_RIGHT_WHEEL,3,1000)
#10000,StopLogging

2000,EndLoop  #  ***END LOOP****

