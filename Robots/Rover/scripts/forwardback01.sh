#forwardback01.sh
0,StartLogging 
0,LogScriptFileName


0,StartLoop(-1,Any,31)  #***START LOOP***  Any key ends loop, escape key aborts script

#forward
0,TurnMotor(MOTOR_FORWARD_BACK,-10,1000)
500,TurnMotor(MOTOR_LEFT_RIGHT,-16,500)
1000,TurnMotor(MOTOR_FORWARD_BACK,10,1000)
2000,EndLoop  #  ***END LOOP****


2010,StartLoop(-1,Any,31)  #***START LOOP***  Any key ends loop, escape key aborts script

#backward
2010,TurnMotor(MOTOR_FORWARD_BACK,-10,1000)
2500,TurnMotor(MOTOR_LEFT_RIGHT,16,500)
3000,TurnMotor(MOTOR_FORWARD_BACK,10,1000)
4000,EndLoop  #  ***END LOOP****

4500,StopLogging 

